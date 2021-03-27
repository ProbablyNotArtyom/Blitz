EXE1=tclsh
EXE2=tcltest
EXE3=tclembed
CFLAGS += -g

OBJS=tclAssem.o tclGet.o tclXgen.o tclBasic.o tclGlob.o tclUtil.o panic.o tclCmdAH.o tclHash.o readdir.o tclCmdIL.o tclParse.o regexp.o tclCmdMZ.o tclEnv.o tclProc.o tclVar.o tclalloc.o tclExpr.o tclXdbg.o tclunxaz.o tcluxutl.o tcluxstr.o

all: $(EXE1) $(EXE2)

$(EXE1): $(OBJS) tinytcl.o
	$(CC) $(LDFLAGS) -o $@ $(OBJS) tinytcl.o $(LDLIBS$(LDLIBS_$@))

$(EXE2): $(OBJS) tclTest.o
	$(CC) $(LDFLAGS) -o $@ $(OBJS) tclTest.o $(LDLIBS$(LDLIBS_$@))

$(EXE3): $(OBJS) tclEmbed.o
	$(CC) $(LDFLAGS) -o $@ $(OBJS) tclEmbed.o $(LDLIBS$(LDLIBS_$@))

clean:
	rm -f *.o $(EXE1) $(EXE2) $(EXE3)

romfs:
	$(ROMFSINST) /bin/tclsh
