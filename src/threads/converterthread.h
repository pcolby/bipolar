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

#ifndef __CONVERTER_THREAD__
#define __CONVERTER_THREAD__

#include <QStringList>
#include <QThread>

namespace polar { namespace v2 { class TrainingSession; } }

class ConverterThread : public QThread {
    Q_OBJECT
    Q_PROPERTY(bool cancelled READ isCancelled)
    Q_PROPERTY(QStringList baseNames READ sessionBaseNames NOTIFY sessionBaseNamesChanged)

public:
    struct { int failed, written; } files;
    struct { int failed, processed, skipped; } sessions;

    explicit ConverterThread(QObject * const parent = 0);
    bool isCancelled() const;
    const QStringList &sessionBaseNames() const;

public slots:
    void cancel();

protected:
    bool cancelled;
    QStringList baseNames;

    void findSessionBaseNames();
    void proccessSession(const QString &baseName);
    virtual void run();
    virtual void setTrainingSessionOptions(polar::v2::TrainingSession * const session);

signals:
    void progress(const int index);
    void sessionBaseNamesChanged(const int size);

};

#endif // __CONVERTER_THREAD__
