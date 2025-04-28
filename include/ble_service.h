#ifndef BLE_SERVICE_H
#define BLE_SERVICE_H

#include <zephyr/types.h>

void ble_init(void (*callback)(const char *));
void ble_send_data(int heart_rate, uint32_t steps);
void ble_send_string(const char *message);
#endif // BLE_SERVICE_H
