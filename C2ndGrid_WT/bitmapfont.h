#ifndef _BITMAPFONT_H_
#define _BITMAPFONT_H_

#ifdef WIN32
#undef UNICODE
#undef _UNICODE
#include <windows.h>

#define MAX_MOUSE_POINTS 100
#endif

#include <GL/gl.h>
#include <GL/glu.h>

#include "InstanceData.h"

#ifdef __cplusplus 
extern "C"{
#endif 

GLvoid BuildFont(struct InstanceData* id);											// Build Our Bitmap Font
//GLvoid BuildFontCustom(struct InstanceData* id, int fontheight, char *fontname);	// Build Our Bitmap Font
//int BuildNewFont(struct InstanceData* id, HDC hDC, HGLRC hRC);						// Build Our Bitmap Font
GLvoid glPrint(struct InstanceData* id, const char *fmt, ...);						// Custom GL "Print" Routine
GLvoid glPrintFont(struct InstanceData* id, GLint base, const char *fmt, ...);		// Custom GL "Print" Routine
GLvoid glPrintCustom(struct InstanceData* id, GLuint fontbase, const char *fmt, ...);// Custom GL "Print" Routine
GLvoid glPrintGUIText(struct InstanceData* id, const char *fmt, ...);				// Custom GL "Print" Routine
GLvoid glPrintGUITextBase(struct InstanceData* id, GLuint fontbase, const char *fmt, ...);
//GLvoid KillFont(struct InstanceData* id);											// Delete The Font List
GLvoid SetFontIndex(struct InstanceData* id, int Idx);
GLvoid SetGUIFontIndex(struct InstanceData* id, int Idx, int SetHDCflag );
GLvoid SetVerdanaFontIndex(struct InstanceData* id, int Idx, int SetHDCflag );
GLvoid BuildCustomFonts(struct InstanceData* id);
GLvoid BuildGUIFonts(struct InstanceData* id);
GLvoid BuildVerdanaFonts(struct InstanceData* id);
GLvoid BuildFreeTypeFonts(struct InstanceData* id);
void PrintFreeType(struct InstanceData* id, int Idx, float x, float y, const float* color, const char *fmt, ...);


#ifdef __cplusplus 
}
#endif 

#endif//_BITMAPFONT_H_