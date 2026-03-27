#include "frame_bluetoothscan.h"
#include "../systeminit.h"
#include <BLEClient.h>
#include <ble.h>

static int s_bt_connect_index = -1;
static bool s_bt_refresh_requested = false;

void key_btdevice_cb(epdgui_args_vector_t &args) {
    EPDGUI_Button *btn = (EPDGUI_Button *)(args[0]);
    int *is_run        = (int *)(args[1]);

    if (btn->GetCustomString() == "_$bt_refresh$_") {
        s_bt_refresh_requested = true;
    } else {
        // args[2] holds the device index
        s_bt_connect_index = (int)(intptr_t)(args[2]);
        *is_run            = 0;  // not used to exit frame, just trigger connect
        *is_run            = 1;
    }
}

Frame_BluetoothScan::Frame_BluetoothScan(void) {
    _frame_name = "Frame_BluetoothScan";
    _language   = GetLanguage();
    _scanning   = false;
    _connecting = false;

    for (int i = 0; i < BT_MAX_DEVICES + 1; i++) {
        _key_device[i] = new EPDGUI_Button(4, 100 + i * 60, 952, 61);
        _key_device[i]->SetHide(true);
        _key_device[i]->CanvasNormal()->setTextSize(26);
        _key_device[i]->CanvasNormal()->setTextDatum(CL_DATUM);
        _key_device[i]->CanvasNormal()->setTextColor(15);
        _key_device[i]->AddArgs(EPDGUI_Button::EVENT_RELEASED, 0, _key_device[i]);
        _key_device[i]->AddArgs(EPDGUI_Button::EVENT_RELEASED, 1, (void *)(&_is_run));
        _key_device[i]->AddArgs(EPDGUI_Button::EVENT_RELEASED, 2, (void *)(intptr_t)i);
        _key_device[i]->Bind(EPDGUI_Button::EVENT_RELEASED, key_btdevice_cb);
    }

    _canvas_status = new M5EPD_Canvas(&M5.EPD);
    _canvas_status->createCanvas(952, 60);
    _canvas_status->setTextSize(26);
    _canvas_status->setTextDatum(CC_DATUM);
    _canvas_status->setTextColor(15);

    if (_language == LANGUAGE_JA) {
        exitbtn("ホーム");
        _canvas_title->drawString("BTキーボード", 480, 34);
    } else if (_language == LANGUAGE_ZH) {
        exitbtn("主页");
        _canvas_title->drawString("蓝牙键盘", 480, 34);
    } else {
        exitbtn("Home");
        _canvas_title->drawString("BT Keyboard", 480, 34);
    }

    _key_exit->AddArgs(EPDGUI_Button::EVENT_RELEASED, 0, (void *)(&_is_run));
    _key_exit->Bind(EPDGUI_Button::EVENT_RELEASED, &Frame_Base::exit_cb);
}

Frame_BluetoothScan::~Frame_BluetoothScan(void) {
    for (int i = 0; i < BT_MAX_DEVICES + 1; i++) {
        delete _key_device[i];
    }
    delete _canvas_status;
}

void Frame_BluetoothScan::DrawDevice(EPDGUI_Button *btn, const char *name) {
    String label = String(name);
    if (label.length() > 22) {
        label = label.substring(0, 22) + "...";
    }
    btn->SetHide(false);
    btn->CanvasNormal()->fillCanvas(0);
    btn->CanvasNormal()->drawRect(0, 0, 952, 61, 15);
    btn->CanvasNormal()->drawString(label, 15, 35);
    *(btn->CanvasPressed()) = *(btn->CanvasNormal());
    btn->CanvasPressed()->ReverseColor();
}

void Frame_BluetoothScan::UpdateDeviceList(int count) {
    _canvas_status->fillCanvas(0);
    _canvas_status->pushCanvas(4, 72, UPDATE_MODE_NONE);

    for (int i = 0; i < BT_MAX_DEVICES + 1; i++) {
        _key_device[i]->SetHide(true);
    }

    const char *savedAddr = BLEClient_saved_device_address();
    int n = count > BT_MAX_DEVICES ? BT_MAX_DEVICES : count;
    for (int i = 0; i < n; i++) {
        const BleDeviceInfo *dev = BLEClient_device(i);
        if (dev == nullptr) break;
        _key_device[i]->SetCustomString(String(dev->address));
        String label = dev->name[0] ? dev->name : dev->address;
        if (savedAddr[0] && strcmp(dev->address, savedAddr) == 0) label += " *";
        DrawDevice(_key_device[i], label.c_str());
        _key_device[i]->Draw(UPDATE_MODE_A2);
    }

    // Refresh button at end
    int ri = n;
    _key_device[ri]->SetCustomString("_$bt_refresh$_");
    _key_device[ri]->SetHide(false);
    _key_device[ri]->CanvasNormal()->fillCanvas(0);
    _key_device[ri]->CanvasNormal()->drawRect(0, 0, 952, 61, 15);
    _key_device[ri]->CanvasNormal()->pushImage(15, 14, 32, 32,
                                               ImageResource_item_icon_refresh_32x32);
    if (_language == LANGUAGE_ZH || _language == LANGUAGE_JA) {
        _key_device[ri]->CanvasNormal()->drawString("刷新", 58, 35);
    } else {
        _key_device[ri]->CanvasNormal()->drawString("Refresh", 58, 35);
    }
    *((_key_device[ri]->CanvasPressed())) = *(_key_device[ri]->CanvasNormal());
    _key_device[ri]->CanvasPressed()->ReverseColor();
    _key_device[ri]->Draw(UPDATE_MODE_A2);

    M5.EPD.UpdateFull(UPDATE_MODE_GL16);

    if (count == 0) {
        _canvas_status->fillCanvas(0);
        if (_language == LANGUAGE_ZH || _language == LANGUAGE_JA) {
            _canvas_status->drawString("未找到设备", 266, 30);
        } else {
            _canvas_status->drawString("No devices found", 266, 30);
        }
        _canvas_status->pushCanvas(4, 72, UPDATE_MODE_GL16);
    }
}

int Frame_BluetoothScan::init(epdgui_args_vector_t &args) {
    _is_run                = 1;
    _scanning              = false;
    _connecting            = false;
    _auto_connect_pending  = false;
    s_bt_connect_index     = -1;
    s_bt_refresh_requested = false;

    M5.EPD.Clear(true);
    _canvas_title->pushCanvas(0, 8, UPDATE_MODE_NONE);

    for (int i = 0; i < BT_MAX_DEVICES + 1; i++) {
        _key_device[i]->SetHide(true);
        EPDGUI_AddObject(_key_device[i]);
    }
    EPDGUI_AddObject(_key_exit);

    _canvas_status->fillCanvas(0);

    if (ble_is_connected()) {
        // Already connected — show status, no scan needed
        const char *devName = BLEClient_saved_device_name();
        String msg = (_language == LANGUAGE_ZH || _language == LANGUAGE_JA)
            ? String("已连接: ") + devName
            : String("Connected: ") + devName;
        _canvas_status->drawString(msg, 15, 30);
        _canvas_status->setTextDatum(CL_DATUM);
        _canvas_status->pushCanvas(4, 72, UPDATE_MODE_NONE);
        _canvas_status->setTextDatum(CC_DATUM);
        M5.EPD.UpdateFull(UPDATE_MODE_GC16);
    } else if (isBLEPaired()) {
        // Paired but not connected — try direct reconnect first, then scan as fallback
        const char *devName = BLEClient_saved_device_name();
        String msg = (_language == LANGUAGE_ZH || _language == LANGUAGE_JA)
            ? String("重连中: ") + (devName[0] ? devName : GetBLEAddress())
            : String("Reconnecting: ") + (devName[0] ? devName : GetBLEAddress());
        _canvas_status->drawString(msg, 15, 30);
        _canvas_status->setTextDatum(CL_DATUM);
        _canvas_status->pushCanvas(4, 72, UPDATE_MODE_NONE);
        _canvas_status->setTextDatum(CC_DATUM);
        M5.EPD.UpdateFull(UPDATE_MODE_GC16);
        BLEClient_request_connect_saved();
        _auto_connect_pending = true;
        g_ble_scan_result = -1;
        BLEClient_request_scan();
        _scanning = true;
    } else {
        // Not paired — show "Scanning..." and kick off scan
        if (_language == LANGUAGE_ZH || _language == LANGUAGE_JA) {
            _canvas_status->drawString("扫描中...", 266, 30);
        } else {
            _canvas_status->drawString("Scanning...", 266, 30);
        }
        _canvas_status->pushCanvas(4, 72, UPDATE_MODE_NONE);
        M5.EPD.UpdateFull(UPDATE_MODE_GC16);
        g_ble_scan_result = -1;
        BLEClient_request_scan();
        _scanning = true;
    }

    return 3;
}

int Frame_BluetoothScan::run() {
    Frame_Base::run();

    if (s_bt_refresh_requested) {
        s_bt_refresh_requested = false;
        for (int i = 0; i < BT_MAX_DEVICES + 1; i++) {
            _key_device[i]->SetHide(true);
        }
        _canvas_status->fillCanvas(0);
        if (_language == LANGUAGE_ZH || _language == LANGUAGE_JA) {
            _canvas_status->drawString("扫描中...", 266, 30);
        } else {
            _canvas_status->drawString("Scanning...", 266, 30);
        }
        _canvas_status->pushCanvas(4, 72, UPDATE_MODE_GL16);
        g_ble_scan_result = -1;
        BLEClient_request_scan();
        _scanning = true;
    }

    if (s_bt_connect_index >= 0) {
        int idx        = s_bt_connect_index;
        s_bt_connect_index = -1;
        _connecting    = true;

        _canvas_status->fillCanvas(0);
        if (_language == LANGUAGE_ZH || _language == LANGUAGE_JA) {
            _canvas_status->drawString("连接中...", 266, 30);
        } else {
            _canvas_status->drawString("Connecting...", 266, 30);
        }
        _canvas_status->pushCanvas(4, 72, UPDATE_MODE_GL16);
        BLEClient_request_connect(idx);
    }

    if ((_connecting || _auto_connect_pending) && ble_is_connected()) {
        _connecting           = false;
        _auto_connect_pending = false;
        _canvas_status->fillCanvas(0);
        _canvas_status->setTextDatum(CL_DATUM);
        const char *devName = BLEClient_saved_device_name();
        String msg = (_language == LANGUAGE_ZH || _language == LANGUAGE_JA)
            ? String("已连接: ") + devName
            : String("Connected: ") + devName;
        _canvas_status->drawString(msg, 15, 30);
        _canvas_status->pushCanvas(4, 72, UPDATE_MODE_GL16);
        _canvas_status->setTextDatum(CC_DATUM);
    }

    if (_scanning && g_ble_scan_result >= 0) {
        _scanning = false;
        int count = g_ble_scan_result;
        g_ble_scan_result = -1;

        if (_auto_connect_pending && !ble_is_connected()) {
            // Try to auto-connect to the paired device from scan results
            const char *savedAddr = BLEClient_saved_device_address();
            const char *savedName = BLEClient_saved_device_name();
            int matchIdx = -1;
            for (int i = 0; i < count && matchIdx < 0; i++) {
                const BleDeviceInfo *dev = BLEClient_device(i);
                if (!dev) break;
                if (savedAddr[0] && strcmp(dev->address, savedAddr) == 0) matchIdx = i;
                else if (savedName[0] && strcmp(dev->name, savedName) == 0) matchIdx = i;
            }
            _auto_connect_pending = false;
            if (matchIdx >= 0) {
                _connecting = true;
                _canvas_status->fillCanvas(0);
                if (_language == LANGUAGE_ZH || _language == LANGUAGE_JA) {
                    _canvas_status->drawString("连接中...", 266, 30);
                } else {
                    _canvas_status->drawString("Connecting...", 266, 30);
                }
                _canvas_status->pushCanvas(4, 72, UPDATE_MODE_GL16);
                BLEClient_request_connect(matchIdx);
                return 1;
            }
        }
        _auto_connect_pending = false;
        UpdateDeviceList(count);
    }

    return 1;
}
