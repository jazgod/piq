#pragma once

#include "wincrypt.h"

#ifdef __cplusplus 
class CSimpleCrypto
{
public:
	CSimpleCrypto(LPTSTR pszPassword, ALG_ID algorithm);
	virtual ~CSimpleCrypto( );

    BOOL Encrypt(LPBYTE pData, LPDWORD pdwDataSize, 
                 DWORD dwBufferSize, BOOL bFinal);
    BOOL Decrypt(LPBYTE pData, LPDWORD dwDataSize,
                 BOOL bFinal);

private:
    HCRYPTPROV m_hProv;
    HCRYPTKEY  m_hKey;
};
#else
	BOOL Encrypt(LPBYTE pData, LPDWORD pdwDataSize, DWORD dwBufferSize, BOOL bFinal);
	BOOL Decrypt(LPBYTE pData, LPDWORD dwDataSize, BOOL bFinal);
#endif//__cplusplus  

