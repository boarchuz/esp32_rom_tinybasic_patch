The ESP32 has a BASIC interpreter in ROM, which the bootloader may fall back to in certain conditions.
For security and reliability reasons, any binary built with ESP-IDF will now permanently disable this feature via efuse (CONSOLE_DEBUG_DISABLE).

If you would like to try the BASIC interpreter on an ESP32 with this efuse burnt, this patch will modify the efuse check in memory to re-enable access.

There is a prebuilt binary included that you can easily flash to your ESP32, eg.:
```
esptool.py write_flash 0x1000 basic.bin
```
Or, to build yourself, the quick and dirty way would be to navigate to ESP-IDF's `bootloader_start.c`, copy/include `esp32_rom_tinybasic_patch.h`, and modify `call_start_cpu0` to call it immediately:
```
void __attribute__((noreturn)) call_start_cpu0(void)
{
    tb_console_patch();
}
```

ESP32s without the efuse burnt may call the ROM function `void start_tb_console(void)` directly.
