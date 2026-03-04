#include "R4_TFT_SPI_Touch.h"

SPISettings settingsLCD(24000000, MSBFIRST, SPI_MODE0);
SPISettings settingsTouch(1000000, MSBFIRST, SPI_MODE0);

static void LCD_WriteReg(uint8_t reg) {
    SPI.beginTransaction(settingsLCD);
    digitalWrite(LCD_DC, LOW); digitalWrite(LCD_CS, LOW);
    SPI.transfer(reg);
    digitalWrite(LCD_CS, HIGH); SPI.endTransaction();
}

static void LCD_WriteData(uint8_t data) {
    SPI.beginTransaction(settingsLCD);
    digitalWrite(LCD_DC, HIGH); digitalWrite(LCD_CS, LOW);
    SPI.transfer(data);
    digitalWrite(LCD_CS, HIGH); SPI.endTransaction();
}

static void SetAddrWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
    LCD_WriteReg(0x2A); 
    LCD_WriteData(x0 >> 8); LCD_WriteData(x0 & 0xFF);
    LCD_WriteData(x1 >> 8); LCD_WriteData(x1 & 0xFF);
    LCD_WriteReg(0x2B); 
    LCD_WriteData(y0 >> 8); LCD_WriteData(y0 & 0xFF);
    LCD_WriteData(y1 >> 8); LCD_WriteData(y1 & 0xFF);
    LCD_WriteReg(0x2C);
}

void TFT_Begin() {
    pinMode(LCD_CS, OUTPUT); pinMode(LCD_DC, OUTPUT); pinMode(LCD_RST, OUTPUT);
    pinMode(LCD_BL, OUTPUT); pinMode(TP_CS, OUTPUT); pinMode(SD_CS, OUTPUT);
    pinMode(TP_IRQ, INPUT_PULLUP);

    digitalWrite(LCD_CS, HIGH); digitalWrite(TP_CS, HIGH); digitalWrite(SD_CS, HIGH);
    digitalWrite(LCD_BL, HIGH);

    SPI.begin();
    digitalWrite(LCD_RST, LOW); delay(50); digitalWrite(LCD_RST, HIGH); delay(150);

    LCD_WriteReg(0x36); LCD_WriteData(0x28); // Landscape
    LCD_WriteReg(0x3A); LCD_WriteData(0x55); 
    LCD_WriteReg(0x11); delay(120);          
    LCD_WriteReg(0x29);                      
    TFT_FillRect(0, 0, 480, 320, BLACK);
}

void TFT_FillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    if (x >= 480 || y >= 320) return;
    if (x + w > 480) w = 480 - x;
    if (y + h > 320) h = 320 - y;

    SetAddrWindow(x, y, x + w - 1, y + h - 1);
    SPI.beginTransaction(settingsLCD);
    digitalWrite(LCD_DC, HIGH); digitalWrite(LCD_CS, LOW);
    uint8_t hi = color >> 8, lo = color & 0xFF;
    for (uint32_t i = 0; i < (uint32_t)w * h; i++) {
        SPI.transfer(hi); SPI.transfer(lo);
    }
    digitalWrite(LCD_CS, HIGH); SPI.endTransaction();
}

void TFT_DrawChar(int16_t x, int16_t y, int16_t c, uint16_t color, uint16_t bg, uint8_t size) {
    if (x >= 480 || y >= 320 || (x + 6 * size - 1) < 0 || (y + 8 * size - 1) < 0) return;
    
    SetAddrWindow(x, y, x + (6 * size) - 1, y + (8 * size) - 1);
    
    SPI.beginTransaction(settingsLCD);
    digitalWrite(LCD_DC, HIGH); digitalWrite(LCD_CS, LOW);
    
    for (int8_t row = 0; row < 8; row++) {
        for (uint8_t r_repeat = 0; r_repeat < size; r_repeat++) {
            for (int8_t col = 0; col < 6; col++) {
                // Calcolo dell'indice corretto per il carattere 256
                uint8_t line = (col < 5) ? pgm_read_byte(&font5x7[c * 5 + col]) : 0x00;
                uint16_t pColor = (line & (1 << row)) ? color : bg;
                uint8_t hi = pColor >> 8, lo = pColor & 0xFF;
                for (uint8_t c_repeat = 0; c_repeat < size; c_repeat++) {
                    SPI.transfer(hi); SPI.transfer(lo);
                }
            }
        }
    }
    digitalWrite(LCD_CS, HIGH); SPI.endTransaction();
}

void TFT_DrawString(int16_t x, int16_t y, const char* str, uint16_t color, uint16_t bg, uint8_t size) {
    while (*str) {
        TFT_DrawChar(x, y, *str++, color, bg, size);
        x += 6 * size;
        if (x + 6 * size >= 480) break; // Clip orizzontale
    }
}

void TFT_DrawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color) {
    // Ottimizzazione per linee dritte (usa FillRect che è molto più veloce)
    if (x0 == x1) {
        TFT_FillRect(x0, min(y0, y1), 1, abs(y1 - y0) + 1, color);
        return;
    }
    if (y0 == y1) {
        TFT_FillRect(min(x0, x1), y0, abs(x1 - x0) + 1, 1, color);
        return;
    }

    int16_t dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int16_t dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int16_t err = dx + dy, e2;

    while (1) {
        SetAddrWindow(x0, y0, x0, y0);
        SPI.beginTransaction(settingsLCD);
        digitalWrite(LCD_DC, HIGH); digitalWrite(LCD_CS, LOW);
        SPI.transfer(color >> 8); SPI.transfer(color & 0xFF);
        digitalWrite(LCD_CS, HIGH); SPI.endTransaction();
        
        if (x0 == x1 && y0 == y1) break;
        e2 = 2 * err;
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
    }
}

void TFT_DrawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    TFT_DrawLine(x, y, x + w, y, color);           // Sopra
    TFT_DrawLine(x, y + h, x + w, y + h, color);   // Sotto
    TFT_DrawLine(x, y, x, y + h, color);           // Sinistra
    TFT_DrawLine(x + w, y, x + w, y + h, color);   // Destra
}

void TFT_DrawRectThick(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    TFT_DrawRect(x, y, w, h, color);             // Bordo esterno
    TFT_DrawRect(x + 1, y + 1, w - 2, h - 2, color); // Bordo interno
}

// implementazione dell'algoritmo di Bresenham per il cerchio
void TFT_DrawCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color) {
    int16_t f = 1 - r;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * r;
    int16_t x = 0;
    int16_t y = r;

    TFT_FillRect(x0, y0 + r, 1, 1, color);
    TFT_FillRect(x0, y0 - r, 1, 1, color);
    TFT_FillRect(x0 + r, y0, 1, 1, color);
    TFT_FillRect(x0 - r, y0, 1, 1, color);

    while (x < y) {
        if (f >= 0) {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }
        x++;
        ddF_x += 2;
        f += ddF_x;

        TFT_FillRect(x0 + x, y0 + y, 1, 1, color);
        TFT_FillRect(x0 - x, y0 + y, 1, 1, color);
        TFT_FillRect(x0 + x, y0 - y, 1, 1, color);
        TFT_FillRect(x0 - x, y0 - y, 1, 1, color);
        TFT_FillRect(x0 + y, y0 + x, 1, 1, color);
        TFT_FillRect(x0 - y, y0 + x, 1, 1, color);
        TFT_FillRect(x0 + y, y0 - x, 1, 1, color);
        TFT_FillRect(x0 - y, y0 - x, 1, 1, color);
    }
}

uint16_t readTouchADC(uint8_t cmd) {
    uint16_t res = 0;
    digitalWrite(LCD_CS, HIGH); 
    digitalWrite(TP_CS, HIGH);
    delayMicroseconds(5); 

    SPI.beginTransaction(settingsTouch);
    digitalWrite(TP_CS, LOW);
    SPI.transfer(cmd);
    delayMicroseconds(200); 
    uint8_t h = SPI.transfer(0x00);
    uint8_t l = SPI.transfer(0x00);
    digitalWrite(TP_CS, HIGH);
    SPI.endTransaction();

    res = ((h << 8) | l) >> 3;
    return res;
}

TouchPoint TFT_GetTouch() {
    TouchPoint p = {0, 0, false};
    if (digitalRead(TP_IRQ) == LOW) {
        uint16_t rx = readTouchADC(0xD0); 
        uint16_t ry = readTouchADC(0x90);
        // Mapping basato sui tuoi parametri precedenti
        p.x = map(ry, 3757, 328, 0, 479);
        p.y = map(rx, 462, 3745, 0, 319);
        if (p.x >= 0 && p.x < 480 && p.y >= 0 && p.y < 320) p.pressed = true;
    }
    return p;
}

void drawStringRightAligned(int xEnd, int y, char* str, uint16_t color, uint16_t bg, uint8_t size) {
    TFT_DrawString(xEnd - (strlen(str) * (6 * size)), y, str, color, bg, size);
}

void drawButton3D(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    uint16_t r = (color >> 11) & 0x1F;
    uint16_t g = (color >> 5) & 0x3F;
    uint16_t b = color & 0x1F;
    uint16_t luce = (min(r + (r >> 2), 31) << 11) | (min(g + (g >> 2), 63) << 5) | min(b + (b >> 2), 31);
    uint16_t ombra = ((r >> 1) << 11) | ((g >> 1) << 5) | (b >> 1);
    TFT_FillRect(x, y, w, h, color);
    for(int i = 0; i < 2; i++) {
        TFT_DrawLine(x+i, y+i, x+w-1-i, y+i, luce);
        TFT_DrawLine(x+i, y+i, x+i, y+h-1-i, luce);
        TFT_DrawLine(x+i, y+h-1-i, x+w-1-i, y+h-1-i, ombra);
        TFT_DrawLine(x+w-1-i, y+i, x+w-1-i, y+h-1-i, ombra);
    }
}