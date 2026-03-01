#ifndef MRBC_PICO_CYW43_ARCH_H
#define MRBC_PICO_CYW43_ARCH_H

struct VM; // #include "mrubyc.h"

// C関数をRubyへ公開する
void mrbc_pico_cyw43_arch_gem_init(struct VM* vm);

#endif // MRBC_PICO_CYW43_ARCH_H
