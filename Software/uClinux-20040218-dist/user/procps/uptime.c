#include "proc/whattime.h"
#include "proc/version.h"

int main(int argc, char *argv[]) {
  if(argc == 1) print_uptime();
  if((argc == 2) && (!strcmp(argv[1], "-V"))) display_version();
  return 0;
}
