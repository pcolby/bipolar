/// LICENSE

//#include "testa.h"
//#include "testb.h"
//#include "testc.h"

#include <QTest>

#include <stdio.h>

typedef QObject * (*ObjectConstructor)();

class ObjectFactory : public QMap<QByteArray, ObjectConstructor> {
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
    app.setAttribute(Qt::AA_Use96Dpi, true);

    // Setup our tests factory object.
    ObjectFactory testFactory;
    //testFactory.registerClass<TestA>();
    //testFactory.registerClass<TestB>();
    //testFactory.registerClass<TestC>();

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
