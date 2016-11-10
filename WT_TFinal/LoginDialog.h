#ifndef __LOGINDIALOG_H__
#define __LOGINDIALOG_H__
/**
 *==============================================================================
 *
 * Filename:         LoginDialog.h
 *
 *==============================================================================
 *
 *  Copyright 2006  Megadata All Rights Reserved
 *
 *==============================================================================
 *
 * Description:
 *  Contains macros, typedefs, and prototypes for login and change
 *  password functions.
 *
 *==============================================================================
 * 
 * Revision History:
 *
 *  Mar. 16, 2006    E.Safranek
 *  Added login result codes.
 *  Oct. 05, 2006    E.Safranek
 *  Added change user password functionality.
 * 
 *==============================================================================
 */



#define LOGIN_MAX_SZ	256		/* Max user id and passwd string lengths			*/
#define MIN_PWD_LEN 6       /* Minimum password length */

//@@@ extern int CkLogin( char *username, char *password );

#define LOGIN_SUCCESS 0
#define LOGIN_FAILED 1
#define LOGIN_CONNECT_FAILED 2      // login failed because we could not connect
#define LOGIN_ERROR 3

BOOL CALLBACK LoginDlgCB(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK PasswordDlgCB(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
#endif
