/*
 * Copyright (c) 2024 The ZMK Contributors
 *
 * SPDX-License-Identifier: MIT
 */
 
#pragma once

#include <lvgl.h>
#include <zephyr/kernel.h>

struct zmk_widget_dongle_battery_status {
    sys_snode_t node;
};

int zmk_widget_dongle_battery_status_init(struct zmk_widget_dongle_battery_status *widget, lv_obj_t *parent);
