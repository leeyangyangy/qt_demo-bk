#include "music.h"

#include <QCoreApplication>
#include <QFileInfoList>
#include <QDir>
#include <QProcess>

Music::Music(QWidget *parent)
    : QMainWindow(parent)
{
    initScreenSize();

    // exit music player.
    exitButton = new QPushButton(this);
    exitButton->setText("exit music");
    connect(exitButton,&QPushButton::clicked,this,&Music::on_exitButton_clicked);

    openMediaDirButton = new QPushButton(this);
    openMediaDirButton->setText("open library");
    openMediaDirButton->setGeometry(200,0,150,30);
    connect(openMediaDirButton,SIGNAL(clicked()),this,SLOT(on_openMediaDirButton_clicked()));

    /* 按钮信号槽连接 */
    connect(pushButton[0],SIGNAL(clicked()),this,SLOT(btn_previous_clicked()));
    connect(pushButton[1],SIGNAL(clicked()),this,SLOT(btn_play_clicked()));
    connect(pushButton[2],SIGNAL(clicked()),this,SLOT(btn_next_clicked()));

    /* 媒体信号槽连接 */

}

Music::~Music()
{
}

void Music::on_exitButton_clicked(){
    this->close();
}

void Music::on_openMediaDirButton_clicked(){
    qDebug() << "Open Media Library Dir." << endl;
    initMediaLibrary();
    scanMusic();
}

void Music::initScreenSize(){
    qDebug() << "init screen" << endl;
    /* 设置位置与大小,这里固定为800, 480 */
    this->setGeometry(0, 0, 800, 480);
    QPalette pal;

    /* 按钮 */
    for (int i = 0; i < 7; i++)
        pushButton[i] = new QPushButton();

    /* 标签 */
    for (int i = 0; i < 4; i++)
        label[i] = new QLabel();

    for (int i = 0; i < 3; i++) {
        /* 垂直容器 */
        vWidget[i] = new QWidget();
        vWidget[i]->setAutoFillBackground(true);
        /* 垂直布局 */
        vBoxLayout[i] = new QVBoxLayout();
    }

    for (int i = 0; i < 4; i++) {
        /* 水平容器 */
        hWidget[i] = new QWidget();
        hWidget[i]->setAutoFillBackground(true);
        /* 水平布局 */
        hBoxLayout[i] = new QHBoxLayout();
    }

    /* 播放进度条 */
    durationSlider = new QSlider(Qt::Horizontal);
    durationSlider->setMinimumSize(300, 15);
    durationSlider->setMaximumHeight(15);
    durationSlider->setObjectName("durationSlider");

    /* 音乐列表 */
    listWidget = new QListWidget();
    listWidget->setObjectName("listWidget");
    listWidget->resize(310, 265);
    listWidget->setVerticalScrollBarPolicy(
                Qt::ScrollBarAlwaysOff);
    listWidget->setHorizontalScrollBarPolicy(
                Qt::ScrollBarAlwaysOff);

    /* 列表遮罩 */
    listMask = new QWidget(listWidget);
    listMask->setMinimumSize(310, 50);
    listMask->setMinimumHeight(50);
    listMask->setObjectName("listMask");
    listMask->setGeometry(0,
                          listWidget->height() - 50,
                          310,
                          50);


    /* 设置对象名称 */
    pushButton[0]->setObjectName("btn_previous");
    pushButton[1]->setObjectName("btn_play");
    pushButton[2]->setObjectName("btn_next");
    pushButton[3]->setObjectName("btn_favorite");
    pushButton[4]->setObjectName("btn_mode");
    pushButton[5]->setObjectName("btn_menu");
    pushButton[6]->setObjectName("btn_volume");

    /* 设置按钮属性 */
    pushButton[1]->setCheckable(true);
    pushButton[3]->setCheckable(true);

    /* H0布局 */
    vWidget[0]->setMinimumSize(310, 480);
    vWidget[0]->setMaximumWidth(310);
    vWidget[1]->setMinimumSize(320, 480);
    QSpacerItem *hSpacer0 = new
            QSpacerItem(70, 480,
                        QSizePolicy::Minimum,
                        QSizePolicy::Maximum);

    QSpacerItem *hSpacer1 = new
            QSpacerItem(65, 480,
                        QSizePolicy::Minimum,
                        QSizePolicy::Maximum);

    QSpacerItem *hSpacer2 = new
            QSpacerItem(60, 480,
                        QSizePolicy::Minimum,
                        QSizePolicy::Maximum);

    hBoxLayout[0]->addSpacerItem(hSpacer0);
    hBoxLayout[0]->addWidget(vWidget[0]);
    hBoxLayout[0]->addSpacerItem(hSpacer1);
    hBoxLayout[0]->addWidget(vWidget[1]);
    hBoxLayout[0]->addSpacerItem(hSpacer2);
    hBoxLayout[0]->setContentsMargins(0, 0, 0, 0);

    hWidget[0]->setLayout(hBoxLayout[0]);
    setCentralWidget(hWidget[0]);

    /* V0布局 */
    listWidget->setMinimumSize(310, 265);
    hWidget[1]->setMinimumSize(310, 80);
    hWidget[1]->setMaximumHeight(80);
    label[0]->setMinimumSize(310, 95);
    label[0]->setMaximumHeight(95);
    QSpacerItem *vSpacer0 = new
            QSpacerItem(310, 10,
                        QSizePolicy::Minimum,
                        QSizePolicy::Maximum);
    QSpacerItem *vSpacer1 = new
            QSpacerItem(310, 30,
                        QSizePolicy::Minimum,
                        QSizePolicy::Minimum);
    vBoxLayout[0]->addWidget(label[0]);
    vBoxLayout[0]->addWidget(listWidget);
    vBoxLayout[0]->addSpacerItem(vSpacer0);
    vBoxLayout[0]->addWidget(hWidget[1]);
    vBoxLayout[0]->addSpacerItem(vSpacer1);
    vBoxLayout[0]->setContentsMargins(0, 0, 0, 0);

    vWidget[0]->setLayout(vBoxLayout[0]);

    /* H1布局 */
    for (int i = 0; i < 3; i++) {
        pushButton[i]->setMinimumSize(80, 80);
    }
    QSpacerItem *hSpacer3 = new
            QSpacerItem(40, 80,
                        QSizePolicy::Expanding,
                        QSizePolicy::Expanding);
    QSpacerItem *hSpacer4 = new
            QSpacerItem(40, 80,
                        QSizePolicy::Expanding,
                        QSizePolicy::Expanding);
    hBoxLayout[1]->addWidget(pushButton[0]);
    hBoxLayout[1]->addSpacerItem(hSpacer3);
    hBoxLayout[1]->addWidget(pushButton[1]);
    hBoxLayout[1]->addSpacerItem(hSpacer4);
    hBoxLayout[1]->addWidget(pushButton[2]);
    hBoxLayout[1]->setContentsMargins(0, 0, 0, 0);

    hWidget[1]->setLayout(hBoxLayout[1]);

    /* V1布局 */
    QSpacerItem *vSpacer2 = new
            QSpacerItem(320, 40,
                        QSizePolicy::Minimum,
                        QSizePolicy::Maximum);
    QSpacerItem *vSpacer3 = new
            QSpacerItem(320, 20,
                        QSizePolicy::Minimum,
                        QSizePolicy::Maximum);
    QSpacerItem *vSpacer4 = new
            QSpacerItem(320, 30,
                        QSizePolicy::Minimum,
                        QSizePolicy::Minimum);
    label[1]->setMinimumSize(320, 320);
    QImage Image;
    Image.load(":/images/resources/images/cd.png");
    QPixmap pixmap = QPixmap::fromImage(Image);
    int with = 320;
    int height = 320;
    QPixmap fitpixmap =
            pixmap.scaled(with, height,
                          Qt::IgnoreAspectRatio,
                          Qt::SmoothTransformation);
    label[1]->setPixmap(fitpixmap);
    label[1]->setAlignment(Qt::AlignCenter);
    vWidget[2]->setMinimumSize(300, 80);
    vWidget[2]->setMaximumHeight(80);
    vBoxLayout[1]->addSpacerItem(vSpacer2);
    vBoxLayout[1]->addWidget(label[1]);
    vBoxLayout[1]->addSpacerItem(vSpacer3);
    vBoxLayout[1]->addWidget(durationSlider);
    vBoxLayout[1]->addWidget(vWidget[2]);
    vBoxLayout[1]->addSpacerItem(vSpacer4);
    vBoxLayout[1]->setContentsMargins(0, 0, 0, 0);

    vWidget[1]->setLayout(vBoxLayout[1]);

    /* V2布局 */
    QSpacerItem *vSpacer5 = new
            QSpacerItem(300, 10,
                        QSizePolicy::Minimum,
                        QSizePolicy::Maximum);
    hWidget[2]->setMinimumSize(320, 20);
    hWidget[3]->setMinimumSize(320, 60);
    vBoxLayout[2]->addWidget(hWidget[2]);
    vBoxLayout[2]->addSpacerItem(vSpacer5);
    vBoxLayout[2]->addWidget(hWidget[3]);
    vBoxLayout[2]->setContentsMargins(0, 0, 0, 0);

    vWidget[2]->setLayout(vBoxLayout[2]);

    /* H2布局 */
    QFont font;

    font.setPixelSize(10);

    /* 设置标签文本 */
    label[0]->setText("Q Music，Enjoy it！");
    label[2]->setText("00:00");
    label[3]->setText("00:00");
    label[2]->setSizePolicy(QSizePolicy::Expanding,
                            QSizePolicy::Expanding);
    label[3]->setSizePolicy(QSizePolicy::Expanding,
                            QSizePolicy::Expanding);
    label[3]->setAlignment(Qt::AlignRight);
    label[2]->setAlignment(Qt::AlignLeft);
    label[2]->setFont(font);
    label[3]->setFont(font);

    pal.setColor(QPalette::WindowText, Qt::white);
    label[0]->setPalette(pal);
    label[2]->setPalette(pal);
    label[3]->setPalette(pal);

    hBoxLayout[2]->addWidget(label[2]);
    hBoxLayout[2]->addWidget(label[3]);

    hBoxLayout[2]->setContentsMargins(0, 0, 0, 0);
    hWidget[2]->setLayout(hBoxLayout[2]);

    /* H3布局 */
    QSpacerItem *hSpacer5 = new
            QSpacerItem(0, 60,
                        QSizePolicy::Minimum,
                        QSizePolicy::Maximum);
    QSpacerItem *hSpacer6 = new
            QSpacerItem(80, 60,
                        QSizePolicy::Maximum,
                        QSizePolicy::Maximum);
    QSpacerItem *hSpacer7 = new
            QSpacerItem(80, 60,
                        QSizePolicy::Maximum,
                        QSizePolicy::Maximum);
    QSpacerItem *hSpacer8 = new
            QSpacerItem(80, 60,
                        QSizePolicy::Maximum,
                        QSizePolicy::Maximum);
    QSpacerItem *hSpacer9 = new
            QSpacerItem(0, 60,
                        QSizePolicy::Minimum,
                        QSizePolicy::Maximum);

    for (int i = 3; i < 7; i++) {
        pushButton[i]->setMinimumSize(25, 25);
        pushButton[i]->setMaximumSize(25, 25);
    }

    hBoxLayout[3]->addSpacerItem(hSpacer5);
    hBoxLayout[3]->addWidget(pushButton[3]);
    hBoxLayout[3]->addSpacerItem(hSpacer6);
    hBoxLayout[3]->addWidget(pushButton[4]);
    hBoxLayout[3]->addSpacerItem(hSpacer7);
    hBoxLayout[3]->addWidget(pushButton[5]);
    hBoxLayout[3]->addSpacerItem(hSpacer8);
    hBoxLayout[3]->addWidget(pushButton[6]);
    hBoxLayout[3]->addSpacerItem(hSpacer9);
    hBoxLayout[3]->setContentsMargins(0, 0, 0, 0);
    hBoxLayout[3]->setAlignment(Qt::AlignHCenter);

    hWidget[3]->setLayout(hBoxLayout[3]);

}

void Music::initMediaLibrary(){
    qDebug() << "init media library" << endl;
    musicPlayer = new QMediaPlayer(this);
    mediaPlaylist = new QMediaPlaylist(this);
    /* 确保列表是空的 */
    mediaPlaylist->clear();
    /* 设置音乐播放器的列表为mediaPlaylist */
    musicPlayer->setPlaylist(mediaPlaylist);
    /* 设置播放模式，Loop是列循环 */
    mediaPlaylist->setPlaybackMode(QMediaPlaylist::Loop);
}

void Music::scanMusic(){
    qDebug() << "scan music" << endl;

    QString dirs = QFileDialog::getExistingDirectory(
        this,
        tr("Open Directory"),
        QDir::currentPath(),
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

//    QDir dir("/home/leeyangy/Music");
    QDir dir(dirs);
    QDir dirbsoultePath(dir.absolutePath());

    // if dir exists
    if(dirbsoultePath.exists()) {
        /* set filter */
        QStringList filter;
        /* filter .mp3 */
        filter << "*.mp3";
        /* get current dir all child dir */
        QFileInfoList files = dirbsoultePath.entryInfoList(filter,QDir::Files);

        foreach (auto &file, files) {
            qDebug() << file << endl;
        }

        for (int i=0;i<files.count();++i) {
            MediaObjectInfo info;
            QString fileName = QString::fromUtf8(
                        files
                        .at(i)
                        .fileName()
                        .replace("*.mp3","")
                        .toUtf8()
                        .data()
                        );
            info.fileName = fileName + "\n" + fileName.split("-").at(1);
            info.filePath = QString::fromUtf8(
                        files.at(i)
                        .filePath()
                        .toUtf8()
                        .data()
                        );
            if(mediaPlaylist->addMedia(
                        QUrl::fromLocalFile(info.filePath)
                        )) {
                mediaObjectInfo.append(info);
                listWidget->addItem(info.fileName);
            } else {
                qDebug() << mediaPlaylist->errorString().toUtf8().data() << endl;
                qDebug() << "Error number:" << mediaPlaylist->error() << endl;
            }
        }

    }
}

void Music::btn_previous_clicked(){
    musicPlayer->stop();
    int count = mediaPlaylist->mediaCount();
    if (0 == count) return;

    /* 列表上一个 */
    mediaPlaylist->previous();
    musicPlayer->play();
}

void Music::btn_next_clicked() {
    musicPlayer->stop();
    int count = mediaPlaylist->mediaCount();
    if (0 == count) return;

    /* 列表上一个 */
    mediaPlaylist->next();
    musicPlayer->play();
}

void Music::btn_play_clicked(){
    int state = musicPlayer->state();

    switch (state) {
    case QMediaPlayer::StoppedState:
        /* 媒体播放 */
        musicPlayer->play();
        break;

    case QMediaPlayer::PlayingState:
        /* 媒体暂停 */
        musicPlayer->pause();
        break;

    case QMediaPlayer::PausedState:
        musicPlayer->play();
        break;
    }
}
