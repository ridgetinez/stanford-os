#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include "libunix.h"

// read in file <name>
// returns:
//  - pointer to the code.  pad code with 0s up to the
//    next multiple of 4.  
//  - bytes of code in <size>
//
// fatal error open/read of <name> fails.
// ^ interpreting this to mean exit() if cannot read a file.
//   not too general purpose... I suppose it's nice not to
//   check for NULL all the time.
// 
// How: 
//    - use stat to get the size of the file.
//    - round up to a multiple of 4.
//    - allocate a buffer  --- 
//    - zero pads to a // multiple of 4.
//    - read entire file into buffer.  
//    - make sure any padding bytes have zeros.
//    - return it.   
//
// make sure to close the file descriptor (this will
// matter for later labs).
// restrict - https://en.wikipedia.org/wiki/Restrict
// | ...only this pointer value, or values directly derived from it (e.g. pointer + 1) 
// | will be used to access the object to which it points.
void *read_file(unsigned *size, const char *name) {
    struct stat fileinfo;
    if (stat(name, &fileinfo)) {
        exit(1);
    }

    *size = fileinfo.st_size;
    void *contents = calloc(sizeof(unsigned char), *size + (*size % 4));
    if (contents == NULL) {
        exit(1);
    }

    // why not fread?
    int fd = open(name, O_RDONLY);
    if (fd < 0) {
        exit(1);
    }
    if (read(fd, contents, *size) < 0) {
        exit(1);
    }
    close(fd);

    return contents;
}
 
// int main(void) {
//     unsigned size;
//     void *code_start = read_file(&size, "read-file.c");
//     printf("%.*s", size, code_start);
// }
