#ifndef BLE_NUS_H
#define BLE_NUS_H

#include <zephyr/kernel.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/hci.h>
#include <bluetooth/services/nus.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Callback for when Bluetooth is initialized */
void bt_ready(int err);

/* Connection callback functions */
void connected(struct bt_conn *conn, uint8_t err);
void disconnected(struct bt_conn *conn, uint8_t reason);

/* Initialize BLE NUS service */
int ble_nus_init(void);

/* Send heart rate and steps data over BLE */
int ble_nus_send_data(int heart_rate, uint32_t steps);

#ifdef __cplusplus
}
#endif

#endif /* BLE_NUS_H */