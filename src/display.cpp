#include "display.h"
#include <Arduino.h>
#include "buttons.h" 

extern TFT_eSPI tft;
extern volatile int selectedAction;

// === Pomocná funkce: Přečte pixel ===
uint16_t readPixel(File &f, int bpp) {
    if (bpp >= 24) {
        uint8_t b = f.read();
        uint8_t g = f.read();
        uint8_t r = f.read();
        if (bpp == 32) f.read(); 
        return tft.color565(r, g, b);
    } else if (bpp == 16) {
        return f.read() | (f.read() << 8);
    }
    return 0;
}

// === Kreslení BMP (Ikony) ===
bool drawBmp(const char *filename, int16_t x, int16_t y) {
    File bmpFS = LittleFS.open(filename, "r");
    if (!bmpFS) return false;
    if (bmpFS.read() != 'B' || bmpFS.read() != 'M') { bmpFS.close(); return false; }

    bmpFS.seek(10); uint32_t seekOffset = bmpFS.read() | (bmpFS.read() << 8) | (bmpFS.read() << 16) | (bmpFS.read() << 24);
    bmpFS.seek(18); int32_t w = bmpFS.read() | (bmpFS.read() << 8) | (bmpFS.read() << 16) | (bmpFS.read() << 24);
    int32_t h = bmpFS.read() | (bmpFS.read() << 8) | (bmpFS.read() << 16) | (bmpFS.read() << 24);
    bmpFS.seek(28); uint16_t bpp = bmpFS.read() | (bmpFS.read() << 8);

    bmpFS.seek(seekOffset);
    uint16_t *lineBuffer = (uint16_t*)malloc(w * 2);
    if (!lineBuffer) { bmpFS.close(); return false; }

    int padding = (bpp == 24) ? (4 - ((w * 3) % 4)) % 4 : 0;

    for (int row = h - 1; row >= 0; row--) {
        for (int col = 0; col < w; col++) {
            uint16_t color = readPixel(bmpFS, bpp);
            lineBuffer[col] = (color >> 8) | (color << 8); // Swap pro správné barvy
        }
        for (int p = 0; p < padding; p++) bmpFS.read();
        tft.pushImage(x, y + row, w, 1, lineBuffer);
    }
    free(lineBuffer);
    bmpFS.close();
    return true;
}

// === Transparentní BMP (Králíček) ===
bool drawBmpTransparent(const char *filename, int16_t x, int16_t y) {
    File bmpFS = LittleFS.open(filename, "r");
    if (!bmpFS) return false;
    if (bmpFS.read() != 'B' || bmpFS.read() != 'M') { bmpFS.close(); return false; }
    
    bmpFS.seek(10); uint32_t seekOffset = bmpFS.read() | (bmpFS.read() << 8) | (bmpFS.read() << 16) | (bmpFS.read() << 24);
    bmpFS.seek(18); int32_t w = bmpFS.read() | (bmpFS.read() << 8) | (bmpFS.read() << 16) | (bmpFS.read() << 24);
    int32_t h = bmpFS.read() | (bmpFS.read() << 8) | (bmpFS.read() << 16) | (bmpFS.read() << 24);
    bmpFS.seek(28); uint16_t bpp = bmpFS.read() | (bmpFS.read() << 8);

    bmpFS.seek(seekOffset);
    int padding = (bpp == 24) ? (4 - ((w * 3) % 4)) % 4 : 0;
    const uint16_t MAGENTA = 0xF81F; 

    for (int row = h - 1; row >= 0; row--) {
        for (int col = 0; col < w; col++) {
            uint16_t color = readPixel(bmpFS, bpp);
            if (color != MAGENTA) tft.drawPixel(x + col, y + row, color);
        }
        for (int p = 0; p < padding; p++) bmpFS.read();
    }
    bmpFS.close();
    return true;
}

// === Částečné kreslení (Pozadí) ===
bool drawBmpPartial(const char *filename, int16_t x, int16_t y, int16_t w, int16_t h) {
    File bmpFS = LittleFS.open(filename, "r");
    if (!bmpFS) return false;
    if (bmpFS.read() != 'B' || bmpFS.read() != 'M') { bmpFS.close(); return false; }

    bmpFS.seek(10); uint32_t seekOffset = bmpFS.read() | (bmpFS.read() << 8) | (bmpFS.read() << 16) | (bmpFS.read() << 24);
    bmpFS.seek(18); int32_t bmp_w = bmpFS.read() | (bmpFS.read() << 8) | (bmpFS.read() << 16) | (bmpFS.read() << 24);
    int32_t bmp_h = bmpFS.read() | (bmpFS.read() << 8) | (bmpFS.read() << 16) | (bmpFS.read() << 24);
    bmpFS.seek(28); uint16_t bpp = bmpFS.read() | (bmpFS.read() << 8);

    uint16_t *lineBuffer = (uint16_t*)malloc(w * 2);
    if (!lineBuffer) { bmpFS.close(); return false; }
    
    int bytesPerPixel = bpp / 8;
    uint32_t rowSize = ((bmp_w * bytesPerPixel + 3) / 4) * 4;

    for (int i = 0; i < h; i++) {
        int targetY = y + i;
        if (targetY >= 107) continue; 

        int targetRowBMP = bmp_h - 1 - targetY;
        if (targetRowBMP < 0 || targetRowBMP >= bmp_h) continue;

        bmpFS.seek(seekOffset + (targetRowBMP * rowSize));
        for (int p = 0; p < x; p++) {
             if(bpp >= 24) { bmpFS.read(); bmpFS.read(); bmpFS.read(); if(bpp==32) bmpFS.read(); }
             else { bmpFS.read(); bmpFS.read(); }
        }
        for (int col = 0; col < w; col++) {
            uint16_t color = readPixel(bmpFS, bpp);
            lineBuffer[col] = (color >> 8) | (color << 8);
        }
        tft.pushImage(x, targetY, w, 1, lineBuffer);
    }
    free(lineBuffer);
    bmpFS.close();
    return true;
}

void initDisplay() {
    pinMode(4, OUTPUT);
    digitalWrite(4, HIGH);
    tft.init();
    tft.setRotation(1);
    // tft.setSwapBytes(true); // SMAZÁNO (pro správné barvy)
    tft.fillScreen(TFT_BLACK);
    Serial.println("✅ Display inicializován");
}

// === MENU S TMAVĚ RŮŽOVOU ČÁRKOU ===
void drawMenu() {
    int iconWidth = 40;
    int menuY = 107;

    // Definice tmavě růžové barvy (Red=200, Green=20, Blue=100)
    uint16_t darkPink = tft.color565(200, 20, 100);

    for(int i = 0; i < ACTION_COUNT; i++) {
        int x = i * iconWidth;
        int y = menuY;

        const char* iconFile = nullptr;
        switch(i) {
            case 0: iconFile = "/eatIcon.bmp"; break;
            case 1: iconFile = "/sleepIcon.bmp"; break;
            case 2: iconFile = "/bathIcon.bmp"; break;
            case 3: iconFile = "/gameIcon.bmp"; break;
            case 4: iconFile = "/healIcon.bmp"; break;
            case 5: iconFile = "/statusIcon.bmp"; break;
        }
        if(iconFile) drawBmp(iconFile, x, y);

        // Tmavě růžový proužek pod vybranou ikonou
        if(i == selectedAction) {
            tft.fillRect(x + 10, y + 24, 20, 3, darkPink);
        }
    }
}

void showLoading() {
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextSize(2);
    tft.setCursor(40, 60);
    tft.println("Loading...");
}

void showError(const char* message) {
    tft.fillScreen(TFT_RED);
    tft.setTextColor(TFT_WHITE, TFT_RED);
    tft.setTextSize(2);
    tft.setCursor(20, 60);
    tft.println(message);
}