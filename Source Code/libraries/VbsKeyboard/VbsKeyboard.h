/*
    VbsKeyboard.h
    
    Copyright (c) 2020, Balazs Vecsey, www.vbstudio.hu
    
    This is a merged, modified and extended version of the Keyboard.h
    and HID.h from the official Arduino library, version 1.8.12.
    Original copyright is as follows:
    
    Copyright (c) 2015, Arduino LLC
    Original code (pre-library): Copyright (c) 2011, Peter Barrett
    
    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.
    
    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.
*/

#ifndef VBS_KEYBOARD_h
#define VBS_KEYBOARD_h


#include <stdint.h>
#include <Arduino.h>
#include "PluggableUSB.h"


#if defined(USBCON)


// HID 'Driver'
// ----------------------------------------------
#define HID_GET_REPORT        0x01
#define HID_GET_IDLE          0x02
#define HID_GET_PROTOCOL      0x03
#define HID_SET_REPORT        0x09
#define HID_SET_IDLE          0x0A
#define HID_SET_PROTOCOL      0x0B

#define HID_HID_DESCRIPTOR_TYPE         0x21
#define HID_REPORT_DESCRIPTOR_TYPE      0x22
#define HID_PHYSICAL_DESCRIPTOR_TYPE    0x23

// HID subclass HID1.11 Page 8 4.2 Subclass
#define HID_SUBCLASS_NONE 0
#define HID_SUBCLASS_BOOT_INTERFACE 1

// HID Keyboard/Mouse bios compatible protocols HID1.11 Page 9 4.3 Protocols
#define HID_PROTOCOL_NONE 0
#define HID_PROTOCOL_KEYBOARD 1
#define HID_PROTOCOL_MOUSE 2

// Normal or bios protocol (Keyboard/Mouse) HID1.11 Page 54 7.2.5 Get_Protocol Request
// "protocol" variable is used for this purpose.
#define HID_BOOT_PROTOCOL	0
#define HID_REPORT_PROTOCOL	1

// HID Request Type HID1.11 Page 51 7.2.1 Get_Report Request
#define HID_REPORT_TYPE_INPUT   1
#define HID_REPORT_TYPE_OUTPUT  2
#define HID_REPORT_TYPE_FEATURE 3

typedef struct
{
    uint8_t len;      // 9
    uint8_t dtype;    // 0x21
    uint8_t addr;
    uint8_t versionL; // 0x101
    uint8_t versionH; // 0x101
    uint8_t country;
    uint8_t desctype; // 0x22 report
    uint8_t descLenL;
    uint8_t descLenH;
} HIDDescDescriptor;

typedef struct 
{
    InterfaceDescriptor hid;
    HIDDescDescriptor desc;
    EndpointDescriptor in;
} HIDDescriptor;

class HIDSubDescriptor {
public:
    HIDSubDescriptor *next = NULL;
    HIDSubDescriptor(const void *d, const uint16_t l) : data(d), length(l) { }

    const void* data;
    const uint16_t length;
};

#define D_HIDREPORT(length) { 9, 0x21, 0x01, 0x01, 0, 1, 0x22, lowByte(length), highByte(length) }


// Keyboard
// ----------------------------------------------
// LEDs
#define KB_LED_NUM_LOCK     0x01
#define KB_LED_CAPS_LOCK    0x02
#define KB_LED_SCROLL_LOCK  0x04

// Modifiers (page 7)
#define MOD_NONE         0x00
#define MOD_LEFT_CTRL    0x01
#define MOD_LEFT_SHIFT   0x02
#define MOD_LEFT_ALT     0x04
#define MOD_LEFT_GUI     0x08
#define MOD_RIGHT_CTRL   0x10
#define MOD_RIGHT_SHIFT  0x20
#define MOD_RIGHT_ALT    0x40
#define MOD_RIGHT_GUI    0x80

// Modifier keys (page 7)
#define KEY_LEFT_CTRL    0x80
#define KEY_LEFT_SHIFT   0x81
#define KEY_LEFT_ALT     0x82
#define KEY_LEFT_GUI     0x83
#define KEY_RIGHT_CTRL   0x84
#define KEY_RIGHT_SHIFT  0x85
#define KEY_RIGHT_ALT    0x86
#define KEY_RIGHT_GUI    0x87

// Character keys (page 7) (some of these are language specific!)
#define KEY_SPACE        0x2c
#define KEY_A            0x04
#define KEY_B            0x05
#define KEY_C            0x06
#define KEY_D            0x07
#define KEY_E            0x08
#define KEY_F            0x09
#define KEY_G            0x0a
#define KEY_H            0x0b
#define KEY_I            0x0c
#define KEY_J            0x0d
#define KEY_K            0x0e
#define KEY_L            0x0f
#define KEY_M            0x10
#define KEY_N            0x11
#define KEY_O            0x12
#define KEY_P            0x13
#define KEY_Q            0x14
#define KEY_R            0x15
#define KEY_S            0x16
#define KEY_T            0x17
#define KEY_U            0x18
#define KEY_V            0x19
#define KEY_W            0x1a
#define KEY_X            0x1b
#define KEY_Y            0x1c
#define KEY_Z            0x1d

// Function keys (page 7)
#define KEY_ENTER        0x28
#define KEY_ESC          0x29
#define KEY_BACKSPACE    0x2a
#define KEY_TAB          0x2b
#define KEY_PRINT_SCREEN 0x46
#define KEY_F1           0x3A
#define KEY_F2           0x3B
#define KEY_F3           0x3C
#define KEY_F4           0x3D
#define KEY_F5           0x3E
#define KEY_F6           0x3F
#define KEY_F7           0x40
#define KEY_F8           0x41
#define KEY_F9           0x42
#define KEY_F10          0x43
#define KEY_F11          0x44
#define KEY_F12          0x45
#define KEY_F13          0x68
#define KEY_F14          0x69
#define KEY_F15          0x6a
#define KEY_F16          0x6b
#define KEY_F17          0x6c
#define KEY_F18          0x6d
#define KEY_F19          0x6e
#define KEY_F20          0x6f
#define KEY_F21          0x70
#define KEY_F22          0x71
#define KEY_F23          0x72
#define KEY_F24          0x73

// Page 1 keys
#define KEY1_SYSTEM_SLEEP 0x82


// Low level key report: up to 6 keys and shift, ctrl etc at once
typedef struct
{
    uint8_t modifiers;
    uint8_t reserved;
    uint8_t keys[6];
} KeyReportPage7;

typedef union {
    // Every usable Consumer key possible, up to 4 keys presses possible
    uint8_t whole8[0];
    uint16_t whole16[0];
    uint32_t whole32[0];
    uint16_t keys[4];
} KeyReportPage1;


class VbsKeyboard : public PluggableUSBModule
{
public:
    VbsKeyboard(void);
    
    // Page 0x01 (Generic Desktop)
    void PressKeyPage1(uint16_t key);
    
    // Page 0x07 (Keyboard/Keypad)
    void PressKey(uint8_t key, uint8_t modifier = MOD_NONE);
    void HoldKey(uint8_t key, uint8_t modifier = MOD_NONE);
    inline void ReleaseKey() { HoldKey(0, 0); }
    
    bool GetLedState(uint8_t mask) const;
    
protected:
    // Implementation of the PluggableUSBModule
    int getInterface(uint8_t* interfaceCount);
    int getDescriptor(USBSetup& setup);
    bool setup(USBSetup& setup);
    uint8_t getShortName(char* name);
    
private:
    // HID
    uint8_t _epType[1];
    HIDSubDescriptor* _rootNode;
    uint16_t _descriptorSize;
    uint8_t _protocol;
    uint8_t _idle;
    
    // Keyboard
    KeyReportPage1 _keyReportPage1;
    KeyReportPage7 _keyReportPage7;
    uint8_t _ledsState;
    
    void SendReport(uint8_t id, void* data, int len);
    
    void AppendDescriptor(HIDSubDescriptor* node);
};

// Singleton instance
extern VbsKeyboard Keyboard;

#endif // USBCON
#endif

