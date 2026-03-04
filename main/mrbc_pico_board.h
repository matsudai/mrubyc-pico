#ifndef MRBC_PICO_BOARD_H
#define MRBC_PICO_BOARD_H

struct VM; // #include "mrubyc.h"

// C関数をRubyへ公開する
void mrbc_pico_board_gem_init(struct VM* vm);

#endif // MRBC_PICO_BOARD_H
