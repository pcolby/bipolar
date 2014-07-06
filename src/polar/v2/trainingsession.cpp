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

#include "trainingsession.h"

namespace polar {
namespace v2 {

TrainingSession::TrainingSession(const QString &basePath)
{
    Q_UNUSED(basePath);
    Q_ASSERT_X(false, __FUNCTION__, "not implemented yet");
}

bool parse()
{
    Q_ASSERT_X(false, __FUNCTION__, "not implemented yet");
    return false;
}

bool writeGPX(const QString &fileName)
{
    Q_UNUSED(fileName);
    Q_ASSERT_X(false, __FUNCTION__, "not implemented yet");
    return false;
}

bool writeGPX(const QIODevice &device)
{
    Q_UNUSED(device);
    Q_ASSERT_X(false, __FUNCTION__, "not implemented yet");
    return false;
}

bool writeGPX(const bool separateFiles)
{
    Q_UNUSED(separateFiles);
    Q_ASSERT_X(false, __FUNCTION__, "not implemented yet");
    return false;
}

bool writeTCX(const QStringList &sport)
{
    Q_UNUSED(sport);
    Q_ASSERT_X(false, __FUNCTION__, "not implemented yet");
    return false;
}

bool writeTCX(const QString &fileName, const QStringList &sport)
{
    Q_UNUSED(fileName);
    Q_UNUSED(sport);
    Q_ASSERT_X(false, __FUNCTION__, "not implemented yet");
    return false;
}

bool writeTCX(const QIODevice &device, const QStringList &sport)
{
    Q_UNUSED(device);
    Q_UNUSED(sport);
    Q_ASSERT_X(false, __FUNCTION__, "not implemented yet");
    return false;
}

bool writeTCX(const bool separateFiles, const QStringList &sport)
{
    Q_UNUSED(separateFiles);
    Q_UNUSED(sport);
    Q_ASSERT_X(false, __FUNCTION__, "not implemented yet");
    return false;
}

bool parse(const QStringList &laps, const QStringList &route,
           const QStringList &samples, const QStringList &zones)
{
    Q_UNUSED(laps);
    Q_UNUSED(route);
    Q_UNUSED(samples);
    Q_UNUSED(zones);
    Q_ASSERT_X(false, __FUNCTION__, "not implemented yet");
    return false;
}

}}
