/*! @file
  @brief mruby/c用のコマンドライン機能

  ユーザーからのコマンド入力を受け取り，解析して適切なコマンド番号を返す機能と，
  バイトコードのヘキサダンプ表示，CRC16計算の機能を提供する．
*/
#include "mrbwrite.h"
#include "pico/stdlib.h" 
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/** @brief ユーザー入力のコマンド番号取得

  シリアル入力からコマンドを受け取り，対応するコマンド番号を返す．
  write・write_libコマンドの場合は，サイズとCRCパラメータも同時に取得する．

  @param timeout_us タイムアウト時間（マイクロ秒）
  @param size write・write_libコマンドのときに書き込みサイズが設定される．呼び出し側で利用しない場合はNULLを指定可能
  @param crc write・write_libコマンドのときにCRC16（16進表記）の指定があれば設定される．未指定時は-1．呼び出し側で利用しない場合はNULLを指定可能
  @return 下記のコマンド番号
      -2: 正しくないコマンド
      -1: タイムアウト
      0: (CR+LF)   - コマンドモード
      1: reset     - ソフトウェアリセット
      2: execute   - mrubyプログラム実行
      3: write     - タスクのmrubyバイトコード書き込み
      4: clear     - 書き込み済みバイトコードの消去
      5: help      - コマンド一覧表示（人間用）
      6: version   - バージョン表示
      7: showprog  - 書き込み済みプログラムサイズ表示（人間用）
      8: write_lib - ライブラリのmrubyバイトコード書き込み
*/
int mrbwrite_get_cmd (uint32_t timeout_us, uint32_t *size, int32_t *crc) {
  const uint32_t buffer_size = 32; // コマンドバッファのサイズ
  uint8_t buffer[buffer_size];
  memset(buffer, 0, buffer_size); // コマンドバッファの初期化

  // コマンド入力の受付
  for (int i = 0; i < buffer_size; i++) {
    int input = getchar_timeout_us(timeout_us);
    if (input == PICO_ERROR_TIMEOUT)
      return MRBWRITE_TIMEOUT;
    const uint8_t ch = (uint8_t)(input & 0xFF);
    buffer[i] = ch;
    if (ch == 0x0a) { // LF (0x0a) が入力されたらコマンド入力終了
      break;
    }
  }

  // コマンドを返す
  uint32_t dummy = 0;
  uint32_t crc_value = 0;
  int parsed = 0;
  if (memcmp(buffer, "\r\n", 2) == 0) {
    return MRBWRITE_COMMAND_MODE;
  } else if (memcmp(buffer, "reset\r\n", 7) == 0) {
    return MRBWRITE_RESET;
  } else if (memcmp(buffer, "execute\r\n", 9) == 0) {
    return MRBWRITE_EXECUTE;
  } else if ((parsed = sscanf(buffer, "write_lib %d %x\r\n", (size == NULL ? &dummy : size), &crc_value)) >= 1) {
    if (crc != NULL) *crc = (parsed == 2) ? (int32_t)crc_value : -1;
    return MRBWRITE_WRITE_LIB;
  } else if ((parsed = sscanf(buffer, "write %d %x\r\n", (size == NULL ? &dummy : size), &crc_value)) >= 1) {
    if (crc != NULL) *crc = (parsed == 2) ? (int32_t)crc_value : -1;
    return MRBWRITE_WRITE;
  } else if (memcmp(buffer, "clear\r\n", 7) == 0) {
    return MRBWRITE_CLEAR;
  } else if (memcmp(buffer, "help\r\n", 6) == 0) {
    return MRBWRITE_HELP;
  } else if (memcmp(buffer, "version\r\n", 9) == 0) {
    return MRBWRITE_VERSION;
  } else if (memcmp(buffer, "showprog\r\n", 10) == 0) {
    return MRBWRITE_SHOWPROG;
  }

  return MRBWRITE_ILLEGAL;
}

/** @brief バイトコードのヘキサダンプ表示

  指定されたバッファのバイトコードを16進数とASCII文字で見やすく表示する．
  デバッグやプログラム確認用途で使用される．

  @param filename 表示対象のファイル名
  @param buffer バイトコードデータが格納されたバッファのポインタ
  @param size バイトコードのサイズ（バイト単位）
  @return void
*/
void mrbwrite_showprog(const char *filename, uint8_t* buffer, uint32_t size) {  
  // ファイル名とヘッダーを表示
  printf("**** %s ****\r\n", filename);
  printf("01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F\r\n");

  // 16バイトごとにヘキサダンプを表示
  for (uint32_t i = 0; i < size; i += 16) {
    printf("%08x: ", i);

    // バイト値をヘキサで表示
    for (int j = 0; j < 16; j++) {
      if (i + j < size) {
        printf("%02x ", buffer[i + j]);
      } else {
        printf("   ");
      }
    }

    // ASCII文字として表示可能な文字を表示
    printf(" | ");
    for (int j = 0; j < 16 && i + j < size; j++) {
      char c = (char)buffer[i + j];
      printf("%c", (c >= 32 && c <= 126) ? c : '.');
    }
    printf(" |\r\n");
  }
}

/** @brief バイトコードのCRC16計算

  指定されたバッファの内容からCRC16チェックサムを計算する．
  生成多項式は0x1021を使用し，初期値は0xFFFFとする．

  @param buffer 計算対象のバイトコードデータが格納されたバッファのポインタ
  @param size バッファのサイズ（バイト単位）
  @return CRC16値
*/
uint16_t mrbwrite_crc16(const uint8_t *buffer, uint32_t size) {
  // CRC16初期値と生成多項式の設定
  uint16_t crc = 0xFFFF;
  const uint16_t poly = 0x1021;

  // CRC16をバイトごとに計算
  for (uint32_t i = 0; i < size; i++) {
    crc ^= (uint16_t)buffer[i] << 8;
    for (int j = 0; j < 8; j++) {
      if (crc & 0x8000) {
        crc = (crc << 1) ^ poly;
      } else {
        crc <<= 1;
      }
    }
  }

  return crc;
}
