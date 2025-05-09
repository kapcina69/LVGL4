#include "battery_monitor.h"

#include <stdbool.h>
#include <inttypes.h>
#include <stddef.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/drivers/adc.h>
#include <zephyr/devicetree.h>
#include <zephyr/device.h>
#include <zephyr/sys/util.h>

#if !DT_NODE_EXISTS(DT_PATH(zephyr_user)) || \
    !DT_NODE_HAS_PROP(DT_PATH(zephyr_user), io_channels)
#error "No suitable devicetree overlay specified"
#endif

#define DT_SPEC_AND_COMMA(node_id, prop, idx) \
    ADC_DT_SPEC_GET_BY_IDX(node_id, idx),

/* ADC kanali iz devicetree-ja */
static const struct adc_dt_spec adc_channels[] = {
    DT_FOREACH_PROP_ELEM(DT_PATH(zephyr_user), io_channels, DT_SPEC_AND_COMMA)
};

bool is_charging = false;
int percentage = 0;


int voltage_to_percent(int battery_mv)
{
    struct {
        float voltage;
        int percent;
    } lut[] = {
        {4200, 100}, {4000, 90}, {3900, 80}, {3800, 70},
        {3700, 60}, {3600, 50}, {3500, 40}, {3400, 30},
        {3300, 20}, {3200, 10}, {3000, 0}
    };

    int size = sizeof(lut) / sizeof(lut[0]);

    if (battery_mv >= lut[0].voltage) return 100;
    if (battery_mv <= lut[size - 1].voltage) return 0;

    for (int i = 0; i < size - 1; i++) {
        if (battery_mv <= lut[i].voltage && battery_mv > lut[i + 1].voltage) {
            float v1 = lut[i].voltage;
            float v2 = lut[i + 1].voltage;
            int p1 = lut[i].percent;
            int p2 = lut[i + 1].percent;

            return p1 + (battery_mv - v1) * (p2 - p1) / (v2 - v1);
        }
    }
    return 0;
}
void battery_monitor_process(void)
{
    static uint32_t count = 0;
    int err;
    uint16_t buf;
    struct adc_sequence sequence = {
        .buffer = &buf,
        .buffer_size = sizeof(buf),
    };

    if (!adc_is_ready_dt(&adc_channels[0]) || !adc_is_ready_dt(&adc_channels[1])) {
        printk("ADC controller not ready\n");
        return;
    }

    printk("=== Measurement #%u ===\n", count++);

    int32_t battery_raw = 0, battery_mv = 0;
    float battery_voltage = 0.0f;

    (void)adc_sequence_init_dt(&adc_channels[0], &sequence);
    err = adc_read_dt(&adc_channels[0], &sequence);
    if (err < 0) {
        printk("Battery voltage read error: %d\n", err);
    } else {
        battery_raw = (int32_t)buf;
        battery_mv = (battery_raw * ADC_REF_VOLTAGE_MV) / ADC_MAX_VALUE;
        battery_voltage = (float)battery_mv / 1000.0f * VOLTAGE_DIVIDER_RATIO;

        printk("Battery voltage (raw): %" PRId32 " mV\n", battery_mv);
        printk("Raw ADC value: %" PRId32 "\n", battery_raw);

        if (battery_mv >= VOLTAGE_FULL) {
            printk("Status: Fully charged (100%%)\n");
            percentage = 100;
        } else if (battery_mv >= VOLTAGE_LOW) {
            percentage = voltage_to_percent(battery_mv * VOLTAGE_DIVIDER_RATIO);
            printk("Status: Good (%" PRId32 "%%)\n", percentage);
        } else if (battery_mv >= CHARGING_THRESHOLD_MV) {
            percentage = voltage_to_percent(battery_mv * VOLTAGE_DIVIDER_RATIO);
            printk("Status: Low (%" PRId32 "%%)\n", percentage);
        } else {
            printk("Status: CRITICAL! (0%%)\n");
        }
    }

    int32_t charging_raw = 0, charging_mv = 0;

    (void)adc_sequence_init_dt(&adc_channels[1], &sequence);
    err = adc_read_dt(&adc_channels[1], &sequence);
    if (err < 0) {
        printk("Charging status read error: %d\n", err);
    } else {
        charging_raw = (int32_t)buf;
        charging_mv = (charging_raw * ADC_REF_VOLTAGE_MV) / ADC_MAX_VALUE;

        is_charging = charging_mv > CHARGING_THRESHOLD_MV;
        printk("Charging status: %s\n", is_charging ? "CHARGING" : "NOT CHARGING");
        printk("Charging pin raw: %" PRId32 " (%" PRId32 " mV)\n", charging_raw, charging_mv);

        if (is_charging) {
            printk("Charging current: %.2f A\n",
                   (float)(charging_mv - CHARGING_THRESHOLD_MV) / 1000.0f);
        }
    }

    printk("\n");
}

void battery_monitor_init(void)
{
    int err;
    for (size_t i = 0U; i < ARRAY_SIZE(adc_channels); i++) {
        if (!adc_is_ready_dt(&adc_channels[i])) {
            printk("ADC controller device %s not ready\n", adc_channels[i].dev->name);
            return;
        }

        err = adc_channel_setup_dt(&adc_channels[i]);
        if (err < 0) {
            printk("Could not setup channel #%d (%d)\n", i, err);
            return;
        }
    }
}
