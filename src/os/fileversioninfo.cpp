/*
    Copyright 2014-2019 Paul Colby

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

#include "fileversioninfo.h"

#include <windows.h>

/// The caller is responsible for freeing the result via HeapFree.
LPTSTR getModuleFileName()
{
    SIZE_T size = MAX_PATH + 1;
    LPTSTR fileName = NULL;

    SetLastError(ERROR_SUCCESS);
    do {
        // [Re]Allocate memory for the module filename.
        if (fileName == NULL) {
            if ((fileName = (LPTSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, size)) == NULL) {
                return NULL;
            }
        } else {
            size *= 2;
            LPTSTR newBuffer = (LPTSTR)HeapReAlloc(GetProcessHeap(), 0, fileName, size);
            if (newBuffer == NULL) {
                HeapFree(GetProcessHeap(), 0, fileName);
                return NULL;
            } else {
                fileName = newBuffer;
            }
        }

        // Fetch the module filename.
        if (GetModuleFileName(NULL, fileName, size) == size) {
            // We need at least one more character for the terminator.
            SetLastError(ERROR_INSUFFICIENT_BUFFER);
        }
    } while (GetLastError() == ERROR_INSUFFICIENT_BUFFER);
    return fileName;
}

FileVersionInfo::FileVersionInfo(const QString &fileName) : versionInfo(NULL)
{
    LPTSTR szFileName = NULL;
#ifdef UNICODE
    if (fileName.isEmpty()) {
        szFileName = getModuleFileName();
    } else {
        szFileName = new TCHAR [fileName.length() + 1];
        fileName.toWCharArray(szFileName);
        szFileName[fileName.length()] = '\0';
    }
#else
    QByteArray local8BitFileName;
    if (fileName.isEmpty()) {
        szFileName = getModuleFileName();
    } else {
        local8BitFileName = fileName.toLocal8Bit();
        szFileName = local8BitFileName.constData();
    }
#endif
    if (szFileName == NULL) {
        return;
    }

    DWORD ignored, versionInfoSize;
    if (((versionInfoSize = GetFileVersionInfoSize(szFileName, &ignored)) > 0) &&
        ((versionInfo = HeapAlloc(GetProcessHeap(), 0, versionInfoSize)) != NULL)) {
        if (!GetFileVersionInfo(szFileName, ignored, versionInfoSize, versionInfo)) {
            HeapFree(GetProcessHeap(), 0, versionInfo);
            versionInfo = NULL;
        }
    }

    if (fileName.isEmpty()) {
        HeapFree(GetProcessHeap(), 0, szFileName);
    }
#ifdef UNICODE
    else {
        delete szFileName;
    }
#endif
}

FileVersionInfo::~FileVersionInfo()
{
    if (versionInfo != NULL) {
        HeapFree(GetProcessHeap(), 0, versionInfo);
    }
}

bool FileVersionInfo::isValid() const
{
    return (versionInfo != NULL);
}

QString FileVersionInfo::fileInfo(const QString &name) const
{
    QString result = fileInfo(name, US_ENGLISH, UTF_16);
    if (result.isEmpty()) {
        result = fileInfo(name, US_ENGLISH, ANSI_LATIN_1);
    }
    return result;
}

QString FileVersionInfo::fileInfo(const QString &name, const quint16 lang,
                                  const quint16 &codepage) const
{
    if (!isValid()) {
        return QString();
    }

    const QString subBlock = QString::fromLatin1("\\StringFileInfo\\%1%2\\%3")
        .arg(lang, 4, 16, QLatin1Char('0')).arg(codepage, 4, 16, QLatin1Char('0'))
        .arg(name);

#ifdef UNICODE
    LPTSTR szSubBlock = new TCHAR [subBlock.length() + 1];
    subBlock.toWCharArray(szSubBlock);
    szSubBlock[subBlock.length()] = '\0';
#else
    QByteArray local8BitSubBlock = subBlock.toLocal8Bit();
    LPTSTR szSubBlock = local8BitSubBlock.constData();
#endif

    // Get the product version from the version info resource.
    QString result;
    LPVOID * value = NULL;
    UINT valueLength = 0;
    if (VerQueryValue(versionInfo, szSubBlock, reinterpret_cast<LPVOID *>(&value), &valueLength)) {
#ifdef UNICODE
        result = QString::fromUtf16(reinterpret_cast<const ushort *>(value));
#else
        result = QString::fromLocal8Bit(reinterpret_cast<const char *>(value));
#endif
    }

#ifdef UNICODE
    delete szSubBlock;
#endif
    return result;
}

QList<quint16> FileVersionInfo::fileVersion() const
{
    QList<quint16> list;
    if (!isValid()) {
        return list;
    }
    VS_FIXEDFILEINFO * fixedInfo = NULL;
    UINT fixedInfoLength = 0;
    if (VerQueryValue(versionInfo, L"\\", reinterpret_cast<LPVOID *>(&fixedInfo), &fixedInfoLength)) {
        list << HIWORD(fixedInfo->dwFileVersionMS)
             << LOWORD(fixedInfo->dwFileVersionMS)
             << HIWORD(fixedInfo->dwFileVersionLS)
             << LOWORD(fixedInfo->dwFileVersionLS);
    }
    return list;
}
