#include "buttons.h"
#include <Arduino.h>

// Globální proměnná pro debounce
volatile unsigned long lastPress = 0; 
const unsigned long DEBOUNCE_DELAY = 250; // Zvýšeno na 250ms pro jistotu

// === LEVÉ TLAČÍTKO: POUZE POSUN VLEVO ===
void IRAM_ATTR handleLeftPress() {
    if (millis() - lastPress < DEBOUNCE_DELAY) return;
    lastPress = millis();

    // Pokud už něco děláme (jídlo, spánek...), tlačítko ignorujeme!
    //if (actionInProgress) return; 
    akce();
    selectedAction = (selectedAction - 1 + ACTION_COUNT) % ACTION_COUNT;
    needsRedraw = true;
}

// === STŘEDNÍ TLAČÍTKO: POUZE POTVRZENÍ / ZRUŠENÍ ===
void IRAM_ATTR handleCenterPress() {
    if (millis() - lastPress < DEBOUNCE_DELAY) return;
    lastPress = millis();

    if(!actionInProgress) {
        actionInProgress = true; // Spustit akci
    } else {
        actionInProgress = false; // Zastavit akci
        needsRedraw = true;       // Překreslit menu
    }
}

// === PRAVÉ TLAČÍTKO: POUZE POSUN VPRAVO ===
void IRAM_ATTR handleRightPress() {
    if (millis() - lastPress < DEBOUNCE_DELAY) return;
    lastPress = millis();

    // Pokud už něco děláme, tlačítko ignorujeme!
    if (actionInProgress) return;

    selectedAction = (selectedAction + 1) % ACTION_COUNT;
    needsRedraw = true;
}

void initButtons() {
    pinMode(PIN_LEFT, INPUT_PULLUP);
    pinMode(PIN_CENTER, INPUT_PULLUP);
    pinMode(PIN_RIGHT, INPUT_PULLUP);

    attachInterrupt(digitalPinToInterrupt(PIN_LEFT), handleLeftPress, FALLING);
    attachInterrupt(digitalPinToInterrupt(PIN_CENTER), handleCenterPress, FALLING);
    attachInterrupt(digitalPinToInterrupt(PIN_RIGHT), handleRightPress, FALLING);

    Serial.println("✅ Tlačítka inicializována (STRICT MODE)");
}