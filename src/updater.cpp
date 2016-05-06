#include "updater.h"
#ifndef Q_OS_WIN

Updater::Updater(QString dir, UNO *parent) {
    this->parent = parent;
    wdir = dir + "/update";
    curr = dir;
    p = new QProcess(parent);
    p->setStandardErrorFile(wdir + "/update.log");
    connect(p, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(next(int, QProcess::ExitStatus)));

    QDir wqdir = QDir(wdir);
    if (wqdir.exists())
        wqdir.removeRecursively();
    wqdir.mkdir(wdir);
}

void Updater::start() {
    git();
}

void Updater::next(int code, QProcess::ExitStatus status) {
    if (code != 0 || status == QProcess::CrashExit) {
        emit error(p->objectName());
        return;
    }

    if (p->objectName() == "git")
        configure();
    else if (p->objectName() == "configure")
        make();
    else if (p->objectName() == "make")
        files();
}

void Updater::git() {
    emit step("git");
    p->setObjectName("git");
    p->setProgram("git");
    p->setArguments(QStringList() << "clone" << "https://github.com/TheShayy/UNO.git");
    p->setWorkingDirectory(wdir);
    p->start();
}

void Updater::configure() {
    emit step("configure");
    p->setObjectName("configure");
    wdir += "/UNO";
    p->setProgram("/bin/bash");
    p->setArguments(QStringList() << wdir + "/configure");
    p->setWorkingDirectory(wdir);
    p->start();
}

void Updater::make() {
    emit step("make");
    p->setObjectName("make");
    wdir += "/src";
    p->setProgram("make");
    p->setArguments(QStringList() << "-j" + QString::number(QThread::idealThreadCount() != -1 ? QThread::idealThreadCount() : 1));
    p->setWorkingDirectory(wdir);
    p->start();
}

void Updater::files() {
    emit step("files");
    p->setObjectName("files");
    QDir wqdir = QDir(wdir + "/translations");
    QFile f("");
    QFileInfoList files = wqdir.entryInfoList();

    f.setFileName(curr + "/UNObot");
    f.remove();
    if (!f.copy(wdir + "/../build/UNObot", curr + "/UNObot")) {
        emit error("files");
        parent->log(UNO::ERROR, "UNObot: " + f.errorString());
        return;
    }

    f.setFileName(curr + "/startUNO");
    f.remove();
    if (!f.copy(wdir + "/startUNO", curr + "/startUNO")) {
        emit error("files");
        parent->log(UNO::ERROR, "startUNO: " + f.errorString());
        return;
    }

    foreach (QFileInfo w, files) {
        if (!w.isDir()) {
            f.setFileName(curr + "/translations/" + w.fileName());
            f.remove();
            QDir(curr + "/translations/" + w.fileName()).mkpath(curr + "/translations/");
            if (!f.copy(w.absoluteFilePath(), curr + "/translations/" + w.fileName())) {
                emit error("files");
                parent->log(UNO::ERROR, w.fileName() + ": " + f.errorString());
                return;
            }
        }
    }

    emit done();
}

#endif
