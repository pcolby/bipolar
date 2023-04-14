// SPDX-FileCopyrightText: 2014-2019 Paul Colby <git@colby.id.au>
// SPDX-License-Identifier: GPL-3.0-or-later

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
