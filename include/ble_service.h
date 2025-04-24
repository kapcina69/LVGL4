#ifndef BLE_SERVICE_H
#define BLE_SERVICE_H

#include <zephyr/types.h>

void ble_init(void);
void ble_send_data(int heart_rate, uint32_t steps);

#endif // BLE_SERVICE_H
