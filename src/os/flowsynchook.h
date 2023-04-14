// SPDX-FileCopyrightText: 2014-2019 Paul Colby <git@colby.id.au>
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef __FLOWSYNC_HOOK_H__
#define __FLOWSYNC_HOOK_H__

#include <QDir>

class FlowSyncHook {

public:

    static QDir flowSyncDir(bool *found = NULL);

    static QDir installableHookDir();

    static bool install(const QDir &fromDir, QDir toDir);

    static int getVersion(const QDir &dir);

};

#endif // __FLOWSYNC_HOOK_H__
