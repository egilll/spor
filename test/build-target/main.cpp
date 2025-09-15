#include "Spor.h"

int main() {
    ZoneScoped;
    return 0;
}

extern "C" {

void __attribute__((weak, used)) _init() {}

void *_sbrk_r(struct _reent *, ptrdiff_t) {
    return nullptr;
}

void __attribute__((weak, used)) _close() {}

void __attribute__((weak, used)) _write(int fd, char *ptr, int len) {}

void __attribute__((weak, used)) _lseek() {}

void __attribute__((weak, used)) _read() {}

void __attribute__((weak, used)) _fstat() {}

void __attribute__((weak, used)) _getpid() {}

void __attribute__((weak, used)) _kill() {}

void __attribute__((weak, used)) _isatty() {}

void __attribute__((weak, used)) _stat() {}

void __attribute__((weak, used)) _exit(int status) {
    for (;;)
        ;
}
}