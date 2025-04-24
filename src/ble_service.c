#include <zephyr/kernel.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/sys/printk.h>
#include <string.h>
#include <stdio.h>

#include "ble_service.h"

static uint8_t notify_enabled;

static char ble_buffer[50];

// Custom 128-bit UUIDs
#define BT_UUID_CUSTOM_SERVICE_VAL \
    BT_UUID_128_ENCODE(0x12345678, 0x1234, 0x5678, 0x9abc, 0xdef012345678)
#define BT_UUID_CUSTOM_CHAR_VAL \
    BT_UUID_128_ENCODE(0x87654321, 0x4321, 0xba98, 0x7654, 0x3210fedcba98)

static struct bt_uuid_128 custom_service_uuid = BT_UUID_INIT_128(BT_UUID_CUSTOM_SERVICE_VAL);
static struct bt_uuid_128 custom_char_uuid    = BT_UUID_INIT_128(BT_UUID_CUSTOM_CHAR_VAL);

static void ccc_cfg_changed(const struct bt_gatt_attr *attr, uint16_t value)
{
    notify_enabled = (value == BT_GATT_CCC_NOTIFY);
    printk("BLE Notify %s\n", notify_enabled ? "enabled" : "disabled");
}

BT_GATT_SERVICE_DEFINE(custom_svc,
    BT_GATT_PRIMARY_SERVICE(&custom_service_uuid),
    BT_GATT_CHARACTERISTIC(&custom_char_uuid.uuid,
                           BT_GATT_CHRC_NOTIFY,
                           BT_GATT_PERM_NONE,
                           NULL, NULL, ble_buffer),
    BT_GATT_CCC(ccc_cfg_changed, BT_GATT_PERM_READ | BT_GATT_PERM_WRITE)
);

void ble_init(void)
{
    int err = bt_enable(NULL);
    if (err) {
        printk("Bluetooth init failed (err %d)\n", err);
        return;
    }

    printk("Bluetooth initialized\n");

    err = bt_le_adv_start(BT_LE_ADV_CONN_NAME, NULL, 0, NULL, 0);
    if (err) {
        printk("Advertising failed to start (err %d)\n", err);
        return;
    }

    printk("Advertising started\n");
}

void ble_send_data(int heart_rate, uint32_t steps)
{
    if (!notify_enabled) return;

    snprintf(ble_buffer, sizeof(ble_buffer), "HR:%d Steps:%u", heart_rate, steps);

    bt_gatt_notify(NULL, &custom_svc.attrs[1], ble_buffer, strlen(ble_buffer));
    printk("BLE Sent: %s\n", ble_buffer);
}
