#include <Arduino.h>
#include <TFT_eSPI.h>
#include <FS.h>
#include <LittleFS.h>
#include <Preferences.h>

TFT_eSPI tft = TFT_eSPI();
Preferences prefs;

volatile bool actionInProgress = false;
volatile int selectedAction = 0;
volatile bool needsRedraw = false;

int hunger = 100;
int sleepiness = 100;
int hygiene = 100;
int health = 100;
bool sick = false;

unsigned long lastDecay = 0;
const unsigned long decayInterval = 10000;
unsigned long lastIllCheck = 0;
unsigned long nextIllTime = 60000;

const char* actionNames[] = {"EAT", "SLEEP", "BATH", "PLAY", "HEAL", "STATUS"};
const char* actionShort[] = {"E", "S", "B", "P", "H", "I"};

#include "buttons.h" 
#include "display.h"
#include "pet.h"
void akce() {
            Serial.printf("\n▶️ Provádím: %s\n", actionNames[selectedAction]);
        
        executeAction(selectedAction);
        
        drawBunny(sick ? "/sick1.bmp" : "/base1.bmp"); 
        
        actionInProgress = false; 
        drawMenu(); // Změna: Bez parametrů
        
        Serial.println("⏹️ Hotovo/Přerušeno\n");
}
void setup() {
    Serial.begin(115200);
    delay(500);
    Serial.println("\n\n=== TAMAGOTCHI START ===");

    initDisplay();
    showLoading();

    // Necháme true pro jistotu opravy disku
    if (!LittleFS.begin(true)) {
        Serial.println("❌ LittleFS FAIL");
        showError("FS ERROR!");
        while (1);
    }
    Serial.println("✅ LittleFS OK");

    initButtons(); 
    loadState();
    randomSeed(analogRead(0));

    Serial.println("🎨 První vykreslení...");
    drawBackground();
    drawBunny(sick ? "/sick1.bmp" : "/base1.bmp");
    
    drawMenu(); // Změna: Bez parametrů

    Serial.println("✅ Setup hotov\n");
}

void loop() {
    updateDecay();
    checkIllness();

    if (needsRedraw) {
        drawMenu(); // Změna: Bez parametrů
        needsRedraw = false;
    }

    //if (actionInProgress) {
     //   Serial.printf("\n▶️ Provádím: %s\n", actionNames[selectedAction]);
        
      //  executeAction(selectedAction);
        
       // drawBunny(sick ? "/sick1.bmp" : "/base1.bmp"); 
        
       // actionInProgress = false; 
        //drawMenu(); // Změna: Bez parametrů
        
         //Serial.println("⏹️ Hotovo/Přerušeno\n");
    //}

    delay(10);
}