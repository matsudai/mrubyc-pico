# ====================================================================
#  MakefileによるCMakeビルド手順の簡略化
#
#  CMakeのビルドプロセスを簡略化するためのラッパー．
#  プロジェクトのルートディレクトリで `make` を実行すると，ビルドディレクトリの作成，CMakeの実行，コンパイルが自動的に行われる．
# ====================================================================

# Makefileで定義されるコマンド一覧（非ファイル）
.PHONY: all clean configure mrbc help pico pico_w

# make コマンドのデフォルトで実行されるターゲット指定
.DEFAULT_GOAL := all

# ボード未指定時はエラー
PICO_BOARD ?= $(error Run 'make pico', 'make pico_w' or set PICO_BOARD environment variable)

# CMakeに渡す引数
CMAKE_ARGS = -DPICO_BOARD=$(PICO_BOARD)

# プロジェクトのビルド（直接呼び出された場合はPICO_BOARDかビルドキャッシュがなければエラー）
all: build/Makefile
	@make --no-print-directory -C build

# ターゲットごとのPICO_BOARD設定
pico: PICO_BOARD=pico
pico_w: PICO_BOARD=pico_w

# ボードに関わらずビルドの実行はallターゲット
pico pico_w: all

# CMakeのビルド設定ファイル（Makefile）の生成ルール
#
# CMakeLists.txt更新時のみ実行される．
build/Makefile: CMakeLists.txt
	@cmake -S . -B build $(CMAKE_ARGS)

# ビルド成果物の削除
clean:
	@rm -rf build/*

mrbc:
	@mkdir -p build
	@mrbc -o build/master.mrbc src/master.rb

# 利用可能なコマンドの表示
help:
	@echo "Usage: make [target]"
	@echo ""
	@echo "Available targets:"
	@echo "  pico        Build for Pico."
	@echo "  pico_w      Build for Pico W (WiFi support)."
	@echo "  clean       Remove all build artifacts."
	@echo "  help        Show this help message."
