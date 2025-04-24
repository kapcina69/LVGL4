#ifndef BLE_HRS_H
#define BLE_HRS_H

#include <zephyr/kernel.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/bluetooth/services/hrs.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Inicijalizacija BLE HRS */
int ble_hrs_init(void);

/* Ažuriraj i pošalji nove podatke o otkucaju srca */
int ble_hrs_update(uint8_t heart_rate);

/* Callback za BLE vezu */
void ble_hrs_on_connect(struct bt_conn *conn, uint8_t err);
void ble_hrs_on_disconnect(struct bt_conn *conn, uint8_t reason);

#ifdef __cplusplus
}
#endif

#endif /* BLE_HRS_H */