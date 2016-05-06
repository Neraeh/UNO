#ifndef UPDATER_H
#define UPDATER_H

#include <QObject>
#include <QProcess>
#include <QDir>
#include <QFile>
#include <QThread>
#include "uno.h"

class Updater : public QObject
{
    Q_OBJECT
public:
    explicit Updater(QString dir, UNO *parent);
    void start();
private slots:
    void next(int code, QProcess::ExitStatus status);
    void git();
    void configure();
    void make();
    void files();
signals:
    void step(QString step);
    void done();
    void error(QString step);
private:
    QString wdir, curr;
    QProcess *p;
    UNO *parent;
};

#endif // UPDATER_H
