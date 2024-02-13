#include "jkr/IO/File.h"
#include <Windows.h>

IntPtr __io__File_GetStd(USize Who) {
    if (Who == 0) return Cast<IntPtr>(GetStdHandle(STD_OUTPUT_HANDLE));
    else if (Who == 1) return Cast<IntPtr>(GetStdHandle(STD_ERROR_HANDLE));

    return IntPtr(-1);
}

IntPtr __io__File_Open(Str FilePath, io::FileMode Mode, io::FileError& Err) {
    DWORD access = 0;
    DWORD creationDispose = 0;

    if (Mode & io::FileRead) {
        access |= GENERIC_READ;
        creationDispose = OPEN_EXISTING;
    }
    if (Mode & io::FileWrite) {
        access |= GENERIC_WRITE;
        creationDispose = CREATE_ALWAYS;
    }

    HANDLE handle = CreateFileA(
        Cast<LPCSTR>(FilePath),
        access, FILE_SHARE_READ,
        nullptr, creationDispose, 0, 0
    );

    DWORD ec = GetLastError();
    if (ec == ERROR_FILE_NOT_FOUND) {
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

