#ifndef _FRAME_BLUETOOTHSCAN_H_
#define _FRAME_BLUETOOTHSCAN_H_

#include "frame_base.h"
#include "../epdgui/epdgui.h"

#define BT_MAX_DEVICES 10

class Frame_BluetoothScan : public Frame_Base {
   public:
    Frame_BluetoothScan();
    ~Frame_BluetoothScan();
    int init(epdgui_args_vector_t &args);
    int run();
    void DrawDevice(EPDGUI_Button *btn, const char *name);
    void UpdateDeviceList(int count);

   private:
    EPDGUI_Button *_key_device[BT_MAX_DEVICES + 1];  // devices + refresh
    M5EPD_Canvas  *_canvas_status;
    uint8_t _language;
    bool _scanning;
    bool _connecting;
    bool _auto_connect_pending;
};

#endif  //_FRAME_BLUETOOTHSCAN_H_
