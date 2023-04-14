// SPDX-FileCopyrightText: 2014-2019 Paul Colby <git@colby.id.au>
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QObject>

namespace tools {
namespace variant {


void sanitize(QVariant &variant);

bool writeAll(const QVariantMap &variant, const QString &fileName);
bool writeData(const QVariantMap &variant, const QString &fileName);
bool writeJson(const QVariantMap &variant, const QString &fileName);

}}
