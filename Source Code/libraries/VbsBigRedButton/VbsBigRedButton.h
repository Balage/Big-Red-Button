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

#ifndef VBS_BIG_RED_BUTTON_h
#define VBS_BIG_RED_BUTTON_h

#include <Arduino.h>
#include <VbsKeyboard.h>

struct VbsSingleButtonEvent
{
    bool Press;
    bool Release;
};

struct VbsDualButtonEvent
{
    bool Click;
    bool LongPress;
};

struct VbsQuadButtonEvent
{
    bool SingleClick;
    bool DoubleClick;
    bool LongPress;
    bool LongPressDoubleClick;
};

class VbsBigRedButton
{
private:
    enum LightOverride
    {
        LIGHT_FREE = 0,
        LIGHT_ON = 1,
        LIGHT_OFF = 2
    };
    
    // PINS
    const uint8_t _pinButton;
    const uint8_t _pinLight;
    const uint8_t _pinSwitch1;
    const uint8_t _pinSwitch2;
    
    // CONFIG
    int _longPressTime = 700;
    int _doubleClickTime = 400;
    float _lightChangeSpeed = 25.0f; // (bigger value -> faster transition)
    int _lightFeedbackFlashSpeed = 150;
    float _lightMaxBrightness = 1.0f;
    bool _lightPulseEnabled = true;
    float _lightPulseFreq = 0.5f; // Hz
    float _lightPulseSize = 0.1; // %
    
    // STATE
    int _programIndex;
    
    bool _buttonLastState;
    unsigned long _longPressStarted;
    unsigned long _doubleClickStarted;
    bool _doubleClickInProgress;
    bool _nextReleaseIsDoubleClick;
    bool _longPressFired;
    
    bool _lightKeepLit = false;
    bool _lightFeedbackFlashRunning = false;
    unsigned long _lightFeedbackFlashTime = 0;
    LightOverride _lightOverride = LIGHT_FREE;
    float _lightBrightness = 0.0f;
    float _lightPulseTime = 0.0f;
    unsigned long _lastTimestamp = 0;
    
    // FUNCTIONS
    int readProgramSwitch() const;
    bool readButton() const;
    
    void resetButtonState();
    void updateLight();
    void triggerFeedbackFlash();
    
public:
    VbsBigRedButton(const uint8_t pinButton, const uint8_t pinLight, const uint8_t pinSwitch1, const uint8_t pinSwitch2);
    
    void SetLongPressTime(const int ms);
    void SetDoubleClickTime(const int ms);
    void SetLightChangeSpeed(const float speed);
    void SetLightFeedbackFlashSpeed(const int ms);
    void SetLightMaxBrightness(const float brightness);
    void SetLightPulse(const float frequency, const float size = 0.1f);
    
    void KeepLightLit(const bool lit);
    
    int GetProgramIndex();
    VbsSingleButtonEvent PollSingleButtonEvent();
    VbsDualButtonEvent PollDualButtonEvent();
    VbsQuadButtonEvent PollQuadButtonEvent();
};

#endif