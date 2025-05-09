#ifndef BATTERY_MONITOR_H
#define BATTERY_MONITOR_H

#include <stdint.h>
#include <stdbool.h>



#ifdef __cplusplus
extern "C" {
#endif

/* Battery voltage calculation constants */
#define VOLTAGE_DIVIDER_RATIO 2.0f
#define ADC_REF_VOLTAGE_MV 3600
#define ADC_MAX_VALUE 4095
#define CHARGING_THRESHOLD_MV 1500

/* Battery voltage thresholds */
#define VOLTAGE_FULL 2100
#define VOLTAGE_GOOD 1850
#define VOLTAGE_LOW 1700
#define VOLTAGE_CRITICAL 1650

extern bool is_charging;
extern int percentage;

/* Prototip funkcije za konverziju napona u procenat */
int voltage_to_percent(int battery_mv);

/* Funkcija za inicijalizaciju i merenje */
void battery_monitor_process(void);
void battery_monitor_init(void);



#ifdef __cplusplus
}
#endif

#endif // BATTERY_MONITOR_H
