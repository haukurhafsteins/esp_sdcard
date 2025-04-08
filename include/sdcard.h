#pragma once
#include <driver/spi_common.h>
#include <driver/gpio.h>
void sdcard_init(spi_host_device_t host_device, gpio_num_t cs, const char* MOUNT_POINT);
