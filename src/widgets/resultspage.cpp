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

#include "resultspage.h"

#include <QDebug>
#include <QDateTime>
#include <QDir>
#include <QProgressBar>
#include <QPushButton>
#include <QRegExp>
#include <QSettings>
#include <QTextEdit>
#include <QVBoxLayout>

ResultsPage * ResultsPage::instance = NULL;

ResultsPage::ResultsPage(QWidget *parent)
    : QWizardPage(parent), previousMessageHandler(NULL)
{
    setTitle(tr("Converting..."));
    setButtonText(QWizard::FinishButton, tr("Close"));

    progressBar = new QProgressBar();

    showDetailsButton = new QPushButton(tr("Show details"));

    detailsBox = new QTextEdit();
    detailsBox->setReadOnly(true);
   //detailsBox->setVisible(false); /// @todo Reinstate this post-dev.

    QVBoxLayout * const vBox = new QVBoxLayout;
    vBox->addWidget(progressBar);
    vBox->addWidget(showDetailsButton);
    vBox->addWidget(detailsBox);
    setLayout(vBox);

    connect(showDetailsButton, SIGNAL(clicked()), this, SLOT(showDetails()));
}

void ResultsPage::initializePage()
{
    Q_ASSERT(instance == NULL);
    instance = this;
    previousMessageHandler = qInstallMessageHandler(&ResultsPage::messageHandler);

    QSettings settings;

    QRegExp regex(QLatin1String("(v2-users-[^-]+-training-sessions-[^-]+)-.*"));
    foreach (const QString &folder,
             settings.value(QLatin1String("inputFolders")).toStringList()) {
        QDir dir(folder);
        foreach (const QFileInfo &info, dir.entryInfoList()) {
            if (regex.exactMatch(info.fileName())) {
                const QString baseName = dir.absoluteFilePath(regex.cap(1));
                if (!sessionBaseNames.contains(baseName)) {
                    sessionBaseNames.append(baseName);
                }
            }
        }
    }

    qDebug() << sessionBaseNames.size() << "training sessions to examine";
    progressBar->setRange(0, sessionBaseNames.size());
    progressBar->reset();
}

bool ResultsPage::isComplete() const
{
    return false;
}

// Protected methods.

void ResultsPage::messageHandler(QtMsgType type,
                                 const QMessageLogContext &context,
                                 const QString &message)
{
    Q_ASSERT(instance != NULL);
    if (instance) {
        instance->onMessage(type, context, message);
    }
}

void ResultsPage::onMessage(QtMsgType type, const QMessageLogContext &context,
                            const QString &message)
{
    Q_UNUSED(context)
    QString level(QLatin1String("invalid"));
    switch (type) {
    case QtDebugMsg:    level = QLatin1String("Debug");    break;
    case QtWarningMsg:  level = QLatin1String("Warning");  break;
    case QtCriticalMsg: level = QLatin1String("Critical"); break;
    case QtFatalMsg:    level = QLatin1String("Fatal");    break;
    }
    detailsBox->append(tr("%1 %2 %3")
        .arg(QDateTime::currentDateTime().toString())
        .arg(level).arg(message));
}

// Protected slots.

void ResultsPage::showDetails()
{
    showDetailsButton->setVisible(false);
    detailsBox->setVisible(true);
}
