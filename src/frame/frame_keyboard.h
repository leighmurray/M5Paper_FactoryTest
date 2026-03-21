#ifndef _FRAME_KEYBOARD_H_
#define _FRAME_KEYBOARD_H_

#include "frame_base.h"
#include "../epdgui/epdgui.h"

class Frame_Keyboard : public Frame_Base {
   public:
    Frame_Keyboard(bool isHorizontal = false);
    ~Frame_Keyboard();
    int run();
    int init(epdgui_args_vector_t &args);

   private:
    void UpdateBLEKeyboardState(bool bleConnected);

    EPDGUI_Textbox *inputbox;
    EPDGUI_Keyboard *keyboard;
    EPDGUI_Button *key_textclear;
    EPDGUI_Button *key_textsize_plus;
    EPDGUI_Button *key_textsize_reset;
    EPDGUI_Button *key_textsize_minus;
    bool _ble_was_connected;
};

#endif  //_FRAME_KEYBOARD_H_