/*! @file
  @brief Raspberry Pi Pico向けmruby/c ボード情報の関数群

  ボード情報をRubyに公開する定数・関数を集約する．
  - mrbc_pico_rp2350?()        : RP2350ベースのボードかどうかの判定
  - mrbc_pico_cyw43_supported?(): CYW43チップ対応ボードかどうかの判定
  - mrbc_pico_default_led_pin() : デフォルトLEDピン番号の取得
*/
#include "mrbc_pico_board.h"
#include "mrubyc.h"

/*! @brief mrbc_pico_rp2350?() RP2350ベースのボードかどうかの判定

  @return true: RP2350（Pico 2等），false: RP2040（Pico等）
*/
static void mrbc_pico_rp2350(mrb_vm* vm, mrb_value* v, int argc)
{
#if PICO_RP2350
  SET_TRUE_RETURN();
#else
  SET_FALSE_RETURN();
#endif
}

/*! @brief mrbc_pico_cyw43_supported?() CYW43チップ対応ボードかどうかの判定

  @return true: CYW43対応（Pico W等），false: 非対応（Pico等）
*/
static void mrbc_pico_cyw43_supported(mrb_vm* vm, mrb_value* v, int argc)
{
#if PICO_CYW43_SUPPORTED
  SET_TRUE_RETURN();
#else
  SET_FALSE_RETURN();
#endif
}

/*! @brief mrbc_pico_default_led_pin() デフォルトLEDピン番号の取得

  ボードに応じたオンボードLEDのピン番号を返す．
  Pico W系: CYW43_WL_GPIO_LED_PIN (0)，Pico系: PICO_DEFAULT_LED_PIN (25)

  @return LEDピン番号
*/
static void mrbc_pico_default_led_pin(mrb_vm* vm, mrb_value* v, int argc)
{
#if defined(CYW43_WL_GPIO_LED_PIN)
  SET_INT_RETURN(CYW43_WL_GPIO_LED_PIN);
#elif defined(PICO_DEFAULT_LED_PIN)
  SET_INT_RETURN(PICO_DEFAULT_LED_PIN);
#else
  SET_INT_RETURN(-1);
#endif
}

/** @brief C関数のRubyへの公開

  @param vm mruby/c VM
*/
void mrbc_pico_board_gem_init(struct VM* vm)
{
  // ボード情報
  mrbc_define_method(0, mrbc_class_object, "mrbc_pico_rp2350?", mrbc_pico_rp2350);
  mrbc_define_method(0, mrbc_class_object, "mrbc_pico_cyw43_supported?", mrbc_pico_cyw43_supported);
  mrbc_define_method(0, mrbc_class_object, "mrbc_pico_default_led_pin", mrbc_pico_default_led_pin);
}
