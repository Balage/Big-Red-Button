/*
    MIT License
    
    Copyright (c) 2021, Balazs Vecsey, www.vbstudio.hu
    
    Permission is hereby granted, free of charge, to any person obtaining a copy of
    this software and associated documentation files (the "Software"), to deal in the
    Software without restriction, including without limitation the rights to use,
    copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the
    Software, and to permit persons to whom the Software is furnished to do so,
    subject to the following conditions:
    
    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.
       
    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
    FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
    COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
    AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
    WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/


//
// PIN CONFIGURATION
//
// The button must be connected to an analog (A1 - A6) pin for the Schmitt trigger to work
#define IO_BUTTON A0

// The swithes can be connected to any I/O pin
#define IO_SWITCH_1 A2
#define IO_SWITCH_2 A1

// The LED must be on a PWM pin, on the Arduino Leonardo (MEGA32U4) these are: D3, D5, D6, D9, D10
#define IO_LIGHT 9

#include <VbsBigRedButton.h>
VbsBigRedButton BigRedButton(IO_BUTTON, IO_LIGHT, IO_SWITCH_1, IO_SWITCH_2);


//
// TIMING AND LED BEHAVIOR
//
void setup()
{
    // This is how long the button must be held to register a long press (in milliseconds).
    BigRedButton.SetLongPressTime(700);
    
    // This is the time frame under which it registers as double click (in milliseconds).
    BigRedButton.SetDoubleClickTime(400);
    
    // Speed of the LED brightness transition, bigger value -> faster transition.
    BigRedButton.SetLightChangeSpeed(25.0f);
    
    // LED maximum brightness (float value between 0.0f and 1.0f).
    BigRedButton.SetLightMaxBrightness(1.0f);
    
    // Light pulse frequency and size (% between 0.0f and 1.0f), when LED is kept lit.
    // Set frequency to 0.0f to disable.
    BigRedButton.SetLightPulse(0.5f, 0.75f);
}


//
// BUTTON BEHAVIOR
//
void loop()
{
    // Keep LED lit when Scroll Lock key is active
    BigRedButton.KeepLightLit(Keyboard.GetLedState(KB_LED_SCROLL_LOCK));

    switch (BigRedButton.GetProgramIndex())
    {
        case 0:
        {
            auto event = BigRedButton.PollSingleButtonEvent();
            
            if (event.Press) Keyboard.HoldKey(KEY_ENTER);
            if (event.Release) Keyboard.ReleaseKey();
            break;
        }
        case 1:
        {
            auto event = BigRedButton.PollSingleButtonEvent();
            
            if (event.Press) Keyboard.HoldKey(KEY_SPACE);
            if (event.Release) Keyboard.ReleaseKey();
            break;
        }
        case 2:
        {
            auto event = BigRedButton.PollQuadButtonEvent();
            
            if (event.SingleClick) Keyboard.PressKey(KEY_F13);
            if (event.DoubleClick) Keyboard.PressKey(KEY_F14);
            if (event.LongPress) Keyboard.PressKey(KEY_F15);
            if (event.LongPressDoubleClick) Keyboard.PressKey(KEY_F16);
            break;
        }
        case 3:
        {
            auto event = BigRedButton.PollDualButtonEvent();
            
            if (event.Click) Keyboard.PressKey(KEY_L, MOD_LEFT_GUI);
            if (event.LongPress) Keyboard.PressKeyPage1(KEY1_SYSTEM_SLEEP);
            break;
        }
    }
}
