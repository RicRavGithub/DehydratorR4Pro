#ifndef R4_TFT_SPI_TOUCH_H
#define R4_TFT_SPI_TOUCH_H

#include <Arduino.h>
#include <SPI.h>
#include "gclfont.h"

// Pinout standard per Shield Waveshare su R4
#define LCD_CS  10
#define LCD_RST 8
#define LCD_DC  7
#define LCD_BL  9
#define TP_CS   4
#define TP_IRQ  3
#define SD_CS   5

// Definizione Colori 16-bit (RGB565)
#define BLACK   0x0000
#define WHITE   0xFFFF
#define RED     0xF800
#define GREEN   0x07E0
#define BLUE    0x001F
#define CYAN    0x07FF
#define YELLOW  0xFFE0
#define GRAY    0x8410
#define ORANGE  0xFD20

struct TouchPoint {
    int16_t x;
    int16_t y;
    bool pressed;
};

// Inizializzazione
void TFT_Begin();

// Primitive Grafiche
void TFT_FillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
void TFT_DrawChar(int16_t x, int16_t y, int16_t c, uint16_t color, uint16_t bg, uint8_t size);
void TFT_DrawString(int16_t x, int16_t y, const char* str, uint16_t color, uint16_t bg, uint8_t size);
void TFT_DrawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color);
void TFT_DrawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
void TFT_DrawCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color);
void TFT_DrawRectThick(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
void drawStringRightAligned(int xEnd, int y, char* str, uint16_t color, uint16_t bg, uint8_t size);
void drawButton3D(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);

// Gestione Touch SPI
TouchPoint TFT_GetTouch();
uint16_t readTouchADC(uint8_t cmd);

#endif