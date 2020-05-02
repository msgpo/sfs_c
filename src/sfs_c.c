//  Lib implementation

#include "sfs_c.h"
#include <stdio.h>

/**
 * Standard signature of an SFS file.  This signature intended to make the file
 * look like an NES ROM when it isn't (hence the 'just kidding' afterward)
 */
#define SFS_SIGNATURE { 'N', 'E', 'S', 26, 'J', 'K' }

int sfs_checkIsSFS(char *filePath) {

    FILE *filePointer;
    if((filePointer = fopen(filePath, "rb")) == NULL) {
        fprintf(stderr, "Could not open %s\n", filePath);
        return -1;
    }

    char prefixBuffer[6];
    if (fread(prefixBuffer, 1, 6, filePointer) == 6) {
        char expectedBytes[6] = SFS_SIGNATURE;
        int i;
        for(i=0; i<6; i++) {
            if(expectedBytes[i] != prefixBuffer[i]) {
                return -1;
            }
        }

        return 1;
    }
    

    return 0;
}