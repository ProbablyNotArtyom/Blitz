/* pptp_ctrl.c ... handle PPTP control connection.
 *                 C. Scott Ananian <cananian@alumni.princeton.edu>
 *
 * $Id: pptp_ctrl.c,v 1.8 2002/08/27 04:35:18 philipc Exp $
 */

#include <stdio.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <signal.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include "pptp_msg.h"
#include "pptp_ctrl.h"
#include "pptp_options.h"
#include "vector.h"

#ifdef EMBED
#include <syslog.h>
#define fprintf(x, a...) syslog(LOG_INFO, ##a)
// #define fprintf(x, a...)
#undef assert
#define assert(x) \
	if (!(x)) syslog(LOG_INFO,"%s,%d ***ASSERT*** - " #x "\n",__FILE__,__LINE__);else
#endif

/* BECAUSE OF SIGNAL LIMITATIONS, EACH PROCESS CAN ONLY MANAGE ONE
 * CONNECTION.  SO THIS 'PPTP_CONN' STRUCTURE IS A BIT MISLEADING.
 * WE'LL KEEP CONNECTION-SPECIFIC INFORMATION IN THERE ANYWAY (AS
 * OPPOSED TO USING GLOBAL VARIABLES), BUT BEWARE THAT THE ENTIRE
 * UNIX SIGNAL-HANDLING SEMANTICS WOULD HAVE TO CHANGE (OR THE
 * TIME-OUT CODE DRASTICALLY REWRITTEN) BEFORE YOU COULD DO A 
 * PPTP_CONN_OPEN MORE THAN ONCE PER PROCESS AND GET AWAY WITH IT.
 */

/* This structure contains connection-specific information that the
 * signal handler needs to see.  Thus, it needs to be in a global
 * variable.  If you end up using pthreads or something (why not
 * just processes?), this would have to be placed in a thread-specific
 * data area, using pthread_get|set_specific, etc., so I've 
 * conveniently encapsulated it for you.
 * [linux threads will have to support thread-specific signals
 *  before this would work at all, which, as of this writing
 *  (linux-threads v0.6, linux kernel 2.1.72), it does not.]
 */
static struct thread_specific {
  struct sigaction old_sigaction; /* evil signals */
  PPTP_CONN * conn;
} global;

#define INITIAL_BUFSIZE 512 /* initial i/o buffer size. */

struct PPTP_CONN {
  int inet_sock;
  
  /* Connection States */
  enum { 
    CONN_IDLE, CONN_WAIT_CTL_REPLY, CONN_WAIT_STOP_REPLY, CONN_ESTABLISHED 
  } conn_state; /* on startup: CONN_IDLE */
  /* Keep-alive states */
  enum { 
    KA_NONE, KA_OUTSTANDING 
  } ka_state;  /* on startup: KA_NONE */
  /* Keep-alive ID; monotonically increasing (watch wrap-around!) */
  u_int32_t ka_id; /* on startup: 1 */

  /* Other properties. */
  u_int16_t version;
  u_int16_t firmware_rev;
  u_int8_t  hostname[64], vendor[64];
  /* XXX these are only PNS properties, currently XXX */

  /* Call assignment information. */
  u_int16_t call_serial_number;

  VECTOR *call;

  void * closure;
  pptp_conn_cb callback;

  /******* IO buffers ******/
  char * read_buffer, *write_buffer;
  size_t read_alloc,   write_alloc;
  size_t read_size,    write_size;
};

struct PPTP_CALL {
  /* Call properties */
  enum {
    PPTP_CALL_PAC, PPTP_CALL_PNS
  } call_type;
  union { 
    enum pptp_pac_state {
      PAC_IDLE, PAC_WAIT_REPLY, PAC_ESTABLISHED, PAC_WAIT_CS_ANS
    } pac;
    enum pptp_pns_state {
      PNS_IDLE, PNS_WAIT_REPLY, PNS_ESTABLISHED, PNS_WAIT_DISCONNECT 
    } pns;
  } state;
  u_int16_t call_id, peer_call_id;
  u_int16_t sernum;
  u_int32_t speed;

  /* For user data: */
  pptp_call_cb callback;
  void * closure;
};

/* Local prototypes */
static void pptp_reset_timer(void);
static void pptp_handle_timer(int sig);
/* Write/read as much as we can without blocking. */
void pptp_write_some(PPTP_CONN * conn);
void pptp_read_some(PPTP_CONN * conn);
/* Make valid packets from read_buffer */
int pptp_make_packet(PPTP_CONN * conn, void **buf, size_t *size);
/* Add packet to write_buffer */
int pptp_send_ctrl_packet(PPTP_CONN * conn, void * buffer, size_t size);
/* Dispatch packets (general) */
void pptp_dispatch_packet(PPTP_CONN * conn, void * buffer, size_t size);
/* Dispatch packets (control messages) */
void pptp_dispatch_ctrl_packet(PPTP_CONN * conn, void * buffer, size_t size);
/*----------------------------------------------------------------------*/
/* Constructors and Destructors.                                        */

/* Open new pptp_connection.  Returns NULL on failure. */
PPTP_CONN * pptp_conn_open(int inet_sock, int isclient, pptp_conn_cb callback)
{
  struct sigaction sigact;
  PPTP_CONN *conn;
  /* Allocate structure */
  if ((conn = malloc(sizeof(*conn))) == NULL) return NULL;
  if ((conn->call = vector_create()) == NULL) { free(conn); return NULL; }
  /* Initialize */
  conn->inet_sock = inet_sock;
  conn->conn_state= CONN_IDLE;
  conn->ka_state  = KA_NONE;
  conn->ka_id     = 1;
  conn->call_serial_number = 0;
  conn->callback  = callback;
  /* Create I/O buffers */
  conn->read_size = conn->write_size = 0;
  conn->read_alloc= conn->write_alloc= INITIAL_BUFSIZE;
  conn->read_buffer = malloc(sizeof(*(conn->read_buffer))*conn->read_alloc);
  conn->write_buffer= malloc(sizeof(*(conn->write_buffer))*conn->write_alloc);
  if (conn->read_buffer == NULL || conn->write_buffer == NULL) {
    if (conn->read_buffer !=NULL) free(conn->read_buffer);
    if (conn->write_buffer!=NULL) free(conn->write_buffer);
    vector_destroy(conn->call); free(conn); return NULL;
  }
 
  /* Make this socket non-blocking. */
  fcntl(conn->inet_sock, F_SETFL, O_NONBLOCK);
 
  /* Request connection from server, if this is a client */
  if (isclient) {
    struct pptp_start_ctrl_conn packet = {
      PPTP_HEADER_CTRL(PPTP_START_CTRL_CONN_RQST),
      hton16(PPTP_VERSION), 0, 0, 
      hton32(PPTP_FRAME_CAP), hton32(PPTP_BEARER_CAP),
      hton16(PPTP_MAX_CHANNELS), hton16(PPTP_FIRMWARE_VERSION), 
      PPTP_HOSTNAME, PPTP_VENDOR
    };
    if (pptp_send_ctrl_packet(conn, &packet, sizeof(packet)))
      conn->conn_state = CONN_WAIT_CTL_REPLY;
    else return NULL; /* could not send initial start request. */
  }

  /* Set up interval/keep-alive timer */
  /*   First, register handler for SIGALRM */
  sigact.sa_handler = pptp_handle_timer;
  sigemptyset(&sigact.sa_mask);
  sigact.sa_flags = SA_RESTART;
  global.conn = conn;
  sigaction(SIGALRM, &sigact, &global.old_sigaction);

  /* Reset event timer */
  pptp_reset_timer();

  /* all done. */
  return conn;
}

/* This currently *only* works for client call requests.
 * We need to do something else to allocate calls for incoming requests.
 */
PPTP_CALL * pptp_call_open(PPTP_CONN * conn,
			   pptp_call_cb callback){
  PPTP_CALL * call;
  int i;
  // assert(conn && conn->call);

  if (!conn)
  	return(NULL);
  if (!conn->call) {
	fprintf(stderr, "Connection is shutting down\n");
  	return(NULL);
  }

  /* Assign call id */
  if (!vector_scan(conn->call, 0, PPTP_MAX_CHANNELS-1, &i)) {
	fprintf(stderr, "No more calls allowed\n");
    return NULL;
  }

  /* allocate structure. */
  if ((call = malloc(sizeof(*call)))==NULL) return NULL;
  /* Initialize call structure */
  call->call_type = PPTP_CALL_PNS;
  call->state.pns = PNS_IDLE;
  call->call_id   = (u_int16_t) i;
  call->sernum    = conn->call_serial_number++;
  call->callback  = callback;
  call->closure   = NULL;
  /* Send off the call request */
  {
    struct pptp_out_call_rqst packet = {
      PPTP_HEADER_CTRL(PPTP_OUT_CALL_RQST),
      hton16(call->call_id), hton16(call->sernum),
      hton32(PPTP_BPS_MIN), hton32(PPTP_BPS_MAX),
      hton32(PPTP_BEARER_CAP), hton32(PPTP_FRAME_CAP), 
      hton16(PPTP_WINDOW), 0, 0, 0, {0}, {0}
    };
    if (pptp_send_ctrl_packet(conn, &packet, sizeof(packet))) {
      pptp_reset_timer();
      call->state.pns = PNS_WAIT_REPLY;
      /* and add it to the call vector */
      vector_insert(conn->call, i, call);
      return call;
    } else { /* oops, unsuccessful. Deallocate. */
	  fprintf(stderr, "Failed to send control packet\n");
      free(call);
      return NULL;
    }
  }
}

void pptp_call_close(PPTP_CONN * conn, PPTP_CALL * call) {
  struct pptp_call_clear_rqst rqst = {
    PPTP_HEADER_CTRL(PPTP_CALL_CLEAR_RQST), 0, 0
  };
  if (!conn || !conn->call || !call)
  	return;
  // assert(conn && conn->call); assert(call);
  assert(vector_contains(conn->call, call->call_id));
  assert(call->call_type == PPTP_CALL_PNS); /* haven't thought about PAC yet */
  assert(call->state.pns != PNS_IDLE);
  rqst.call_id = hton16(call->call_id);

  /* don't check state against WAIT_DISCONNECT... allow multiple disconnect
   * requests to be made.
   */

  pptp_send_ctrl_packet(conn, &rqst, sizeof(rqst));
  pptp_reset_timer();
  call->state.pns = PNS_WAIT_DISCONNECT;

  /* call structure will be freed when we have confirmation of disconnect. */
}
/* hard close. */
void pptp_call_destroy(PPTP_CONN *conn, PPTP_CALL *call) {
  // assert(conn && conn->call); assert(call);
  if (!conn || !conn->call || !call)
  	return;
  assert(vector_contains(conn->call, call->call_id));
  /* notify */
  if (call->callback!=NULL) call->callback(conn, call, CALL_CLOSE_DONE);
  /* deallocate */
  vector_remove(conn->call, call->call_id);
  memset(call, 0, sizeof(*call));
  free(call);
}
/* this is a soft close */
void pptp_conn_close(PPTP_CONN * conn, u_int8_t close_reason) {
  struct pptp_stop_ctrl_conn rqst = {
    PPTP_HEADER_CTRL(PPTP_STOP_CTRL_CONN_RQST), 
    hton8(close_reason), 0, 0};
  int i;
  
  // assert(conn && conn->call);
  if (!conn)
  	return;

  /* avoid repeated close attempts */
  if (conn->conn_state==CONN_IDLE || conn->conn_state==CONN_WAIT_STOP_REPLY) 
    return;

  /* close open calls, if any */
  for (i=0; conn->call && i<vector_size(conn->call); i++)
    pptp_call_close(conn, vector_get_Nth(conn->call, i));
    
  /* now close connection */
  fprintf(stderr, "Closing PPTP connection\n");
  pptp_send_ctrl_packet(conn, &rqst, sizeof(rqst));
  pptp_reset_timer(); /* wait 60 seconds for reply */
  conn->conn_state = CONN_WAIT_STOP_REPLY;
  return;
}
/* this is a hard close */
void pptp_conn_destroy(PPTP_CONN * conn, int dispose) {
  int i;
  // assert(conn != NULL);
  // assert(conn->call != NULL);
  if (!conn)
  	return;
  /* shut out the timers */
  if (conn == global.conn)
    global.conn = NULL;
  /* destroy all open calls */
  for (i=0; conn->call && i<vector_size(conn->call); i++)
    pptp_call_destroy(conn, vector_get_Nth(conn->call, i));
  /* notify */
  if (conn->callback!=NULL) {
    conn->callback(conn, CONN_CLOSE_DONE);
    conn->callback = NULL;
  }
  sigaction(SIGALRM, &global.old_sigaction, NULL);
  if (conn->inet_sock >= 0) {
    fprintf(stderr, "closing %d (inet)\n", conn->inet_sock);
    close(conn->inet_sock);
    conn->inet_sock = -1;
  }
  /* deallocate */
  if (conn->call != NULL) {
    vector_destroy(conn->call);
    conn->call = NULL;
  }
  if (dispose) {
    memset(conn, 0, sizeof(*conn));
    free(conn);
  }
}

/************** Deal with messages, in a non-blocking manner *************/

/* Add file descriptors used by pptp to fd_set. */
void pptp_fd_set(PPTP_CONN * conn, fd_set * read_set, fd_set * write_set) {
  // assert(conn && conn->call);
  if (!conn)
    return;
  /* Add fd to write_set if there are outstanding writes. */
  if (conn->inet_sock >= 0 && conn->write_size > 0)
    FD_SET(conn->inet_sock, write_set);
  /* Always add fd to read_set. (always want something to read) */
  if (conn->inet_sock >= 0)
    FD_SET(conn->inet_sock, read_set);
}
/* handle any pptp file descriptors set in fd_set, and clear them */
void pptp_dispatch(PPTP_CONN * conn, fd_set * read_set, fd_set * write_set) {
  // assert(conn && conn->call);
  if (!conn)
    return;
  /* Check write_set could be set. */
  if (conn->inet_sock >= 0 && FD_ISSET(conn->inet_sock, write_set)) {
    FD_CLR(conn->inet_sock, write_set);
    if (conn->write_size > 0)
      pptp_write_some(conn); /* write as much as we can without blocking */
  }
  /* Check read_set */
  if (conn->inet_sock >= 0 && FD_ISSET(conn->inet_sock, read_set)) {
    void *buffer; size_t size;
    FD_CLR(conn->inet_sock, read_set);
    pptp_read_some(conn); /* read as much as we can without blocking */
    /* make packets of the buffer, while we can. */
    while (pptp_make_packet(conn, &buffer, &size)) {
      pptp_dispatch_packet(conn, buffer, size);
      free(buffer);
    }
  }
  /* That's all, folks.  Simple, eh? */
}
/********************* Non-blocking I/O buffering *********************/
void pptp_write_some(PPTP_CONN * conn) {
  ssize_t retval;
  // assert(conn && conn->call);
  if (!conn || conn->inet_sock == -1)
  	return;
  retval = write(conn->inet_sock, conn->write_buffer, conn->write_size);
  if (retval < 0) { /* error. */
    if (errno == EAGAIN || errno == EINTR) { 
      /* ignore */;
    } else { /* a real error */
      fprintf(stderr,"write error: %s\n", strerror(errno));
      pptp_conn_destroy(conn, 0); /* shut down fast. */
    }
    return;
  }
  assert(retval <= conn->write_size);
  conn->write_size-=retval;
  memmove(conn->write_buffer, conn->write_buffer+retval, 
	  conn->write_size);
}
void pptp_read_some(PPTP_CONN * conn) {
  ssize_t retval;
  // assert(conn && conn->call);
  if (!conn || conn->inet_sock == -1)
  	return;
  if (conn->read_size == conn->read_alloc) { /* need to alloc more memory */
    char *new_buffer=realloc(conn->read_buffer, 
			     sizeof(*(conn->read_buffer))*conn->read_alloc*2);
    if (new_buffer == NULL) {
      fprintf(stderr,"Out of memory"); return;
    }
    conn->read_alloc*=2;
    conn->read_buffer = new_buffer;
  }
  retval = read(conn->inet_sock, 
		conn->read_buffer + conn->read_size,
		conn->read_alloc  - conn->read_size);
  if (retval < 0) {
    if (errno == EINTR || errno == EAGAIN)
      /* ignore */ ;
    else { /* a real error */
      fprintf(stderr,"read error %d: %s", conn->inet_sock, strerror(errno));
      pptp_conn_destroy(conn, 0); /* shut down fast. */
    }
    return;
  }
  else if (retval == 0) {
    fprintf(stderr,"Control connection closed by peer");
    pptp_conn_destroy(conn, 0); /* shut down fast. */
  }
  conn->read_size += retval;
  assert(conn->read_size <= conn->read_alloc);
}
/********************* Packet formation *******************************/
int pptp_make_packet(PPTP_CONN * conn, void **buf, size_t *size) {
  struct pptp_header *header;
  size_t bad_bytes = 0;
  // assert(conn && conn->call);

  if (!conn || !conn->call)
    return(0);
  assert(buf != NULL); assert(size != NULL);

  /* Give up unless there are at least sizeof(pptp_header) bytes */
  while ((conn->read_size-bad_bytes) >= sizeof(struct pptp_header)) {
    /* Throw out bytes until we have a valid header. */
    header = (struct pptp_header *) (conn->read_buffer+bad_bytes);
    if (ntoh32(header->magic) != PPTP_MAGIC) goto throwitout;
    if (ntoh16(header->length) < sizeof(struct pptp_header)) goto throwitout;
    if (ntoh16(header->length) > PPTP_CTRL_SIZE_MAX) goto throwitout;

    /* At least one PPTP server implementation (Cisco) ignores RFC 2637's
     * requirement that Reserved0 "MUST be 0". In the interests of
     * interoperability, we'll just log this and pretend it never happened.
     */
    if (ntoh16(header->reserved0)!=0) {
      fprintf(stderr,"Protocol violation: header->reserved0(0x%x) != 0\n",
  		ntoh16(header->reserved0));
    }

    /* well.  I guess it's good. Let's see if we've got it all. */
    if (ntoh16(header->length) > (conn->read_size-bad_bytes))
      /* nope.  Let's wait until we've got it, then. */
      goto flushbadbytes;
    /* One last check: */
    if ((ntoh16(header->pptp_type) == PPTP_MESSAGE_CONTROL) &&
	(ntoh16(header->length) != PPTP_CTRL_SIZE(ntoh16(header->ctrl_type))))
      goto throwitout;

    /* well, I guess we've got it. */
    *size= ntoh16(header->length);
    *buf = malloc(*size);
    if (*buf == NULL) { fprintf(stderr,"Out of memory."); return 0; /* ack! */ }
    memcpy(*buf, conn->read_buffer, *size);
    /* Delete this packet from the read_buffer. */
    conn->read_size -= (bad_bytes + *size);
    memmove(conn->read_buffer, conn->read_buffer+bad_bytes+*size, 
	    conn->read_size);
    if (bad_bytes > 0) 
      fprintf(stderr,"%lu bad bytes thrown away.", (unsigned long) bad_bytes);
    return 1;

  throwitout:
    bad_bytes++;
  }
flushbadbytes:
  /* no more packets.  Let's get rid of those bad bytes */
  conn->read_size -= bad_bytes;
  memmove(conn->read_buffer, conn->read_buffer+bad_bytes, conn->read_size);
  if (bad_bytes > 0) 
    fprintf(stderr,"%lu bad bytes thrown away.", (unsigned long) bad_bytes);
  return 0;
}
int pptp_send_ctrl_packet(PPTP_CONN * conn, void * buffer, size_t size) {
  // assert(conn && conn->call);
  if (!conn)
  	return;
  assert(buffer);
  /* Shove this into the write buffer */
  if (conn->write_size + size > conn->write_alloc) { /* need more memory */
    char *new_buffer=realloc(conn->write_buffer, 
			   sizeof(*(conn->write_buffer))*conn->write_alloc*2);
    if (new_buffer == NULL) {
      fprintf(stderr,"Out of memory"); return 0;
    }
    conn->write_alloc*=2;
    conn->write_buffer = new_buffer;
  }
  memcpy(conn->write_buffer + conn->write_size, buffer, size);
  conn->write_size+=size;
  return 1;
}
/********************** Packet Dispatch ****************************/
void pptp_dispatch_packet(PPTP_CONN * conn, void * buffer, size_t size) {
  struct pptp_header *header = (struct pptp_header *)buffer;
  assert(conn && conn->call); assert(buffer);
  assert(ntoh32(header->magic)==PPTP_MAGIC);
  assert(ntoh16(header->length)==size);

  switch (ntoh16(header->pptp_type)) {
  case PPTP_MESSAGE_CONTROL:
    pptp_dispatch_ctrl_packet(conn, buffer, size);
    break;
  case PPTP_MESSAGE_MANAGE:
    /* MANAGEMENT messages aren't even part of the spec right now. */
    fprintf(stderr,"PPTP management message received, but not understood.");
    break;
  default:
    fprintf(stderr,"Unknown PPTP control message type received: %u", 
	(unsigned) ntoh16(header->pptp_type));
    break;
  }
}

void pptp_dispatch_ctrl_packet(PPTP_CONN * conn, void * buffer, size_t size) {
  struct pptp_header *header = (struct pptp_header *)buffer;
  u_int8_t close_reason = PPTP_STOP_NONE;

  assert(conn && conn->call); assert(buffer);
  assert(ntoh32(header->magic)==PPTP_MAGIC);
  assert(ntoh16(header->length)==size);
  assert(ntoh16(header->pptp_type)==PPTP_MESSAGE_CONTROL);

  if (size < PPTP_CTRL_SIZE(ntoh16(header->ctrl_type))) {
    fprintf(stderr,"Invalid packet received [type: %d; length: %d].\n",
	(int) ntoh16(header->ctrl_type), (int) size);
    return;
  }

  switch (ntoh16(header->ctrl_type)) {
    /* ----------- STANDARD Start-Session MESSAGES ------------ */
  case PPTP_START_CTRL_CONN_RQST:
    {
      struct pptp_start_ctrl_conn *packet = 
	(struct pptp_start_ctrl_conn *) buffer;
      struct pptp_start_ctrl_conn reply = {
	PPTP_HEADER_CTRL(PPTP_START_CTRL_CONN_RPLY),
	hton16(PPTP_VERSION), 0, 0,
	hton32(PPTP_FRAME_CAP), hton32(PPTP_BEARER_CAP),
	hton16(PPTP_MAX_CHANNELS), hton16(PPTP_FIRMWARE_VERSION),
	PPTP_HOSTNAME, PPTP_VENDOR };
      if (conn->conn_state == CONN_IDLE) {
	if (ntoh16(packet->version) < PPTP_VERSION) {
	  /* Can't support this (earlier) PPTP_VERSION */
	  reply.version = packet->version;
	  reply.result_code = hton8(5); /* protocol version not supported */
	  pptp_send_ctrl_packet(conn, &reply, sizeof(reply));
	  pptp_reset_timer(); /* give sender a chance for a retry */
	} else { /* same or greater version */
	  if (pptp_send_ctrl_packet(conn, &reply, sizeof(reply))) {
	    conn->conn_state=CONN_ESTABLISHED;
	    fprintf(stderr,"server connection ESTABLISHED.");
	    pptp_reset_timer();
	  }
	}
      }
      break;
    }
  case PPTP_START_CTRL_CONN_RPLY:
    {
      struct pptp_start_ctrl_conn *packet = 
	(struct pptp_start_ctrl_conn *) buffer;
      if (conn->conn_state == CONN_WAIT_CTL_REPLY) {
	/* XXX handle collision XXX [see rfc] */
	if (ntoh16(packet->version) != PPTP_VERSION) {
	  if (conn->callback!=NULL) conn->callback(conn, CONN_OPEN_FAIL);
	  close_reason = PPTP_STOP_PROTOCOL;
	  goto pptp_err_conn_close;
	}
	if (ntoh8(packet->result_code)!=1) { /* some problem with start */
	  /* if result_code == 5, we might fall back to different version */
	  if (conn->callback!=NULL) conn->callback(conn, CONN_OPEN_FAIL);
	  close_reason = PPTP_STOP_PROTOCOL;
	  goto pptp_err_conn_close;
	}
	conn->conn_state = CONN_ESTABLISHED;

	/* log session properties */
	conn->version      = ntoh16(packet->version);
	conn->firmware_rev = ntoh16(packet->firmware_rev);
	memcpy(conn->hostname, packet->hostname, sizeof(conn->hostname));
	memcpy(conn->vendor, packet->vendor, sizeof(conn->vendor));

	pptp_reset_timer(); /* 60 seconds until keep-alive */
	fprintf(stderr, "Client connection established.\n");

	if (conn->callback!=NULL) conn->callback(conn, CONN_OPEN_DONE);
      } /* else goto pptp_err_conn_close; */
      break;
    }
  /* ----------- STANDARD Stop-Session MESSAGES ------------ */
  case PPTP_STOP_CTRL_CONN_RQST:
    {
      struct pptp_stop_ctrl_conn *packet = /* XXX do something with this XXX */
	(struct pptp_stop_ctrl_conn *) buffer; 
      /* conn_state should be CONN_ESTABLISHED, but it could be 
       * something else */
      struct pptp_stop_ctrl_conn reply = {
	PPTP_HEADER_CTRL(PPTP_STOP_CTRL_CONN_RPLY), 
	hton8(1), hton8(PPTP_GENERAL_ERROR_NONE), 0};

      if (conn->conn_state==CONN_IDLE) break;
      if (pptp_send_ctrl_packet(conn, &reply, sizeof(reply))) {
	if (conn->callback!=NULL) conn->callback(conn, CONN_CLOSE_RQST);
	conn->conn_state=CONN_IDLE;
	pptp_conn_destroy(conn, 0);
      }
      break;
    }
  case PPTP_STOP_CTRL_CONN_RPLY:
    {
      struct pptp_stop_ctrl_conn *packet = /* XXX do something with this XXX */
	(struct pptp_stop_ctrl_conn *) buffer;
      /* conn_state should be CONN_WAIT_STOP_REPLY, but it 
       * could be something else */

      if (conn->conn_state == CONN_IDLE) break;
      conn->conn_state=CONN_IDLE;
      pptp_conn_destroy(conn, 0);
      break;
    }
  /* ----------- STANDARD Echo/Keepalive MESSAGES ------------ */
  case PPTP_ECHO_RPLY:
    {
      struct pptp_echo_rply *packet = 
	(struct pptp_echo_rply *) buffer;
      if ((conn->ka_state == KA_OUTSTANDING) && 
	  (ntoh32(packet->identifier)==conn->ka_id)) {
	conn->ka_id++;
	conn->ka_state=KA_NONE;
	pptp_reset_timer();
      }
      break;
    }
  case PPTP_ECHO_RQST:
    {
      struct pptp_echo_rqst *packet = 
	(struct pptp_echo_rqst *) buffer;
      struct pptp_echo_rply reply = {
	PPTP_HEADER_CTRL(PPTP_ECHO_RPLY), 
	packet->identifier, /* skip hton32(ntoh32(id)) */
	hton8(1), hton8(PPTP_GENERAL_ERROR_NONE), 0};
      pptp_send_ctrl_packet(conn, &reply, sizeof(reply));
      pptp_reset_timer();
      break;
    }
  /* ----------- OUTGOING CALL MESSAGES ------------ */
  case PPTP_OUT_CALL_RQST:
    {
      struct pptp_out_call_rqst *packet =
	(struct pptp_out_call_rqst *)buffer;
      struct pptp_out_call_rply reply = {
	PPTP_HEADER_CTRL(PPTP_OUT_CALL_RPLY),
	0 /* callid */, packet->call_id, 1, PPTP_GENERAL_ERROR_NONE, 0,
	hton32(PPTP_CONNECT_SPEED), 
	hton16(PPTP_WINDOW), hton16(PPTP_DELAY), 0 };
      /* XXX PAC: eventually this should make an outgoing call. XXX */
      reply.result_code = hton8(7); /* outgoing calls verboten */
      pptp_send_ctrl_packet(conn, &reply, sizeof(reply));
      break;
    }
  case PPTP_OUT_CALL_RPLY:
    {
      struct pptp_out_call_rply *packet =
	(struct pptp_out_call_rply *)buffer;
      PPTP_CALL * call;
      u_int16_t callid = ntoh16(packet->call_id_peer);
      if (!vector_search(conn->call, (int) callid, &call)) {
	fprintf(stderr,"PPTP_OUT_CALL_RPLY received for non-existant call.");
	break;
      }
      if (call->call_type!=PPTP_CALL_PNS) {
	fprintf(stderr,"Ack!  How did this call_type get here?"); /* XXX? */
	break; 
      }
      if (call->state.pns == PNS_WAIT_REPLY) {
	/* check for errors */
	if (packet->result_code!=1) {
	  /* An error.  Log it. */
	  fprintf(stderr,"Error opening call. [callid %d]", (int) callid);
	  call->state.pns = PNS_IDLE;
	  if (call->callback!=NULL) call->callback(conn, call, CALL_OPEN_FAIL);
	  pptp_call_destroy(conn, call);
	} else {
	  /* connection established */
	  call->state.pns = PNS_ESTABLISHED;
	  call->peer_call_id = ntoh16(packet->call_id);
	  call->speed        = ntoh32(packet->speed);
	  pptp_reset_timer();
	  if (call->callback!=NULL) call->callback(conn, call, CALL_OPEN_DONE);
	  fprintf(stderr, "Outgoing call established.\n");
	}
      }
      break;
    }
  /* ----------- INCOMING CALL MESSAGES ------------ */
  /* XXX write me XXX */
  /* ----------- CALL CONTROL MESSAGES ------------ */
  case PPTP_CALL_CLEAR_RQST:
    {
      struct pptp_call_clear_rqst *packet =
	(struct pptp_call_clear_rqst *)buffer;
      struct pptp_call_clear_ntfy reply = {
	PPTP_HEADER_CTRL(PPTP_CALL_CLEAR_NTFY), packet->call_id,
	1, PPTP_GENERAL_ERROR_NONE, 0, 0, {0}
      };
      if (vector_contains(conn->call, ntoh16(packet->call_id))) {
	PPTP_CALL * call;
	vector_search(conn->call, ntoh16(packet->call_id), &call);
	if (call->callback!=NULL) call->callback(conn, call, CALL_CLOSE_RQST);
	pptp_send_ctrl_packet(conn, &reply, sizeof(reply));
	pptp_call_destroy(conn, call);
	fprintf(stderr,"Call closed (RQST) (call id %d)", (int) call->call_id);
      }
      break;
    }
  case PPTP_CALL_CLEAR_NTFY:
    {
      struct pptp_call_clear_ntfy *packet =
	(struct pptp_call_clear_ntfy *)buffer;
      if (vector_contains(conn->call, ntoh16(packet->call_id))) {
	PPTP_CALL * call;
	vector_search(conn->call, ntoh16(packet->call_id), &call);
	pptp_call_destroy(conn, call);
	fprintf(stderr, "Call closed (NTFY) (call id %d)", (int) call->call_id);
      }
      /* XXX we could log call stats here XXX */
      break;
    }
  case PPTP_SET_LINK_INFO:
    {
      /* I HAVE NO CLUE WHAT TO DO IF send_accm IS NOT 0! */
      /* this is really dealt with in the HDLC deencapsulation, anyway. */
      struct pptp_set_link_info *packet =
	(struct pptp_set_link_info *)buffer;
      if (ntoh32(packet->send_accm)==0 && ntoh32(packet->recv_accm)==0)
	break; /* this is what we expect. */
      /* log it, otherwise. */
      fprintf(stderr,"PPTP_SET_LINK_INFO recieved from peer_callid %u",
	  (unsigned) ntoh16(packet->call_id_peer));
      fprintf(stderr,"  send_accm is %08lX, recv_accm is %08lX",
	  (unsigned long) ntoh32(packet->send_accm),
	  (unsigned long) ntoh32(packet->recv_accm));
      break;
    }
  default:
    fprintf(stderr,"Unrecognized Packet %d received.", 
	(int) ntoh16(((struct pptp_header *)buffer)->ctrl_type));
    /* goto pptp_err_conn_close; */
    break;
  }
  return;

pptp_err_conn_close:
  fprintf(stderr,"pptp_conn_close(%d)\n", (int) close_reason);
  pptp_conn_close(conn, close_reason);
}


/********************* Get info from call structure *******************/

/* NOTE: The peer_call_id is undefined until we get a server response. */
void pptp_call_get_ids(PPTP_CONN * conn, PPTP_CALL * call,
		       u_int16_t * call_id, u_int16_t * peer_call_id) {
  assert(conn!=NULL); assert(call != NULL);
  *call_id = call->call_id;
  *peer_call_id = call->peer_call_id;
}
void   pptp_call_closure_put(PPTP_CONN * conn, PPTP_CALL * call, void *cl) {
  assert(conn != NULL); assert(call != NULL);
  call->closure = cl;
}
void * pptp_call_closure_get(PPTP_CONN * conn, PPTP_CALL * call) {
  assert(conn != NULL); assert(call != NULL);
  return call->closure;
}
void   pptp_conn_closure_put(PPTP_CONN * conn, void *cl) {
  assert(conn != NULL);
  conn->closure = cl;
}
void * pptp_conn_closure_get(PPTP_CONN * conn) {
  assert(conn != NULL);
  return conn->closure;
}

int pptp_conn_down(PPTP_CONN * conn) {
  assert(conn != NULL);
  return(!conn->call);
}

int pptp_conn_established(PPTP_CONN * conn) {
  assert(conn != NULL);
  return(conn->conn_state == CONN_ESTABLISHED);
}

/*********************** Handle keep-alive timer ***************/
static void pptp_reset_timer(void) {
  const struct itimerval tv = { {  0, 0 },   /* stop on time-out */
				{ PPTP_TIMEOUT, 0 } }; /* 60-second timer */
  if (global.conn)
    setitimer(ITIMER_REAL, &tv, NULL);
}

static void pptp_handle_timer(int sig) {
  int i;

  if (!global.conn) /* things have disappeared under us */
    return;

  /* "Keep Alives and Timers, 1": check connection state */
  if (global.conn->conn_state != CONN_ESTABLISHED) {
    if (global.conn->conn_state == CONN_WAIT_STOP_REPLY) {
      /* hard close. */
      pptp_conn_destroy(global.conn, 0);
      return; /* not much to do after we destroy it ! */
    } else /* soft close */
      pptp_conn_close(global.conn, PPTP_STOP_NONE);
  }
  /* "Keep Alives and Timers, 2": check echo status */
  if (global.conn->ka_state == KA_OUTSTANDING) /*no response to keep-alive*/
    pptp_conn_close(global.conn, PPTP_STOP_NONE);
  else { /* ka_state == NONE */ /* send keep-alive */
    struct pptp_echo_rqst rqst = {
      PPTP_HEADER_CTRL(PPTP_ECHO_RQST), hton32(global.conn->ka_id) };
    pptp_send_ctrl_packet(global.conn, &rqst, sizeof(rqst));
    global.conn->ka_state = KA_OUTSTANDING;
    /* XXX FIXME: wake up ctrl thread -- or will the SIGALRM do that
     * automagically? XXX
     */
  }
  /* check incoming/outgoing call states for !IDLE && !ESTABLISHED */
  for (i=0; i<vector_size(global.conn->call); i++) {
    PPTP_CALL * call = vector_get_Nth(global.conn->call, i);
    if (call->call_type == PPTP_CALL_PNS) {
      if (call->state.pns==PNS_WAIT_REPLY) {
	/* send close request */
	pptp_call_close(global.conn, call);
	assert(call->state.pns==PNS_WAIT_DISCONNECT);
      } else if (call->state.pns==PNS_WAIT_DISCONNECT) {
	/* hard-close the call */
	pptp_call_destroy(global.conn, call);
      }
    } else if (call->call_type == PPTP_CALL_PAC) {
      if (call->state.pac == PAC_WAIT_REPLY) {
	/* XXX FIXME -- drop the PAC connection XXX */
      } else if (call->state.pac == PAC_WAIT_CS_ANS) {
	/* XXX FIXME -- drop the PAC connection XXX */
      }
    }
  }
  pptp_reset_timer();
}
