#include <stdio.h>

#include "bitmapfont.h"

// FreeType Headers
#include <ft2build.h>
#include <freetype.h>
#include <ftglyph.h>
#include <ftoutln.h>
#include <fttrigon.h>

#include "glutil.h"

// External Prototypes
char *GetLastErrorStr();
extern int GetLocalFile( struct InstanceData* id, const char* localpath, const char* filename, const char* remotepath, int eMode);
int	vo_log_info( va_alist );
int	vo_log_error( va_alist );

GLvoid BuildFont(struct InstanceData* id)				// Build Our Bitmap Font
{
	
	HFONT	oldfont;									// Used For Good House Keeping

	id->m_gluBase = glGenLists(223);					// Storage For 223 Characters

	id->m_hCustomFont = CreateFont(	-12,				// Height Of Font
						0,								// Width Of Font
						0,								// Angle Of Escapement
						0,								// Orientation Angle
						FW_BOLD,						// Font Weight
						FALSE,							// Italic
						FALSE,							// Underline
						FALSE,							// Strikeout
						ANSI_CHARSET,					// Character Set Identifier
						OUT_TT_PRECIS,					// Output Precision
						CLIP_DEFAULT_PRECIS,			// Clipping Precision
						ANTIALIASED_QUALITY,			// Output Quality
						FF_DONTCARE|DEFAULT_PITCH,		// Family And Pitch
						"Veranda");						// Font Name

	oldfont = (HFONT)SelectObject(id->m_hDC, id->m_hCustomFont);	// Selects The Font We Want
	if(!wglUseFontBitmaps(id->m_hDC, 32, 223, id->m_gluBase))			// Builds 223 Characters Starting At Character 32
		OutputDebugString(GetLastErrorStr());
	//SelectObject(hDC, oldfont);							// Selects The Font We Want
	//  DeleteObject(font);									// Delete The Font
	GetTextMetrics(id->m_hDC, &id->m_tmBitmap );
}


int BuildNewFont(struct InstanceData* id, HDC hDC, HGLRC hRC)								// Build Our Bitmap Font
{
	GLuint base;
	HFONT	oldfont;									// Used For Good House Keeping

	base = glGenLists(223);								// Storage For 223 Characters

	id->m_hCustomFont = CreateFont(	-12,				// Height Of Font
						0,								// Width Of Font
						0,								// Angle Of Escapement
						0,								// Orientation Angle
						FW_BOLD,						// Font Weight
						FALSE,							// Italic
						FALSE,							// Underline
						FALSE,							// Strikeout
						ANSI_CHARSET,					// Character Set Identifier
						OUT_TT_PRECIS,					// Output Precision
						CLIP_DEFAULT_PRECIS,			// Clipping Precision
						ANTIALIASED_QUALITY,			// Output Quality
						FF_DONTCARE|DEFAULT_PITCH,		// Family And Pitch
						"Courier New");					// Font Name

	oldfont = (HFONT)SelectObject(hDC, id->m_hCustomFont);           // Selects The Font We Want
	wglUseFontBitmaps(hDC, 32, 223, base);				// Builds 223 Characters Starting At Character 32
	//SelectObject(hDC, oldfont);							// Selects The Font We Want
	//  DeleteObject(font);									// Delete The Font
  GetTextMetrics(hDC, &id->m_tmBitmap );
  return( base );
}




GLvoid KillFont(struct InstanceData* id)				// Delete The Font List
{
	glDeleteLists(id->m_gluBase, 223);					// Delete All 223 Characters
}


GLvoid glPrint(struct InstanceData* id, const char *fmt, ...)					// Custom GL "Print" Routine
{
	char		text[256];								// Holds Our String
	va_list		ap;										// Pointer To List Of Arguments

	if (fmt == NULL)									// If There's No Text
		return;											// Do Nothing

	va_start(ap, fmt);									// Parses The String For Variables
	    vsprintf_s(text, _countof(text), fmt, ap);		// And Converts Symbols To Actual Numbers
	va_end(ap);											// Results Are Stored In Text

	if (!id->m_bFontLoaded) {
		id->OuterGLPrint(text);
		return;
	}
	glPushAttrib(GL_LIST_BIT);							// Pushes The Display List Bits
	glListBase(id->m_gluBase - 32);						// Sets The Base Character to 32
	glCallLists(strlen(text), GL_UNSIGNED_BYTE, text);	// Draws The Display List Text
	glPopAttrib();										// Pops The Display List Bits
}



GLvoid glPrintFont(struct InstanceData* id, GLint base, const char *fmt, ...)					// Custom GL "Print" Routine
{
	char		text[256];								// Holds Our String
	va_list		ap;										// Pointer To List Of Arguments

	if (fmt == NULL)									// If There's No Text
		return;											// Do Nothing

	va_start(ap, fmt);									// Parses The String For Variables
	    vsprintf_s(text, _countof(text), fmt, ap);		// And Converts Symbols To Actual Numbers
	va_end(ap);											// Results Are Stored In Text

	glPushAttrib(GL_LIST_BIT);							// Pushes The Display List Bits
	glListBase(base - 32);								// Sets The Base Character to 32
	glCallLists(strlen(text), GL_UNSIGNED_BYTE, text);	// Draws The Display List Text
	glPopAttrib();										// Pops The Display List Bits
}

GLvoid BuildCustomFonts(struct InstanceData* id)		// Build Our Bitmap Font
{
  	const DWORD cdwThreadId = GetCurrentThreadId();
	GLsizei size = 223;
	int nBaseFontSize = 12;	
	int nIdx = 0;

	for(nIdx = 0; nIdx < FONTCOUNT; nIdx++, nBaseFontSize += 2){
		id->m_arFonts[nIdx].m_glListBase = glGenLists(size);			// Storage For 223 Characters
		if(!id->m_arFonts[nIdx].m_glListBase){
			GLenum e = glGetError();
			switch (e) 
			{
			case GL_NO_ERROR:
				OutputDebugString("GL_NO_ERROR\n");
				vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Unable to create OpenGlLists for Font (%d), error: GL_NO_ERROR\n", cdwThreadId, nBaseFontSize);
				break;
			case GL_INVALID_VALUE:
				OutputDebugString("GL_INVALID_VALUE\n");
				vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Unable to create OpenGlLists for Font (%d), error: GL_INVALID_VALUE\n", cdwThreadId, nBaseFontSize);
				break;
			case GL_INVALID_OPERATION:
				OutputDebugString("GL_INVALID_OPERATION\n");
				vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Unable to create OpenGlLists for Font (%d), error: GL_INVALID_OPERATION\n", cdwThreadId, nBaseFontSize);
				break;
			};			
		}

		id->m_arFonts[nIdx].m_hFont = CreateFont(nBaseFontSize,					// Height Of Font
						0,								// Width Of Font
						0,								// Angle Of Escapement
						0,								// Orientation Angle
						FW_BOLD,						// Font Weight
						FALSE,							// Italic
						FALSE,							// Underline
						FALSE,							// Strikeout
						ANSI_CHARSET,					// Character Set Identifier
						OUT_TT_PRECIS,					// Output Precision
						CLIP_DEFAULT_PRECIS,			// Clipping Precision
						ANTIALIASED_QUALITY,			// Output Quality
						FF_DONTCARE|DEFAULT_PITCH,		// Family And Pitch
						"Courier New");					// Font Name

		SelectObject(id->m_hDC, id->m_arFonts[nIdx].m_hFont);			// Selects The Font We Want
		GetTextMetrics(id->m_hDC, &id->m_arFonts[nIdx].m_tmCustom );	// Load The TextMetrics for the font
	
		if(!wglUseFontBitmaps(id->m_hDC, 32, size, id->m_arFonts[nIdx].m_glListBase)){		// Builds 223 Characters Starting At Character 32
			GLenum e = glGetError();
			char* strError = GetLastErrorStr();
			switch (e) 
			{
			case GL_NO_ERROR:
				OutputDebugString("GL_NO_ERROR\n");
				vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Unable to create OpenGL font Bitmaps for Font (%d), error: GL_NO_ERROR\n", cdwThreadId, nBaseFontSize);
				break;
			case GL_INVALID_VALUE:
				OutputDebugString("GL_INVALID_VALUE\n");
				vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Unable to create OpenGL font Bitmaps for Font (%d), error: GL_INVALID_VALUE\n", cdwThreadId, nBaseFontSize);
				break;
			case GL_INVALID_OPERATION:
				OutputDebugString("GL_INVALID_OPERATION\n");
				vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Unable to create OpenGL font Bitmaps for Font (%d), error: GL_INVALID_OPERATION\n", cdwThreadId, nBaseFontSize);
				break;
			};
			OutputDebugString(strError);
			vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Unable to create OpenGL font Bitmaps for Font (%d), error: %s\n", cdwThreadId, nBaseFontSize, strError);
		}		
	}
	id->m_bFontLoaded = TRUE;
}

int next_p2 ( int a )
{
	int rval=1;
	while(rval<a) rval<<=1;
	return rval;
}

///Create a display list coresponding to the give character.
int MakeFreeTypeDisplayList( FREETYPEFONT* font, FT_Face face, char ch, GLuint list_base, GLuint * tex_base ) {
	FT_Glyph glyph;
	FT_BitmapGlyph bitmap_glyph;
	GLubyte* expanded_data = NULL;
	FT_Error error;
	unsigned int width;
	unsigned int height;
	unsigned int j, i;
	float x,y;
	//The first thing we do is get FreeType to render our character
	//into a bitmap.  This actually requires a couple of FreeType commands:

	//Load the Glyph for our character.
	if(error = FT_Load_Glyph( face, FT_Get_Char_Index( face, ch ), FT_LOAD_DEFAULT )){
		//throw std::runtime_error("FT_Load_Glyph failed");
		return FALSE;
	}

	//Move the face's glyph into a Glyph object.
   
	if(error = FT_Get_Glyph( face->glyph, &glyph )){
		//throw std::runtime_error("FT_Get_Glyph failed");
		return FALSE;
	}

	//Convert the glyph to a bitmap.
	if( error = FT_Glyph_To_Bitmap( &glyph, ft_render_mode_normal, 0, 1 ) ){
		return FALSE;
	}
    bitmap_glyph = (FT_BitmapGlyph)glyph;

	//This reference will make accessing the bitmap easier
	//FT_Bitmap& bitmap=bitmap_glyph->bitmap;

	//Use our helper function to get the widths of
	//the bitmap data that we will need in order to create
	//our texture.
	width = next_p2( bitmap_glyph->bitmap.width );
	height = next_p2( bitmap_glyph->bitmap.rows );

	//Allocate memory for the texture data.
	expanded_data = calloc( 2 * width * height, sizeof(GLubyte));

	//Here we fill in the data for the expanded bitmap.
	//Notice that we are using two channel bitmap (one for
	//luminocity and one for alpha), but we assign
	//both luminocity and alpha to the value that we
	//find in the FreeType bitmap. 
	//We use the ?: operator so that value which we use
	//will be 0 if we are in the padding zone, and whatever
	//is the the Freetype bitmap otherwise.
	for(j=0; j <height;j++) {
		for(i=0; i < width; i++){
			expanded_data[2*(i+j*width)]= expanded_data[2*(i+j*width)+1] = 
				(i>=bitmap_glyph->bitmap.width || j>=bitmap_glyph->bitmap.rows) ?
				0 : bitmap_glyph->bitmap.buffer[i + bitmap_glyph->bitmap.width*j];
		}
	}


	//Now we just setup some texture paramaters.
	glBindTexture( GL_TEXTURE_2D, tex_base[ch]);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);

	//Here we actually create the texture itself, notice
	//that we are using GL_LUMINANCE_ALPHA to indicate that
	//we are using 2 channel data.
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, width, height,
		  0, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, expanded_data );

	//With the texture created, we don't need to expanded data anymore
    free( expanded_data );
	expanded_data = NULL;

	//So now we can create the display list
	glNewList(font->m_glListBase+ch,GL_COMPILE);

	glBindTexture(GL_TEXTURE_2D,tex_base[ch]);

	glPushMatrix();

	//first we need to move over a little so that
	//the character has the right amount of space
	//between it and the one before it.
	glTranslatef((GLfloat)bitmap_glyph->left,0,0);

	//Now we move down a little in the case that the
	//bitmap extends past the bottom of the line 
	//(this is only true for characters like 'g' or 'y'.
	glTranslatef(0,(GLfloat)bitmap_glyph->top - bitmap_glyph->bitmap.rows,0);

	//Now we need to account for the fact that many of
	//our textures are filled with empty padding space.
	//We figure what portion of the texture is used by 
	//the actual character and store that information in 
	//the x and y variables, then when we draw the
	//quad, we will only reference the parts of the texture
	//that we contain the character itself.
	x=(float)bitmap_glyph->bitmap.width / (float)width;
	y=(float)bitmap_glyph->bitmap.rows / (float)height;

	//Here we draw the texturemaped quads.
	//The bitmap that we got from FreeType was not 
	//oriented quite like we would like it to be,
	//so we need to link the texture to the quad
	//so that the result will be properly aligned.
	glBegin(GL_QUADS);
	glTexCoord2d(0,0); glVertex2f(0,(GLfloat)bitmap_glyph->bitmap.rows);
	glTexCoord2d(0,y); glVertex2f(0,0);
	glTexCoord2d(x,y); glVertex2f((GLfloat)bitmap_glyph->bitmap.width,0);
	glTexCoord2d(x,0); glVertex2f((GLfloat)bitmap_glyph->bitmap.width,(GLfloat)bitmap_glyph->bitmap.rows);
	glEnd();
	glPopMatrix();
	glTranslatef((GLfloat)(face->glyph->advance.x >> 6) ,0,0);


	//increment the raster position as if we were a bitmap font.
	//(only needed if you want to calculate text length)
	//glBitmap(0,0,0,0,face->glyph->advance.x >> 6,0,NULL);

	//Finnish the display list
	glEndList();
	return TRUE;
}

int FreeTypeInit(FREETYPEFONT* font, const char * fname, unsigned int h) {
	FT_Library library;
	FT_Face face;
	FT_Error error;
	unsigned char i;
	font->m_glTextures = calloc(128, sizeof(GLuint));
	font->m_fHeight = (float)h;

	//Create and initilize a freetype font library.
	if (error = FT_Init_FreeType( &library )){ 
		return FALSE;
	}

	if (error = FT_New_Face( library, fname, 0, &face )){
		//throw std::runtime_error("FT_New_Face failed (there is probably a problem with your font file)");
		return FALSE;
	}

	//For some twisted reason, Freetype measures font size
	//in terms of 1/64ths of pixels.  Thus, to make a font
	//h pixels high, we need to request a size of h*64.
	//(h << 6 is just a prettier way of writting h*64)
	if (error = FT_Set_Char_Size( face, h << 6, h << 6, 96, 96)){
		return FALSE;
	}

	//Here we ask opengl to allocate resources for
	//all the textures and displays lists which we
	//are about to create.  
	font->m_glListBase = glGenLists(128);
	glGenTextures( 128, font->m_glTextures );

	//This is where we actually create each of the fonts display lists.
	for(i=0; i < 128; i++){
		if(!MakeFreeTypeDisplayList(font, face, i, font->m_glListBase, font->m_glTextures)){
		}
	}

	//We don't need the face information now that the display
	//lists have been created, so we free the assosiated resources.
	if( error = FT_Done_Face(face) ){
		return FALSE;
	}

	//Ditto for the library.
	if( error = FT_Done_FreeType(library) ){
		return FALSE;
	}

	return TRUE;
}

void FreeTypeClean(FREETYPEFONT* font) {
	glDeleteLists(font->m_glListBase,128);
	font->m_glListBase = 0;
	glDeleteTextures(128, font->m_glTextures);
	free(font->m_glTextures);
	font->m_glTextures = NULL;
}


void PrintFreeType(struct InstanceData* id, int Idx, float x, float y, const float* color, const char *fmt, ...)  {
	
	// We want a coordinate system where things coresponding to window pixels.
	char		text[256];								// Holds Our String
	va_list		ap;										// Pointer To List Of Arguments
	GLuint font;
	float h;
	int glError = GL_NO_ERROR;
	//float modelview_matrix[16];
	unsigned int i=0;
	GLint	viewport[4];
	if(Idx < 0 || Idx > (FONTCOUNT - 1))
	{
		Idx = id->m_nCustFontIdx;
	}

	// Clear all OpenGL Errors
	while(GL_NO_ERROR != glGetError());	
	
	GL_ASSERT(glColor3fv(color));
	GL_ASSERT_WITH_LOGSTATE(glPushAttrib(GL_TRANSFORM_BIT), GL_MAX_ATTRIB_STACK_DEPTH, GL_ATTRIB_STACK_DEPTH);	
	GL_ASSERT(glGetIntegerv(GL_VIEWPORT, viewport));	
	GL_ASSERT(glMatrixMode(GL_PROJECTION));	
	GL_ASSERT_WITH_LOGSTATE(glPushMatrix(), GL_MAX_PROJECTION_STACK_DEPTH, GL_PROJECTION_STACK_DEPTH);
	GL_ASSERT(glLoadIdentity());	
	GL_ASSERT(gluOrtho2D(viewport[0],viewport[2],viewport[1],viewport[3]));
	GL_ASSERT_WITH_LOGSTATE(glPopAttrib(), GL_MAX_ATTRIB_STACK_DEPTH, GL_ATTRIB_STACK_DEPTH);
	
	font=id->m_arFTFonts[Idx].m_glListBase;
	h=id->m_arFTFonts[Idx].m_fHeight/.63f;						//We make the height about 1.5* that of
	

	if (fmt == NULL)									// If There's No Text
		*text=0;											// Do Nothing

	else {
		va_start(ap, fmt);									// Parses The String For Variables
			vsprintf_s(text, 256, fmt, ap);						// And Converts Symbols To Actual Numbers
		va_end(ap);											// Results Are Stored In Text
	}
	vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X)[%d] PrintFreeType <%s>\n", GetCurrentThreadId(), __LINE__, text);

	//Here is some code to split the text that we have been
	//given into a set of lines.  
	//This could be made much neater by using
	//a regular expression library such as the one avliable from
	//boost.org (I've only done it out by hand to avoid complicating
	//this tutorial with unnecessary library dependencies).
	/*const char *start_line=text;
	vector<string> lines;
	const char *c = NULL;
	for(c=text;*c;c++) {
		if(*c=='\n') {
			string line;
			for(const char *n=start_line;n<c;n++) line.append(1,*n);
			lines.push_back(line);
			start_line=c+1;
		}
	}
	if(start_line) {
		string line;
		for(const char *n=start_line;n<c;n++) line.append(1,*n);
		lines.push_back(line);
	}*/

	GL_ASSERT_WITH_LOGSTATE(glPushAttrib(GL_LIST_BIT | GL_CURRENT_BIT | GL_ENABLE_BIT | GL_TRANSFORM_BIT), GL_MAX_ATTRIB_STACK_DEPTH, GL_ATTRIB_STACK_DEPTH);	
	GL_ASSERT(glMatrixMode(GL_MODELVIEW));
	GL_ASSERT(glDisable(GL_LIGHTING));
	GL_ASSERT(glEnable(GL_TEXTURE_2D));
	GL_ASSERT(glDisable(GL_DEPTH_TEST));
	GL_ASSERT(glEnable(GL_BLEND));
	GL_ASSERT(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
	GL_ASSERT(glListBase(font));

	//glGetFloatv(GL_MODELVIEW_MATRIX, modelview_matrix);

	//This is where the text display actually happens.
	//For each line of text we reset the modelview matrix
	//so that the line's text will start in the correct position.
	//Notice that we need to reset the matrix, rather than just translating
	//down by h. This is because when each character is
	//draw it modifies the current matrix so that the next character
	//will be drawn immediatly after it.  
	//for(unsigned int i=0;i<lines.size();i++) {
		

		GL_ASSERT(glPushMatrix());
		GL_ASSERT(glLoadIdentity());
		GL_ASSERT(glTranslatef(x,y-h*i,0));
		//glMultMatrixf(modelview_matrix);
		

	//  The commented out raster position stuff can be useful if you need to
	//  know the length of the text that you are creating.
	//  If you decide to use it make sure to also uncomment the glBitmap command
	//  in make_dlist().
	//	glRasterPos2f(0,0);		
		//GL_ASSERT(glColor3fv(color));
		//GL_ASSERT(glCallLists(strlen(text), GL_UNSIGNED_BYTE, text));
		glCallLists(strlen(text), GL_UNSIGNED_BYTE, text);
#ifdef _DEBUG
		while(GL_NO_ERROR != (glError = glGetError()))
			vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X)[%d] OpenGL glCallLists error <0x%x:%s>\n", GetCurrentThreadId(), __LINE__, glError, gl_errors[glError - GL_INVALID_ENUM]);
#endif//_DEBUG
	//	float rpos[4];
	//	glGetFloatv(GL_CURRENT_RASTER_POSITION ,rpos);
	//	float len=x-rpos[0];

		GL_ASSERT_WITH_LOGSTATE(glPopMatrix(), GL_MAX_MODELVIEW_STACK_DEPTH, GL_MODELVIEW_STACK_DEPTH);

	//}	

	GL_ASSERT_WITH_LOGSTATE(glPopAttrib(), GL_MAX_ATTRIB_STACK_DEPTH, GL_ATTRIB_STACK_DEPTH);
	GL_ASSERT_WITH_LOGSTATE(glPushAttrib(GL_TRANSFORM_BIT), GL_MAX_ATTRIB_STACK_DEPTH, GL_ATTRIB_STACK_DEPTH);
	GL_ASSERT(glMatrixMode(GL_PROJECTION));
	GL_ASSERT_WITH_LOGSTATE(glPopMatrix(), GL_MAX_PROJECTION_STACK_DEPTH, GL_PROJECTION_STACK_DEPTH);
	GL_ASSERT_WITH_LOGSTATE(glPopAttrib(), GL_MAX_ATTRIB_STACK_DEPTH, GL_ATTRIB_STACK_DEPTH);	
}

GLvoid BuildFreeTypeFonts(struct InstanceData* id)
{
	char fontfile[MAX_PATH] = {0};
	int i = 0;
	int nBaseFontSize = 8;	
	const DWORD cdwThreadId = GetCurrentThreadId();

	sprintf_s(fontfile, MAX_PATH, "%s\\Test.ttf", id->m_strFontDir );
	if( !GetLocalFile(id, id->m_strFontDir, "Test.ttf", id->m_strHttpFontPath, 1) ){
		vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) could not open/find file '%s'\n", cdwThreadId, fontfile );
		return;
	}
	
	for(i = 0; i < FONTCOUNT; i++, nBaseFontSize++)
	{
		FreeTypeInit(&id->m_arFTFonts[i], fontfile, nBaseFontSize);
	}
}

GLvoid SetFontIndex(struct InstanceData* id, int nIdx){
	const DWORD cdwThreadId = GetCurrentThreadId();
	if(nIdx >= 0 || nIdx < FONTCOUNT){
		id->m_nCustFontIdx = nIdx;
		id->m_gluCustomBase = id->m_arFonts[nIdx].m_glListBase;
		id->m_hCustomFont = id->m_arFonts[nIdx].m_hFont;
		// Selecting Font into DC needed because we use it to calculate Large Tag Width (Roll Panel)
		SelectObject(id->m_hDC, id->m_hCustomFont);
		memcpy(&id->m_tmCustom, &id->m_arFonts[nIdx].m_tmCustom, sizeof(TEXTMETRIC));
		vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Setting font index to (%d), %d Point \"Courier New\"\n", cdwThreadId, id->m_nCustFontIdx, id->m_tmCustom.tmHeight );
	}
}




GLvoid BuildVerdanaFonts(struct InstanceData* id)		// Build Our Bitmap Font
{
  const DWORD cdwThreadId = GetCurrentThreadId();
	GLsizei size = 223;
	int nBaseFontSize = 8;	
	int nIdx = 0;

	for (nIdx = 0; nIdx < GUI_FONTCOUNT; nIdx++, nBaseFontSize += 2 ){
		id->m_verdanaFonts[nIdx].m_glListBase = glGenLists(size);			// Storage For 223 Characters
		if(!id->m_verdanaFonts[nIdx].m_glListBase){
			GLenum e = glGetError();
			switch (e) 
			{
			case GL_NO_ERROR:
				OutputDebugString("GL_NO_ERROR\n");
				vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Unable to create OpenGlLists for Font (%d), error: GL_NO_ERROR\n", cdwThreadId, nBaseFontSize);
				break;
			case GL_INVALID_VALUE:
				OutputDebugString("GL_INVALID_VALUE\n");
				vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Unable to create OpenGlLists for Font (%d), error: GL_INVALID_VALUE\n", cdwThreadId, nBaseFontSize);
				break;
			case GL_INVALID_OPERATION:
				OutputDebugString("GL_INVALID_OPERATION\n");
				vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Unable to create OpenGlLists for Font (%d), error: GL_INVALID_OPERATION\n", cdwThreadId, nBaseFontSize);
				break;
			};			
		}

		id->m_verdanaFonts[nIdx].m_hFont = CreateFont(nBaseFontSize,					// Height Of Font
						0,								// Width Of Font
						0,								// Angle Of Escapement
						0,								// Orientation Angle
						FW_REGULAR,						// Font Weight FW_REGULAR or FW_BOLD or FW_NORMAL
						FALSE,							// Italic
						FALSE,							// Underline
						FALSE,							// Strikeout
						ANSI_CHARSET,					// Character Set Identifier
						OUT_TT_PRECIS,					// Output Precision
						CLIP_DEFAULT_PRECIS,			// Clipping Precision
						// ANTIALIASED_QUALITY,			// Output Quality
						CLEARTYPE_QUALITY,	  // Output Quality
						FF_DONTCARE|DEFAULT_PITCH,		// Family And Pitch
						"Verdana");					// Font Name

		SelectObject(id->m_hDC, id->m_verdanaFonts[nIdx].m_hFont);			// Selects The Font We Want
		GetTextMetrics(id->m_hDC, &id->m_verdanaFonts[nIdx].m_tmCustom );	// Load The TextMetrics for the font
	
		if(!wglUseFontBitmaps(id->m_hDC, 32, size, id->m_verdanaFonts[nIdx].m_glListBase)){		// Builds 223 Characters Starting At Character 32
			GLenum e = glGetError();
			char* strError = GetLastErrorStr();
			switch (e) 
			{
			case GL_NO_ERROR:
				OutputDebugString("GL_NO_ERROR\n");
				vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Unable to create OpenGL font Bitmaps for Font (%d), error: GL_NO_ERROR\n", cdwThreadId, nBaseFontSize);
				break;
			case GL_INVALID_VALUE:
				OutputDebugString("GL_INVALID_VALUE\n");
				vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Unable to create OpenGL font Bitmaps for Font (%d), error: GL_INVALID_VALUE\n", cdwThreadId, nBaseFontSize);
				break;
			case GL_INVALID_OPERATION:
				OutputDebugString("GL_INVALID_OPERATION\n");
				vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Unable to create OpenGL font Bitmaps for Font (%d), error: GL_INVALID_OPERATION\n", cdwThreadId, nBaseFontSize);
				break;
			};
			OutputDebugString(strError);
			vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Unable to create OpenGL font Bitmaps for Font (%d), error: %s\n", cdwThreadId, nBaseFontSize, strError);
		}		
	}
}




GLvoid BuildGUIFonts(struct InstanceData* id)		// Build Our Bitmap Font
{
  const DWORD cdwThreadId = GetCurrentThreadId();
	GLsizei size = 223;
	int nBaseFontSize = 9;	
	int nIdx = 0;

	for(nIdx = 0; nIdx < GUI_FONTCOUNT; nIdx++, nBaseFontSize += 2 ){
		id->m_guiFonts[nIdx].m_glListBase = glGenLists(size);			// Storage For 223 Characters
		if(!id->m_guiFonts[nIdx].m_glListBase){
			GLenum e = glGetError();
			switch (e) 
			{
			case GL_NO_ERROR:
				OutputDebugString("GL_NO_ERROR\n");
				vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Unable to create OpenGlLists for Font (%d), error: GL_NO_ERROR\n", cdwThreadId, nBaseFontSize);
				break;
			case GL_INVALID_VALUE:
				OutputDebugString("GL_INVALID_VALUE\n");
				vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Unable to create OpenGlLists for Font (%d), error: GL_INVALID_VALUE\n", cdwThreadId, nBaseFontSize);
				break;
			case GL_INVALID_OPERATION:
				OutputDebugString("GL_INVALID_OPERATION\n");
				vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Unable to create OpenGlLists for Font (%d), error: GL_INVALID_OPERATION\n", cdwThreadId, nBaseFontSize);
				break;
			};			
		}

		id->m_guiFonts[nIdx].m_hFont = CreateFont(nBaseFontSize,					// Height Of Font
						0,								// Width Of Font
						0,								// Angle Of Escapement
						0,								// Orientation Angle
						FW_BOLD,						// Font Weight
						FALSE,							// Italic
						FALSE,							// Underline
						FALSE,							// Strikeout
						ANSI_CHARSET,					// Character Set Identifier
						OUT_TT_PRECIS,					// Output Precision
						CLIP_DEFAULT_PRECIS,			// Clipping Precision
						ANTIALIASED_QUALITY,			// Output Quality
						FF_DONTCARE|DEFAULT_PITCH,		// Family And Pitch
						"Arial");					// Font Name

		SelectObject(id->m_hDC, id->m_guiFonts[nIdx].m_hFont);			// Selects The Font We Want
		GetTextMetrics(id->m_hDC, &id->m_guiFonts[nIdx].m_tmCustom );	// Load The TextMetrics for the font
	
		if(!wglUseFontBitmaps(id->m_hDC, 32, size, id->m_guiFonts[nIdx].m_glListBase)){		// Builds 223 Characters Starting At Character 32
			GLenum e = glGetError();
			char* strError = GetLastErrorStr();
			switch (e) 
			{
			case GL_NO_ERROR:
				OutputDebugString("GL_NO_ERROR\n");
				vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Unable to create OpenGL font Bitmaps for Font (%d), error: GL_NO_ERROR\n", cdwThreadId, nBaseFontSize);
				break;
			case GL_INVALID_VALUE:
				OutputDebugString("GL_INVALID_VALUE\n");
				vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Unable to create OpenGL font Bitmaps for Font (%d), error: GL_INVALID_VALUE\n", cdwThreadId, nBaseFontSize);
				break;
			case GL_INVALID_OPERATION:
				OutputDebugString("GL_INVALID_OPERATION\n");
				vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Unable to create OpenGL font Bitmaps for Font (%d), error: GL_INVALID_OPERATION\n", cdwThreadId, nBaseFontSize);
				break;
			};
			OutputDebugString(strError);
			vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Unable to create OpenGL font Bitmaps for Font (%d), error: %s\n", cdwThreadId, nBaseFontSize, strError);
		}		
	}
}



GLvoid SetVerdanaFontIndex(struct InstanceData* id, int nIdx, int SetHDCflag ){
	const DWORD cdwThreadId = GetCurrentThreadId();
	if(nIdx >= 0 || nIdx < GUI_FONTCOUNT){
		id->m_nverdanaFontIdx = nIdx;
		id->m_gluverdanaBase = id->m_verdanaFonts[nIdx].m_glListBase;
		id->m_hverdanaFont = id->m_verdanaFonts[nIdx].m_hFont;
		if ( SetHDCflag ){
			// Selecting Font into DC needed because we use it to calculate Large Tag Width (Roll Panel)
			SelectObject(id->m_hDC, id->m_hverdanaFont);
			memcpy(&id->m_tmverdanaCustom, &id->m_verdanaFonts[nIdx].m_tmCustom, sizeof(TEXTMETRIC));
			vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Setting font index to (%d), %d Point \"VERDANA\"\n", cdwThreadId, id->m_nverdanaFontIdx, id->m_tmCustom.tmHeight );
		}
	}
}



GLvoid SetGUIFontIndex(struct InstanceData* id, int nIdx, int SetHDCflag ){
	const DWORD cdwThreadId = GetCurrentThreadId();
	if(nIdx >= 0 || nIdx < GUI_FONTCOUNT){
		id->m_nGUIFontIdx = nIdx;
		id->m_gluGUIBase = id->m_guiFonts[nIdx].m_glListBase;
		id->m_hGUIFont = id->m_guiFonts[nIdx].m_hFont;
		if ( SetHDCflag ){
			// Selecting Font into DC needed because we use it to calculate Large Tag Width (Roll Panel)
			SelectObject(id->m_hDC, id->m_hGUIFont);
			memcpy(&id->m_tmGUICustom, &id->m_guiFonts[nIdx].m_tmCustom, sizeof(TEXTMETRIC));
			vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Setting font index to (%d), %d Point \"VERDANA\"\n", cdwThreadId, id->m_nCustFontIdx, id->m_tmCustom.tmHeight );
		}
	}
}




GLvoid BuildFontCustom(struct InstanceData* id, int fontheight, char *fontname)		// Build Our Bitmap Font
{
  	HFONT	newfont;								// Used For Good House Keeping
	TEXTMETRIC tmCustom;
	const DWORD cdwThreadId = GetCurrentThreadId();
	GLsizei size = 223;
	GLuint base = 0, old = 0;
	
	
	base = glGenLists(size);			// Storage For 223 Characters
	if(!base){
		GLenum e = glGetError();
		switch (e) 
		{
		case GL_NO_ERROR:
			OutputDebugString("GL_NO_ERROR\n");
			break;
		case GL_INVALID_VALUE:
			OutputDebugString("GL_INVALID_VALUE\n");			
			break;
		case GL_INVALID_OPERATION:
			OutputDebugString("GL_INVALID_OPERATION\n");
			break;
		};
	}

	newfont = CreateFont(	fontheight,			// Height Of Font
						0,								// Width Of Font
						0,								// Angle Of Escapement
						0,								// Orientation Angle
						FW_BOLD,						// Font Weight
						FALSE,							// Italic
						FALSE,							// Underline
						FALSE,							// Strikeout
						ANSI_CHARSET,					// Character Set Identifier
						OUT_TT_PRECIS,					// Output Precision
						CLIP_DEFAULT_PRECIS,			// Clipping Precision
						ANTIALIASED_QUALITY,			// Output Quality
						FF_DONTCARE|DEFAULT_PITCH,		// Family And Pitch
						fontname);						// Font Name

	SelectObject(id->m_hDC, newfont);				// Selects The Font We Want
	GetTextMetrics(id->m_hDC, &tmCustom );

	memcpy(&id->m_tmCustom, &tmCustom, sizeof(TEXTMETRIC));
	id->m_hCustomFont = newfont;

	if(!wglUseFontBitmaps(id->m_hDC, 32, size, base)){		// Builds 223 Characters Starting At Character 32
		GLenum e = glGetError();
		switch (e) 
		{
		case GL_NO_ERROR:
			OutputDebugString("GL_NO_ERROR\n");
			break;
		case GL_INVALID_VALUE:
			OutputDebugString("GL_INVALID_VALUE\n");			
			break;
		case GL_INVALID_OPERATION:
			OutputDebugString("GL_INVALID_OPERATION\n");
			break;
		};
		OutputDebugString(GetLastErrorStr());
	}
	
	old = id->m_gluCustomBase;
	id->m_gluCustomBase = base;
	glDeleteLists(old, size);
}

GLvoid glPrintCustom(struct InstanceData* id, GLuint fontbase, const char *fmt, ...)				// Custom GL "Print" Routine
{
	//float modelview_matrix[16];
	char		text[1025] = {0};						// Holds Our String
	va_list		ap;										// Pointer To List Of Arguments

	if (fmt == NULL )									// If There's No Text
		return;											// Do Nothing

	va_start(ap, fmt);									// Parses The String For Variables
	    vsprintf_s(text, _countof(text), fmt, ap);		// And Converts Symbols To Actual Numbers
	va_end(ap);											// Results Are Stored In Text

	if (!id->m_bFontLoaded) {
		id->OuterGLPrintCustom(text);
		return;
	}
	
	glPushAttrib(GL_LIST_BIT);							// Pushes The Display List Bits

	if (fontbase){
		glListBase(fontbase - 32);				// Sets The Base Character to 32
	}
	else {
		// use default font
		glListBase(id->m_gluCustomBase - 32);				// Sets The Base Character to 32
	}

	glCallLists(strlen(text), GL_UNSIGNED_BYTE, text);	// Draws The Display List Text
	glPopAttrib();										// Pops The Display List Bits
	/*{
		if(!id->m_ftFont.m_glListBase){
			BuildFreeTypeFont(id);
		}
		glPushAttrib(GL_LIST_BIT | GL_CURRENT_BIT  | GL_ENABLE_BIT | GL_TRANSFORM_BIT);
		glDisable(GL_LIGHTING);
		glEnable(GL_TEXTURE_2D);
		glEnable(GL_BLEND);
		glDisable(GL_DEPTH_TEST);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glGetFloatv(GL_MODELVIEW_MATRIX, modelview_matrix);

		glPushMatrix();
		glLoadIdentity();
		//glTranslatef(x,y-h*i,0);
		glMultMatrixf(modelview_matrix);

		glListBase(id->m_ftFont.m_glListBase);// - 32);				// Sets The Base Character to 32
		glCallLists(strlen(text), GL_UNSIGNED_BYTE, text);	// Draws The Display List Text
		glPopMatrix();
	}*/

}



GLvoid glPrintGUIText(struct InstanceData* id, const char *fmt, ...)				// Custom GL "Print" Routine
{
	char		text[256] = {0};						// Holds Our String
	va_list		ap;										// Pointer To List Of Arguments

	if (fmt == NULL)									// If There's No Text
		return;											// Do Nothing

	va_start(ap, fmt);									// Parses The String For Variables
	    vsprintf_s(text, _countof(text), fmt, ap);		// And Converts Symbols To Actual Numbers
	va_end(ap);											// Results Are Stored In Text

	glPushAttrib(GL_LIST_BIT);							// Pushes The Display List Bits
	glListBase(id->m_gluGUIBase - 32);				// Sets The Base Character to 32
	glCallLists(strlen(text), GL_UNSIGNED_BYTE, text);	// Draws The Display List Text
	glPopAttrib();										// Pops The Display List Bits
}



GLvoid glPrintGUITextBase(struct InstanceData* id, GLuint fontbase, const char *fmt, ...)				// Custom GL "Print" Routine
{
	char		text[256] = {0};						// Holds Our String
	va_list		ap;										// Pointer To List Of Arguments

	if (fmt == NULL)									// If There's No Text
		return;											// Do Nothing

	va_start(ap, fmt);									// Parses The String For Variables
	    vsprintf_s(text, _countof(text), fmt, ap);		// And Converts Symbols To Actual Numbers
	va_end(ap);											// Results Are Stored In Text

	glPushAttrib(GL_LIST_BIT);							// Pushes The Display List Bits
	
	if ( fontbase ){
		glListBase(fontbase - 32);				// Sets The Base Character to 32
	} else {
		// use default font
		glListBase(id->m_gluGUIBase - 32);				// Sets The Base Character to 32
	}

	glCallLists(strlen(text), GL_UNSIGNED_BYTE, text);	// Draws The Display List Text
	glPopAttrib();										// Pops The Display List Bits
}
