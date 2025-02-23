#include <stdint.h>
#include "soc/timer_group_reg.h"
#include "soc/rtc_cntl_reg.h"

#define ESP_ROM_TINYBASIC_TEXT_OBFUSCATED_ADDR              0x4005acf0
#define ESP_ROM_TINYBASIC_TEXT_OBFUSCATED_SIZE              0x12f0
#define ESP_ROM_TINYBASIC_TEXT_OBFUSCATED_XOR               0xa5a5a5a5
#define ESP_ROM_TINYBASIC_TEXT_ADDR                         0x40090000
#define ESP_ROM_TINYBASIC_MEM_ADDR                          0x3ffd2000
#define ESP_ROM_TINYBASIC_MEM_SIZE                          0xe000
#define ESP_ROM_TINYBASIC_DATA_OBFUSCATED_ADDR              0x4005a9e4
#define ESP_ROM_TINYBASIC_DATA_OBFUSCATED_SIZE              0x30c
#define ESP_ROM_TINYBASIC_DATA_OBFUSCATED_XOR               0xa5a5a5a5
#define ESP_ROM_TINYBASIC_DATA_ADDR                         0x3ffd2000
#define ESP_ROM_TINYBASIC_CONSOLE_DISABLE_CHECK_FN_ADDR     0x400912bc
#define ESP_ROM_TINYBASIC_CONSOLE_ENTRY_FN_PTR_ADDR         0x40090000

static void tb_console_patch_text_init(void)
{
    const uint32_t *tbtext_bin = (const uint32_t*)ESP_ROM_TINYBASIC_TEXT_OBFUSCATED_ADDR;
    uint32_t *tb_text = (uint32_t*)ESP_ROM_TINYBASIC_TEXT_ADDR;
    for(size_t i = 0; i < (ESP_ROM_TINYBASIC_TEXT_OBFUSCATED_SIZE / sizeof(uint32_t)); ++i)
    {
        *tb_text = (*tbtext_bin ^ ESP_ROM_TINYBASIC_TEXT_OBFUSCATED_XOR);
        ++tbtext_bin;
        ++tb_text;
    }
}

static void tb_console_patch_data_init(void)
{
    memset(ESP_ROM_TINYBASIC_MEM_ADDR, 0, ESP_ROM_TINYBASIC_MEM_SIZE);

    const uint32_t *tbdata_obfs = (const uint32_t*)ESP_ROM_TINYBASIC_DATA_OBFUSCATED_ADDR;
    uint32_t *tb_data = (uint32_t*)ESP_ROM_TINYBASIC_DATA_ADDR;
    for(size_t i = 0; i < (ESP_ROM_TINYBASIC_DATA_OBFUSCATED_SIZE / sizeof(uint32_t)); ++i)
    {
        *tb_data = (*tbdata_obfs ^ ESP_ROM_TINYBASIC_DATA_OBFUSCATED_XOR);
        ++tbdata_obfs;
        ++tb_data;
    }
}

static void tb_console_patch_modify_efuse_check_fn(void)
{
    // Overwrite function that checks EFUSE_RD_CONSOLE_DEBUG_DISABLE bit to always return 0
    const uint8_t efuse_check_patch_fn[] = {
        0x36, 0x41, 0x00,   // entry
        0x0c, 0x02,         // movi.n a2, 0
        0x1d, 0xf0,         // retw.n
        0x00,
    };
    _Static_assert(sizeof(efuse_check_patch_fn) % sizeof(uint32_t) == 0, "");

    uint32_t *efuse_check_fn_addr = (uint32_t*)ESP_ROM_TINYBASIC_CONSOLE_DISABLE_CHECK_FN_ADDR;
    const uint32_t *efuse_check_patch_fn_ptr = (const uint32_t*)&efuse_check_patch_fn;
    for(size_t i = 0; i < (sizeof(efuse_check_patch_fn) / sizeof(uint32_t)); ++i)
    {
        *efuse_check_fn_addr = *efuse_check_patch_fn_ptr;
        ++efuse_check_fn_addr;
        ++efuse_check_patch_fn_ptr;
    }
}

static void __attribute__((noreturn)) tb_console_patch_start(void)
{
    void (*tb_entry)(void) = *(void (**)(void))ESP_ROM_TINYBASIC_CONSOLE_ENTRY_FN_PTR_ADDR;
    tb_entry();
    __builtin_unreachable();
}

static void tb_console_patch_init(void)
{
    tb_console_patch_text_init();
    tb_console_patch_data_init();
    tb_console_patch_modify_efuse_check_fn();
}

static void __attribute__((noreturn, unused)) tb_console_patch(void)
{
    REG_WRITE(TIMG_WDTCONFIG0_REG(0), 0);
    REG_WRITE(RTC_CNTL_WDTCONFIG0_REG, 0);
    tb_console_patch_init();
    tb_console_patch_start();
}
