/*! @file
  @brief Raspberry Pi Pico W向けmruby/c lwIPネットワーク情報の関数群

  lwIPスタックを使用したネットワーク情報取得関数をRubyから利用可能にする．
  Ruby向けクラス・メソッドはRubyコードにて実装される．

  APIガイドは下記を参照:
  - https://github.com/mruby/microcontroller-peripheral-interface-guide

  pico-sdkのAPIは下記を参照:
  - https://www.raspberrypi.com/documentation/pico-sdk/networking.html
*/
#include "mrbc_pico_lwip.h"
#include "mrubyc.h"
#include "pico/cyw43_arch.h"
#include "lwip/netif.h"
#include "lwip/ip4_addr.h"
#include "lwip/dns.h"

/*! @brief mrbc_pico_lwip_begin() lwIPロックの取得

  @return void
*/
static void mrbc_pico_lwip_begin(mrb_vm* vm, mrb_value* v, int argc)
{
  cyw43_arch_lwip_begin();
}

/*! @brief mrbc_pico_lwip_end() lwIPロックの解放

  @return void
*/
static void mrbc_pico_lwip_end(mrb_vm* vm, mrb_value* v, int argc)
{
  cyw43_arch_lwip_end();
}

/*! @brief mrbc_pico_lwip_ip() IPアドレスの取得

  lwIPロック内で呼ぶこと．

  @return IPアドレスのバイト列（4バイト），未接続時は全て0
*/
static void mrbc_pico_lwip_ip(mrb_vm* vm, mrb_value* v, int argc)
{
  if (netif_default == NULL) {
    SET_RETURN(mrbc_string_new(vm, "\0\0\0\0", 4));
    return;
  }
  const ip4_addr_t *addr = netif_ip4_addr(netif_default);
  SET_RETURN(mrbc_string_new(vm, &addr->addr, 4));
}

/*! @brief mrbc_pico_lwip_netmask() サブネットマスクの取得

  lwIPロック内で呼ぶこと．

  @return サブネットマスクのバイト列（4バイト），未接続時は全て0
*/
static void mrbc_pico_lwip_netmask(mrb_vm* vm, mrb_value* v, int argc)
{
  if (netif_default == NULL) {
    SET_RETURN(mrbc_string_new(vm, "\0\0\0\0", 4));
    return;
  }
  const ip4_addr_t *addr = netif_ip4_netmask(netif_default);
  SET_RETURN(mrbc_string_new(vm, &addr->addr, 4));
}

/*! @brief mrbc_pico_lwip_gw() デフォルトゲートウェイの取得

  lwIPロック内で呼ぶこと．

  @return ゲートウェイのバイト列（4バイト），未接続時は全て0
*/
static void mrbc_pico_lwip_gw(mrb_vm* vm, mrb_value* v, int argc)
{
  if (netif_default == NULL) {
    SET_RETURN(mrbc_string_new(vm, "\0\0\0\0", 4));
    return;
  }
  const ip4_addr_t *addr = netif_ip4_gw(netif_default);
  SET_RETURN(mrbc_string_new(vm, &addr->addr, 4));
}

/*! @brief mrbc_pico_lwip_dns() DNSサーバアドレスの取得

  lwIPロック内で呼ぶこと．

  @return DNSサーバのバイト列（4バイト），未接続時は全て0
*/
static void mrbc_pico_lwip_dns(mrb_vm* vm, mrb_value* v, int argc)
{
  if (netif_default == NULL) {
    SET_RETURN(mrbc_string_new(vm, "\0\0\0\0", 4));
    return;
  }
  const ip4_addr_t *addr = ip_2_ip4(dns_getserver(0));
  SET_RETURN(mrbc_string_new(vm, &addr->addr, 4));
}

/** @brief C関数のRubyへの公開

  lwIPネットワーク情報取得関数をRubyに公開する．

  @param vm mruby/c VM
*/
void mrbc_pico_lwip_gem_init(struct VM* vm)
{
  mrbc_define_method(0, mrbc_class_object, "mrbc_pico_lwip_begin", mrbc_pico_lwip_begin);
  mrbc_define_method(0, mrbc_class_object, "mrbc_pico_lwip_end", mrbc_pico_lwip_end);
  mrbc_define_method(0, mrbc_class_object, "mrbc_pico_lwip_ip", mrbc_pico_lwip_ip);
  mrbc_define_method(0, mrbc_class_object, "mrbc_pico_lwip_netmask", mrbc_pico_lwip_netmask);
  mrbc_define_method(0, mrbc_class_object, "mrbc_pico_lwip_gw", mrbc_pico_lwip_gw);
  mrbc_define_method(0, mrbc_class_object, "mrbc_pico_lwip_dns", mrbc_pico_lwip_dns);
}
