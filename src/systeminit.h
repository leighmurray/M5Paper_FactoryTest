#ifndef __SYSTEMINIT_H
#define __SYSTEMINIT_H

#include <M5EPD.h>
#include <Arduino.h>

void SysInit_Start(void);
void SysInit_Loading(void *pvParameters);
void SysInit_UpdateInfo(String info);

// BLE keyboard globals
extern volatile int g_ble_scan_result;  // -1=idle/scanning, >=0=device count
extern String       g_ble_chars_pending; // accumulates keypresses for Frame_Keyboard

#endif  //__SYSTEMINIT_H