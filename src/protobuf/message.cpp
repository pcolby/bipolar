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

#include "message.h"

#include "fixnum.h"
#include "varint.h"

#include <QBuffer>
#include <QDebug>

namespace ProtoBuf {

QVariantList parseMessage(QByteArray &data, int maxItems)
{
    QBuffer buffer(&data);
    buffer.open(QIODevice::ReadOnly);
    return parseMessage(buffer);
}

QVariantList parseMessage(QIODevice &data, int maxItems)
{
    Q_UNUSED(data);
    Q_UNUSED(maxItems);
    Q_ASSERT_X(false, __FUNCTION__, "not implemented yet");
    return QVariantList();
}

}
