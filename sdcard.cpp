/* SD card and FAT filesystem example.
   This example uses SPI peripheral to communicate with SD card.

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include <esp_log.h>
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"
#if SOC_SDMMC_IO_POWER_EXTERNAL
#include "sd_pwr_ctrl_by_on_chip_ldo.h"
#endif
#include "driver/sdspi_host.h"
#include "sdcard.h"

// #define EXAMPLE_MAX_CHAR_SIZE    64

static const char *TAG = "SDCARD";

// static esp_err_t s_example_write_file(const char *path, char *data)
// {
//     ESP_LOGI(TAG, "Opening file %s", path);
//     FILE *f = fopen(path, "w");
//     if (f == NULL) {
//         ESP_LOGE(TAG, "Failed to open file for writing");
//         return ESP_FAIL;
//     }
//     fprintf(f, data);
//     fclose(f);
//     ESP_LOGI(TAG, "File written");

//     return ESP_OK;
// }

// static esp_err_t s_example_read_file(const char *path)
// {
//     ESP_LOGI(TAG, "Reading file %s", path);
//     FILE *f = fopen(path, "r");
//     if (f == NULL) {
//         ESP_LOGE(TAG, "Failed to open file for reading");
//         return ESP_FAIL;
//     }
//     char line[EXAMPLE_MAX_CHAR_SIZE];
//     fgets(line, sizeof(line), f);
//     fclose(f);

//     // strip newline
//     char *pos = strchr(line, '\n');
//     if (pos) {
//         *pos = '\0';
//     }
//     ESP_LOGI(TAG, "Read from file: '%s'", line);

//     return ESP_OK;
// }

void sdcard_init(spi_host_device_t host_device, gpio_num_t cs, const char* mount_point)
{
    esp_err_t ret;

    // Options for mounting the filesystem.
    // If format_if_mount_failed is set to true, SD card will be partitioned and
    // formatted in case when mounting fails.
    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = true,
        .max_files = 5,
        .allocation_unit_size = 16 * 1024
    };
    sdmmc_card_t *card;
    ESP_LOGI(TAG, "Initializing SD card");

    // Use settings defined above to initialize SD card and mount FAT filesystem.
    // Note: esp_vfs_fat_sdmmc/sdspi_mount is all-in-one convenience functions.
    // Please check its source code and implement error recovery when developing
    // production applications.
    ESP_LOGI(TAG, "Using SPI peripheral");

    // By default, SD card frequency is initialized to SDMMC_FREQ_DEFAULT (20MHz)
    // For setting a specific frequency, use host.max_freq_khz (range 400kHz - 20MHz for SDSPI)
    // Example: for fixed frequency of 10MHz, use host.max_freq_khz = 10000;
    sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    host.slot = host_device;

    // spi_bus_config_t bus_cfg = {
    //     .mosi_io_num = PIN_NUM_MOSI,
    //     .miso_io_num = PIN_NUM_MISO,
    //     .sclk_io_num = PIN_NUM_CLK,
    //     .quadwp_io_num = -1,
    //     .quadhd_io_num = -1,
    //     .max_transfer_sz = 4000,
    // };

    // ret = spi_bus_initialize(host.slot, &bus_cfg, SDSPI_DEFAULT_DMA);
    // if (ret != ESP_OK) {
    //     ESP_LOGE(TAG, "Failed to initialize bus.");
    //     return;
    // }

    // This initializes the slot without card detect (CD) and write protect (WP) signals.
    // Modify slot_config.gpio_cd and slot_config.gpio_wp if your board has these signals.
    sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_config.gpio_cs = cs;
    slot_config.host_id = host_device;

    ESP_LOGI(TAG, "Mounting filesystem");
    ret = esp_vfs_fat_sdspi_mount(mount_point, &host, &slot_config, &mount_config, &card);

    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG, "Failed to mount filesystem. "
                     "If you want the card to be formatted, set the CONFIG_EXAMPLE_FORMAT_IF_MOUNT_FAILED menuconfig option.");
        } else {
            ESP_LOGE(TAG, "Failed to initialize the card (%s). "
                     "Make sure SD card lines have pull-up resistors in place.", esp_err_to_name(ret));
#ifdef CONFIG_EXAMPLE_DEBUG_PIN_CONNECTIONS
            check_sd_card_pins(&config, pin_count);
#endif
        }
        return;
    }
    ESP_LOGI(TAG, "Filesystem mounted");

//     // Card has been initialized, print its properties
//     sdmmc_card_print_info(stdout, card);

//     // Use POSIX and C standard library functions to work with files.

//     // First create a file.
//     char file_hello[64];
//     snprintf(file_hello, sizeof(file_hello), "%s/hello.txt", mount_point);
//     char data[EXAMPLE_MAX_CHAR_SIZE];
//     snprintf(data, EXAMPLE_MAX_CHAR_SIZE, "%s %s!\n", "Hello", card->cid.name);
//     ret = s_example_write_file(file_hello, data);
//     if (ret != ESP_OK) {
//         return;
//     }

//     char file_foo[64];
//     snprintf(file_foo, sizeof(file_foo), "%s/foo.txt", mount_point);

//     // Check if destination file exists before renaming
//     struct stat st;
//     if (stat(file_foo, &st) == 0) {
//         // Delete it if it exists
//         unlink(file_foo);
//     }

//     // Rename original file
//     ESP_LOGI(TAG, "Renaming file %s to %s", file_hello, file_foo);
//     if (rename(file_hello, file_foo) != 0) {
//         ESP_LOGE(TAG, "Rename failed");
//         return;
//     }

//     ret = s_example_read_file(file_foo);
//     if (ret != ESP_OK) {
//         return;
//     }

//     char file_nihao[64];
//     snprintf(file_nihao, sizeof(file_nihao), "%s/nihao.txt", mount_point);
//     memset(data, 0, EXAMPLE_MAX_CHAR_SIZE);
//     snprintf(data, EXAMPLE_MAX_CHAR_SIZE, "%s %s!\n", "Nihao", card->cid.name);
//     ret = s_example_write_file(file_nihao, data);
//     if (ret != ESP_OK) {
//         return;
//     }

//     //Open file for reading
//     ret = s_example_read_file(file_nihao);
//     if (ret != ESP_OK) {
//         return;
//     }

//     // Deinitialize the power control driver if it was used
// #if CONFIG_EXAMPLE_SD_PWR_CTRL_LDO_INTERNAL_IO
//     ret = sd_pwr_ctrl_del_on_chip_ldo(pwr_ctrl_handle);
//     if (ret != ESP_OK) {
//         ESP_LOGE(TAG, "Failed to delete the on-chip LDO power control driver");
//         return;
//     }
// #endif
}