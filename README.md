# Big Red Button™

A big red button with an Arduino Leonardo (MEGA32U4) acting as a keyboard.

This code is an extended version of the Keyboard library already present in the Arduino IDE. What the official version lacks is the support for Page 0x01 keys (Volume Up/Down, Mute, Sleep, Power Off), and the support for reading LED status.

The intention with this library was to build a big red button that acts like a keyboard, allowing a software in the background to listen on the pressed keys and do things when that happen. I mainly use it as a party tool to make sound effects, like for a video game event. I wrote an app for this specific purpose, and it's also publicly available: https://vbstudio.hu/apps/vasparittya

The button can also be used to host quiz shows (given you have more than one button).

## How to install code
- The Arduino IDE by default stores all Sketches in a folder named "Arduino" in your Documents folder.
- Download the repository and copy everything from the "Source Code" folder to there.
-- It might ask whether you want to merge the "libraries" folder with the new one, choose yes.
- Start the Arduino IDE and select "File", "Sketches", and then choose "BigRedButton" from the list.

The relevant part of the code is all placed in the beginning of the sketch with some expanations. There you can change and tweak behavior, or switch I/O pins.

## Build and upload sketch
- After the Leonardo board is plugged in, go to "Tools", "Board", then select "Arduino Leonardo".
- Go to "Tools", "Port", and select the port that looks like "COM# (Arduino Leonardo)".
- Press and hold the reset button on the Leonardo board. (Add one if it does not have it)
- Start Upload.
- As soon as the console at the bottom says "Uploading...", release the reset button.
- It should finish in a few seconds, and then done.

## Changing keys
The Keyboard class specifies separate function calls for page 0x01 and page 0x07 keys.
Constants for the most common key codes, and modifier keys are defined in `VbsKeyboard.h`.

### Page 0x07 key calls (regular keys)
``` c++
Keyboard.PressKey(uint8_t key, uint8_t modifier = MOD_NONE)
```
Issues a key press and then immediately a release for the specified `key`. One or multiple `modifier`-s can specified to be pressed with the key, like Ctrl, Alt, Shift or GUI (Window key).

``` c++
Keyboard.HoldKey(uint8_t key, uint8_t modifier = MOD_NONE)
```
Same as `PressKey()`, except the key remains pressed until `ReleaseKey()` is called.

``` c++
Keyboard.ReleaseKey()
```
Release all currently pressed (page 0x07) keys.

### Page 0x01 key calls (system and media keys)
``` c++
Keyboard.PressKeyPage1(uint16_t key)
```
Issues a key press and then immediately a release for the specified `key`.

## License
MIT