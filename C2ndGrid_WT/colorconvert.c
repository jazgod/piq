#include "colorconvert.h"
#include "colordefs.h"

/* Color_RGBtoHSV - convert color from RGB to HSV representation */
void
Color_RGBtoHSV(rgb, hsv)
	PColor          rgb;	/* RGB color */
	PColor          hsv;	/* HSV color */
{
	double          min,
	                max;
	double          delta;

	/* find HSV value */
	min = Min(Min(rgb[RED], rgb[GREEN]), rgb[BLUE]);
	max = Max(Max(rgb[RED], rgb[GREEN]), rgb[BLUE]);
	hsv[VALUE] = max;

	/* find HSV saturation */
	hsv[SATURATION] = (max != 0) ? (max - min) / max : 0;

	/* find HSV hue */
	if (hsv[SATURATION] == 0)
		hsv[HUE] = UNDEFINED;
	else {
		delta = max - min;
		if (rgb[RED] == max)
			hsv[HUE] = (rgb[GREEN] - rgb[BLUE]) / delta;
		else if (rgb[GREEN] == max)
			hsv[HUE] = 2 + (rgb[BLUE] - rgb[RED]) / delta;
		else if (rgb[BLUE] == max)
			hsv[HUE] = 4 + (rgb[RED] - rgb[GREEN]) / delta;

		/* convert hue to degrees */
		hsv[HUE] *= 60;
		if (hsv[HUE] < 0)
			hsv[HUE] += 360;
	}
}

/* Color_HSVtoRGB - convert color from HSV to RGB representation */
void
Color_HSVtoRGB(PColor hsv, PColor rgb )
{
	int             i;
	double          f,
	                p,
	                q,
	                t;
  double h, s, v;


  h = hsv[0];
  s = hsv[1];
  v = hsv[2];

	if (hsv[SATURATION] == 0) {
		/* achromatic color: there is no hue */
		if (hsv[HUE] == UNDEFINED)
			rgb[RED] = rgb[GREEN] = rgb[BLUE] = hsv[VALUE];
		/* else error */
	} else {
		/* convert hue to [1,6] range */
		if (hsv[HUE] == 360)
			hsv[HUE] = 0;
		hsv[HUE] /= 60;
		i = (int)(hsv[HUE]);

		/* find R, G and B components */
		f = hsv[HUE] - i;
		p = hsv[VALUE] * (1 - hsv[SATURATION]);
		q = hsv[VALUE] * (1 - hsv[SATURATION] * f);
		t = hsv[VALUE] * (1 - hsv[SATURATION] * (1 - f));
		switch (i) {
		case 0:
			rgb[RED] = hsv[VALUE];
			rgb[GREEN] = t;
			rgb[BLUE] = p;
			break;

		case 1:
			rgb[RED] = q;
			rgb[GREEN] = hsv[VALUE];
			rgb[BLUE] = p;
			break;

		case 2:
			rgb[RED] = p;
			rgb[GREEN] = hsv[VALUE];
			rgb[BLUE] = t;
			break;

		case 3:
			rgb[RED] = p;
			rgb[GREEN] = q;
			rgb[BLUE] = hsv[VALUE];
			break;

		case 4:
			rgb[RED] = t;
			rgb[GREEN] = p;
			rgb[BLUE] = hsv[VALUE];
			break;

		case 5:
			rgb[RED] = hsv[VALUE];
			rgb[GREEN] = p;
			rgb[BLUE] = q;
			break;
		}
	}
}

