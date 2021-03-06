//  Main entry point for running about this lib function
#include "sfs_c.c"
#include "sfs_about.h"
#include <stdio.h>
#include "../dependencies/log.c/src/log.h"

int main(int argc, char const *argv[]) {
    
    log_info("**************************");
    log_info("SFS C Library v. %s", SFS_C_VERSION);
    log_info("**************************");

    #ifdef SFS_DEBUG
    log_warn("THIS BUILD WITH DEBUGGING ENABLED");
    #endif

    return 0;

}
