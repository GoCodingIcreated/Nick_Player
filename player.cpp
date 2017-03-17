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
    endClock = new QLabel("23.59");

    QHBoxLayout *hSliderLayout = new QHBoxLayout();
    hSliderLayout->addWidget(startClock);
    hSliderLayout->addWidget(songSlider);
    hSliderLayout->addWidget(endClock);

    prevSongButton = new QPushButton("|<");
    nextSongButton = new QPushButton(">|");
    playButton = new QPushButton(">");
    pauseButton = new QPushButton("||");


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
    vLayout->addWidget(playListNameLabel);
    vLayout->addWidget(playList);
}

void Player::createPlayer()
{
    player = new QMediaPlayer(this);
    createManager();
    player->setPlaylist(manager->getPlaylistByName("default"));
    playListNameLabel->setText("default");
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


}

void Player::createConnections()
{
    connect(playButton, SIGNAL(clicked(bool)), this, SLOT(play()));
    connect(pauseButton, SIGNAL(clicked(bool)), this, SLOT(pause()));
    connect(nextSongButton, SIGNAL(clicked(bool)), this, SLOT(nextSong()));
    connect(prevSongButton, SIGNAL(clicked(bool)), this, SLOT(prevSong()));

    connect(playList, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(setPlaylistIndex()));
    connect(player, SIGNAL(currentMediaChanged(QMediaContent)), this, SLOT(updateSongName()));
    connect(player, SIGNAL(currentMediaChanged(QMediaContent)), this, SLOT(updateSliderScale()));

    connect(player, SIGNAL(durationChanged(qint64)), this, SLOT(slotDurationMax(qint64)));
    connect(player, SIGNAL(positionChanged(qint64)), this, SLOT(slotDurationCurrent(qint64)));
    connect(player, SIGNAL(positionChanged(qint64)), this, SLOT(slotSetSliderPosition(qint64)));
    connect(songSlider, SIGNAL(sliderReleased()), this, SLOT(slotSetMediaPostion()));
    connect(songSlider, SIGNAL(sliderPressed()), this, SLOT(slotBlockSliderChange()));
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


void Player::play()
{
    qDebug() << "play";
    player->play();
}

void Player::pause()
{

    qDebug() << "stop";
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
        playlist->setCurrentIndex(0);
        player->stop();
        player->setPlaylist(playlist);

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
    qDebug() << duration;
    qint32 seconds = (duration / 1000) % 60;
    qint32 minutes = (duration / 60000) % 60;
    qint32 houres = (duration / 3600000) % 24;
    QTime time(houres, minutes, seconds);
    qDebug() << time.toString();
    startClock->setText(time.toString());
}

void Player::updateSliderScale() {
    songSlider->setRange(0, player->duration());
}

void Player::slotSetSliderPosition(qint64 position) {
    if (wasSliderReleased) {
        songSlider->setValue(position);
    }
}

void Player::slotSetMediaPostion() {
    int position = songSlider->value();
    wasSliderReleased = true;
    player->setPosition(position);
}

void Player::slotBlockSliderChange() {
    wasSliderReleased = false;
}
