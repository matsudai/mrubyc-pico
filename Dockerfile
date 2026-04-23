# ファームウェアのビルドツール
# 使い方 :
#     docker build -t localhost/mrubyc-pico .
#     docker run --rm -it -v $(pwd):/workspaces/mrubyc-pico localhost/mrubyc-pico make all
FROM ruby:4.0.6-slim-trixie

# Raspberry Pi Pico SDK + mrbc / picotool のビルドツールチェーン
#
# - build-essential, cmake, git : 汎用のビルドツール
# - gcc-arm-none-eabi, libnewlib-arm-none-eabi, libstdc++-arm-none-eabi-newlib : Cortex-M向けクロスツールチェーン
# - libusb-1.0-0-dev, pkg-config, python3 : picotoolが必要とする
RUN apt-get update && \
    apt-get install -y --no-install-recommends \
        build-essential \
        cmake \
        git \
        gcc-arm-none-eabi \
        libnewlib-arm-none-eabi \
        libstdc++-arm-none-eabi-newlib \
        libusb-1.0-0-dev \
        pkg-config \
        python3 && \
    apt-get clean && rm -rf /var/lib/apt/lists/*

ARG uid=1000
ARG gid=1000
ARG app_root=/workspaces/mrubyc-pico

RUN groupadd -g ${gid} ruby && \
    useradd -u ${uid} -g ${gid} -s /bin/bash -m ruby

USER ruby

WORKDIR ${app_root}
