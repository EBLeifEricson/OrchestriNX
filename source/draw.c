#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <switch.h>
#include <string.h>

#include "draw.h"
#include "font.h"

static u32* framebuf;
static u32 fbwidth;
static u32 fbheight;

int centerX(int width) {
	int result = (fbwidth/2 - width/2);
	return result;
}

int centerY(int height) {
	int result = fbheight/2 - height/2;
	return result;
}

void drawStart() {
    framebuf = (u32*)gfxGetFramebuffer(&fbwidth, &fbheight);
}

void drawClearScreen(u32 color) {
	int i;
    for (i = 0; i < fbwidth * fbheight; ++i) {
        framebuf[i] = color;
    }
}

void drawPixel(int x, int y, u32 color) {
    if (x >= 0 && x < fbwidth && y >= 0 && y < fbheight) {
        int pos = y * fbwidth + x;
        framebuf[pos] = color;
    }
}

void drawLine(int x1, int y1, int x2, int y2, u32 color) {
    int x, y;
    if (x1 == x2) {
        for (y = y1; y <= y2; ++y) {
            drawPixel(x1, y, color);
        }
    } else {
        for (x = x1; x <= x2; ++x) {
            drawPixel(x, y1, color);
        }
    }
}

void drawRectangle(int x1, int y1, int x2, int y2, u32 color) {
    drawLine(x1, y1, x2, y1, color);
    drawLine(x2, y1, x2, y2, color);
    drawLine(x1, y2, x2, y2, color);
    drawLine(x1, y1, x1, y2, color);
}

void drawFillRect(int x1, int y1, int x2, int y2, u32 color) {
    int i, j;
    for (i = x1; i <= x2; ++i) {
        for (j = y1; j <= y2; ++j) {
            drawPixel(i, j, color);
        }
    }
}

static void _drawBitmap(int x, int y, Bitmap* bmp, bool isalpha, u32 alpha) {
    for (int yy = 0; yy < bmp->height; ++yy) {
        for (int xx = 0; xx < bmp->width; ++xx) {
            if (x >= 0 && xx+x < fbwidth && y >= 0 && yy+y < fbheight) {
				int pos = yy*(bmp->width)+xx;
				u32 curralpha = bmp->buf[pos];
				if (isalpha) {
					if (curralpha != alpha) {
						drawPixel(xx+x, yy+y, bmp->buf[pos]);
					}
				}
				else {
					drawPixel(xx+x, yy+y, bmp->buf[pos]);
				}
            }
        }
    }
}

void drawBitmap(int x, int y, Bitmap* bmp) {
    _drawBitmap(x, y, bmp, 0, 0);
}

void drawBitmapA(int x, int y, Bitmap* bmp, u32 alpha) {
	_drawBitmap(x, y, bmp, 1, alpha);
}

u32* resizePixels(u32* pixels, int w1, int h1, int w2, int h2) {
    u32* temp = (u32*)calloc(w2*h2, sizeof(u32));
	double x_ratio = w1/(double)w2 ;
    double y_ratio = h1/(double)h2 ;
    double px, py ;
    for (int i=0;i<h2;i++) {
        for (int j=0;j<w2;j++) {
            px = (int)(j*x_ratio) ;
            py = (int)(i*y_ratio) ;
            temp[(i*w2)+j] = pixels[(int)((py*w1)+px)] ;
        }
    }               
    return temp;
}

Bitmap* scaleBitmap(Bitmap* bmp, int scale) {
	Bitmap* scaled = malloc(sizeof(Bitmap));
	scaled->width = bmp->width*scale;
	scaled->height = bmp->height*scale;
	u32* scalebuf = resizePixels(bmp->buf, bmp->width, bmp->height, scaled->width, scaled->height);
	scaled->buf = scalebuf;
	return scaled;
}

Bitmap* openFileBitmap(const char* path, int width, int height) {
	FILE* bmp = fopen(path, "rb");
	if (!bmp) {
		return NULL;
	}
	fseek(bmp, 0, SEEK_END);
	long size = ftell(bmp);
	fseek(bmp, 0, SEEK_SET);
	
	u32* buf = calloc(size+1, sizeof(u32));
	char currpixel[3];
	
	int offset = 0;
	while (fread(currpixel, 3, 1, bmp) > 0) {
		buf[offset] = RGBA8_MAXALPHA(currpixel[0],currpixel[1],currpixel[2]);
		++offset;
	}
	fclose(bmp);
	
	Bitmap* bitmap = (Bitmap*)malloc(sizeof(Bitmap));
	bitmap->width = (u32)width;
	bitmap->height = (u32)height;
	bitmap->buf = buf;
	
	return bitmap;
}

void drawText(u32 font, int x, int y, u32 color, const char* str) {
    color_t clr;
    clr.abgr = color;
    DrawText(font, x, y, clr, str);
}

void drawTextFormat(u32 font, int x, int y, u32 color, const char* str, ...) {
    char buffer[256];
    va_list valist;
    va_start(valist, str);
    vsnprintf(buffer, 255, str, valist);
    drawText(font, x, y, color, buffer);
    va_end(valist);
}
