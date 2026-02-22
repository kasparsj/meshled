# ESP Stacktrace Decoder

Utility wrapper for decoding ESP32 stack traces with `EspStackTraceDecoder.jar`.

## Usage

```bash
./tools/esp-stacktrace-decoder/esp32decoder.sh \
  <addr2line_path> \
  <firmware_elf> \
  <dump_file>
```

Example with PlatformIO toolchain and build outputs:

```bash
./tools/esp-stacktrace-decoder/esp32decoder.sh \
  ~/.platformio/packages/toolchain-xtensa-esp32/bin/xtensa-esp32-elf-addr2line \
  firmware/esp/.pio/build/esp32dev/firmware.elf \
  ./crash-dump.txt
```

## Notes

- Set `DECODER_JAR` if you want to use a jar outside this folder.
- This helper is optional and not part of CI.
