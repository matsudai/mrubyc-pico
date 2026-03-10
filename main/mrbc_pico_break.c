/*! @file
  @brief Break信号による再起動機能

  USB CDCのBreak信号を検出し，ウォッチドッグタイマによるソフトリセットを実行する．
  Break信号の検出はTinyUSBのコールバックで行い，mrbc_runのタスクスイッチ時にメインコンテキストからリブートを実行する．
*/
#include "mrbc_pico_break.h"
#include "mrubyc.h"
#include "tusb.h"
#include "hardware/watchdog.h"

static volatile bool break_received = false;

/** @brief Break信号受信コールバック

  TinyUSBのコールバック設定．
  Break信号受信時にフラグを立てる．
*/
void tud_cdc_send_break_cb(uint8_t itf, uint16_t duration_ms) {
  (void)itf; // avoid warning.
  (void)duration_ms; // avoid warning.
  break_received = true;
}

/** @brief Break信号によるソフトウェアリセット

  mrbc_set_task_switch_callback()で登録するコールバック．
  Break信号が検出された場合，100ms後に再起動を予約する．
*/
static void break_check(void) {
  if (break_received) {
    mrbc_set_task_switch_callback(NULL);
    watchdog_reboot(0, 0, 100);
  }
}

/** @brief Break信号によるリブート機能の初期化

  mrbc_runのタスクスイッチコールバックにbreak_checkを登録する．
*/
void mrbc_pico_break_init(struct VM* vm) {
  (void)vm;
  mrbc_set_task_switch_callback(break_check);
}
