#include "StdAfx.h"
#include "SimpleCrypto.h"

CSimpleCrypto g_Crypto("P@s5Ur", CALG_RC4);

CSimpleCrypto::CSimpleCrypto(LPTSTR pszPassword, ALG_ID algorithm)
{
	m_hProv = NULL;
    m_hKey  = NULL;

    // Get handle to the default key container in default CSP
    if (CryptAcquireContext(&m_hProv,     // handle to CSP
                            NULL,         // default container
                            MS_DEF_PROV,  // default CSP
                            PROV_RSA_FULL,// provider type
                            0)) {         // action

        HCRYPTHASH hHash;

        // Get handle to hash object
        if (CryptCreateHash(m_hProv,   // handle to CSP
                            CALG_MD5,  // hashing algorithm
                            0,         // non-keyed hash
                            0,         // reserved, must be zero
                            &hHash)) { // hash object

            // Hash the password
            if (CryptHashData(hHash,   // handle to hash object
                              (LPBYTE)pszPassword, // data buffer
                              lstrlen(pszPassword)*sizeof(TCHAR),
                              0)) {    // flags
        
                // Create the session key using the hashed password
                CryptDeriveKey(m_hProv,   // handle to CSP
                               algorithm, // encryption algorithm 
                               hHash,     // hashed data
                               CRYPT_EXPORTABLE | 0x00280000,
                               &m_hKey);  // session key
            }
            CryptDestroyHash(hHash);
        }
    }

}

CSimpleCrypto::~CSimpleCrypto()
{
    if (m_hKey != NULL) {
        CryptDestroyKey(m_hKey);
    }
    if (m_hProv != NULL) {
        CryptReleaseContext(m_hProv, 0);
    }
}

BOOL CSimpleCrypto::Encrypt(LPBYTE pData, LPDWORD pdwDataSize, 
                        DWORD dwBufferSize, BOOL bFinal)
{
    return CryptEncrypt(m_hKey,        // encryption key handle
                        0,             // optional hash handle
                        bFinal,        // true if last block
                        0,             // flag (reserved)
                        pData,         // data to encrypt
                        pdwDataSize,   // data size (in bytes)
                        dwBufferSize); // buffer size (in bytes)
}

BOOL CSimpleCrypto::Decrypt(LPBYTE pData, LPDWORD pdwDataSize,
                        BOOL bFinal)
{
    return CryptDecrypt(m_hKey,        // encryption key handle
                        0,             // optional hash handle
                        bFinal,        // true if last block
                        0,             // flag (reserved)
                        pData,         // data to encrypt
                        pdwDataSize);  // data size (in bytes)
}

inline BOOL LocalEncrypt(LPBYTE pData, LPDWORD pdwDataSize, 
             DWORD dwBufferSize, BOOL bFinal)
{
	return g_Crypto.Encrypt(pData, pdwDataSize, dwBufferSize, bFinal);
}

inline BOOL LocalDecrypt(LPBYTE pData, LPDWORD dwDataSize,
             BOOL bFinal)
{
	return g_Crypto.Decrypt(pData, dwDataSize, bFinal);
}


extern "C"
{
BOOL Encrypt(LPBYTE pData, LPDWORD pdwDataSize, 
             DWORD dwBufferSize, BOOL bFinal)
{
	return LocalEncrypt(pData, pdwDataSize, dwBufferSize, bFinal);
}

BOOL Decrypt(LPBYTE pData, LPDWORD dwDataSize,
             BOOL bFinal)
{
	return LocalDecrypt(pData, dwDataSize, bFinal);
}
}
