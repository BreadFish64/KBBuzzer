/*
    Name:       KBBuzzer.ino
    Created:	11/25/2018 9:39:31 PM
    Author:     Marshall Mohror
*/

#include "buzzer.h"

Buzzer buzzer;

constexpr uint8_t buttons[] = {
    2, 3, 4, 5, A0, A1, A2, A3, A4, A5,
};

void setup() {
    for (auto button : buttons)
        pinMode(button, INPUT_PULLUP);
    buzzer = Buzzer();
}

void loop() {
    buzzer.refresh();
}
