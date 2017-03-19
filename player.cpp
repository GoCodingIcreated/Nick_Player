#include "player.h"




Player::Player(QWidget *parent)
    : QMainWindow(parent)
{
    mainWidget = new QWidget(this);
    setCentralWidget(mainWidget);
    createMenu();
    createInterface();
    createPlayer();

    createConnections();
    loadPlaylistWidget();
    updateSongName();
    wasSliderReleased = true;

    wasEverMuted = false;
}

Player::~Player()
{

}

void Player::createMenu()
{
    fileMenu = new QMenu("Playlist");
    fileMenu->addAction("&Add new songs...", this, SLOT(choseFile()));
    fileMenu->addAction("&Load...", this, SLOT(loadPlaylist()));
    fileMenu->addAction("&Chose playlist...", this, SLOT(chosePlaylist()));
    fileMenu->addAction("&Rename playlist...", this, SLOT(renamePlaylist()));
    menuBar()->addMenu(fileMenu);
    menuBar()->setNativeMenuBar(false);
}

void Player::createInterface()
{
    QVBoxLayout *vLayout = new QVBoxLayout();

    startClock = new QLabel("00.00");
    songNameLabel = new QLabel("Song Name");
    songNameLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed);
    songSlider = new QSlider();
    songSlider->setOrientation(Qt::Horizontal);
    songSlider->setFocusPolicy(Qt::NoFocus);
    endClock = new QLabel("23.59");

    QHBoxLayout *hSliderLayout = new QHBoxLayout();
    hSliderLayout->addWidget(startClock);
    hSliderLayout->addWidget(songSlider);
    hSliderLayout->addWidget(endClock);

    prevSongButton = new QPushButton("|<");
    nextSongButton = new QPushButton(">|");
    playButton = new QPushButton(">");
    pauseButton = new QPushButton("||");
    prevSongButton->setFixedWidth(50);
    nextSongButton->setFixedWidth(50);
    playButton->setFixedWidth(50);
    pauseButton->setFixedWidth(50);

    volumeButton = new QPushButton("S");
    volumeButton->setFixedWidth(40);
    volumeSlider = new QSlider(Qt::Horizontal);
    volumeSlider->setRange(0, 100);
    volumeSlider->setValue(DEFAULT_VOLUME);
    volumeSlider->setFixedWidth(100);
    volumeSlider->setFocusPolicy(Qt::NoFocus);
    QHBoxLayout *hVolumeLayout = new QHBoxLayout();
    hVolumeLayout->addWidget(volumeButton);
    hVolumeLayout->addWidget(volumeSlider);

    QHBoxLayout *hLayout = new QHBoxLayout();
    hLayout->addWidget(prevSongButton);
    hLayout->addWidget(pauseButton);
    hLayout->addWidget(playButton);
    hLayout->addWidget(nextSongButton);

    playListNameLabel = new QLabel("playlist name");
    playList = new QListWidget();


    mainWidget->setLayout(vLayout);

    vLayout->addWidget(songNameLabel);

    vLayout->addLayout(hSliderLayout);
    vLayout->addLayout(hLayout);
    vLayout->addLayout(hVolumeLayout);
    vLayout->addWidget(playListNameLabel);
    vLayout->addWidget(playList);
}

void Player::createPlayer()
{
    player = new QMediaPlayer(this);
    player->setNotifyInterval(100);
    createManager();
    player->setPlaylist(manager->getPlaylistByName("default"));
    playListNameLabel->setText("default");
    player->setVolume(DEFAULT_VOLUME);
    //player->setMuted(false);
    qDebug() << player->isMuted() << "1";
}

void Player::createManager()
{
    manager = new ContentManager();
    QStringList dlist;
    dlist << QDir::current().filePath("default.m3u");
    manager->load(dlist);
    if (manager->getPlaylistByName("default") == NULL) {
        qDebug() << "default playlist wasnt find. create new default.";
        QMediaPlaylist *list = new QMediaPlaylist();
        list->setCurrentIndex(0);
        manager->addPlaylist("default", list);
    }

disconnect(player, SIGNAL(volumeChanged(int)), this, SLOT(slotDefaultMutingBugFix()));
}

void Player::createConnections()
{
    // Control buttons settings
    connect(playButton, SIGNAL(clicked(bool)), this, SLOT(play()));
    connect(pauseButton, SIGNAL(clicked(bool)), this, SLOT(pause()));
    connect(nextSongButton, SIGNAL(clicked(bool)), this, SLOT(nextSong()));
    connect(prevSongButton, SIGNAL(clicked(bool)), this, SLOT(prevSong()));

    // Song changes settings
    connect(playList, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(setPlaylistIndex()));
    connect(player, SIGNAL(currentMediaChanged(QMediaContent)), this, SLOT(updateSongName()));
    connect(player, SIGNAL(currentMediaChanged(QMediaContent)), this, SLOT(updateSliderScale()));

    // Position\Duration settings
    connect(player, SIGNAL(durationChanged(qint64)), this, SLOT(slotDurationMax(qint64)));
    connect(player, SIGNAL(positionChanged(qint64)), this, SLOT(slotDurationCurrent(qint64)));
    connect(songSlider, SIGNAL(sliderReleased()), this, SLOT(slotSetMediaPostion()));
    connect(songSlider, SIGNAL(sliderPressed()), this, SLOT(slotBlockSliderChange()));

    // Volume settings
    connect(volumeSlider, SIGNAL(sliderMoved(int)), this, SLOT(slotSetVolume()));
    connect(volumeButton, SIGNAL(clicked()), this, SLOT(slotChangeMutePlayer()));

    // There is a bug in Qt, I think. When app finished with muted player after restarting player
    // is going to mute himself after he start to play song. So, here is some bugfix function.
    connect(player, SIGNAL(mutedChanged(bool)), this, SLOT(slotDefaultMutingBugFix()));

}

void Player::addSong(const QString &song)
{
    playList->addItem(song);
}

void Player::addSongs(const QStringList &songList)
{
    playList->addItems(songList);
}

void Player::choseFile()
{
    QStringList tmp = QFileDialog::getOpenFileNames();
    QStringList songs;
    QMediaPlaylist *list = player->playlist();//manager->getPlaylistByName("default");
    foreach (QString song, tmp) {

        list->addMedia(QUrl::fromLocalFile(song));
        songs << QUrl::fromLocalFile(song).fileName();
    }
    addSongs(songs);
}


void Player::play() {

    qDebug() << "play:" << player->isMuted();

    player->play();
}

void Player::pause()
{

    qDebug() << "stop" << player->isMuted() << "3" << player->volume();
    player->pause();
}

void Player::nextSong()
{
    qDebug() << "next";
    player->playlist()->next();
}

void Player::prevSong()
{
    qDebug() << "prev";
    player->playlist()->previous();
}

void Player::setPlaylistIndex()
{
    QMediaPlaylist *list = player->playlist();
    qDebug() << "Was:" << list->currentIndex() << ' ' << playList->currentIndex().row();
    list->setCurrentIndex(playList->currentIndex().row());
    qDebug() << "Is:" << list->currentIndex();
}

void Player::updateSongName()
{
    if (!player->playlist()->isEmpty()) {
        if (player->playlist()->currentIndex() != -1) {
            songNameLabel->setText(player->playlist()->currentMedia().canonicalUrl().fileName());
        }
        else {
            songNameLabel->setText(player->playlist()->media(0).canonicalUrl().fileName());
        }
    }
}

void Player::closeEvent(QCloseEvent *event)
{
    qDebug() << "close";
    saveSettings();
    QMainWindow::closeEvent(event);
}

void Player::saveSettings()
{
    manager->save();
}

void Player::loadPlaylist()
{
    QList<QString> names = QFileDialog::getOpenFileNames();

    manager->load(names);

}

void Player::chosePlaylist()
{

    qDebug() << "chose playlist";
    QStringList playlistNames = manager->getAllPlaylistsName();
    int index = 0;
    foreach(QString name, playlistNames) {
        if (name == playListNameLabel->text()) {
            break;
        }
        index++;
    }
    bool wasOk;
    QString choosenPlaylist = QInputDialog::getItem(this, "Chose playlist", "Current playlist:",
                                                    playlistNames, index, false, &wasOk);
    if (wasOk) {
        qDebug() << choosenPlaylist;
        QMediaPlaylist *playlist = manager->getPlaylistByName(choosenPlaylist);
        player->stop();
        player->setPlaylist(playlist);
        player->setPosition(0);
        player->playlist()->setCurrentIndex(0);
        playListNameLabel->setText(choosenPlaylist);

        loadPlaylistWidget();

    }
}

void Player::loadPlaylistWidget()
{
    playList->clear();
    for (int i = 0; i < player->playlist()->mediaCount(); ++i) {
        playList->addItem(player->playlist()->media(i).canonicalUrl().fileName());
    }
}

void Player::renamePlaylist() {
    QString newName = QInputDialog::getText(this, "Rename", "New name:", QLineEdit::Normal, playListNameLabel->text());
    manager->rename(playListNameLabel->text(), newName);
    playListNameLabel->setText(newName);
}

void Player::slotDurationMax(qint64 duration) {
    qint32 seconds = (duration / 1000) % 60;
    qint32 minutes = (duration / 60000) % 60;
    qint32 houres = (duration / 3600000) % 24;
    QTime time(houres, minutes, seconds);
    endClock->setText(time.toString());
    updateSliderScale();
}

void Player::slotDurationCurrent(qint64 duration) {
    //duration = player->position();

    qint32 seconds = (duration / 1000) % 60;
    qint32 minutes = (duration / 60000) % 60;
    qint32 houres = (duration / 3600000) % 24;
    QTime time(houres, minutes, seconds);
    qDebug() << "SlotDurationCurrent:" << time.toString();
    startClock->setText(time.toString());
    slotSetSliderPosition(duration);
}

void Player::updateSliderScale() {
    songSlider->setRange(0, player->duration());
    qDebug() << "updateSliderScale.";
}

void Player::slotSetSliderPosition(qint64 position) {

    if (wasSliderReleased) {
        songSlider->setValue(player->position());
    }
    qDebug() << "slotSetSliderPosition:" << wasSliderReleased << player->position();
}

void Player::slotSetMediaPostion() {

    int position = songSlider->value();
    qDebug() << "Slider Released:" << position;
    wasSliderReleased = true;
    player->setPosition(position);
}

void Player::slotBlockSliderChange() {
    wasSliderReleased = false;
}


void Player::slotSetVolume() {
    // Does not work since qt 5.8
    /*qreal linearVolume = convertVolume(volumeSlider->value() / qreal(100.0),
                                               QAudio::LogarithmicVolumeScale,
                                               QAudio::LinearVolumeScale);

    player->setVolume(qRound(linearVolume * 100));*/

    // TODO: rescale it with logscale somehow
    qreal volume = volumeSlider->value();
    volume /= 100.0;
    volume = qExp(volume);
    volume -= 1;
    volume = volume * 100.0 / (qExp(1.0) - 1);
    qDebug() << "VOLUME: " << volumeSlider->value() << ' ' << volume;
    player->setVolume(qRound(volume));
}

void Player::slotChangeMutePlayer() {
    qDebug() << "slotChangeMutePlayer:" << player->isMuted();

    if (!player->isMuted()) {
        wasEverMuted = true;
        volumeButton->setText("\\S");
        volumeSlider->setEnabled(false);
    }
    else {
        volumeButton->setText("S");
        volumeSlider->setEnabled(true);
    }
    player->setMuted(!player->isMuted());
}

void Player::slotDEBUG() {
    qDebug() << "slotDEBUG.";
}

void Player::slotDefaultMutingBugFix() {

    qDebug() << "slotDefaultMutingBugFix.";
    if (!wasEverMuted && player->isMuted()) {
        player->setMuted(false);
    }
    if (wasEverMuted) {
        disconnect(player, SIGNAL(mutedChanged(bool)), this, SLOT(slotDefaultMutingBugFix()));
    }

}
