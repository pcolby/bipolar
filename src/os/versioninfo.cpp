/*
    Copyright 2014 Paul Colby

    This file is part of Bipolar.

    Bipolar is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Biplar is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Bipolar.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "versioninfo.h"

#ifdef Q_OS_WIN // VersionInfo is a Windows-only class.
#include <windows.h>

// This function is pure win32 code.
bool VersionInfo::getAppVersion(quint16 &major, quint16 &minor, quint16 &error, quint16 &build) {
    // Get the name of this executable.
    WCHAR FileName[MAX_PATH+1];
    if (!GetModuleFileName(NULL,FileName,MAX_PATH)) return false;

    // Get the size of the version info resource.
    DWORD Ignored, VerInfoSize=GetFileVersionInfoSize(FileName,&Ignored);
    if (!VerInfoSize) return false;

    // Get the version info resource.
    LPVOID VerInfo=HeapAlloc(GetProcessHeap(),0,VerInfoSize);
    if (!VerInfo) return false;
    if (!GetFileVersionInfo(FileName,Ignored,VerInfoSize,VerInfo)) {
            HeapFree(GetProcessHeap(),0,VerInfo);
            return false;
    }

    // Get the product version from the version info resource.
    VS_FIXEDFILEINFO *Ver; UINT VerLen;
    if (VerQueryValue(VerInfo, L"\\", reinterpret_cast<LPVOID *>(&Ver), &VerLen)) {
            major=HIWORD(Ver->dwFileVersionMS);
            minor=LOWORD(Ver->dwFileVersionMS);
            error=HIWORD(Ver->dwFileVersionLS);
            build=LOWORD(Ver->dwFileVersionLS);
    }

    // Clean up and return.
    HeapFree(GetProcessHeap(),0,VerInfo);
    return true;
}

bool VersionInfo::getAppVersion(quint16 &major, quint16 &minor, quint16 &error) {
    quint16 ignored;
    return getAppVersion(major, minor, error, ignored);
}

bool VersionInfo::getAppVersion(quint16 &major, quint16 &minor) {
    quint16 ignored;
    return getAppVersion(major, minor, ignored);
}

QString VersionInfo::getAppVersionStr() {
    quint16 major, minor, error, build;
    return (getAppVersion(major,minor,error,build)) ?
            QString::fromLatin1("%1.%2.%3.%4").arg(major).arg(minor).arg(error).arg(build) : QString();
}

#endif // Q_OS_WIN
