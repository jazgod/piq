/**
 *==============================================================================
 *
 * Filename:         LoginDialog.c
 *
 *==============================================================================
 *
 *  Copyright 2006  Megadata All Rights Reserved
 *
 *==============================================================================
 *
 * Description:
 *  Contains functions for login and changing the password.
 *
 *==============================================================================
 * 
 * Revision History:
 *
 *  Mar. 16, 2007   E.Safranek
 *  Modified CkLogin to return more detailed result of login attempt.
 *  Modified ProcessLogin and LoginDlgCB to display more informative
 *  message on login failure so that we can distinquish between login
 *  failures due to bad user name / passwords, and login failures
 *  because we are unable to connect to the login server.
 *  
 *  Mar. 06, 2007   E.Safranek
 *  Added diagnostic messages on errors in CkLogin.
 *
 *  Nov, 09, 2006   E.Safranek
 *  In PasswordDlgCB, on a successful password change, need to also save new 
 *  password in Login.Password and curlupwd.
 *
 *  Oct. 05 2006    ESafranek
 *  Added change user password functionality.
 * 
 *==============================================================================
 */

#include "windows.h"
#include "resource.h"
#include <curl/curl.h>
#include <sys/timeb.h>
#include "LoginDialog.h"
#include "vo.h"
#include "vo_extern.h"
#include "SSI.h"
#include "ExecUtil.h"
#include "vo_db.h"
#include "TexFont.h"
#include "shapefil.h"
#include "Tracks.h"
//#include "nexrad.h"
#include "mgl.h"
#include "resource.h" 
//#include "gfx.h"   
#include "srfc.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#include <cassert>


