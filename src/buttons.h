#ifndef BUTTONS_H
#define BUTTONS_H

#include <Arduino.h>

// === KLÍČOVÉ KONSTANTY PRO CELÝ PROJEKT (Definované zde) ===
const int PIN_LEFT = 32;
const int PIN_CENTER = 33;
const int PIN_RIGHT = 25;
const int ACTION_COUNT = 6;
const int DEBOUNCE_DELAY_MS = 200; 

// === Globální proměnné (Externí) ===
extern volatile bool actionInProgress;
extern volatile int selectedAction;
extern volatile bool needsRedraw;
extern void akce();
// === Interrupt handlery - IMPLEMENTOVANÉ V buttons.cpp ===
void IRAM_ATTR handleLeftPress();
void IRAM_ATTR handleCenterPress();
void IRAM_ATTR handleRightPress();

// === Inicializace tlačítek - IMPLEMENTOVANÉ V buttons.cpp ===
void initButtons();


#endif
