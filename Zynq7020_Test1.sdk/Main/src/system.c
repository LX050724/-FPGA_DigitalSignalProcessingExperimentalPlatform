//
// Created by yaoji on 2022/3/27.
//

#ifdef IN_CLION

#include <sys/types.h>
#include <sys/stat.h>
#include "xil_types.h"
#include <errno.h>

void _exit(sint32 status) {
    (void) status;
    while (1) { ;
    }
}


extern u8 _heap_start[];
extern u8 _heap_end[];

caddr_t _sbrk(s32 incr) {
    static u8 *heap = NULL;
    u8 *prev_heap;
    static u8 *HeapEndPtr = (u8 *) &_heap_end;
    caddr_t Status;

    if (heap == NULL) {
        heap = (u8 *) &_heap_start;
    }
    prev_heap = heap;

    if (((heap + incr) <= HeapEndPtr) && (prev_heap != NULL)) {
        heap += incr;
        Status = (caddr_t) ((void *) prev_heap);
    } else {
        Status = (caddr_t) -1;
    }

    return Status;
}


sint32 _write(sint32 fd, char8 *buf, sint32 nbytes) {
#if HYP_GUEST && EL1_NONSECURE && XEN_USE_PV_CONSOLE
    sint32 length;

    (void)fd;
    (void)nbytes;
    length = XPVXenConsole_Write(buf);
    return length;
#else
#ifdef STDOUT_BASEADDRESS
    s32 i;
    char8 *LocalBuf = buf;

    (void) fd;
    for (i = 0; i < nbytes; i++) {
        if (LocalBuf != NULL) {
            LocalBuf += i;
        }
        if (LocalBuf != NULL) {
            if (*LocalBuf == '\n') {
                outbyte('\r');
            }
            outbyte(*LocalBuf);
        }
        if (LocalBuf != NULL) {
            LocalBuf -= i;
        }
    }
    return (nbytes);
#else
    (void) fd;
    (void) buf;
    (void) nbytes;
    return 0;
#endif
#endif
}

s32 _read(s32 fd, char8 *buf, s32 nbytes) {
#ifdef STDIN_BASEADDRESS
    s32 i;
  s32 numbytes = 0;
  char8* LocalBuf = buf;

  (void)fd;
  if(LocalBuf != NULL) {
    for (i = 0; i < nbytes; i++) {
        numbytes++;
        *(LocalBuf + i) = inbyte();
        if ((*(LocalBuf + i) == '\n' )|| (*(LocalBuf + i) == '\r')) {
            break;
        }
    }
  }

  return numbytes;
#else
    (void) fd;
    (void) buf;
    (void) nbytes;
    return 0;
#endif
}

s32 _close(s32 fd) {
    (void) fd;
    return (0);
}

off_t _lseek(s32 fd, off_t offset, s32 whence) {
    (void) fd;
    (void) offset;
    (void) whence;
    errno = ESPIPE;
    return ((off_t) -1);
}

s32 _fstat(s32 fd, struct stat *buf) {
    (void) fd;
    buf->st_mode = S_IFCHR; /* Always pretend to be a tty */

    return (0);
}

sint32 _isatty(sint32 fd) {
    (void) fd;
    return (1);
}

#endif