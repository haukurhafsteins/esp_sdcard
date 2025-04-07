#pragma once
#include <driver/spi_common.h>
void sdcard_init(spi_host_device_t host_device, const char* mount_point);