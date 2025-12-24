#ifndef DISPLAY_H
#define DISPLAY_H

#include <TFT_eSPI.h>
#include <FS.h>
#include <LittleFS.h>
#include "buttons.h" 

extern TFT_eSPI tft;
extern volatile int selectedAction;
extern const char* actionShort[];
extern bool sick;
extern volatile bool needsRedraw;

const int BUNNY_X = 0; 
const int BUNNY_Y = 0; 
const int BUNNY_W = 240; 
const int BUNNY_H = 107; 

// Funkce
bool drawBmp(const char *filename, int16_t x, int16_t y);
bool drawBmpTransparent(const char *filename, int16_t x, int16_t y);
bool drawBmpPartial(const char *filename, int16_t x, int16_t y, int16_t w, int16_t h);

void initDisplay();

// Zpět na jednoduchou deklaraci bez parametrů
void drawMenu(); 

void showLoading();
void showError(const char* message);

inline void drawBackground() {
    drawBmp("/pozadi.bmp", 0, 0); 
    Serial.println("✅ Pozadí nakresleno");
}

inline void drawBunny(const char* bmpFile) {
    drawBmpPartial("/pozadi.bmp", BUNNY_X, BUNNY_Y, BUNNY_W, BUNNY_H);
    drawBmpTransparent(bmpFile, BUNNY_X, BUNNY_Y);
}

#endif