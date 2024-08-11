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
#include <QFileDialog>

/* 媒体信息结构体 */
struct MediaObjectInfo {
    /* 用于保存歌曲文件名 */
    QString fileName;
    /* 用于保存歌曲文件路径 */
    QString filePath;
};

class Music : public QMainWindow
{
    Q_OBJECT

public:
    Music(QWidget *parent = nullptr);
    ~Music();

private:
    void initMediaLibrary();
    void initScreenSize();

    /* 音乐播放器按钮 */
    QPushButton *pushButton[7];
    QPushButton *exitButton;
    QPushButton *openMediaDirButton;

    /* 垂直布局 */
    QVBoxLayout *vBoxLayout[3];

    /* 水平布局 */
    QHBoxLayout *hBoxLayout[4];

    /* 垂直容器 */
    QWidget *vWidget[3];

    /* 水平容器 */
    QWidget *hWidget[4];

    /* 标签文本 */
    QLabel *label[4];

    /* 用于遮罩 */
    QWidget *listMask;

    /* 媒体播放器，用于播放音乐 */
    QMediaPlayer *musicPlayer;

    /* 媒体信息存储 */
    QVector<MediaObjectInfo> mediaObjectInfo;

    /* 媒体列表 */
    QMediaPlaylist *mediaPlaylist;

    /* 音乐列表 */
    QListWidget *listWidget;

    /* 播放进度条 */
    QSlider *durationSlider;

    /* scan music position */
    void scanMusic();

signals:


private slots:
    void on_exitButton_clicked();
    void on_openMediaDirButton_clicked();
    void btn_previous_clicked();
    void btn_play_clicked();
    void btn_next_clicked();

};
#endif // MUSIC_H
