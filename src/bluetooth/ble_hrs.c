#include "ble_hrs.h"
#include <zephyr/sys/printk.h>

/* Callback-ovi za BLE vezu */
static struct bt_conn_cb conn_callbacks = {
    .connected    = ble_hrs_on_connect,
    .disconnected = ble_hrs_on_disconnect,
};

int ble_hrs_init(void) 
{
    int err;
    
    /* Inicijalizacija Bluetooth stack-a */
    err = bt_enable(NULL);
    if (err) {
        printk("Bluetooth init failed (err %d)\n", err);
        return err;
    }

    /* Registruj callback-ove za vezu */
    bt_conn_cb_register(&conn_callbacks);

    /* Startuj oglašavanje (advertising) */
    err = bt_le_adv_start(BT_LE_ADV_CONN_NAME, NULL, 0, NULL, 0);
    if (err) {
        printk("Advertising failed to start (err %d)\n", err);
        return err;
    }

    printk("BLE HRS initialized\n");
    return 0;
}

int ble_hrs_update(uint8_t heart_rate) 
{
    /* Pošalji nove podatke preko HRS notifikacija */
    int err = bt_hrs_notify(heart_rate);
    if (err) {
        printk("HRS notify failed (err %d)\n", err);
        return err;
    }
    
    printk("HRS updated: %d bpm\n", heart_rate);
    return 0;
}

/* Implementacije callback-a */
void ble_hrs_on_connect(struct bt_conn *conn, uint8_t err) 
{
    if (err) {
        printk("Connection failed (err %u)\n", err);
    } else {
        printk("Connected\n");
    }
}

void ble_hrs_on_disconnect(struct bt_conn *conn, uint8_t reason) 
{
    printk("Disconnected (reason %u)\n", reason);
}