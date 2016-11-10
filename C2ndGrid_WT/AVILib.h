//////////////////////////////////////////////////////////////////////
//
// AVILib.h
//
// Structure to keep track of the video recording parameters.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_AVILIB_H)
#define AFX_AVILIB_H

#include "Wingdi.h"
#include "vfw.h"

typedef struct _avilib   
{
	unsigned char * m_pVideoBuffer;		// Points to OpenGL saved Video Buffer
	int m_bRecordVideoButton;			// Record video button. 0 - inactive, 1 - active
	int m_bRecordVideo;					// Record video flag. 0 - inactive, 1 - active
	int m_bVideoBufferReady;			// Video buffer is available
	int m_nVideoWidth;					// Width of copied buffer
	int m_nVideoHeight;					// Height of copied buffer
	DWORD m_dwRate;						// fps
	char * m_sError;					// last error message
	long   m_lFrame;					// frame counter
	long   m_lRecDurSec;                // Max record duration in seconds. 0 - None
	time_t m_RecStart;                  // Time the recording started
	BITMAPINFOHEADER m_bih;				// single stream info
    IAVIFile * m_pAVIFile;				// AVI File Interface Pointer
	IAVIStream * m_pStream;				// Address of the stream interface
	IAVIStream * m_pStreamCompressed;	// Address of the compressed video stream
	char m_videoFileName[512];			// AVI File name
} AVILIB;

#endif // !defined(AFX_AVILIB_H)
