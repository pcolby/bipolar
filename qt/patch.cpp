    // Example: /v2/users/1234567/training-sessions/23456789/exercises/34567890/autolaps
    if ((data != NULL) && (!data->isSequential())) {
        const QString &urlPath = request.url().path();
        if (urlPath.contains(QLatin1String("training-sessions"))) {
            QString dataPath = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
            if (dataPath.isEmpty()) {
                dataPath = QDir::currentPath();
            } else {
                QDir dir;
                dir.mkpath(dataPath);
            }
            QString fileName;
            foreach (const QChar &c, urlPath) {
                fileName += (c.isLetterOrNumber()) ? c : QChar::fromLatin1('-');
            }
            QFile file(dataPath + QDir::separator() + fileName);
            if (file.open(QIODevice::WriteOnly)) {
                file.write(data->readAll());
            }
        }
    }