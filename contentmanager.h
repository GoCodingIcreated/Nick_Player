#ifndef CONTENTMANAGER_H
#define CONTENTMANAGER_H

#include <QObject>
#include <QMap>
#include <QMediaPlaylist>
#include <QDir>
#include <QFileDialog>

class ContentManager : public QObject
{
    Q_OBJECT
    QMap<QString, QMediaPlaylist*> library;
    bool wasLoadedSuccesfull;
public:
    explicit ContentManager(QObject *parent = 0);
    ~ContentManager();
    bool addPlaylist(const QString&, QMediaPlaylist *);
    QMediaPlaylist* getPlaylistByName(const QString &);
    QStringList getAllPlaylistsName();
    void save();
    void load(const QStringList&);
    void rename(const QString&, const QString &);
signals:
    void loadedPlaylistAlreadyExist(QString);
public slots:
    void loadSucces();
    void loadFailed();
};

#endif // CONTENTMANAGER_H
