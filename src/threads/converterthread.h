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

#ifndef __CONVERTER_THREAD__
#define __CONVERTER_THREAD__

#include <QThread>

class ConverterThread : public QThread {
    Q_OBJECT
    Q_PROPERTY(bool cancelled READ isCancelled)

public:
    ConverterThread(QObject * const parent = 0);
    bool isCancelled() const;

public slots:
    void cancel();

protected:
    bool cancelled;

    virtual void run();

signals:
    void progress(const int index);

};

#endif // __CONVERTER_THREAD__
