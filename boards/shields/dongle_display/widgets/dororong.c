/*
 * Copyright (c) 2024 The ZMK Contributors
 *
 * SPDX-License-Identifier: MIT
 */

#include <zephyr/kernel.h>
#include <zephyr/bluetooth/services/bas.h>

#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#include <zmk/display.h>
#include <zmk/event_manager.h>
#include <zmk/events/wpm_state_changed.h>
#include <zmk/wpm.h>

#include "dororong.h"

#define SRC(array) (const void **)array, sizeof(array) / sizeof(lv_img_dsc_t *)

static sys_slist_t widgets = SYS_SLIST_STATIC_INIT(&widgets);
static int64_t last_anim_update_time = 0;
#define ANIM_UPDATE_INTERVAL_MS 200  // Throttle: max 5 animation checks per second

LV_IMG_DECLARE(dororong_frame_00);
LV_IMG_DECLARE(dororong_frame_01);
LV_IMG_DECLARE(dororong_frame_02);
LV_IMG_DECLARE(dororong_frame_03);
LV_IMG_DECLARE(dororong_frame_04);
LV_IMG_DECLARE(dororong_frame_05);

static const lv_img_dsc_t *dororong_imgs[] = {
    &dororong_frame_00,
    &dororong_frame_01,
    &dororong_frame_02,
    &dororong_frame_03,
    &dororong_frame_04,
    &dororong_frame_05,
};

#define ANIMATION_SPEED_IDLE 10000
#define ANIMATION_SPEED_SLOW 2000
#define ANIMATION_SPEED_MID 500
#define ANIMATION_SPEED_FAST 200

struct dororong_wpm_status_state {
    uint8_t wpm;
};

enum anim_state {
    anim_state_none,
    anim_state_idle,
    anim_state_slow,
    anim_state_mid,
    anim_state_fast
} current_anim_state;

static void set_animation(lv_obj_t *animing, struct dororong_wpm_status_state state) {
    // Throttle animation state changes to prevent display thread flooding
    int64_t now = k_uptime_get();
    if ((now - last_anim_update_time) < ANIM_UPDATE_INTERVAL_MS) {
        return;
    }
    last_anim_update_time = now;

    if (state.wpm < 5) {
        if (current_anim_state != anim_state_idle) {
            lv_animimg_set_src(animing, SRC(dororong_imgs));
            lv_animimg_set_duration(animing, ANIMATION_SPEED_IDLE);
            lv_animimg_set_repeat_count(animing, LV_ANIM_REPEAT_INFINITE);
            lv_animimg_start(animing);
            current_anim_state = anim_state_idle;
        }
    } else if (state.wpm < 30) {
        if (current_anim_state != anim_state_slow) {
            lv_animimg_set_src(animing, SRC(dororong_imgs));
            lv_animimg_set_duration(animing, ANIMATION_SPEED_SLOW);
            lv_animimg_set_repeat_count(animing, LV_ANIM_REPEAT_INFINITE);
            lv_animimg_start(animing);
            current_anim_state = anim_state_slow;
        }
    } else if (state.wpm < 70) {
        if (current_anim_state != anim_state_mid) {
            lv_animimg_set_src(animing, SRC(dororong_imgs));
            lv_animimg_set_duration(animing, ANIMATION_SPEED_MID);
            lv_animimg_set_repeat_count(animing, LV_ANIM_REPEAT_INFINITE);
            lv_animimg_start(animing);
            current_anim_state = anim_state_mid;
        }
    } else {
        if (current_anim_state != anim_state_fast) {
            lv_animimg_set_src(animing, SRC(dororong_imgs));
            lv_animimg_set_duration(animing, ANIMATION_SPEED_FAST);
            lv_animimg_set_repeat_count(animing, LV_ANIM_REPEAT_INFINITE);
            lv_animimg_start(animing);
            current_anim_state = anim_state_fast;
        }
    }
}

struct dororong_wpm_status_state dororong_wpm_status_get_state(const zmk_event_t *eh) {
    struct zmk_wpm_state_changed *ev = as_zmk_wpm_state_changed(eh);
    // Add NULL check to prevent crash if event is NULL
    return (struct dororong_wpm_status_state) { .wpm = ev ? ev->state : 0 };
};

void dororong_wpm_status_update_cb(struct dororong_wpm_status_state state) {
    struct zmk_widget_dororong *widget;
    SYS_SLIST_FOR_EACH_CONTAINER(&widgets, widget, node) { set_animation(widget->obj, state); }
}

ZMK_DISPLAY_WIDGET_LISTENER(widget_dororong, struct dororong_wpm_status_state,
                            dororong_wpm_status_update_cb, dororong_wpm_status_get_state)

ZMK_SUBSCRIPTION(widget_dororong, zmk_wpm_state_changed);

int zmk_widget_dororong_init(struct zmk_widget_dororong *widget, lv_obj_t *parent) {
    widget->obj = lv_animimg_create(parent);
    lv_obj_center(widget->obj);

    sys_slist_append(&widgets, &widget->node);

    widget_dororong_init();

    return 0;
}

lv_obj_t *zmk_widget_dororong_obj(struct zmk_widget_dororong *widget) {
    return widget->obj;
}
