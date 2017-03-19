#ifndef PLAYER_H
#define PLAYER_H

#include <QMainWindow>
#include <QtWidgets>
#include <QMediaPlayer>
#include <QMediaPlaylist>
#include <QtMath>

#include "contentmanager.h"
#include "choseplaylistdialog.h"

class Player : public QMainWindow
{
    Q_OBJECT
    QWidget *mainWidget;
    QMenu *fileMenu;
    QLabel *songNameLabel;
    QLabel *startClock;
    QSlider *songSlider;

    bool wasSliderReleased;
    QLabel *endClock;

    QPushButton *prevSongButton;
    QPushButton *nextSongButton;
    QPushButton *playButton;
    QPushButton *pauseButton;
    QPushButton *volumeButton;
    QSlider *volumeSlider;

    QListWidget *playList;

    QLabel *playListNameLabel;
    QMediaPlayer *player;
    ContentManager *manager;

    static const qint8 DEFAULT_VOLUME = 50;
    bool wasEverMuted;
public:
    Player(QWidget *parent = 0);
    void createMenu();
    void createInterface();
    void createPlayer();
    void createManager();
    void createConnections();

    ~Player();

    void addSong(const QString &);
    void addSongs(const QStringList &);

    void saveSettings();

protected:
    virtual void closeEvent(QCloseEvent *);

public slots:
    void slotDEBUG();
    void choseFile();
    void play();
    void pause();
    void nextSong();
    void prevSong();
    void setPlaylistIndex();
    void updateSongName();
    void loadPlaylistWidget();
    void loadPlaylist();
    void chosePlaylist();
    void renamePlaylist();

    void slotDurationMax(qint64);
    void slotDurationCurrent(qint64);
    void updateSliderScale();
    void slotSetSliderPosition(qint64);
    void slotSetMediaPostion();
    void slotBlockSliderChange();

    void slotSetVolume();
    void slotChangeMutePlayer();

    void slotDefaultMutingBugFix();
};

#endif // PLAYER_H
