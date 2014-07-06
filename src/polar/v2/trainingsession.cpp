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

#include <QDir>
#include <QFileInfo>

namespace polar {
namespace v2 {

TrainingSession::TrainingSession(const QString &baseName)
    : baseName(baseName)
{

}

bool TrainingSession::isValid() const
{
    return parsedExercises.isEmpty();
}

bool TrainingSession::parse(const QString &baseName)
{
    parsedExercises.clear();

    if (!baseName.isEmpty()) {
        this->baseName = baseName;
    }

    if (this->baseName.isEmpty()) {
        emit parseError(QLatin1String("parse called with no baseName specified"));
        return false;
    }

    const QFileInfo fileInfo(baseName);
    const QDir dir(fileInfo.baseName(), fileInfo.fileName(), QDir::NoSort, QDir::Readable|QDir::Files);
    QMap<QString, QMap<QString, QString> > fileNames;
    foreach (const QFileInfo &fileInfo, dir.entryInfoList()) {
        const QStringList nameParts = fileInfo.fileName().split(QLatin1Char('_'));
        if ((nameParts.size() >= 3) && (nameParts.at(nameParts.size() - 3) == QLatin1String("exercises"))) {
            fileNames[nameParts.at(nameParts.size() - 2)][nameParts.at(nameParts.size() - 1)] = fileInfo.path();
        }
    }

    return parse(fileNames);
}

bool TrainingSession::parse(const QMap<QString, QMap<QString, QString> > &fileNames)
{
    Q_UNUSED(fileNames);
    Q_ASSERT_X(false, __FUNCTION__, "not implemented yet");
    return false;
}

bool TrainingSession::writeGPX(const QString &fileName)
{
    Q_UNUSED(fileName);
    Q_ASSERT_X(false, __FUNCTION__, "not implemented yet");
    return false;
}

bool TrainingSession::writeGPX(const QIODevice &device)
{
    Q_UNUSED(device);
    Q_ASSERT_X(false, __FUNCTION__, "not implemented yet");
    return false;
}

bool TrainingSession::writeGPX(const bool separateFiles)
{
    Q_UNUSED(separateFiles);
    Q_ASSERT_X(false, __FUNCTION__, "not implemented yet");
    return false;
}

bool TrainingSession::writeTCX(const QStringList &sport)
{
    Q_UNUSED(sport);
    Q_ASSERT_X(false, __FUNCTION__, "not implemented yet");
    return false;
}

bool TrainingSession::writeTCX(const QString &fileName, const QStringList &sport)
{
    Q_UNUSED(fileName);
    Q_UNUSED(sport);
    Q_ASSERT_X(false, __FUNCTION__, "not implemented yet");
    return false;
}

bool TrainingSession::writeTCX(const QIODevice &device, const QStringList &sport)
{
    Q_UNUSED(device);
    Q_UNUSED(sport);
    Q_ASSERT_X(false, __FUNCTION__, "not implemented yet");
    return false;
}

bool TrainingSession::writeTCX(const bool separateFiles, const QStringList &sport)
{
    Q_UNUSED(separateFiles);
    Q_UNUSED(sport);
    Q_ASSERT_X(false, __FUNCTION__, "not implemented yet");
    return false;
}

}}
