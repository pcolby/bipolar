// SPDX-FileCopyrightText: 2014-2019 Paul Colby <git@colby.id.au>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "polar/v2/testtrainingsession.h"
#include "protobuf/testfixnum.h"
#include "protobuf/testmessage.h"
#include "protobuf/testvarint.h"

#include <QTest>

#include <locale.h>
#include <stdio.h>

typedef QObject * (*ObjectConstructor)();

class ObjectFactory : public QMultiMap<QByteArray, ObjectConstructor> {
public:
    template<class T> void registerClass()
    {
        insert(T::staticMetaObject.className(), &objectConstructor<T>);
    }

    template<class T> T * createObject(const QByteArray &className) const
    {
        ObjectConstructor constructor = value(className);
        return (constructor == NULL) ? NULL : (*constructor)();
    }

private:
    template<class T> static QObject * objectConstructor()
    {
        return new T();
    }
};

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);
    app.setApplicationVersion(QLatin1String("1.2.3.4"));
    app.setAttribute(Qt::AA_Use96Dpi, true);

    // If the BIPOLAR_TEST_LOCALE environment variable is set, then set this
    // test application's locale accordingly.  The same can be done other ways,
    // but its near impossible to acheive externally on AppVeyor, and at least
    // here we get to bail if the setlocale call fails.
    const QByteArray locale = qgetenv("BIPOLAR_TEST_LOCALE");
    if (!locale.isEmpty()) {
        if (setlocale(LC_ALL, locale.data()) == NULL) {
            fprintf(stderr, "failed to set locale '%s'\n", locale.data());
            return EXIT_FAILURE;
        }
        fprintf(stdout, "set locale '%s'\n", locale.data());
    }

    // Setup our tests factory object.
    ObjectFactory testFactory;
    testFactory.registerClass<TestFixnum>();
    testFactory.registerClass<TestMessage>();
    testFactory.registerClass<TestTrainingSession>();
    testFactory.registerClass<TestVarint>();

    // If the user has specified a Test* class name, execute that test class only.
    for (int index = 1; index < argc; ++index) {
        if (qstrcmp(argv[index], "-classes") == 0) {
            foreach (const QByteArray &className, testFactory.uniqueKeys()) {
                fprintf(stdout, "%s\n", className.data());
            }
            return EXIT_SUCCESS;
        } else if (qstrncmp(argv[index], "Test", 4) == 0) {
            QStringList args = app.arguments();
            args.removeOne(QString::fromLocal8Bit(argv[index]));
            QObject * testObject = testFactory.createObject<QObject>(argv[index]);
            if (!testObject) {
                fprintf(stderr, "test class %s is unknown\n", argv[index]);
                return EXIT_FAILURE;
            }
            return (QTest::qExec(testObject, args) == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
        }
    }

    // Otherwise, execute all registered test classes.
    QList<QByteArray> failedClassNames;
    foreach (const QByteArray &className, testFactory.uniqueKeys()) {
        QObject * testObject = testFactory.createObject<QObject>(className);
        if ((!testObject) || (QTest::qExec(testObject, argc, argv) != 0)) {
            failedClassNames.append(className);
        }
    }

    // Report our success / failure.
    fprintf(stdout, "%d of %d test classes completed successfully\n",
            (testFactory.size() - failedClassNames.size()), testFactory.size());
    foreach (const QByteArray &className, failedClassNames) {
        fprintf(stdout, "Failed class: %s\n", className.data());
    }
    return (failedClassNames.empty()) ? EXIT_SUCCESS : EXIT_FAILURE;
}
