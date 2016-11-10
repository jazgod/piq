#include <windows.h>
#include <crypto.h>

static HANDLE *lock_cs;
void win32_locking_callback(int mode, int type, const char *file, int line)
{
	if (mode & CRYPTO_LOCK)	{
		WaitForSingleObject(lock_cs[type],INFINITE);
	}
	else {
		ReleaseMutex(lock_cs[type]);
	}
}

void openssl_thread_setup( )
{	
	int i = 0;
	int nLockCount = CRYPTO_num_locks();
	lock_cs=(HANDLE *)OPENSSL_malloc(nLockCount * sizeof(HANDLE));
	memset(lock_cs, 0, nLockCount * sizeof(HANDLE));
	for (i = 0; i < nLockCount; i++) {
		lock_cs[i]=CreateMutex(NULL,FALSE,NULL);
	}

	CRYPTO_set_locking_callback((void (*)(int,int,const char *,int))win32_locking_callback);
	/* id callback defined */
}

void openssl_thread_cleanup( )
{
	int nLockCount = CRYPTO_num_locks();
	int i = 0;
	CRYPTO_set_locking_callback(NULL);
	for (i = 0; i < nLockCount; i++)
		CloseHandle(lock_cs[i]);
	OPENSSL_free(lock_cs);
}

