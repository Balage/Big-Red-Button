/*
    MIT License
    
    Copyright (c) 2020, Balazs Vecsey, www.vbstudio.hu
    
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

    v1.2
    - Added F1-F12 key definitions
    - Added use-case example for GeForce Experience screenshot/recording
    
    v1.1
    - Fixed Schmitt trigger to be symmetrical
    - Removed Schmitt trigger from swithes
    
    v1.0
    - Initial version
*/

#include <VbsKeyboard.h>

//
// PIN CONFIGURATION
//
// The button must be connected to analog (A1 - A6) pins
// for the Schmitt trigger to work
#define IO_BUTTON A0

// The swithes can be connected to any I/O pin
#define IO_SWITCH_1 A2
#define IO_SWITCH_2 A1

// The LED must be on a PWM pin,
// on the Arduino Leonardo (MEGA32U4) these are: D3, D5, D6, D9, D10
#define IO_LIGHT 9

//
// TIMING
//
// This is how long the button must be held to register a long press (in milliseconds)
#define LONG_PRESS_TIME 750

// This is the time frame under which it registers as double click (in milliseconds)
#define DOUBLE_CLICK_TIME 300

// Speed of the LED brightness transition, bigger value -> faster transition
#define LED_CHANGE_SPEED 20.0f

//
// BUTTON BEHAVIOR
//
void buttonEvent(
    int selectedProgram,
    bool pressed, bool released, bool shortReleased,
    bool singleClick, bool doubleClick, bool longPressed, bool longDoubleClick)
{
    // All inputs are events and fired only once.
    // They are paired into 3 groups, and each program should only rely on one group.
    // Group #1: [ pressed, released ]
    // Group #2: [ shortReleased, longPressed ]
    // Group #3: [ singleClick, doubleClick, longPressed, longDoubleClick ]

    switch (selectedProgram)
    {
        case 0:
            if (pressed) Keyboard.HoldKey(KEY_ENTER);
            if (released) Keyboard.ReleaseKey();
            break;

        case 1:
            if (pressed) Keyboard.HoldKey(KEY_SPACE);
            if (released) Keyboard.ReleaseKey();
            break;

        case 2:
            if (singleClick) Keyboard.PressKey(KEY_F13);
            if (doubleClick) Keyboard.PressKey(KEY_F14);
            if (longPressed) Keyboard.PressKey(KEY_F15);
            if (longDoubleClick) Keyboard.PressKey(KEY_F16);
            break;

        case 3:
            if (shortReleased) Keyboard.PressKey(KEY_L, MOD_LEFT_GUI);
            if (longPressed) Keyboard.PressKeyPage1(KEY1_SYSTEM_SLEEP);
            break;
    }

    // Other potential use-cases.
    // Overwrite one of the existing programs with these:

    /* [GeForce Experience screenshot/recording]
        // Short press to take screenshot, long press to start/stop video recording
        if (shortReleased) Keyboard.PressKey(KEY_F1, MOD_LEFT_ALT);
        if (longPressed) Keyboard.PressKey(KEY_F9, MOD_LEFT_ALT);
    */
}


//
// If you only wish to change keys, add macros and tweak timing, scroll no further!
// After this point starts the "under the hood" section.
//


// Global variables
unsigned long _lastTimestamp;
unsigned long _longPressStarted = 0;
unsigned long _doubleClickStarted = 0;
bool _doubleClickInProgress = false;
bool _nextReleaseIsDoubleClick = false;
bool _longPressFired = false;
bool _buttonLastState = false;
float _ledBrightness = 0.0f;
int _lastProgram;

void setup()
{
    pinMode(IO_BUTTON, INPUT);
    pinMode(IO_SWITCH_1, INPUT);
    pinMode(IO_SWITCH_2, INPUT);
    pinMode(IO_LIGHT, OUTPUT);

    // Set default values
    digitalWrite(IO_LIGHT, HIGH);
    _lastProgram = readProgramSwitch();
    _lastTimestamp = millis();
}

bool schmittRead(int pin, bool lastState)
{
    // (Note that the value will be inverse because of the pull-up resistor)
    int value = analogRead(pin);

    // Schmitt trigger
    if (lastState && value >= 768) return false;
    if (!lastState && value < 256) return true;
    return lastState;
}

int readProgramSwitch()
{
    return (
        (digitalRead(IO_SWITCH_1) ? 0 : 1) |
        (digitalRead(IO_SWITCH_2) ? 0 : 2));
}

void loop()
{
    unsigned long timestamp = millis();
    unsigned long delta_i = timestamp - _lastTimestamp;

    if (delta_i != 0)
    {
        _lastTimestamp = timestamp;
        float delta = delta_i / 1000.0f;

        int program = getAndUpdateProgram();
        updateButton(program);
        updateLED(delta);
    }
}

int getAndUpdateProgram()
{
    int program = readProgramSwitch();
    if (program != _lastProgram)
    {
        // Avoid any keys getting stuck while changing program
        Keyboard.ReleaseKey();
        _lastProgram = program;
    }
    return program;
}

void updateButton(int program)
{
    unsigned long timestamp = millis();
    bool buttonState = schmittRead(IO_BUTTON, _buttonLastState);

    // Normal button press and release, both fired once
    bool buttonPressed = buttonState && buttonState != _buttonLastState;
    bool buttonReleased = !buttonState && buttonState != _buttonLastState;

    // Button holding started
    if (buttonPressed)
    {
        _longPressStarted = timestamp;
        _longPressFired = false;
    }

    // Short release event, only if released before long press
    bool buttonShortReleased = buttonReleased && !_longPressFired;

    // Long press event, fires once while pressed if button is held for some time
    // (release will not trigger short release event if long press has fired)
    bool buttonLongPressed = !_longPressFired && buttonState && _longPressStarted + LONG_PRESS_TIME < millis();
    bool buttonLongDoubleClick = false;

    if (buttonLongPressed)
    {
        _longPressFired = true;
        _doubleClickInProgress = false;

        if (_nextReleaseIsDoubleClick)
        {
            buttonLongPressed = false;
            buttonLongDoubleClick = true;
        }
    }

    // Single/double click
    bool buttonSingleClick = false;
    bool buttonDoubleClick = false;

    unsigned long doubleClickDelta = timestamp - _doubleClickStarted;
    bool withinDoubleClickTime = doubleClickDelta <= DOUBLE_CLICK_TIME;

    if (_doubleClickInProgress)
    {
        if (withinDoubleClickTime)
        {
            if (buttonPressed)
            {
                // mark double click
                _nextReleaseIsDoubleClick = true;
            }
        }
        else
        {
            if (!buttonState && !_nextReleaseIsDoubleClick)
            {
                // single click
                buttonSingleClick = true;
                _doubleClickInProgress = false;
            }
        }
    }
    else
    {
        if (buttonPressed)
        {
            _doubleClickStarted = timestamp;
            _doubleClickInProgress = true;
            _nextReleaseIsDoubleClick = false;
        }
    }

    if (_doubleClickInProgress && buttonReleased && _nextReleaseIsDoubleClick)
    {
        // double click
        buttonDoubleClick = true;
        _doubleClickInProgress = false;
    }

    buttonEvent(
        program,
        buttonPressed, buttonReleased, buttonShortReleased,
        buttonSingleClick, buttonDoubleClick, buttonLongPressed, buttonLongDoubleClick
    );

    _buttonLastState = buttonState;
}

void updateLED(float delta)
{
    bool scrollLedState = Keyboard.GetLedState(KB_LED_SCROLL);
    float ledNewBrightness = scrollLedState || _buttonLastState ? 255.0f : 0.0f;

    float chageSpeed = delta * LED_CHANGE_SPEED;
    _ledBrightness = (chageSpeed * ledNewBrightness) + ((1.0f - chageSpeed) * _ledBrightness);

    analogWrite(IO_LIGHT, 255 - (int)_ledBrightness);
}
