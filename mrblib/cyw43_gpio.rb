# CYW43 GPIO（CYW43チップ経由のGPIO制御）クラス
#
# Pico WのオンボードLED等，CYW43チップに接続されたGPIOを制御する．
# CYW43ドライバはgem_initで自動初期化される．
# APIはGPIOクラスと同一だが，ピンモード（params）は無視される．
#
# APIガイドは下記を参照:
# - https://github.com/mruby/microcontroller-peripheral-interface-guide
#
# @example
#   led = CYW43::GPIO.new(CYW43::GPIO::LED_PIN, GPIO::OUT)
#   led.write(1)
#   value = led.read
class CYW43
  class GPIO
    # CYW43 GPIOピン定数
    LED_PIN = 0 # オンボードLED

    attr_reader :pin

    # 指定されたピンのCYW43 GPIOインスタンスの初期化
    #
    # @param pin [Integer, String] ピン番号または識別子（文字列の場合は#to_iが呼び出される）
    # @param params [Integer] ピンモード（GPIOとのAPI互換性のため受け取るが無視される）
    #
    # @example
    #   led = CYW43::GPIO.new(CYW43::GPIO::LED_PIN, GPIO::OUT)
    def initialize(pin, params)
      @pin = pin.to_i
    end

    # ピンの値の読み取り
    #
    # @return [Integer] ピンの値（0または1）
    #
    # @example
    #   value = led.read
    def read
      mrbc_pico_cyw43_gpio_get(@pin)
    end

    # ピンの値がハイレベル（1）かどうかの確認
    #
    # @return [Boolean] ピンがハイレベルの場合はtrue
    #
    # @example
    #   if led.high?
    #     puts "ON"
    #   end
    def high?
      read == 1
    end

    # ピンの値がローレベル（0）かどうかの確認
    #
    # @return [Boolean] ピンがローレベルの場合はtrue
    #
    # @example
    #   if led.low?
    #     puts "OFF"
    #   end
    def low?
      read == 0
    end

    # ピンへ値の書き込み
    #
    # @param integer_data [Integer] 書き込む値（0 または 1）
    #
    # @example
    #   led.write(1)
    def write(integer_data)
      mrbc_pico_cyw43_gpio_put(@pin, integer_data)
      nil
    end

    # ピンモードの設定
    #
    # GPIOとのAPI互換性のために存在する．CYW43 GPIOでは何もしない．
    #
    # @param params [Integer] ピンモード（無視される）
    def setmode(params)
      nil
    end
  end
end
