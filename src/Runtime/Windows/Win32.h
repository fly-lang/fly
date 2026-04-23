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

WIN32_IMPORT BOOL WINAPI WaitOnAddress(volatile LPVOID Address,
                                       LPVOID CompareAddress,
                                       SIZE_T AddressSize,
                                       DWORD dwMilliseconds);
WIN32_IMPORT void WINAPI WakeByAddressSingle(LPVOID Address);
WIN32_IMPORT void WINAPI WakeByAddressAll(LPVOID Address);

#endif /* FLY_RUNTIME_WINDOWS_WIN32_H */
