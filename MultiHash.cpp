#include <stdio.h>
#include <assert.h>
#include <windows.h>
#include <Wincrypt.h>

#define BUFSIZE 4096


DWORD PrintHash(HCRYPTHASH hHash)
{
    DWORD cbData = sizeof(DWORD);
    PBYTE pbData = NULL;
    DWORD cHashSize;
    CHAR Digits[] = "0123456789ABCDEF";
    DWORD dwStatus = 0;

    
    if (!CryptGetHashParam(hHash, HP_HASHSIZE, (PBYTE)&cHashSize, &cbData, 0) && (cbData != sizeof(cHashSize)))
        goto ErrorExit;

    pbData = (PBYTE)malloc(cHashSize);

    if ((pbData) && (CryptGetHashParam(hHash, HP_HASHVAL, pbData, &cHashSize, 0)))
    {
        for (DWORD i = 0; i < cHashSize; i++)
        {
            printf("%c%c", Digits[pbData[i] >> 4], Digits[pbData[i] & 0xf]);
        }
        printf("\n");
        goto Exit;
    }

ErrorExit:
    dwStatus = GetLastError();
    printf("ERROR: CryptGetHashParam failed: %08x\n", dwStatus);
Exit:
    if (pbData) free(pbData);
    return dwStatus;
    
}

LONGLONG mySetFilePointer(HANDLE hFile, LONGLONG distance, DWORD MoveMethod)
{
    LARGE_INTEGER dist;

    dist.QuadPart = distance;
    dist.LowPart = SetFilePointer(hFile, dist.LowPart, &dist.HighPart, MoveMethod);

    if (dist.LowPart == INVALID_SET_FILE_POINTER && GetLastError() != NO_ERROR)
    {
        dist.QuadPart = -1;
    }

    return dist.QuadPart;
}


int wmain(int argc, wchar_t* argv[])
{
    LARGE_INTEGER fsize;
    ULONGLONG NextToRead = 0;
    ULONGLONG SegFirst=0;
    ULONGLONG SegLast = 0;
    ULONGLONG SegSize;
    ULONGLONG WindowFirst = 0;
    ULONGLONG WindowLast = 0;
    ULONGLONG WindowSize;
    ULONGLONG nSegments = 1;
    ULONGLONG Remainder = 0;
    DWORD dwStatus = 0;
    BOOL bResult = FALSE;
    HCRYPTPROV hProv = 0;
    HCRYPTHASH hHash = 0;
    HANDLE hFile = NULL;
    BYTE Bufffer[BUFSIZE];
    DWORD cbRead = 0;
    wchar_t* EndPtr;
    ULONGLONG tmp;

    LPCWSTR filename = argv[1];
    // Logic to check usage goes here.

    if (argc < 2)
    {
        printf("USAGE: FileToHash NumberOfHashSegments <OffsetRange> e.g.: 1BEEF-20000\n");
        return -1;
    }

    hFile = CreateFileW(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);

    if (INVALID_HANDLE_VALUE == hFile)
    {
        dwStatus = GetLastError();
        printf("Error opening file %ls\nError: %08x\n", filename, dwStatus);
        return dwStatus;
    }

    if (!GetFileSizeEx(hFile, &fsize))
    {
        CloseHandle(hFile);
        dwStatus = GetLastError();
        printf("Error obtainig file size %ls\nError: %08x\n", filename, dwStatus);
        return dwStatus;
    }
    WindowLast = fsize.QuadPart-1;

    if (argc > 2)
        nSegments = max (_wtoi(argv[2]), 1);

    if (argc > 3)
    {
        if (*argv[3] == L'-')
            WindowLast = wcstoull(argv[3] + 1, NULL, 16);  //_wcstoui64
        else
        {
            WindowFirst = wcstoull(argv[3], &EndPtr, 16);  //_wcstoui64
            if (*EndPtr == L'-')
            {
                if (*(EndPtr + 1) == L'\0')
                    WindowLast = (ULONGLONG)fsize.QuadPart - 1;
                else
                    WindowLast = wcstoull(EndPtr + 1, NULL, 16);  //_wcstoui64
            }
        }
    }

    if (WindowFirst > WindowLast)
    {
        tmp = WindowFirst;
        WindowLast = WindowFirst;
        WindowFirst = tmp;
    }

    WindowLast = min(WindowLast, (ULONGLONG)fsize.QuadPart-1);
    WindowSize = WindowLast - WindowFirst + 1;
    nSegments = min(nSegments, WindowSize);

    Remainder = WindowSize % nSegments;
    SegSize = WindowSize / nSegments + (Remainder > 0);   

    // Get handle to the crypto provider
    if (!CryptAcquireContext(&hProv, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT))
    {
        dwStatus = GetLastError();
        printf("ERROR: CryptAcquireContext failed: %08x\n", dwStatus);
        CloseHandle(hFile);
        return dwStatus;
    }
 
    if (!CryptCreateHash(hProv, CALG_MD5, 0, 0, &hHash))
    {
        dwStatus = GetLastError();
        printf("ERROR: CryptCreateHash failed: %08x\n", dwStatus);
        CloseHandle(hFile);
        CryptReleaseContext(hProv, 0);
        return dwStatus;
    }

    printf("\nCalculating hashes for %llu segments of the file %ls\nfrom ofset %016llx to %016llx (inclusive)\n\n", nSegments, filename, WindowFirst, WindowLast);
    printf("|       File Offset Range       |\t|          MD5 Hash            |\n");
    printf("|-------------------------------|\t|------------------------------|\n");
    

    mySetFilePointer(hFile, WindowFirst, FILE_BEGIN);
    NextToRead = WindowFirst;
    SegFirst = WindowFirst;
    SegLast = min(WindowFirst+SegSize-1, WindowLast);

    while (bResult = ReadFile(hFile, Bufffer, (DWORD)min(BUFSIZE, SegLast - NextToRead+1), &cbRead, NULL))
    {
        assert(cbRead == min(BUFSIZE, SegLast - NextToRead+1));
        assert(NextToRead + cbRead - 1 <= WindowLast);

        NextToRead += cbRead;
        
        if ( (cbRead > 0) && (!CryptHashData(hHash, Bufffer, cbRead, 0)) )
        {
            dwStatus = GetLastError();
            printf("ERROR: CryptHashData failed: %08x\n", dwStatus);
            goto Exit;
        }
      
        if ( (NextToRead > SegLast) || (cbRead == 0) )
        {
            printf("%016llx-%016llx\t", SegFirst, SegLast);
            PrintHash(hHash);

            if ((NextToRead > WindowLast) || (cbRead == 0))
                break;

            CryptDestroyHash(hHash);

            if (!CryptCreateHash(hProv, CALG_MD5, 0, 0, &hHash))
            {
                dwStatus = GetLastError();
                printf("ERROR: CryptCreateHash failed: %08x\n", dwStatus);
                goto Exit;
            }

            SegFirst = min(SegLast + 1, WindowLast);
            SegLast = min(SegFirst + SegSize - 1, WindowLast);

            if (Remainder == 1)
                SegSize--;

            if (Remainder > 0)
                Remainder--;
        }
    }

    if (!bResult)
    {
        dwStatus = GetLastError();
        printf("ERROR: ReadFile failed: %08x\n", dwStatus);
        goto Exit;
    }

    dwStatus = 0;

Exit:
    CryptDestroyHash(hHash);
    CryptReleaseContext(hProv, 0);
    CloseHandle(hFile);

    return dwStatus;
}