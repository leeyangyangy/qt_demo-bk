#include "music.h"

#include <QCoreApplication>
#include <QFileInfoList>
#include <QDir>
#include <QProcess>

Music::Music(QWidget *parent)
    : QMainWindow(parent)
{

    psButton = new QPushButton(this);

    psButton->setText("play/stop");

    psButton->show();
}

Music::~Music()
{
}

