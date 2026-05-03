/*! @file
  @brief Raspberry Pi Pico W / Pico 2 W向けmruby/c CYW43アーキテクチャの関数群

  CYW43ドライバを使用したGPIO制御の関数をRubyから利用可能にする．
  Ruby向けクラス・メソッドはRubyコードにて実装される．

  pico-sdkのAPIは下記を参照:
  - https://www.raspberrypi.com/documentation/pico-sdk/networking.html
*/
#include "mrbc_pico_cyw43_arch.h"
#include "mrubyc.h"
#include "pico/cyw43_arch.h"
#include <stdio.h>

/*! @brief mrbc_pico_cyw43_gpio_put(pin, level) CYW43 GPIOピンの出力レベルの設定

  @param pin CYW43 GPIOピン番号
  @param level 出力レベル（0: Low，1: High）
  @return void
*/
static void mrbc_pico_cyw43_gpio_put(mrb_vm* vm, mrb_value* v, int argc)
{
  int pin = GET_INT_ARG(1);
  int level = GET_INT_ARG(2);
  cyw43_arch_gpio_put(pin, level);
}

/*! @brief mrbc_pico_cyw43_gpio_get(pin) CYW43 GPIOピンの入力レベルの取得

  @param pin CYW43 GPIOピン番号
  @return GPIOから読み取られた値（0: Low，1: High）
*/
static void mrbc_pico_cyw43_gpio_get(mrb_vm* vm, mrb_value* v, int argc)
{
  int pin = GET_INT_ARG(1);
  SET_INT_RETURN(cyw43_arch_gpio_get(pin));
}

/** @brief C関数のRubyへの公開

  @param vm mruby/c VM
*/
void mrbc_pico_cyw43_arch_gem_init(struct VM* vm)
{
  // CYW43ドライバの初期化
  if (cyw43_arch_init()) {
    printf("WARNING: cyw43_arch_init() failed\r\n");
  }

  // Rubyのメソッド定義（Objectクラスに追加）
  mrbc_define_method(0, mrbc_class_object, "mrbc_pico_cyw43_gpio_put", mrbc_pico_cyw43_gpio_put);
  mrbc_define_method(0, mrbc_class_object, "mrbc_pico_cyw43_gpio_get", mrbc_pico_cyw43_gpio_get);
}
