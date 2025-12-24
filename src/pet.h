#ifndef PET_H
#define PET_H

#include <Preferences.h>
#include "display.h"
#include "buttons.h" 

extern Preferences prefs;
extern TFT_eSPI tft;
extern volatile bool needsRedraw;
extern volatile bool actionInProgress;
extern volatile int selectedAction;

extern int hunger;
extern int sleepiness;
extern int hygiene;
extern int health;
extern bool sick;

extern unsigned long lastDecay;
extern const unsigned long decayInterval;
extern unsigned long lastIllCheck;
extern unsigned long nextIllTime;
extern const char* actionNames[];

inline void saveState() {
    prefs.begin("pet", false);
    prefs.putInt("hunger", hunger);
    prefs.putInt("sleepiness", sleepiness);
    prefs.putInt("hygiene", hygiene);
    prefs.putInt("health", health);
    prefs.putBool("sick", sick);
    prefs.end();
}

inline void loadState() {
    prefs.begin("pet", false);
    hunger = prefs.getInt("hunger", 100);
    sleepiness = prefs.getInt("sleepiness", 100);
    hygiene = prefs.getInt("hygiene", 100);
    health = prefs.getInt("health", 100);
    sick = prefs.getBool("sick", false);
    prefs.end();
    Serial.printf("📂 Loaded: H=%d S=%d Hy=%d He=%d Sick=%d\n", hunger, sleepiness, hygiene, health, sick);
}

inline void updateDecay() {
    if(millis() - lastDecay > decayInterval) {
        lastDecay = millis();
        hunger = max(0, hunger - 1);
        sleepiness = max(0, sleepiness - 1);
        hygiene = max(0, hygiene - 1);
        health = max(0, health - (sick ? 2 : 0));
        saveState();
        Serial.printf("📉 Decay: H=%d S=%d Hy=%d He=%d\n", hunger, sleepiness, hygiene, health);
    }
}

inline void checkIllness() {
    if(!sick && millis() - lastIllCheck > nextIllTime) {
        lastIllCheck = millis();
        int illChance = 7;
        if(hunger < 30) illChance -= 1;
        if(sleepiness < 30) illChance -= 1;
        if(hygiene < 30) illChance -= 2;
        if(random(0, 10) > illChance) {
            sick = true;
            Serial.println("🤒 ONEMOCNĚL!");
        }
        nextIllTime = random(60000, 180000);
    }
}

inline bool playActionAnimated(const char* bmp1, const char* bmp2) {
    Serial.println("🎬 Spouštím animaci (2x)...");
    for(int cycle = 0; cycle < 2; cycle++) {
        if(!actionInProgress) return false;

        drawBunny(bmp1);
        drawMenu(); // Změna: Bez parametrů

        for(int j = 0; j < 30; j++) {
            if(!actionInProgress) return false;
            delay(10);
        }

        drawBunny(bmp2);
        drawMenu(); // Změna: Bez parametrů

        for(int j = 0; j < 30; j++) {
            if(!actionInProgress) return false;
            delay(10);
        }
    }
    return true;
}

inline void executeAction(int action) {
    Serial.printf("🎬 Akce: %s\n", actionNames[action]);
    if(action == 5) {
        tft.fillScreen(TFT_BLACK);
        tft.setTextSize(2);
        tft.setTextColor(TFT_CYAN, TFT_BLACK);
        tft.setCursor(10, 10); tft.println("=== STAV ===");
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
        tft.setCursor(10, 40);
        tft.printf("Hlad:    %d%%\n", hunger);
        tft.printf("Spanek:  %d%%\n", sleepiness);
        tft.printf("Hygiena: %d%%\n", hygiene);
        tft.printf("Zdravi:  %d%%\n", health);
        if(sick) {
            tft.setTextColor(TFT_RED, TFT_BLACK);
            tft.setCursor(10, 100); tft.println("NEMOCNY!");
        }
        unsigned long startWait = millis();
        while(millis() - startWait < 3000) {
            if(!actionInProgress) break;
            delay(10);
        }
        drawBackground(); 
        return;
    }

    switch(action) {
        case 0: if(!playActionAnimated("/eat1.bmp", "/eat2.bmp")) break; hunger = min(100, hunger + 20); break;
        case 1: if(!playActionAnimated("/sleep1.bmp", "/sleep2.bmp")) break; sleepiness = min(100, sleepiness + 20); break;
        case 2: if(!playActionAnimated("/bath1.bmp", "/bath2.bmp")) break; hygiene = min(100, hygiene + 20); break;
        case 3: if(!playActionAnimated("/game1.bmp", "/game2.bmp")) break; health = min(100, health + 10); hunger = max(0, hunger - 5); break;
        case 4: if(!playActionAnimated("/heal1.bmp", "/heal2.bmp")) break; if(sick) { sick = false; health = min(100, health + 30); } break;
    }
    saveState();
}

#endif