#include "stdjk/IO/File.h"
#include <Windows.h>

IntPtr __io__File_GetStd(USize Who) {
    if (Who == 0) return Cast<IntPtr>(GetStdHandle(STD_OUTPUT_HANDLE));
    else if (Who == 1) return Cast<IntPtr>(GetStdHandle(STD_ERROR_HANDLE));

    return IntPtr(-1);
}

IntPtr __io__File_Create(Str FilePath, io::FileError& Err) {
    HANDLE handle = CreateFileA(
        Cast<LPCSTR>(FilePath),
        GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ,
        nullptr, CREATE_ALWAYS, 0, 0
    );

    DWORD ec = GetLastError();
    if (ec != S_OK && ec != ERROR_ALREADY_EXISTS) {
        Err = io::WriteError;
    }

    return Cast<IntPtr>(handle);
}

IntPtr __io__File_Open(Str FilePath, io::FileError& Err) {
    HANDLE handle = CreateFileA(
        Cast<LPCSTR>(FilePath),
        GENERIC_READ, FILE_SHARE_READ,
        nullptr, OPEN_EXISTING, 0, 0
    );

    DWORD ec = GetLastError();
    if (ec == ERROR_FILE_NOT_FOUND || ec == ERROR_PATH_NOT_FOUND) {
        Err = io::NotExists;
    }

    return Cast<IntPtr>(handle);
}

Int __io__File_Write(IntPtr Handle, const Byte* Buff, USize Count) {
    DWORD writed = 0;

    WriteFile(
        Cast<HANDLE>(Handle), 
        Buff, IntCast<DWORD>(Count),
        &writed, nullptr
    );

    if (writed != Count) {
        return io::WriteError;
    }

    return io::Success;
}

Int __io__File_Read(IntPtr Handle, Byte* Buff, USize Count) {
    DWORD readed = 0;

    ReadFile(
        Cast<HANDLE>(Handle), 
        Buff, IntCast<DWORD>(Count), 
        &readed, nullptr
    );

    if (readed != Count) {
        return io::ReadError;
    }

    return io::Success;
}

USize __io__File_Size(IntPtr Handle) {
    LARGE_INTEGER size;
    GetFileSizeEx(Cast<HANDLE>(Handle), &size);
    return size.QuadPart;
}

void __io__File_Close(IntPtr Handle) {
    CloseHandle(Cast<HANDLE>(Handle));
}

