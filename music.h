#ifndef MUSIC_H
#define MUSIC_H

#include <QMainWindow>
#include <QMediaPlayer>
#include <QMediaPlaylist>
#include <QPushButton>
#include <QSlider>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QListWidget>
#include <QLabel>
#include <QSpacerItem>
#include <QDebug>

class Music : public QMainWindow
{
    Q_OBJECT

public:
    Music(QWidget *parent = nullptr);
    ~Music();

private:
    QPushButton *psButton;

};
#endif // MUSIC_H
