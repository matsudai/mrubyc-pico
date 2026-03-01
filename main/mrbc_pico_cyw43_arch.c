/*! @file
  @brief Raspberry Pi Pico W向けmruby/c CYW43アーキテクチャの関数群

  CYW43ドライバを使用したGPIO制御およびWiFi制御関数をRubyから利用可能にする．
  Ruby向けクラス・メソッドはRubyコードにて実装される．

  APIガイドは下記を参照:
  - https://github.com/mruby/microcontroller-peripheral-interface-guide

  pico-sdkのAPIは下記を参照:
  - https://www.raspberrypi.com/documentation/pico-sdk/networking.html
*/
#include "mrbc_pico_cyw43_arch.h"
#include "mrubyc.h"
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"

// スキャン結果の最大保持件数
#define WIFI_SCAN_MAX_RESULTS 20

// スキャン結果の格納用バッファ
static cyw43_ev_scan_result_t scan_results[WIFI_SCAN_MAX_RESULTS];
static volatile int scan_result_count = 0;

// スキャンコールバック
static int scan_callback(void *env, const cyw43_ev_scan_result_t *result)
{
  (void)env;
  if (result && scan_result_count < WIFI_SCAN_MAX_RESULTS) {
    scan_results[scan_result_count] = *result;
    scan_result_count++;
  }
  return 0;
}

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

/*! @brief mrbc_pico_cyw43_wifi_enable_sta_mode() STAモードの有効化

  @return void
*/
static void mrbc_pico_cyw43_wifi_enable_sta_mode(mrb_vm* vm, mrb_value* v, int argc)
{
  cyw43_arch_enable_sta_mode();
}

/*! @brief mrbc_pico_cyw43_wifi_connect(ssid, password) WiFiネットワークへの接続

  WPA2認証でWiFiネットワークにブロッキング接続する．

  @param ssid SSID文字列
  @param password パスワード文字列
  @return void
*/
static void mrbc_pico_cyw43_wifi_connect(mrb_vm* vm, mrb_value* v, int argc)
{
  const char *ssid = (const char*)GET_STRING_ARG(1);
  const char *password = (const char*)GET_STRING_ARG(2);

  cyw43_arch_wifi_connect_blocking(ssid, password, CYW43_AUTH_WPA2_AES_PSK);
}

/*! @brief mrbc_pico_cyw43_wifi_status() WiFi接続ステータスの取得

  @return ステータス値（0: 未接続，3: 接続済み，-1: 接続失敗，-2: ネットワーク未検出，-3: 認証失敗）
*/
static void mrbc_pico_cyw43_wifi_status(mrb_vm* vm, mrb_value* v, int argc)
{
  int status = cyw43_tcpip_link_status(&cyw43_state, CYW43_ITF_STA);
  SET_INT_RETURN(status);
}

/*! @brief mrbc_pico_cyw43_wifi_scan_start() WiFiスキャンの実行

  非同期スキャンを開始し，完了まで待機する．

  @return スキャン結果の件数．エラーの場合は-1
*/
static void mrbc_pico_cyw43_wifi_scan_start(mrb_vm* vm, mrb_value* v, int argc)
{
  scan_result_count = 0;

  cyw43_wifi_scan_options_t scan_options = {0};
  int err = cyw43_wifi_scan(&cyw43_state, &scan_options, NULL, scan_callback);
  if (err != 0) {
    SET_INT_RETURN(-1);
    return;
  }

  while (cyw43_wifi_scan_active(&cyw43_state)) {
    sleep_ms(100);
  }

  SET_INT_RETURN(scan_result_count);
}

/*! @brief mrbc_pico_cyw43_wifi_scan_ssid(index) スキャン結果のSSIDの取得

  @param index スキャン結果のインデックス
  @return SSID文字列
*/
static void mrbc_pico_cyw43_wifi_scan_ssid(mrb_vm* vm, mrb_value* v, int argc)
{
  int i = GET_INT_ARG(1);
  if (i < 0 || i >= scan_result_count) {
    SET_NIL_RETURN();
    return;
  }
  int ssid_len = scan_results[i].ssid_len;
  if (ssid_len > 32) ssid_len = 32;

  SET_RETURN(mrbc_string_new(vm, scan_results[i].ssid, ssid_len));
}

/*! @brief mrbc_pico_cyw43_wifi_scan_bssid(index) スキャン結果のBSSIDの取得

  @param index スキャン結果のインデックス
  @return BSSIDのバイト列（6バイト）
*/
static void mrbc_pico_cyw43_wifi_scan_bssid(mrb_vm* vm, mrb_value* v, int argc)
{
  int i = GET_INT_ARG(1);
  if (i < 0 || i >= scan_result_count) {
    SET_NIL_RETURN();
    return;
  }
  SET_RETURN(mrbc_string_new(vm, scan_results[i].bssid, 6));
}

/*! @brief mrbc_pico_cyw43_wifi_scan_channel(index) スキャン結果のチャンネル番号の取得

  @param index スキャン結果のインデックス
  @return チャンネル番号
*/
static void mrbc_pico_cyw43_wifi_scan_channel(mrb_vm* vm, mrb_value* v, int argc)
{
  int i = GET_INT_ARG(1);
  if (i < 0 || i >= scan_result_count) {
    SET_NIL_RETURN();
    return;
  }
  SET_INT_RETURN(scan_results[i].channel);
}

/*! @brief mrbc_pico_cyw43_wifi_scan_rssi(index) スキャン結果の信号強度の取得

  @param index スキャン結果のインデックス
  @return RSSI値（dBm）
*/
static void mrbc_pico_cyw43_wifi_scan_rssi(mrb_vm* vm, mrb_value* v, int argc)
{
  int i = GET_INT_ARG(1);
  if (i < 0 || i >= scan_result_count) {
    SET_NIL_RETURN();
    return;
  }
  SET_INT_RETURN(scan_results[i].rssi);
}

/*! @brief mrbc_pico_cyw43_wifi_scan_auth(index) スキャン結果の認証モードの取得

  @param index スキャン結果のインデックス
  @return 認証モード値（ビットマスク）
*/
static void mrbc_pico_cyw43_wifi_scan_auth(mrb_vm* vm, mrb_value* v, int argc)
{
  int i = GET_INT_ARG(1);
  if (i < 0 || i >= scan_result_count) {
    SET_NIL_RETURN();
    return;
  }
  SET_INT_RETURN(scan_results[i].auth_mode);
}

/*! @brief mrbc_pico_cyw43_wifi_mac() MACアドレスの取得

  @return MACアドレスのバイト列（6バイト）
*/
static void mrbc_pico_cyw43_wifi_mac(mrb_vm* vm, mrb_value* v, int argc)
{
  uint8_t mac[6];
  cyw43_wifi_get_mac(&cyw43_state, CYW43_ITF_STA, mac);
  SET_RETURN(mrbc_string_new(vm, mac, 6));
}

/** @brief C関数のRubyへの公開

  CYW43ドライバの初期化を行い，GPIO制御およびWiFi制御関数をRubyに公開する．

  @param vm mruby/c VM
*/
void mrbc_pico_cyw43_arch_gem_init(struct VM* vm)
{
  // CYW43ドライバの初期化
  if (cyw43_arch_init()) {
    printf("WARNING: cyw43_arch_init() failed\r\n");
  }

  // CYW43 GPIO
  mrbc_define_method(0, mrbc_class_object, "mrbc_pico_cyw43_gpio_put", mrbc_pico_cyw43_gpio_put);
  mrbc_define_method(0, mrbc_class_object, "mrbc_pico_cyw43_gpio_get", mrbc_pico_cyw43_gpio_get);

  // CYW43 WiFi
  mrbc_define_method(0, mrbc_class_object, "mrbc_pico_cyw43_wifi_enable_sta_mode", mrbc_pico_cyw43_wifi_enable_sta_mode);
  mrbc_define_method(0, mrbc_class_object, "mrbc_pico_cyw43_wifi_connect", mrbc_pico_cyw43_wifi_connect);
  mrbc_define_method(0, mrbc_class_object, "mrbc_pico_cyw43_wifi_status", mrbc_pico_cyw43_wifi_status);
  mrbc_define_method(0, mrbc_class_object, "mrbc_pico_cyw43_wifi_scan_start", mrbc_pico_cyw43_wifi_scan_start);
  mrbc_define_method(0, mrbc_class_object, "mrbc_pico_cyw43_wifi_scan_ssid", mrbc_pico_cyw43_wifi_scan_ssid);
  mrbc_define_method(0, mrbc_class_object, "mrbc_pico_cyw43_wifi_scan_bssid", mrbc_pico_cyw43_wifi_scan_bssid);
  mrbc_define_method(0, mrbc_class_object, "mrbc_pico_cyw43_wifi_scan_channel", mrbc_pico_cyw43_wifi_scan_channel);
  mrbc_define_method(0, mrbc_class_object, "mrbc_pico_cyw43_wifi_scan_rssi", mrbc_pico_cyw43_wifi_scan_rssi);
  mrbc_define_method(0, mrbc_class_object, "mrbc_pico_cyw43_wifi_scan_auth", mrbc_pico_cyw43_wifi_scan_auth);
  mrbc_define_method(0, mrbc_class_object, "mrbc_pico_cyw43_wifi_mac", mrbc_pico_cyw43_wifi_mac);
}
