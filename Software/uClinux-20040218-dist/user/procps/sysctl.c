
/*
 * Sysctl 1.01 - A utility to read and manipulate the sysctl parameters
 *
 *
 * "Copyright 1999 George Staikos
 * This file may be used subject to the terms and conditions of the
 * GNU General Public License Version 2, or any later version
 * at your option, as published by the Free Software Foundation.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details."
 *
 * Changelog:
 *            v1.01:
 *                   - added -p <preload> to preload values from a file
 */


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <errno.h>
#include <config/autoconf.h>

/*
 *    Additional types we might need.
 */
typedef int bool;

static bool true  = 1;
static bool false = 0;


/*
 *    Function Prototypes
 */
int Usage(const char *name);
void Preload(const char *filename);
int WriteSetting(const char *setting);
int ReadSetting(const char *setting);
int DisplayAll(const char *path, bool ShowTableUtil);


/*
 *    Globals...
 */

const char *PROC_PATH = "/proc/sys/";
#ifdef CONFIG_USER_FLATFSD_FLATFSD
const char *DEFAULT_PRELOAD = "/etc/config/sysctl.conf";
#else
const char *DEFAULT_PRELOAD = "/etc/sysctl.conf";
#endif
static bool PrintName;

/* error messages */
const char *ERR_UNKNOWN_PARAMETER = "error: Unknown parameter '%s'\n";
const char *ERR_MALFORMED_SETTING = "error: Malformed setting '%s'\n";
const char *ERR_NO_EQUALS = "error: '%s' must be of the form name=value\n";
const char *ERR_INVALID_KEY = "error: '%s' is an unknown key\n";
const char *ERR_UNKNOWN_WRITING = "error: unknown error %d setting key '%s'\n";
const char *ERR_UNKNOWN_READING = "error: unknown error %d reading key '%s'\n";
const char *ERR_PERMISSION_DENIED = "error: permission denied on key '%s'\n";
const char *ERR_OPENING_DIR = "error: unable to open directory '%s'\n";
const char *ERR_PRELOAD_FILE = "error: unable to open preload file '%s'\n";
const char *WARN_BAD_LINE = "warning: %s(%d): invalid syntax, continuing...\n";


/*
 *    Main... 
 *
 */
int main(int argc, char **argv) {
const char *me = (const char *)basename(argv[0]);
bool SwitchesAllowed = true;
bool WriteMode = false;
int ReturnCode = 0;
const char *preloadfile = DEFAULT_PRELOAD;

   PrintName = true;

   if (argc < 2) {
       return Usage(me);
   } /* endif */

   argv++;

   for (; argv && *argv && **argv; argv++) {
      if (SwitchesAllowed && **argv == '-') {        /* we have a switch */
         switch((*argv)[1]) {
         case 'n':
              PrintName = false;
           break;
         case 'w':
              SwitchesAllowed = false;
              WriteMode = true;
           break;
         case 'p':
              argv++;
              if (argv && *argv && **argv) {
                 preloadfile = *argv;
              } /* endif */

              Preload(preloadfile);
              return(0);
           break;
         case 'a':
         case 'A':
              SwitchesAllowed = false;
              return DisplayAll(PROC_PATH, ((*argv)[1] == 'a') ? false : true);
         case 'h':
         case '?':
              return Usage(me);
         default:
              fprintf(stderr, ERR_UNKNOWN_PARAMETER, *argv);
              return Usage(me);
         } /* end switch */
      } else {
         SwitchesAllowed = false;
         if (WriteMode)
            ReturnCode = WriteSetting(*argv);
         else ReadSetting(*argv);
      } /* end if */
   } /* end for */      

return ReturnCode;
} /* end main */





/*
 *     Display the usage format
 *
 */
int Usage(const char *name) {
   printf("usage:  %s [-n] variable ... \n"
          "        %s [-n] -w variable=value ... \n" 
          "        %s [-n] -a \n" 
          "        %s [-n] -p <file>   (default %s) \n"
          "        %s [-n] -A\n", name, name, name, name, DEFAULT_PRELOAD, name);
return -1;
}  /* end Usage() */


/*
 *     Strip the leading and trailing spaces from a string
 *
 */
char *StripLeadingAndTrailingSpaces(char *oneline) {
char *t;

if (!oneline || !*oneline)
   return oneline;

t = oneline;
t += strlen(oneline)-1;

while ((*t == ' ' || *t == '\t' || *t == '\n' || *t == '\r') && t != oneline)
   *t-- = 0;

t = oneline;

while ((*t == ' ' || *t == '\t') && *t != 0)
   t++;

return t;
} /* end StripLeadingAndTrailingSpaces() */



/*
 *     Preload the sysctl's from the conf file
 *           - we parse the file and then reform it (strip out whitespace)
 *
 */
void Preload(const char *filename) {
FILE *fp;
char oneline[257];
char buffer[257];
char *t;
int n = 0;
char *name, *value;

   if (!filename || ((fp = fopen(filename, "r")) == NULL)) {
      fprintf(stderr, ERR_PRELOAD_FILE, filename);
      return;
   } /* endif */

   while (fgets(oneline, 256, fp)) {
      oneline[256] = 0;
      n++;
      t = StripLeadingAndTrailingSpaces(oneline);

      if (strlen(t) < 2)
         continue;

      if (*t == '#' || *t == ';')
         continue;

      name = strtok(t, "=");
      if (!name || !*name) {
         fprintf(stderr, WARN_BAD_LINE, filename, n);
         continue;
      } /* endif */

      StripLeadingAndTrailingSpaces(name);

      value = strtok(NULL, "\n\r");
      if (!value || !*value) {
         fprintf(stderr, WARN_BAD_LINE, filename, n);
         continue;
      } /* endif */

      while ((*value == ' ' || *value == '\t') && *value != 0)
         value++;

      sprintf(buffer, "%s=%s", name, value);
      WriteSetting(buffer);
   } /* endwhile */

   fclose(fp);
} /* end Preload() */



/*
 *     Write a sysctl setting 
 *
 */
int WriteSetting(const char *setting) {
int rc = 0;
const char *name = setting;
const char *value;
const char *equals;
char *tmpname;
char *cptr;
FILE *fp;
char *outname;

   if (!name) {                /* probably dont' want to display this  err */
      return 0;
   } /* end if */

   equals = index(setting, '=');
 
   if (!equals) {
      fprintf(stderr, ERR_NO_EQUALS, setting);
      return -1;
   } /* end if */

   value = equals + sizeof(char);      /* point to the value in name=value */   

   if (!*name || !*value || name == equals) { 
      fprintf(stderr, ERR_MALFORMED_SETTING, setting);
      return -2;
   } /* end if */

   tmpname = (char *)malloc((equals-name+1+strlen(PROC_PATH))*sizeof(char));
   outname = (char *)malloc((equals-name+1)*sizeof(char));

   strcpy(tmpname, PROC_PATH);
   strncat(tmpname, name, (int)(equals-name)); 
   tmpname[equals-name+strlen(PROC_PATH)] = 0;
   strncpy(outname, name, (int)(equals-name)); 
   outname[equals-name] = 0;
 
   while (cptr = strchr(tmpname, '.')) {      /* change . to / */
      *cptr = '/';
   } /* endwhile */

   while (cptr = strchr(outname, '/')) {      /* change / to . */
      *cptr = '.';
   } /* endwhile */

   fp = fopen(tmpname, "w");

   if (!fp) {
      switch(errno) {
      case ENOENT:
         fprintf(stderr, ERR_INVALID_KEY, outname);
        break;
      case EACCES:
         fprintf(stderr, ERR_PERMISSION_DENIED, outname);
        break;
      default:
         fprintf(stderr, ERR_UNKNOWN_WRITING, errno, outname);
        break;
      } /* end switch */
      rc = -1;
   } else {
      fprintf(fp, "%s\n", value);
      fclose(fp);
      if (PrintName)
         fprintf(stdout, "%s = ", outname);
      fprintf(stdout, "%s\n", value);
   } /* endif */

   free(tmpname);
   free(outname);
return rc;
} /* end WriteSetting() */



/*
 *     Read a sysctl setting 
 *
 */
int ReadSetting(const char *setting) {
int rc = 0;
char *tmpname, *outname, *cptr;
char inbuf[1025];
const char *name = setting;
FILE *fp;

   if (!setting || !*setting) {
      fprintf(stderr, ERR_INVALID_KEY, setting);
   } /* endif */

   tmpname = (char *)malloc((strlen(name)+strlen(PROC_PATH)+1)*sizeof(char));
   outname = (char *)malloc((strlen(name)+1)*sizeof(char));

   strcpy(tmpname, PROC_PATH);
   strcat(tmpname, name); 
   strcpy(outname, name); 
 
   while (cptr = strchr(tmpname, '.')) {      /* change . to / */
      *cptr = '/';
   } /* endwhile */

   while (cptr = strchr(outname, '/')) {      /* change / to . */
      *cptr = '.';
   } /* endwhile */

   fp = fopen(tmpname, "r");

   if (!fp) {
      switch(errno) {
      case ENOENT:
         fprintf(stderr, ERR_INVALID_KEY, outname);
        break;
      case EACCES:
         fprintf(stderr, ERR_PERMISSION_DENIED, outname);
        break;
      default:
         fprintf(stderr, ERR_UNKNOWN_READING, errno, outname);
        break;
      } /* end switch */
      rc = -1;
   } else {
      while(fgets(inbuf, 1024, fp)) {
         if (PrintName)
            fprintf(stdout, "%s = ", outname);
         fprintf(stdout, "%s", inbuf);  /* this already has the CR in it */
      } /* endwhile */
      fclose(fp);
   } /* endif */

   free(tmpname);
   free(outname);
return rc;
} /* end ReadSetting() */



/*
 *     Display all the sysctl settings 
 *
 */
int DisplayAll(const char *path, bool ShowTableUtil) {
int rc = 0;
int rc2;
DIR *dp;
struct dirent *de;
char *tmpdir;
struct stat ts;

   dp = opendir(path);

   if (!dp) {
      fprintf(stderr, ERR_OPENING_DIR, path);
      rc = -1;
   } else {
      readdir(dp); readdir(dp);   /* skip . and .. */
      while (de = readdir(dp)) {
         tmpdir = (char *)malloc(strlen(path)+strlen(de->d_name)+2);
         sprintf(tmpdir, "%s%s", path, de->d_name);
         rc2 = stat(tmpdir, &ts);       /* should check this return code */
         if (rc2 != 0) {
            perror(tmpdir);
         } else {
            if (S_ISDIR(ts.st_mode)) {
               strcat(tmpdir, "/");
               DisplayAll(tmpdir, ShowTableUtil);
            } else {
               rc |= ReadSetting(tmpdir+strlen(PROC_PATH));
            } /* endif */
         } /* endif */
         free(tmpdir);
      } /* end while */
      closedir(dp);
   } /* endif */

return rc;
} /* end DisplayAll() */
