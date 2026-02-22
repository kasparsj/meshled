#!/bin/bash

DUMP=/Users/kasparsj/Work2/homo_deus/EspStackTraceDecoder/dump.txt

DECODER_JAR=~/Downloads/EspStackTraceDecoder.jar

# Hardcoded paths
ELF_FILE="/Users/kasparsj/Library/Caches/arduino/sketches/8EA0552AA4E8E7F16E2C2E3D022AFCD8/homo_deus.ino.elf"

# for esp32
TOOLCHAIN_PATH=/Users/kasparsj/Library/Arduino15/packages/esp32/tools/esp-x32/2411/bin/xtensa-esp32-elf-addr2line

# Run the decoder
java -jar "$DECODER_JAR" "$TOOLCHAIN_PATH" "$ELF_FILE" "$DUMP"
