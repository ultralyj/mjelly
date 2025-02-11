/*
 * SPDX-FileCopyrightText: 2022-2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "mlx90393.h"

#ifdef __cplusplus
extern "C" {
#endif

extern mjd_mlx90393_config_t mlx90393_handle[4];
extern bool sensor_present[0x4+1][4];

void register_mconsole(void);

#ifdef __cplusplus
}
#endif
