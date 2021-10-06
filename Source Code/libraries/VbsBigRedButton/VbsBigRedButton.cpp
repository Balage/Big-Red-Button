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

#include "VbsBigRedButton.h"

static int MinMax(const int min, const int max, const int value)
{
    return value < min ? min : (value > max ? max : value);
}

static float MinMax(const float min, const float max, const float value)
{
    return value < min ? min : (value > max ? max : value);
}

VbsBigRedButton::VbsBigRedButton(const uint8_t pinButton, const uint8_t pinLight, const uint8_t pinSwitch1, const uint8_t pinSwitch2) :
    _pinButton(pinButton),
    _pinLight(pinLight),
    _pinSwitch1(pinSwitch1),
    _pinSwitch2(pinSwitch2)
{
    pinMode(_pinButton, INPUT);
    pinMode(_pinLight, OUTPUT);
    pinMode(_pinSwitch1, INPUT);
    pinMode(_pinSwitch2, INPUT);
    
    // Set default values
    digitalWrite(_pinLight, HIGH);
    resetButtonState();
    _programIndex = readProgramSwitch();
}

bool VbsBigRedButton::readButton() const
{
    // (Note that the value is inverse because of the pull-up resistor)
    int value = analogRead(_pinButton);
    
    // Schmitt trigger
    if (_buttonLastState && value >= 768) return false;
    if (!_buttonLastState && value < 256) return true;
    return _buttonLastState;
}

int VbsBigRedButton::readProgramSwitch() const
{
    return (
        (digitalRead(_pinSwitch1) ? 0 : 1) |
        (digitalRead(_pinSwitch2) ? 0 : 2)
    );
}

void VbsBigRedButton::resetButtonState()
{
    _buttonLastState = false;
    _longPressStarted = 0;
    _doubleClickStarted = 0;
    _doubleClickInProgress = false;
    _nextReleaseIsDoubleClick = false;
    _longPressFired = false;
    
    _lightOverride = LIGHT_FREE;
    _lightFeedbackFlashRunning = false;
}

void VbsBigRedButton::updateLight()
{
    // Calculate delta time
    const unsigned long timestamp = millis();
    const unsigned long delta_i = timestamp - _lastTimestamp;
    
    if (delta_i > 0)
    {
        const float delta = delta_i / 1000.0f;
        _lastTimestamp = timestamp;
        
        // Update feedback flashing
        if (_lightFeedbackFlashRunning)
        {
            _lightFeedbackFlashTime += delta_i;
            
            // Sequence: (0)OFF, (1)ON, (2)OFF, (3)ON, (4+)FREE
            const int timeframe = _lightFeedbackFlashTime / _lightFeedbackFlashSpeed;
            if (timeframe >= 4)
            {
                _lightFeedbackFlashRunning = false;
                _lightOverride = LIGHT_FREE;
            }
            else
            {
                _lightOverride = timeframe == 1 || timeframe == 3 ? LIGHT_ON : LIGHT_OFF;
            }
        }
        
        // Determine desired brightness
        float newBrightness = 0.0f;
        if (_lightOverride != LIGHT_FREE)
        {
            newBrightness = _lightOverride == LIGHT_ON ? 1.0f : 0.0f;
        }
        else
        {
            if (_buttonLastState)
            {
                newBrightness = 1.0f;
            }
            else if (_lightKeepLit)
            {
                if (_lightPulseEnabled)
                {
                    _lightPulseTime = fmod(_lightPulseTime + delta, 1.0f / _lightPulseFreq);
                    const float pulseBrightness = sin(_lightPulseTime * _lightPulseFreq * PI * 2.0f) * 0.5f + 0.5f;
                    newBrightness = pulseBrightness * _lightPulseSize + (1.0f - _lightPulseSize);
                }
                else
                {
                    newBrightness = 1.0f;
                }
            }
        }

        // Imitate thermal inertia
        const float changeRatio = MinMax(0.0f, 1.0f, delta * _lightChangeSpeed);
        _lightBrightness = (changeRatio * newBrightness) + ((1.0f - changeRatio) * _lightBrightness);

        analogWrite(_pinLight, 255 - (int)(_lightBrightness * _lightMaxBrightness * 255.0f));
    }
}

void VbsBigRedButton::triggerFeedbackFlash()
{
    _lightFeedbackFlashRunning = true;
    _lightFeedbackFlashTime = 0;
}

void VbsBigRedButton::SetLongPressTime(const int ms)
{
    _longPressTime = MinMax(1, 10000, ms);
}

void VbsBigRedButton::SetDoubleClickTime(const int ms)
{
    _doubleClickTime = MinMax(1, 10000, ms);
}

void VbsBigRedButton::SetLightChangeSpeed(const float speed)
{
    _lightChangeSpeed = MinMax(0.1f, 10000.0f, speed);
}

void VbsBigRedButton::SetLightFeedbackFlashSpeed(const int ms)
{
    _lightFeedbackFlashSpeed = MinMax(1, 10000, ms);
}

void VbsBigRedButton::SetLightMaxBrightness(const float brightness)
{
    _lightMaxBrightness = MinMax(0.0f, 1.0f, brightness);
}

void VbsBigRedButton::SetLightPulse(const float frequency, const float size)
{
    _lightPulseEnabled = frequency > 0.0f;
    _lightPulseFreq = MinMax(0.01f, 100.0f, frequency);
    _lightPulseSize = MinMax(0.01f, 1.0f, size);
}

int VbsBigRedButton::GetProgramIndex()
{
    const int newProgramIndex = readProgramSwitch();
    if (_programIndex != newProgramIndex)
    {
        _programIndex = newProgramIndex;
        resetButtonState();
        
        // Avoid any keys getting stuck while changing program
        Keyboard.ReleaseKey();
    }
    return _programIndex;
}

void VbsBigRedButton::KeepLightLit(const bool lit)
{
    if (_lightKeepLit != lit)
    {
        _lightPulseTime = 0.0f;
    }
    _lightKeepLit = lit;
}

VbsSingleButtonEvent VbsBigRedButton::PollSingleButtonEvent()
{
    VbsSingleButtonEvent event;
    const bool buttonState = readButton();
    
    // Normal button press and release, both fired once
    event.Press = buttonState && buttonState != _buttonLastState;
    event.Release = !buttonState && buttonState != _buttonLastState;
    _buttonLastState = buttonState;
    
    updateLight();
    return event;
}

VbsDualButtonEvent VbsBigRedButton::PollDualButtonEvent()
{
    VbsDualButtonEvent event;
    const unsigned long timestamp = millis();
    const bool buttonState = readButton();
    
    // Normal button press and release, both fired once
    const bool buttonPressed = buttonState && buttonState != _buttonLastState;
    const bool buttonReleased = !buttonState && buttonState != _buttonLastState;
    _buttonLastState = buttonState;
    
    // Button holding started
    if (buttonPressed)
    {
        _longPressStarted = timestamp;
        _longPressFired = false;
    }
    
    // Short release event, only if released before long press
    event.Click = buttonReleased && !_longPressFired;
    if (event.Click && _lightKeepLit)
    {
        triggerFeedbackFlash();
    }
    
    // Long press event, fires once while pressed if button is held for some time
    // (release will not trigger short release event if long press has fired)
    event.LongPress = !_longPressFired && buttonState && _longPressStarted + _longPressTime < timestamp;
    if (event.LongPress)
    {
        _longPressFired = true;
        triggerFeedbackFlash();
    }
    
    updateLight();
    return event;
}

VbsQuadButtonEvent VbsBigRedButton::PollQuadButtonEvent()
{
    VbsQuadButtonEvent event;
    const unsigned long timestamp = millis();
    const bool buttonState = readButton();
    
    // Normal button press and release, both fired once
    const bool buttonPressed = buttonState && buttonState != _buttonLastState;
    const bool buttonReleased = !buttonState && buttonState != _buttonLastState;
    _buttonLastState = buttonState;
    
    // Button holding started
    if (buttonPressed)
    {
        _longPressStarted = timestamp;
        _longPressFired = false;
    }
    
    // Long press event, fires once while pressed if button is held for some time
    // (release will not trigger short release event if long press has fired)
    event.LongPress = !_longPressFired && buttonState && _longPressStarted + _longPressTime < timestamp;
    event.LongPressDoubleClick = false;
    
    if (event.LongPress)
    {
        _longPressFired = true;
        _doubleClickInProgress = false;
        triggerFeedbackFlash();
        
        if (_nextReleaseIsDoubleClick)
        {
            event.LongPress = false;
            event.LongPressDoubleClick = true;
        }
    }
    
    // Single/double click
    event.SingleClick = false;
    event.DoubleClick = false;
    
    const unsigned long doubleClickDelta = timestamp - _doubleClickStarted;
    const bool withinDoubleClickTime = doubleClickDelta <= _doubleClickTime;

    if (_doubleClickInProgress)
    {
        if (withinDoubleClickTime)
        {
            if (buttonPressed)
            {
                // Mark double click
                _nextReleaseIsDoubleClick = true;
            }
        }
        else
        {
            if (!buttonState && !_nextReleaseIsDoubleClick)
            {
                // Single click
                event.SingleClick = true;
                _doubleClickInProgress = false;
                
                if (_lightKeepLit)
                {
                    triggerFeedbackFlash();
                }
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
        // Double click
        event.DoubleClick = true;
        _doubleClickInProgress = false;
        
        if (_lightKeepLit)
        {
            triggerFeedbackFlash();
        }
    }
    
    updateLight();
    return event;
}