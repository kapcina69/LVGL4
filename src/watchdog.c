#include "watchdog.h"
#include <zephyr/kernel.h>
#include <zephyr/drivers/watchdog.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(watchdog_module, LOG_LEVEL_INF);

#define WDT_TIMEOUT_MS 5000

#define WDT_STACK_SIZE 512
#define WDT_PRIORITY 5
K_THREAD_STACK_DEFINE(wdt_stack, WDT_STACK_SIZE);
static struct k_thread wdt_thread_data;

static const struct device *wdt;
static int wdt_channel_id;

static void wdt_thread_fn(void *a, void *b, void *c) {
    while (1) {
        k_msleep(1000);
        int err = wdt_feed(wdt, wdt_channel_id);
        if (err) {
            LOG_ERR("WDT feed error: %d", err);
        } else {
            LOG_INF("WDT ping");
        }
    }
}

void watchdog_init(void) {
    struct wdt_timeout_cfg wdt_config = {
        .window.min = 0,
        .window.max = WDT_TIMEOUT_MS,
        .callback = NULL,
        .flags = WDT_FLAG_RESET_SOC
    };

    wdt = DEVICE_DT_GET(DT_ALIAS(watchdog0));
    if (!device_is_ready(wdt)) {
        LOG_ERR("Watchdog device not ready");
        return;
    }

    wdt_channel_id = wdt_install_timeout(wdt, &wdt_config);
    if (wdt_channel_id < 0) {
        LOG_ERR("WDT install failed");
        return;
    }

    if (wdt_setup(wdt, 0) < 0) {
        LOG_ERR("WDT setup failed");
        return;
    }

    k_thread_create(&wdt_thread_data, wdt_stack, WDT_STACK_SIZE,
                    wdt_thread_fn, NULL, NULL, NULL,
                    WDT_PRIORITY, 0, K_NO_WAIT);

    LOG_INF("Watchdog initialized and thread started");
}
