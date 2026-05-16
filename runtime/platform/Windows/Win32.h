/*===-- src/Runtime/Windows/Win32.h - Minimal Win32 declarations ----------===
 *
 * Private header — not installed, used only by src/Runtime/Windows/ sources.
 * Avoids including <windows.h>.
 *===----------------------------------------------------------------------===*/

#ifndef FLY_RUNTIME_WINDOWS_WIN32_H
#define FLY_RUNTIME_WINDOWS_WIN32_H

#define WIN32_IMPORT  __declspec(dllimport)
#define WINAPI        __stdcall

typedef void *             HANDLE;
typedef unsigned long      DWORD;
typedef unsigned long long ULONG_PTR;
typedef unsigned long long SIZE_T;
typedef int                BOOL;
typedef void *             LPVOID;
typedef const void *       LPCVOID;
typedef unsigned long *    LPDWORD;

#define WIN32_NULL      ((HANDLE)0)
#define INVALID_HANDLE  ((HANDLE)(ULONG_PTR)-1)

/* VirtualAlloc / VirtualFree */
#define PAGE_READWRITE  0x04
#define MEM_COMMIT      0x00001000
#define MEM_RESERVE     0x00002000
#define MEM_RELEASE     0x00008000

WIN32_IMPORT LPVOID WINAPI VirtualAlloc(LPVOID lpAddress, SIZE_T dwSize,
                                        DWORD flAllocationType, DWORD flProtect);
WIN32_IMPORT BOOL   WINAPI VirtualFree(LPVOID lpAddress, SIZE_T dwSize,
                                       DWORD dwFreeType);

/* Console / file I/O */
#define STD_INPUT_HANDLE   ((DWORD)-10)
#define STD_OUTPUT_HANDLE  ((DWORD)-11)
#define STD_ERROR_HANDLE   ((DWORD)-12)

WIN32_IMPORT HANDLE WINAPI GetStdHandle(DWORD nStdHandle);
WIN32_IMPORT BOOL   WINAPI WriteFile(HANDLE hFile, LPCVOID lpBuffer,
                                     DWORD nNumberOfBytesToWrite,
                                     LPDWORD lpNumberOfBytesWritten,
                                     LPVOID lpOverlapped);

/* Process / thread */
WIN32_IMPORT void   WINAPI ExitProcess(DWORD uExitCode);

typedef DWORD (WINAPI *LPTHREAD_START_ROUTINE)(LPVOID lpThreadParameter);

WIN32_IMPORT HANDLE WINAPI CreateThread(LPVOID lpThreadAttributes,
                                        SIZE_T dwStackSize,
                                        LPTHREAD_START_ROUTINE lpStartAddress,
                                        LPVOID lpParameter,
                                        DWORD dwCreationFlags,
                                        LPDWORD lpThreadId);
WIN32_IMPORT BOOL   WINAPI CloseHandle(HANDLE hObject);

/* WaitOnAddress / WakeByAddress (Windows 8+ / kernelbase.dll) */
#define WIN32_INFINITE  0xFFFFFFFFUL

WIN32_IMPORT BOOL WINAPI WaitOnAddress(volatile void *Address,
                                       LPVOID CompareAddress,
                                       SIZE_T AddressSize,
                                       DWORD dwMilliseconds);
WIN32_IMPORT void WINAPI WakeByAddressSingle(LPVOID Address);
WIN32_IMPORT void WINAPI WakeByAddressAll(LPVOID Address);

/* ── File I/O (via UCRT low-level CRT functions) ────────────────────────── */

/* Flag values for _open (same semantics as O_* on POSIX, different numbers) */
#define WIN_O_RDONLY   0x0000
#define WIN_O_WRONLY   0x0001
#define WIN_O_RDWR     0x0002
#define WIN_O_APPEND   0x0008
#define WIN_O_CREAT    0x0100
#define WIN_O_TRUNC    0x0200

/* UCRT low-level I/O — available in msvcrt.dll / ucrt.dll */
typedef unsigned int uint_rt;

WIN32_IMPORT int       __cdecl _open(const char *path, int oflag, int pmode);
WIN32_IMPORT int       __cdecl _close(int fd);
WIN32_IMPORT int       __cdecl _read(int fd, void *buf, uint_rt count);
WIN32_IMPORT int       __cdecl _write(int fd, const void *buf, uint_rt count);
WIN32_IMPORT long long __cdecl _lseeki64(int fd, long long offset, int origin);

/* stat-like via GetFileAttributesExA */
typedef struct {
    DWORD dwFileAttributes;
    /* FILETIME = two DWORDs */
    DWORD ftCreationTime_lo;    DWORD ftCreationTime_hi;
    DWORD ftLastAccessTime_lo;  DWORD ftLastAccessTime_hi;
    DWORD ftLastWriteTime_lo;   DWORD ftLastWriteTime_hi;
    DWORD nFileSizeHigh;
    DWORD nFileSizeLow;
} WIN32_FILE_ATTRIBUTE_DATA;

#define FILE_ATTRIBUTE_DIRECTORY 0x10

WIN32_IMPORT BOOL WINAPI GetFileAttributesExA(const char *lpFileName,
                                               int fInfoLevelId,
                                               void *lpFileInformation);

WIN32_IMPORT BOOL WINAPI CreateDirectoryA(const char *lpPathName,
                                           LPVOID lpSecurityAttributes);
WIN32_IMPORT BOOL WINAPI RemoveDirectoryA(const char *lpPathName);
WIN32_IMPORT BOOL WINAPI DeleteFileA(const char *lpFileName);
WIN32_IMPORT BOOL WINAPI MoveFileExA(const char *lpExistingFileName,
                                      const char *lpNewFileName,
                                      DWORD dwFlags);
WIN32_IMPORT BOOL WINAPI CreateSymbolicLinkA(const char *lpSymlinkFileName,
                                              const char *lpTargetFileName,
                                              DWORD dwFlags);
WIN32_IMPORT HANDLE WINAPI CreateFileA(const char *lpFileName, DWORD dwDesiredAccess,
                                        DWORD dwShareMode, LPVOID lpSecurityAttributes,
                                        DWORD dwCreationDisposition,
                                        DWORD dwFlagsAndAttributes, HANDLE hTemplateFile);
WIN32_IMPORT BOOL   WINAPI FlushFileBuffers(HANDLE hFile);
WIN32_IMPORT BOOL   WINAPI SetFilePointerEx(HANDLE hFile, LARGE_INTEGER_rt liDistanceToMove,
                                             LPVOID lpNewFilePointer, DWORD dwMoveMethod);
WIN32_IMPORT BOOL   WINAPI SetEndOfFile(HANDLE hFile);
WIN32_IMPORT BOOL   WINAPI SetFileAttributesA(const char *lpFileName, DWORD dwFileAttributes);

#define MOVEFILE_REPLACE_EXISTING   0x00000001UL
#define GENERIC_WRITE               0x40000000UL
#define OPEN_EXISTING               3UL
#define FILE_ATTRIBUTE_NORMAL       0x00000080UL
#define FILE_ATTRIBUTE_READONLY     0x00000001UL
#define FILE_BEGIN                  0UL

/* ── Time ───────────────────────────────────────────────────────────────── */

typedef struct { DWORD dwLowDateTime; DWORD dwHighDateTime; } FILETIME_rt;
typedef union  { FILETIME_rt ft; unsigned long long val; }   FILETIME_u;
typedef struct { long long QuadPart; } LARGE_INTEGER_rt;

WIN32_IMPORT void  WINAPI GetSystemTimeAsFileTime(FILETIME_rt *lpSystemTimeAsFileTime);
WIN32_IMPORT BOOL  WINAPI QueryPerformanceCounter(LARGE_INTEGER_rt *lpPerformanceCount);
WIN32_IMPORT BOOL  WINAPI QueryPerformanceFrequency(LARGE_INTEGER_rt *lpFrequency);
WIN32_IMPORT void  WINAPI Sleep(DWORD dwMilliseconds);

/* ── Environment ────────────────────────────────────────────────────────── */

WIN32_IMPORT DWORD WINAPI GetCurrentDirectoryA(DWORD nBufferLength, char *lpBuffer);
WIN32_IMPORT BOOL  WINAPI SetCurrentDirectoryA(const char *lpPathName);
WIN32_IMPORT DWORD WINAPI GetEnvironmentVariableA(const char *lpName, char *lpBuffer, DWORD nSize);
WIN32_IMPORT BOOL  WINAPI SetEnvironmentVariableA(const char *lpName, const char *lpValue);
WIN32_IMPORT char *WINAPI GetEnvironmentStringsA(void);
WIN32_IMPORT BOOL  WINAPI FreeEnvironmentStringsA(char *lpszEnvironmentBlock);
WIN32_IMPORT BOOL  WINAPI GetComputerNameA(char *lpBuffer, LPDWORD lpnSize);

#endif /* FLY_RUNTIME_WINDOWS_WIN32_H */
