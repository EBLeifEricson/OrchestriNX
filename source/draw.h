#ifndef DRAW_H
#define DRAW_H
#include <switch.h>

#include "font.h"

typedef struct {
    u32 width;
    u32 height;
    u32* buf;
} Bitmap;

int centerX(int width);
int centerY(int height);

void drawStart();
void drawClearScreen(u32 color);

void drawPixel(int x, int y, u32 color);
void drawLine(int x1, int y1, int x2, int y2, u32 color);
void drawRectangle(int x1, int y1, int x2, int y2, u32 color);
void drawFillRect(int x1, int y1, int x2, int y2, u32 color);
void drawBitmapA(int x, int y, Bitmap* bmp, u32 alpha);
void drawBitmap(int x, int y, Bitmap* bmp);
u32* resizePixels(u32* pixels, int w1, int h1, int w2, int h2);
Bitmap* scaleBitmap(Bitmap* bmp, int scale);
void drawText(u32 font, int x, int y, u32 color, const char* str);
void drawTextFormat(u32 font, int x, int y, u32 color, const char* str, ...);

Bitmap* openFileBitmap(const char* path, int width, int height);

#endif /* DRAW_H */
