

#ifndef COLOR_H
#define COLOR_H

/* color component indices */
#define RED         0
#define GREEN       1
#define BLUE        2

#define HUE         0
#define SATURATION  1
#define VALUE       2
#define UNDEFINED   -1

/* Color class */
typedef double  PColor[3];

/* Color class public operations */
void            Color_RGBtoHSV(PColor, PColor);
void            Color_HSVtoRGB(PColor, PColor);

#endif
