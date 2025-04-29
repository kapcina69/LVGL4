#include "ble_nus.h"
#include <zephyr/device.h>
#include <zephyr/sys/printk.h>
#include <string.h>
#include <stdio.h>
#include <bluetooth/services/nus.h>
#include <zephyr/bluetooth/hci.h>
#include "lcd_display.h"


/* Connection callbacks structure */
BT_CONN_CB_DEFINE(conn_callbacks) = {
    .connected = connected,
    .disconnected = disconnected,
};

void bt_ready(int err)
{
    if (err) {
        printk("Bluetooth init failed (err %d)\n", err);
        return;
    }

    printk("Bluetooth initialized\n");

    err = bt_le_adv_start(BT_LE_ADV_CONN_NAME, NULL, 0, NULL, 0);
    if (err) {
        printk("Advertising failed to start (err %d)\n", err);
    } else {
        printk("Advertising started\n");
    }
}

void connected(struct bt_conn *conn, uint8_t err)
{
    if (err == 0) {
        printk("Connected\n");
        lv_obj_clear_flag(bt_label, LV_OBJ_FLAG_HIDDEN);  // Prikazi BT simbol
    } else {
        printk("Connection failed (err %u)\n", err);
    }
}

void disconnected(struct bt_conn *conn, uint8_t reason)
{
    printk("Disconnected (reason %u)\n", reason);
    lv_obj_add_flag(bt_label, LV_OBJ_FLAG_HIDDEN);  // Sakrij BT simbol
}


int ble_nus_init(void)
{
    int err = bt_enable(bt_ready);
    if (err) {
        printk("Bluetooth enable failed (err %d)\n", err);
        return err;
    }

    err = bt_nus_init(NULL);
    if (err) {
        printk("Failed to initialize NUS (err %d)\n", err);
    }

    return err;
}

int ble_nus_send_data(int heart_rate, uint32_t steps)
{
    char buffer[32];
    int len = snprintf(buffer, sizeof(buffer), "HR:%d,STEPS:%u\n", heart_rate, steps);
    
    if (len < 0) {
        printk("Failed to format BLE data\n");
        return -1;
    }

    return bt_nus_send(NULL, buffer, len);
    
}

