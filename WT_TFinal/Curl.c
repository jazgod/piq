#include <windows.h>
#include <wininet.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "Curl.h"
#include "vo.h"
#include "vo_extern.h"
#include "SimpleCrypto.h"
#include "SurfaceC.h"
#include "dbg_mem.h"

extern int IsDebugNameDefined(struct InstanceData* id, const char* strDebugName);
#define D_CONTROL(vcolname, exp) {if ( 5 == id->m_nDebugControl || (id->m_nDebugControl && IsDebugNameDefined(id, vcolname)) ) {exp;}}

struct curlPrxyOpt comOpts[] =
{
	{ "CURLOPT_VERBOSE",			CURLOPT_VERBOSE,			0 },	// prints debug info if not 0
	{ "CURLOPT_PROXY",				CURLOPT_PROXY,				1 },	// http proxy to use (url or dotted ip)
	{ "CURLOPT_PROXYTYPE",			CURLOPT_PROXYTYPE,			2 },	// proxy type (CURLPROXY_HTTP is the default)
	{ "CURLOPT_HTTPPROXYTUNNEL",	CURLOPT_HTTPPROXYTUNNEL,	0 },	// tunnel all operations through proxy
	{ "CURLOPT_PROXYUSERPWD",		CURLOPT_PROXYUSERPWD,		1 },	// proxy [usr name]:[password] string
	{ "",							(CURLoption) 0,				0 }		// end of table marker
};

#ifdef WIN32
const char *pCACertFile= NULL;
const char *pPassphrase = "curlpwd";
const char *pKeyName = NULL;
const char *pKeyType = "PEM";
const char *pEngine = NULL;
#else
const char *pCertFile = "/home/matt/clientcurl.pem";
const char *pCACertFile="/home/matt/cakey.pem";
const char *pPassphrase = "curlpwd";
const char *pKeyName = "/home/matt/clientcurlkey.pem";
const char *pKeyType = "PEM";
const char *pEngine = NULL;
#endif

#ifdef CURL_ERRORS_FOR_SHOW
typedef enum {
  CURLE_OK = 0,
  CURLE_UNSUPPORTED_PROTOCOL,    /* 1 */
  CURLE_FAILED_INIT,             /* 2 */
  CURLE_URL_MALFORMAT,           /* 3 */
  CURLE_URL_MALFORMAT_USER,      /* 4 */
  CURLE_COULDNT_RESOLVE_PROXY,   /* 5 */
  CURLE_COULDNT_RESOLVE_HOST,    /* 6 */
  CURLE_COULDNT_CONNECT,         /* 7 */
  CURLE_FTP_WEIRD_SERVER_REPLY,  /* 8 */
  CURLE_FTP_ACCESS_DENIED,       /* 9 */
  CURLE_FTP_USER_PASSWORD_INCORRECT, /* 10 */
  CURLE_FTP_WEIRD_PASS_REPLY,    /* 11 */
  CURLE_FTP_WEIRD_USER_REPLY,    /* 12 */
  CURLE_FTP_WEIRD_PASV_REPLY,    /* 13 */
  CURLE_FTP_WEIRD_227_FORMAT,    /* 14 */
  CURLE_FTP_CANT_GET_HOST,       /* 15 */
  CURLE_FTP_CANT_RECONNECT,      /* 16 */
  CURLE_FTP_COULDNT_SET_BINARY,  /* 17 */
  CURLE_PARTIAL_FILE,            /* 18 */
  CURLE_FTP_COULDNT_RETR_FILE,   /* 19 */
  CURLE_FTP_WRITE_ERROR,         /* 20 */
  CURLE_FTP_QUOTE_ERROR,         /* 21 */
  CURLE_HTTP_RETURNED_ERROR,     /* 22 */
  CURLE_WRITE_ERROR,             /* 23 */
  CURLE_MALFORMAT_USER,          /* 24 - user name is illegally specified */
  CURLE_FTP_COULDNT_STOR_FILE,   /* 25 - failed FTP upload */
  CURLE_READ_ERROR,              /* 26 - could open/read from file */
  CURLE_OUT_OF_MEMORY,           /* 27 */
  CURLE_OPERATION_TIMEOUTED,     /* 28 - the timeout time was reached */
  CURLE_FTP_COULDNT_SET_ASCII,   /* 29 - TYPE A failed */
  CURLE_FTP_PORT_FAILED,         /* 30 - FTP PORT operation failed */
  CURLE_FTP_COULDNT_USE_REST,    /* 31 - the REST command failed */
  CURLE_FTP_COULDNT_GET_SIZE,    /* 32 - the SIZE command failed */
  CURLE_HTTP_RANGE_ERROR,        /* 33 - RANGE "command" didn't work */
  CURLE_HTTP_POST_ERROR,         /* 34 */
  CURLE_SSL_CONNECT_ERROR,       /* 35 - wrong when connecting with SSL */
  CURLE_BAD_DOWNLOAD_RESUME,     /* 36 - couldn't resume download */
  CURLE_FILE_COULDNT_READ_FILE,  /* 37 */
  CURLE_LDAP_CANNOT_BIND,        /* 38 */
  CURLE_LDAP_SEARCH_FAILED,      /* 39 */
  CURLE_LIBRARY_NOT_FOUND,       /* 40 */
  CURLE_FUNCTION_NOT_FOUND,      /* 41 */
  CURLE_ABORTED_BY_CALLBACK,     /* 42 */
  CURLE_BAD_FUNCTION_ARGUMENT,   /* 43 */
  CURLE_BAD_CALLING_ORDER,       /* 44 */
  CURLE_HTTP_PORT_FAILED,        /* 45 - HTTP Interface operation failed */
  CURLE_BAD_PASSWORD_ENTERED,    /* 46 - my_getpass() returns fail */
  CURLE_TOO_MANY_REDIRECTS ,     /* 47 - catch endless re-direct loops */
  CURLE_UNKNOWN_TELNET_OPTION,   /* 48 - User specified an unknown option */
  CURLE_TELNET_OPTION_SYNTAX ,   /* 49 - Malformed telnet option */
  CURLE_OBSOLETE,	         /* 50 - removed after 7.7.3 */
  CURLE_SSL_PEER_CERTIFICATE,    /* 51 - peer's certificate wasn't ok */
  CURLE_GOT_NOTHING,             /* 52 - when this is a specific error */
  CURLE_SSL_ENGINE_NOTFOUND,     /* 53 - SSL crypto engine not found */
  CURLE_SSL_ENGINE_SETFAILED,    /* 54 - can not set SSL crypto engine as
                                    default */
  CURLE_SEND_ERROR,              /* 55 - failed sending network data */
  CURLE_RECV_ERROR,              /* 56 - failure in receiving network data */
  CURLE_SHARE_IN_USE,            /* 57 - share is in use */
  CURLE_SSL_CERTPROBLEM,         /* 58 - problem with the local certificate */
  CURLE_SSL_CIPHER,              /* 59 - couldn't use specified cipher */
  CURLE_SSL_CACERT,              /* 60 - problem with the CA cert (path?) */
  CURLE_BAD_CONTENT_ENCODING,    /* 61 - Unrecognized transfer encoding */

  CURL_LAST /* never use! */
} CURLcode;
#endif

#define INTPAR	0
#define CHRPAR	1
#define CURPAR	2
#define MAXSTRNGSZ	512

int DoesProxyAuthenticate( );
int setCurlProxyOpts ( struct InstanceData* id, CURL *handle );
size_t WriteMemoryCallback(void *ptr, size_t size, size_t nmemb, void *data);

void CurlErrorCount()
{
	static int CErrCount;
	static int ErrorThreshold = 10;
	static time_t StartErrInterval;
	time_t nowsecs;
	static time_t lastshownsecs;
	// char tmpstr[TMPBUF_SIZE];

	CErrCount++;
	if ( CErrCount >= ErrorThreshold ){
		nowsecs = time(0);
		if ( nowsecs - lastshownsecs > 300 ){
			// do not advertise the errors anymore, these are all logged by calling subroutine

			// sprintf(tmpstr, "Internet Connection Problems, c=%d", CErrCount );
			// ShowMyMessage(tmpstr, 600 );
			lastshownsecs = time(0);
		}


		CErrCount = 0;
		ErrorThreshold += 5;
		if (ErrorThreshold > 25){
			ErrorThreshold = 25;
		}
	}

	nowsecs = time(0);
	if ( !StartErrInterval ){
		StartErrInterval = nowsecs;
		return;
	}

	if ( nowsecs - StartErrInterval > 300 ){
		/* restart error interval */
		StartErrInterval = nowsecs;
		CErrCount = 0; // reset alarm count
	}

}


int DoesProxyAuthenticate(struct InstanceData* id)
{
	INTERNET_PROXY_INFO* pInfo = (INTERNET_PROXY_INFO*)id->m_sInetProxyInfo;
	const DWORD cdwThreadId = GetCurrentThreadId();
	static int nTested = FALSE;
	static int nRequired = FALSE;
	if(!nTested && pInfo)
	{
		int nLen = 0;
		HINTERNET hInternet; 
		nTested = TRUE;
		vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) IE Proxy Setting - %s\n", cdwThreadId, pInfo->lpszProxy);

		// Open WinInet to Use Cached Proxy Information
		hInternet = InternetOpen("PASSUR",
			pInfo->dwAccessType, pInfo->lpszProxy, pInfo->lpszProxyBypass, 0);
		if(!hInternet)
			vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) InternetOpen failed %d\n", cdwThreadId, GetLastError());
		else
		{
			// Specify Connection to server
			HINTERNET hConnect;
			hConnect = InternetConnect(hInternet, "www1.passur.com", INTERNET_DEFAULT_HTTP_PORT,
					NULL, NULL,	INTERNET_SERVICE_HTTP, 0, 0);
			if(!hConnect)
				vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) InternetConnect failed %d\n", cdwThreadId, GetLastError());
			else
			{
				HINTERNET hRequest;
				// Force Unset Username/password (Required for XP)
				char szUserName[] = "username";
				char szPassWord[] = "password";
				if(!InternetSetOption(hConnect, INTERNET_OPTION_PROXY_USERNAME, szUserName, strlen(szUserName)))
					vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) InternetSetOption (force unset username) failed %d\n", cdwThreadId, GetLastError());

				if(!InternetSetOption(hConnect, INTERNET_OPTION_PROXY_PASSWORD, szPassWord, strlen(szUserName)))
					vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) InternetSetOption (force unset password) failed %d\n", cdwThreadId, GetLastError());

				// Specify GET Request
				hRequest = HttpOpenRequest(hConnect, "GET", "/surf/.proxytestfile", NULL,
					NULL, NULL, INTERNET_FLAG_KEEP_CONNECTION|INTERNET_FLAG_RELOAD, 0);
				if(!hRequest)
					vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) HttpOpenRequest failed %d\n", cdwThreadId, GetLastError());
				else
				{
					// Extract Header Information
					DWORD dwSize = 0;
					char *pInfoBuffer = NULL;
					BOOL bReq = HttpSendRequest(hRequest, NULL, 0, NULL, 0);
					if(!bReq){
						vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) HttpSendRequest failed %d (IE Proxy Settings prevent connection).\n", cdwThreadId, GetLastError());
					}else{
						HttpQueryInfo(hRequest, HTTP_QUERY_RAW_HEADERS_CRLF, NULL, &dwSize, NULL);
						pInfoBuffer = (char*)calloc(dwSize+1, sizeof(char));
						if(!HttpQueryInfo(hRequest, HTTP_QUERY_RAW_HEADERS_CRLF, pInfoBuffer, &dwSize, NULL))
						{
							vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) HttpQueryInfo failed %d\n", cdwThreadId, GetLastError());
						}
						else
						{
							// Check if Authorized ... "407" means we need Username/Password
							if(strstr(pInfoBuffer, "407 Unauthorized"))
							{
								nRequired = TRUE;
								vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Proxy Authentication Required!\n", cdwThreadId);
							}						
						}
						free(pInfoBuffer);
					}
					InternetCloseHandle(hRequest);					
				}
				InternetCloseHandle(hConnect);
			}
			InternetCloseHandle(hInternet);
		}
	}
	return nRequired;
}

void setCurlSecureMode( struct InstanceData* id, CURL* curl_handle, enum SecureMode eMode )
{
	const DWORD cdwThreadId = GetCurrentThreadId();
	if (!curl_handle) return;

	D_CONTROL("CURL", vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) CURLOPT_SSLCERT(%s), CURLOPT_SSLKEYPASSWD(%s), CURLOPT_SSLKEYTYPE(%s), CURLOPT_USERPWD(%s)\n", 
		cdwThreadId, id->m_strCertFilePath, pPassphrase, pKeyType, id->m_strCurlupwd));
	/* always verify host and peer */
	if ( curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYPEER,0)  != CURLE_OK ){
		vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) CURLOPT_SSL_VERIFYPEER failed \n", cdwThreadId);
	}
	if ( curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYHOST, 0l)  != CURLE_OK ){
		vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) CURLOPT_SSL_VERIFYHOST failed \n", cdwThreadId);
	}

	if (!eMode) return;

	/* cert is stored PEM coded in file... */
	/* since PEM is default, we needn't set it for PEM */
	if ( curl_easy_setopt(curl_handle,CURLOPT_SSLCERTTYPE,"PEM") != CURLE_OK ){
		vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) CURLOPT_SSLCERTTYPE failed \n", cdwThreadId);
	}
	/* set the cert for client authentication */
	if ( curl_easy_setopt(curl_handle,CURLOPT_SSLCERT, id->m_strCertFilePath)  != CURLE_OK ){
		vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) CURLOPT_SSLCERT failed \n", cdwThreadId);
	}
	/* sorry, for engine we must set the passphrase
	(if the key has one...) */
	if (pPassphrase) {
		curl_easy_setopt(curl_handle,CURLOPT_SSLKEYPASSWD,pPassphrase);
	}
	/* if we use a key stored in a crypto engine,
	we must set the key type to "ENG" */
	if ( curl_easy_setopt(curl_handle, CURLOPT_SSLKEYTYPE,pKeyType) != CURLE_OK ){
		vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) CURLOPT_SSLKEYTYPE failed \n", cdwThreadId);
	}
	/* set the private key (file or ID in engine) */
	if ( curl_easy_setopt(curl_handle,CURLOPT_SSLKEY,pKeyName) != CURLE_OK ){
		vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) CURLOPT_SSLKEY failed \n", cdwThreadId);
	}
	if ( curl_easy_setopt(curl_handle, CURLOPT_USERPWD, id->m_strCurlupwd )  != CURLE_OK ){
		vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) CURLOPT_USERPWD failed \n", cdwThreadId );
	}
}

void setCurlTimeout(CURL* curl_handle, long timeout)
{
	curl_easy_setopt(curl_handle, CURLOPT_TIMEOUT, timeout);
}

CURL* GetCurlHandle(struct InstanceData* id, enum SecureMode eMode)
{
	CURL *curl_handle;
	const DWORD cdwThreadId = GetCurrentThreadId();
	
	curl_handle = curl_easy_init();
	if (!curl_handle) {
		vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) curl_easy_init() failed!", cdwThreadId );
		//Callback2Javascript(id, "InternetStatus:FAIL");
		return NULL;
	}

	/* no progress meter please */
	curl_easy_setopt(curl_handle, CURLOPT_NOPROGRESS, 1);
    /* shut up completely */
    curl_easy_setopt(curl_handle, CURLOPT_MUTE, 1);

	/* Force Version to TLSv1 */
	curl_easy_setopt(curl_handle, CURLOPT_SSLVERSION, CURL_SSLVERSION_TLSv1);

	/* set default timeout */
	setCurlTimeout(curl_handle, 15L);

	setCurlSecureMode(id, curl_handle, eMode);

	if ( setCurlProxyOpts ( id, curl_handle ) != CURLE_OK ) {
		vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) setCurlProxyOpts() failed!", cdwThreadId);
		curl_easy_cleanup(curl_handle);
		//Callback2Javascript(id, "InternetStatus:FAIL");
		return NULL;
	}

	return curl_handle;
}

int GetCurlPage_r(struct InstanceData* id, CURL* curl_handle, struct MemoryStruct *chunk, char *url, enum SecureMode eMode, int maxtries, unsigned long timeout)
{
	int curlstatus;
	int ntries;
	char errorbuf[CURL_ERROR_SIZE] = {0};
	CURLcode res;
	long response_code = 0L;
	const DWORD cdwThreadId = GetCurrentThreadId();
	
	if (!curl_handle) return FALSE;

	curlstatus = FALSE;

	for (ntries = 0; curlstatus == FALSE && ntries < maxtries; ntries++) {

		if ( ntries && chunk->size && chunk->memory ){
			free_chunk_memory(chunk);
		}

		vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Retrieving url %s, ntries =%d\n", cdwThreadId, url, ntries );

		curl_easy_setopt(curl_handle, CURLOPT_URL, url );

		/* send all data to this function  */
		curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
		/* we pass our 'chunk' struct to the callback function */
		curl_easy_setopt(curl_handle, CURLOPT_FILE, (void *)chunk);
		curl_easy_setopt(curl_handle, CURLOPT_ERRORBUFFER, errorbuf );
		setCurlTimeout(curl_handle, timeout);

		/* get it! */
		res = curl_easy_perform(curl_handle);

		/* check http response code */
		curl_easy_getinfo(curl_handle, CURLINFO_HTTP_CODE, &response_code);
   
		if ( CURLE_OK != res || 200 != response_code ){
			strcpy_s(id->m_strLatestErrorURL, URL_SZ, url);
			vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) url <%s> - Curl errnum=%d, errorbuf='%s', response_code='%d'\n", cdwThreadId, url, res, errorbuf, response_code );
			free_chunk_memory(chunk);
			CurlErrorCount();
			curlstatus = FALSE;
			//Callback2Javascript(id, "InternetStatus:FAIL");
			// On failure to connect to host, do NOT retry
			if(CURLE_COULDNT_CONNECT == res || CURLE_COULDNT_RESOLVE_PROXY == res || CURLE_COULDNT_RESOLVE_HOST == res){
				vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Path to url inaccessible from host, skipping retries.\n", cdwThreadId );
				break;
			}
		} else {
			memset(id->m_strLatestErrorURL, 0, URL_SZ*sizeof(char));
			curlstatus = TRUE;
			//Callback2Javascript(id, "InternetStatus:OK");
		}

		/*
		* Now, our chunk.memory points to a memory block that is chunk.size
		* bytes big and contains the remote file.
		*
		* Do something nice with it!
		*/

		// test for file not found errors
		if ( curlstatus == TRUE && chunk && chunk->size && chunk->memory &&
				strstr(chunk->memory, "404 Not Found" ) ){
			if ( ntries+1 < maxtries ){
				vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) 404 Not Found error, retrying\n", cdwThreadId);
				curlstatus = FALSE;
			} else {
				vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) 404 Not Found error, skipping\n", cdwThreadId);
				curlstatus = FALSE;
			}
		}
	}
	
	/* set default timeout */
	setCurlTimeout(curl_handle, 15L);

	return( curlstatus );
}

int GetCurlPage(struct InstanceData* id, struct MemoryStruct *chunk, char *url, enum SecureMode eMode, int maxtries, unsigned long timeout)
{
  CURL *curl_handle;
  int ret;
	
  curl_handle = GetCurlHandle(id, eMode);
  if (!curl_handle) {
    vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Could not get curl handle\n", GetCurrentThreadId() );
	//Callback2Javascript(id, "InternetStatus:FAIL");
	return(FALSE);
  }

  ret = GetCurlPage_r(id, curl_handle, chunk, url, eMode, maxtries, timeout);

  curl_easy_cleanup(curl_handle);

  return(ret);
}

//int GetCurlPage( struct InstanceData* id, struct MemoryStruct *chunk, char *url, enum SecureMode eMode, int maxtries, unsigned long timeout)
//{
//  CURL *curl_handle;
//  static int pflag = 0;
//	int curlstatus;
//	int ntries = 0;
//  char errorbuf[CURL_ERROR_SIZE];
//	CURLcode res;
//  const DWORD cdwThreadId = GetCurrentThreadId();
//
//	curlstatus = FALSE;
//	do {
//    if ( ntries && chunk->size && chunk->memory ){
//      free( chunk->memory );
//      chunk->memory = NULL;
//    }
//		vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Retrieving url %s, ntries =%d\n", cdwThreadId, url, ntries );
//		curl_handle = curl_easy_init();
//		if(curl_handle) {
//			/* set URL to get */
//
//			curl_easy_setopt(curl_handle, CURLOPT_URL, url );
//			/* no progress meter please */
//			curl_easy_setopt(curl_handle, CURLOPT_NOPROGRESS, 1);
//			/* shut up completely */
//			curl_easy_setopt(curl_handle, CURLOPT_MUTE, 1);
//			/* send all data to this function  */
//			curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
//			/* we pass our 'chunk' struct to the callback function */
//			curl_easy_setopt(curl_handle, CURLOPT_FILE, (void *)chunk);
//			curl_easy_setopt(curl_handle, CURLOPT_ERRORBUFFER, errorbuf );
//			if(timeout)
//				curl_easy_setopt(curl_handle, CURLOPT_TIMEOUT, timeout);
//			/* get it! */
//
//
//			if (eMode){
//				/* cert is stored PEM coded in file... */
//				/* since PEM is default, we needn't set it for PEM */
//				if ( curl_easy_setopt(curl_handle,CURLOPT_SSLCERTTYPE,"PEM") != CURLE_OK ){
//					vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) CURLOPT_SSLCERTTYPE failed \n", cdwThreadId);
//				}
//				/* set the cert for client authentication */
//				if ( curl_easy_setopt(curl_handle,CURLOPT_SSLCERT, id->m_strCertFilePath)  != CURLE_OK ){
//					vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) CURLOPT_SSLCERT failed \n", cdwThreadId);
//				}
//				/* sorry, for engine we must set the passphrase
//				(if the key has one...) */
//				if (pPassphrase)
//					curl_easy_setopt(curl_handle,CURLOPT_SSLKEYPASSWD,pPassphrase);
//					/* if we use a key stored in a crypto engine,
//				we must set the key type to "ENG" */
//
//				if ( curl_easy_setopt(curl_handle, CURLOPT_SSLKEYTYPE,pKeyType) != CURLE_OK ){
//					vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) CURLOPT_SSLKEYTYPE failed \n", cdwThreadId);
//				}
//				/* set the private key (file or ID in engine) */
//				if ( curl_easy_setopt(curl_handle,CURLOPT_SSLKEY,pKeyName) != CURLE_OK ){
//					vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) CURLOPT_SSLKEY failed \n", cdwThreadId);
//				}
//				if ( curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYPEER,0)  != CURLE_OK ){
//					vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) CURLOPT_SSL_VERIFYPEER failed \n", cdwThreadId);
//				}
//				if ( curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYHOST, 0l)  != CURLE_OK ){
//					vo_log_error(id->m_pfVoLogFile, "__FUNCTION__, "(0x%X) CURLOPT_SSL_VERIFYPEER failed \n", cdwThreadId);
//				}
//
//				if ( curl_easy_setopt(curl_handle, CURLOPT_USERPWD, id->m_strCurlupwd )  != CURLE_OK ){
//					vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) CURLOPT_USERPWD failed \n", cdwThreadId );
//				}
//			} else {
//				if ( curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYPEER, 0l)  != CURLE_OK ){
//					vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) CURLOPT_SSL_VERIFYPEER failed \n", cdwThreadId);
//				}
//				if ( curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYHOST, 0l)  != CURLE_OK ){
//					vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) CURLOPT_SSL_VERIFYPEER failed \n", cdwThreadId);
//				}
//			}
//
//			// Proxy Starts
//			if ( setCurlProxyOpts ( id, curl_handle ) != CURLE_OK ){
//				vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) setCurlProxyOpts() failed!", cdwThreadId);
//				Callback2Javascript(id, "InternetStatus:FAIL");
//				return( FALSE );
//			}
//			// Proxy Ends
//
//		if ( (res = curl_easy_perform(curl_handle)) != CURLE_OK ){
//				vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) curl perform failed, Curl errnum=%d, errorbuf='%s'\n", cdwThreadId, res, errorbuf );
//        CurlErrorCount();
//				curlstatus = FALSE;
//				Callback2Javascript(id, "InternetStatus:FAIL");
//			} else {
//				curlstatus = TRUE;
//				Callback2Javascript(id, "InternetStatus:OK");
//			}
//
//			/* cleanup curl stuff */
//			curl_easy_cleanup(curl_handle);
//			/*
//			* Now, our chunk.memory points to a memory block that is chunk.size
//			* bytes big and contains the remote file.
//			*
//			* Do something nice with it!
//			*/
//		} else {
//			// printf("Could not get curl handle\n");
//			curlstatus = FALSE;
//			vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Could not get curl handle\n", cdwThreadId );
//			Callback2Javascript(id, "InternetStatus:FAIL");
//		}
//    // test for file not found errors
//    if ( curlstatus == TRUE && chunk && chunk->size && chunk->memory &&
//      strstr(chunk->memory, "404 Not Found" ) ){
//      if ( ntries < maxtries ){
//        vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) 404 Not Found error, retrying\n", cdwThreadId);
//        curlstatus = FALSE;
//      } else {
//        vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) 404 Not Found error, skipping\n", cdwThreadId);
//        curlstatus = FALSE;
//      }
//    }
//    ntries++;
//	} while (curlstatus == FALSE && ntries < maxtries );
//
//  return( curlstatus );
//
//}

int GetCurlFormGetResults_r(struct InstanceData* id, CURL* curl_handle, struct MemoryStruct *chunk, char *url, char *postfields, enum SecureMode eMode, int logflag )
{
	CURLcode res;
	char fullurl[ TMPBUF_SIZE ] = {0};
	char errorbuf[CURL_ERROR_SIZE] = {0};
	long response_code = 0L;
	const DWORD cdwThreadId = GetCurrentThreadId();

	if (logflag){
		vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Retrieving url <%s> postfields <%s>\n", cdwThreadId, url,
			postfields );
	}

	if (!curl_handle) return FALSE;

	curl_easy_setopt( curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
    /* we pass our 'chunk' struct to the callback function */
    curl_easy_setopt(curl_handle, CURLOPT_FILE, (void *)chunk);
	curl_easy_setopt(curl_handle, CURLOPT_ERRORBUFFER, errorbuf );

    sprintf_s(fullurl, TMPBUF_SIZE, "%s?%s", url, postfields );
    if ( curl_easy_setopt(curl_handle, CURLOPT_URL, fullurl ) != CURLE_OK ){
			vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) CURLOPT_URL failed <%s>.\n", cdwThreadId, url);
    }

    /* get it! */
    res = curl_easy_perform(curl_handle);

	/* check http response code */
	curl_easy_getinfo(curl_handle, CURLINFO_HTTP_CODE, &response_code);
   
    if ( CURLE_OK != res || 200 != response_code ){
      strcpy_s(id->m_strLatestErrorURL, URL_SZ, url);
      vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) url <%s> - Curl errnum=%d, errorbuf='%s', response_code='%d'\n", cdwThreadId, url, res, errorbuf, response_code );
	  free_chunk_memory(chunk);
	  CurlErrorCount();
	  //Callback2Javascript(id, "InternetStatus:FAIL");
      return( FALSE );
    }
	memset(id->m_strLatestErrorURL, 0, URL_SZ*sizeof(char));
	//Callback2Javascript(id, "InternetStatus:OK");
  
    /*
    * Now, our chunk.memory points to a memory block that is chunk.size
    * bytes big and contains the remote file.
    *
    * Do something nice with it!
    */

    /* printf("retrieved text='%s'\n", chunk->memory ); */

	return( TRUE );
}

int GetCurlFormGetResultsHdrs_r( struct InstanceData* id, CURL* curl_handle, struct MemoryStruct *chunk, struct MemoryStruct *hdrs,
								 char *url, char *postfields, enum SecureMode eMode, int logflag )
{
	int ret;

	if (!curl_handle) return FALSE;

	curl_easy_setopt(curl_handle, CURLOPT_HEADERFUNCTION, WriteMemoryCallback);
	curl_easy_setopt(curl_handle, CURLOPT_HEADERDATA, (void*) hdrs);

	ret = GetCurlFormGetResults_r(id, curl_handle, chunk, url, postfields, eMode, logflag);

	curl_easy_setopt(curl_handle, CURLOPT_HEADERFUNCTION, NULL);
	curl_easy_setopt(curl_handle, CURLOPT_HEADERDATA, NULL);

	return ret;
}

int GetCurlFormGetResults(struct InstanceData* id, struct MemoryStruct *chunk, char *url, char *postfields, enum SecureMode eMode, int logflag )
{
  CURL *curl_handle;
  int ret;

  curl_handle = GetCurlHandle(id, eMode);
  if (!curl_handle) {
	//Callback2Javascript(id, "InternetStatus:FAIL");
	return( FALSE );
  }

  ret = GetCurlFormGetResults_r(id, curl_handle, chunk, url, postfields, eMode, logflag);

  curl_easy_cleanup(curl_handle);
   
  return( ret );
}

//int GetCurlFormGetResults( struct InstanceData* id, struct MemoryStruct *chunk, char *url, char *postfields, enum SecureMode eMode, int logflag )
//{
//  CURL *curl_handle;
//  CURLcode res;
//  char fullurl[ TMPBUF_SIZE ];
//  static int pflag = 0;
//	char errorbuf[CURL_ERROR_SIZE];
//  const DWORD cdwThreadId = GetCurrentThreadId();
//
//  if ( logflag ){
//    vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Retrieving url '%s?%s'\n", cdwThreadId, url,
//      postfields );
//  }
//
//  curl_handle = curl_easy_init();
//  if(curl_handle) {
//    /* set URL to get */
//
//    /* no progress meter please */
//    curl_easy_setopt(curl_handle, CURLOPT_NOPROGRESS, 1);
//    /* shut up completely */
//    curl_easy_setopt(curl_handle, CURLOPT_MUTE, 1);
//    /* send all data to this function  */
//
//    curl_easy_setopt( curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
//    /* we pass our 'chunk' struct to the callback function */
//    curl_easy_setopt(curl_handle, CURLOPT_FILE, (void *)chunk);
//    curl_easy_setopt(curl_handle, CURLOPT_ERRORBUFFER, errorbuf );
//    curl_easy_setopt(curl_handle, CURLOPT_TIMEOUT, 15L);
//
//
//    if (eMode){
//      /* cert is stored PEM coded in file... */
//      /* since PEM is default, we needn't set it for PEM */
//      if ( curl_easy_setopt(curl_handle,CURLOPT_SSLCERTTYPE,"PEM") != CURLE_OK ){
//        vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) CURLOPT_SSLCERTTYPE failed \n", cdwThreadId);
//      }
//      /* set the cert for client authentication */
//      if ( curl_easy_setopt(curl_handle,CURLOPT_SSLCERT, id->m_strCertFilePath)  != CURLE_OK ){
//        vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) CURLOPT_SSLCERT failed \n", cdwThreadId);
//      }
//      /* sorry, for engine we must set the passphrase
//      (if the key has one...) */
//      if (pPassphrase)
//        curl_easy_setopt(curl_handle,CURLOPT_SSLKEYPASSWD,pPassphrase);
//        /* if we use a key stored in a crypto engine,
//      we must set the key type to "ENG" */
//
//      if ( curl_easy_setopt(curl_handle, CURLOPT_SSLKEYTYPE,pKeyType) != CURLE_OK ){
//        vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) CURLOPT_SSLKEYTYPE failed \n", cdwThreadId);
//      }
//      /* set the private key (file or ID in engine) */
//      if ( curl_easy_setopt(curl_handle,CURLOPT_SSLKEY,pKeyName) != CURLE_OK ){
//        vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) CURLOPT_SSLKEY failed \n", cdwThreadId);
//      }
//      if ( curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYPEER, 0l)  != CURLE_OK ){
//        vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) CURLOPT_SSL_VERIFYPEER failed \n", cdwThreadId);
//      }
//
//  	  if ( curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYHOST, 0l)  != CURLE_OK ){
//        vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) CURLOPT_SSL_VERIFYPEER failed \n", cdwThreadId);
//      }
//
//      if ( curl_easy_setopt(curl_handle, CURLOPT_USERPWD, id->m_strCurlupwd )  != CURLE_OK ){
//        vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) CURLOPT_USERPWD failed \n", cdwThreadId);
//      }
//    }
//
//
//    sprintf(fullurl, "%s?%s", url, postfields );
//    if ( curl_easy_setopt(curl_handle, CURLOPT_URL, fullurl ) != CURLE_OK ){
//			vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) CURLOPT_URL failed \n", cdwThreadId);
//    }
//		if ( curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYPEER, 0l)  != CURLE_OK ){
//			vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) CURLOPT_SSL_VERIFYPEER failed \n", cdwThreadId);
//		}
//
//		if ( curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYHOST, 0l)  != CURLE_OK ){
//			vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) CURLOPT_SSL_VERIFYPEER failed \n", cdwThreadId);
//		}
//
//	// Proxy Starts
//	if ( setCurlProxyOpts ( id, curl_handle ) != CURLE_OK ){
//		vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) setCurlProxyOpts() failed!", cdwThreadId);
//		curl_easy_cleanup(curl_handle);
//		Callback2Javascript(id, "InternetStatus:FAIL");
//		return( FALSE );
//	}
//	// Proxy Ends
//
//
//    /* get it! */
//    // vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) getting page\n", cdwThreadId);
//    if ( (res = curl_easy_perform(curl_handle)) != 0 ){
//      vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Curl errnum=%d, errorbuf='%s'\n", cdwThreadId, res, errorbuf );
//	  CurlErrorCount();
//	  curl_easy_cleanup(curl_handle);
//	  Callback2Javascript(id, "InternetStatus:FAIL");
//      return( FALSE );
//    }
//	Callback2Javascript(id, "InternetStatus:OK");
//    // vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) getting page...Done\n", cdwThreadId);
//    /* cleanup curl stuff */
//    curl_easy_cleanup(curl_handle);
//    /*
//    * Now, our chunk.memory points to a memory block that is chunk.size
//    * bytes big and contains the remote file.
//    *
//    * Do something nice with it!
//    */
//
//    /* printf("retrieved text='%s'\n", chunk->memory ); */
//
//  } else {
//    Callback2Javascript(id, "InternetStatus:FAIL");
//    return( FALSE );
//  }
//  return( TRUE );
//
//}

int GetCurlFormPostResults( struct InstanceData* id, struct MemoryStruct *chunk, char *url, char *postfields, enum SecureMode eMode, int logflag )
{
  CURL *curl_handle;
  CURLcode res;
  int stat;
  long response_code = 0L;
  char errorbuf[CURL_ERROR_SIZE] = {0};
  const DWORD cdwThreadId = GetCurrentThreadId();

  if ( url && postfields ){
    vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) url='%s' postfields='%s'\n",
      cdwThreadId, url, postfields );
  } else if (url ){
    vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) url='%s'\n",
      cdwThreadId, url);
  }
  
  curl_handle = GetCurlHandle(id, eMode);
  if (!curl_handle) {
    //Callback2Javascript(id, "InternetStatus:FAIL");
    return( FALSE );
  }
    /* set URL to get */

    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
    curl_easy_setopt(curl_handle, CURLOPT_ERRORBUFFER, errorbuf );

    /* we pass our 'chunk' struct to the callback function */
    curl_easy_setopt(curl_handle, CURLOPT_FILE, (void *)chunk);
    if ( (stat = curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDS, postfields)) != CURLE_OK ){
      vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Curl error code %d\n", cdwThreadId, stat );
      CurlErrorCount();
	  //Callback2Javascript(id, "InternetStatus:FAIL");
	  curl_easy_cleanup(curl_handle);
      return( FALSE );
    }

    curl_easy_setopt(curl_handle, CURLOPT_URL, url );
    

    /* get it! */
    res = curl_easy_perform(curl_handle);

	/* check http response code */
	curl_easy_getinfo(curl_handle, CURLINFO_HTTP_CODE, &response_code);
   
    if ( CURLE_OK != res || 200 != response_code ){
      strcpy_s(id->m_strLatestErrorURL, URL_SZ, url);
      vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) url <%s> - Curl errnum=%d, errorbuf='%s', response_code='%d'\n", cdwThreadId, url, res, errorbuf, response_code );
	  free_chunk_memory(chunk);
      CurlErrorCount();
	  //Callback2Javascript(id, "InternetStatus:FAIL");
	  curl_easy_cleanup(curl_handle);
      return( FALSE );
    }
	memset(id->m_strLatestErrorURL, 0, URL_SZ*sizeof(char));
	//Callback2Javascript(id, "InternetStatus:OK");
    /* cleanup curl stuff */
    curl_easy_cleanup(curl_handle);
    /*
    * Now, our chunk.memory points to a memory block that is chunk.size
    * bytes big and contains the remote file.
    *
    * Do something nice with it!
    */

    /* printf("%s", chunk->memory ); */

  return( TRUE );

}

int IsProxyRequired(struct InstanceData* id)
{
	INTERNET_PROXY_INFO* pInfo = NULL;
	const DWORD cdwThreadId = GetCurrentThreadId();
	static int nTested = FALSE;
	static int nRequired = FALSE;
	if(!nTested)
	{		
		unsigned long	nSize = 0;
		nTested = TRUE;		
		InternetQueryOption(NULL, INTERNET_OPTION_PROXY, NULL, &nSize);
		if(nSize > 0)
		{
			// Query WinInet for Proxy Settings		
			pInfo = (INTERNET_PROXY_INFO*)calloc(nSize + 1, sizeof(char));
			if(!InternetQueryOption(NULL, INTERNET_OPTION_PROXY, pInfo, &nSize))
				vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) InternetQueryOption(INTERNET_OPTION_PROXY) failed %d\n", cdwThreadId, GetLastError());
			else
			{
				if(INTERNET_OPEN_TYPE_PROXY == pInfo->dwAccessType)
				{
					nRequired = TRUE;
					id->m_sInetProxyInfo = pInfo;
					vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Proxy settings required.\n", cdwThreadId);
				}
				else
					vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Proxy settings NOT required.\n", cdwThreadId);
			}
		}
	}
	return nRequired;
}

/*
** saveCurlProxyOpts() writes curl communications proxy settings to file.
**
** This function writes proxy related settings to data/communication.txt.
**
** Return Values:    TRUE  for success
**					 FALSE for File could not be created.
**
**
*/
int saveCurlProxyOpts (struct InstanceData* id, int bEncrypt)
{
	int			i;
	FILE		*fp;
	char		proxyfile[512];
	struct		curlCachedProxyOpt* pCurr;
	struct		curlCachedProxyOpt* curlCachedProxyOpts = (struct curlCachedProxyOpt* )id->m_sCurlCachedProxyOpts;
	const DWORD cdwThreadId = GetCurrentThreadId();
	
	if(bEncrypt)
		_snprintf_s(proxyfile, _countof(proxyfile), 127, "%s\\data\\passur", id->m_strPassurOutPath );
	else
		_snprintf_s(proxyfile, _countof(proxyfile), 127, "%s\\data\\communications.txt", id->m_strPassurOutPath );

	vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Saving proxy settings to <%s>.\n", cdwThreadId, proxyfile);
	if ( fopen_s(&fp, proxyfile, "w") )
	{
		vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Unable to create/open file: %s\n", cdwThreadId, proxyfile);
		return ( FALSE );
	}
	if(bEncrypt)
	{		
		DWORD len = 0;
		int l = 0;
		char* data = NULL;			
		pCurr = (struct curlCachedProxyOpt*)id->m_sCurlCachedProxyOpts;
		while(pCurr)
		{
			char buffer[100] = {0};
			for ( i=0; comOpts[i].option != pCurr->option; i++ );
			switch(pCurr->type)
			{
			case INTPAR:
				sprintf_s(buffer, _countof(buffer), "%s\t%d\n", comOpts[i].name, pCurr->val);
				break;
			case CHRPAR:
				sprintf_s(buffer, _countof(buffer), "%s\t%s\n", comOpts[i].name, pCurr->str);
				break;
			case CURPAR:
				// Only CURLOPT_PROXYTYPE supported
				sprintf_s(buffer, _countof(buffer), "CURLOPT_PROXYTYPE\tCURLPROXY_HTTP\n");
				break;
			}
			
			if(data)
			{
				l = strlen(buffer) + strlen(data) + 1;
				data = (char*)realloc(data, l * sizeof(char));
			}
			else
			{
				l = strlen(buffer) + 1;
				data = (char*)calloc(l, sizeof(char));
			}
			strcat_s(data, l, buffer);
			pCurr = pCurr->next;
		}
		len = strlen(data);
		Encrypt(data, &len, 1000, TRUE); 
		fwrite(data, 1, len, fp);
		free(data);
	}
	else
	{
		fprintf(fp, "#\n# Passur Construction Module Proxy Configuration File\n#\n"
			"#\n# This file was auto-generated\n#\n"
			"# Format:  Proxy_parameter proxy_parameter_value\n"
			"#\n# Valid parameters currently are:\n"
			"#\n#  CURLOPT_VERBOSE		0		// prints debug info if not 0\n"
			"#  CURLOPT_PROXY		172.16.2.40 	// http proxy to use (url or dotted ip)\n"
			"#  CURLOPT_PROXYTYPE		CURLPROXY_HTTP 	// proxy type (CURLPROXY_HTTP is the default) CURLPROXY_SOCKS5 is the only other value allowed\n"
			"#  CURLOPT_HTTPPROXYTUNNEL	0		// tunnel all operations through proxy if not 0\n"
			"#  CURLOPT_PROXYUSERPWD		id:password	// proxy [usr name]:[password] string\n"
			"#\n\n");
		
		pCurr = curlCachedProxyOpts;
		while(pCurr)
		{
			for ( i=0; comOpts[i].option != pCurr->option; i++ );
			switch(pCurr->type)
			{
			case INTPAR:
				fprintf(fp, "%s\t%d\n", comOpts[i].name, pCurr->val);
				vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Saving proxy setting: <%s:%d>.\n", cdwThreadId, comOpts[i].name, pCurr->val);
				break;
			case CHRPAR:
				fprintf(fp, "%s\t%s\n", comOpts[i].name, pCurr->str);
				vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Saving proxy setting: <%s:%s>.\n", cdwThreadId, comOpts[i].name, pCurr->str);
				break;
			case CURPAR:
				// Only CURLOPT_PROXYTYPE supported
				fprintf(fp, "CURLOPT_PROXYTYPE CURLPROXY_HTTP\n");
				vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Saving proxy setting: <CURLOPT_PROXYTYPE:CURLPROXY_HTTP>.\n", cdwThreadId);
				break;
			}
			pCurr = pCurr->next;
		}
	}
	fclose ( fp );

	return ( TRUE );
}


/*
** setCurlProxyOpts() sets curl communications proxy settings.
**
** This function reads proxy related communication settings from memory, then calls
** curl lib function curl_easy_setopt() to set those parameters.  Currently, only proxy
** related parameters are recognized (see comOpts[] above).
**
** Return Values:    CURL_OK  for success
**					 CURLCODE (CURL FAILURE CODE) for failure.
**
**
**
** If a call to curl_easy_setopt() fails, this function will stop processing and return failure.
*/

int setCurlProxyOpts ( struct InstanceData* id, CURL *handle )
{
	CURLcode	rc;
	struct		curlCachedProxyOpt* pCurr;
	struct		curlCachedProxyOpt* curlCachedProxyOpts = (struct curlCachedProxyOpt* )id->m_sCurlCachedProxyOpts;

	// Proxy options cached in loadCurlProxyOpts
	if(curlCachedProxyOpts)
	{
		pCurr = curlCachedProxyOpts;
		while(pCurr)
		{
			switch(pCurr->type)
			{
			case INTPAR: rc = curl_easy_setopt ( handle, pCurr->option, pCurr->val );
				break;
			case CHRPAR: rc = curl_easy_setopt ( handle, pCurr->option, pCurr->str );
				break;
			case CURPAR: rc = curl_easy_setopt ( handle, pCurr->option, pCurr->opt );
				break;
			}
			if ( CURLE_OK != rc )
			{
				// Log failed parameters and return failure
				vo_log_error(id->m_pfVoLogFile, __FUNCTION__,
				   "(0x%X) curl_easy_setopt failed for cached proxy option\n", GetCurrentThreadId());
			}
			pCurr = pCurr->next;
		}
	}

	return ( CURLE_OK );
}

/*
** loadCurlProxyOpts() reads curl communications proxy settings from file.
**
** This function reads proxy related settings from data/communication.txt, 
** and caches the values for current run.
**
** Return Values:    TRUE  for success
**					 FALSE for File does not exist.
**
**
** If there are parameters in data/communication.txt that are not in comOpts[], this function will log
** them and continue processing.
**
*/
int loadCurlProxyOpts (struct InstanceData* id, int* pnProxyRequired, int* pnProxyAuthenticates, int* pnProxySet, int* pnProxyUserPwdSet)
{
	int			i;
	char		eol[2];
	char		name [MAXSTRNGSZ];
	char		value[MAXSTRNGSZ];
	char		line [MAXSTRNGSZ*2];
	FILE		*fp;
	char		proxyfile[512];
	struct		curlCachedProxyOpt* pCurr, *pNew;
	struct		curlCachedProxyOpt* curlCachedProxyOpts = (struct curlCachedProxyOpt* )id->m_sCurlCachedProxyOpts;
	int			bEncrypt = FALSE;
	const DWORD cdwThreadId = GetCurrentThreadId();
	static		int firstTime = TRUE;
  
	// Check IE Settings
	if(*pnProxyRequired = IsProxyRequired(id))
	{
		*pnProxyAuthenticates = DoesProxyAuthenticate(id);
	}

	// Read Existing File
	if(firstTime)
	{
		firstTime = FALSE;

		_snprintf_s(proxyfile, _countof(proxyfile), 127, "%s\\data\\passur", id->m_strPassurOutPath );
		if ( !fopen_s(&fp, proxyfile, "rb") )
		{
			int fd = 0;
			struct stat s;
			char *buffer = NULL;
			char *p = NULL;
			DWORD len = 0;
			
			// Get File Size
			fd = _fileno(fp);
			fstat(fd, &s);
			buffer = (char*)calloc(s.st_size + 1, sizeof(char)); 
			p = buffer;

			len = fread(buffer, sizeof(char), s.st_size, fp);
			Decrypt(buffer, &len, TRUE);
			vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Using encrypted settings file.\n", cdwThreadId);
			while ( p && sscanf_s ( p, "%[^\n]", line, _countof(line) ) > 0 )
			{	
				p = strchr(p, '\n');
				if(p) p++;

				if ( *line == '#' || sscanf_s ( line, "%s %s", name, MAXSTRNGSZ, value, MAXSTRNGSZ ) != 2 )
					continue;

				for ( i=0; *comOpts[i].name && strcmp(name, comOpts[i].name); i++ );

				if ( *comOpts[i].name )
				{
					pNew = (struct curlCachedProxyOpt*)calloc(1,sizeof(struct curlCachedProxyOpt));
					pNew->option = comOpts[i].option;
					pNew->type = comOpts[i].type;

					// Identify Settings
					if ( CURLOPT_PROXY == comOpts[i].option) 
						*pnProxySet = TRUE;

					if ( CURLOPT_PROXYUSERPWD == comOpts[i].option) 
						*pnProxyUserPwdSet = TRUE;

					// Store Data Values
					if ( comOpts[i].type == INTPAR )
						pNew->val = atoi(value);
					else if ( comOpts[i].type == CHRPAR )
					{
						int len = strlen(value) + 1;
						pNew->str = (char*)calloc(len, sizeof(char));
						strcpy_s(pNew->str, len, value);
					}
					else if ( comOpts[i].type == CURPAR )
					{
						// currently only CURLOPT_PROXYTYPE is supported, so we'll check only for that
						// if more options with "Curl" type parameters are added in the future, then we
						// would need to check and see which one it is here
						if ( strcmp ( value, "CURLPROXY_HTTP" ) )
							pNew->opt = CURLPROXY_SOCKS5;
						else
							pNew->opt = CURLPROXY_HTTP;
					}

					// Cache Options
					if(curlCachedProxyOpts)
					{
						pCurr = curlCachedProxyOpts;
						while(pCurr->next)
							pCurr = pCurr->next;
						pCurr->next = pNew;
					}else{
						curlCachedProxyOpts = pNew;
						id->m_sCurlCachedProxyOpts = pNew;
					}
				}
				else
				{
					// Log Empty message and continue
					vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) *comOpts[i].name = null\n", cdwThreadId);
					continue;
				}
			}
			free(buffer);
			fclose(fp);
		}
		else
		{

			// Read proxy settings from config file.
			_snprintf_s(proxyfile, _countof(proxyfile), 127, "%s\\data\\communications.txt", id->m_strPassurOutPath );

			if ( fopen_s(&fp, proxyfile, "rb") )
			{
				vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Unable to open file: %s\n", cdwThreadId, proxyfile);
				if(*pnProxyRequired)
				{
					vo_log_warning(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Proxy Required, CURLOPT_PROXY not set in %s!\n", cdwThreadId, proxyfile);
					if(*pnProxyAuthenticates)
						vo_log_error(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Proxy Authentication Required, CURLOPT_PROXYUSERPWD not set in %s!\n", cdwThreadId, proxyfile);	  
				}
				// File Does not Exist
				return ( FALSE );
			}
			vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Using communications.txt settings file.\n", cdwThreadId);
			while ( fscanf_s ( fp, "%[^\n]", line, _countof(line) ) > 0 )
			{	
				fscanf_s( fp, "%[\n]", &eol, _countof(eol) );

				if ( *line == '#' || sscanf_s ( line, "%s %s", name, MAXSTRNGSZ, value, MAXSTRNGSZ ) != 2 )
					continue;

				for ( i=0; *comOpts[i].name && strcmp(name, comOpts[i].name); i++ );

				if ( *comOpts[i].name )
				{
					pNew = (struct curlCachedProxyOpt*)calloc(1,sizeof(struct curlCachedProxyOpt));
					pNew->option = comOpts[i].option;
					pNew->type = comOpts[i].type;
					vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) Curl setting <%s=%s>\n", cdwThreadId, name, value);

					// Identify Settings
					if ( CURLOPT_PROXY == comOpts[i].option) 
						*pnProxySet = TRUE;

					if ( CURLOPT_PROXYUSERPWD == comOpts[i].option) 
						*pnProxyUserPwdSet = TRUE;

					// Store Data Values
					if ( comOpts[i].type == INTPAR )
						pNew->val = atoi(value);
					else if ( comOpts[i].type == CHRPAR )
					{
						int len = strlen(value) + 1;
						pNew->str = (char*)calloc(len, sizeof(char));
						strcpy_s(pNew->str, len, value);
					}
					else if ( comOpts[i].type == CURPAR )
					{
						// currently only CURLOPT_PROXYTYPE is supported, so we'll check only for that
						// if more options with "Curl" type parameters are added in the future, then we
						// would need to check and see which one it is here
						if ( strcmp ( value, "CURLPROXY_HTTP" ) )
							pNew->opt = CURLPROXY_SOCKS5;
						else
							pNew->opt = CURLPROXY_HTTP;
					}

					// Cache Options
					if(curlCachedProxyOpts)
					{
						pCurr = curlCachedProxyOpts;
						while(pCurr->next)
							pCurr = pCurr->next;
						pCurr->next = pNew;
					}else{
						curlCachedProxyOpts = pNew;
						id->m_sCurlCachedProxyOpts = pNew;
					}
				}
				else
				{
					// Log Empty message and continue
					vo_log_info(id->m_pfVoLogFile, __FUNCTION__, "(0x%X) *comOpts[i].name = null\n", cdwThreadId);
					continue;
				}
			}

			fclose ( fp );
		}		
	}
	return ( TRUE );
}

size_t WriteMemoryCallback(void *ptr, size_t size, size_t nmemb, void *data)
{
  register int realsize = size * nmemb;
  struct MemoryStruct *mem = (struct MemoryStruct *)data;

  mem->memory = (char *)realloc(mem->memory, mem->size + realsize + 1);
  if (mem->memory) {
    memcpy(&(mem->memory[mem->size]), ptr, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;
  }
  return realsize;
}

void clear_chunk_memory(struct MemoryStruct* chunk)
{
	chunk->memory = NULL;
	chunk->size = 0;
}

void free_chunk_memory(struct MemoryStruct* chunk)
{
	if (chunk->memory) free(chunk->memory);
	chunk->memory = NULL;
	chunk->size = 0;
}
