#include "contentmanager.h"


ContentManager::ContentManager(QObject *parent) : QObject(parent)
{
    wasLoadedSuccesfull = false;
}

ContentManager::~ContentManager()
{
    foreach(QMediaPlaylist *list, library.values())
    {
        // list.save()
        delete list;
    }

}

bool ContentManager::addPlaylist(const QString &name, QMediaPlaylist *list)
{
    if (!library.contains(name)) {
        library.insert(name, list);

    }
    else {
        return false;
        qDebug() << "contentmanager: playlist already exists. Dont make any changes";
    }
    return true;
}

QMediaPlaylist* ContentManager::getPlaylistByName(const QString &name)
{

    return library.value(name, NULL);
}


QStringList ContentManager::getAllPlaylistsName()
{
    return QStringList(library.keys());
}


void ContentManager::save()
{


    for (QMap<QString, QMediaPlaylist*>::iterator it = library.begin(); it != library.end(); ++it)
    {
        //it.value()->save(QUrl(QD))
        //QFileDialog::getSaveFileUrl()
        qDebug() << "Save file name:" << QUrl::fromLocalFile(QDir::current().absoluteFilePath(it.key() + ".m3u")) << ' ' << it.value()->mediaCount();;
        for (int i = 0; i < it.value()->mediaCount(); ++i) {
            qDebug() << it.value()->media(i).canonicalUrl().fileName();
        }
        // TODO:
        // here I must to rewrite follow code
        // because it has trouble with Russian letters
        // I have to replace it with code works with QFile and utf-8 filenames
        // UPDATE: this is fine code by the way. Looks like problem in Load function.
        bool flag = it.value()->save(QUrl::fromLocalFile(QDir::current().absoluteFilePath(it.key() + ".m3u")), "m3u");
        qDebug() << "Result of save:" << flag;

    }
}

void ContentManager::load(const QStringList &names)
{
    foreach (QString name, names) {
        QMediaPlaylist *playlist = new QMediaPlaylist();
        connect(playlist, SIGNAL(loaded()), this, SLOT(loadSucces()));
        connect(playlist, SIGNAL(loadFailed()), this, SLOT(loadFailed()));
        qDebug() << "try to load:" << QUrl::fromLocalFile(name);

        // TODO:
        // I must to replace this code
        // There is a problem with Russian letters in m3u files
        // I thougth the problem was in save function.
        // But m3u file was with correct Russian letters after save.
        playlist->load(QUrl::fromLocalFile(name), "m3u8");

        disconnect(playlist, SIGNAL(loaded()), this, SLOT(loadSucces()));
        disconnect(playlist, SIGNAL(loadFailed()), this, SLOT(loadFailed()));
        if (!wasLoadedSuccesfull) {
            qDebug() << playlist->errorString();
            continue;
        }
        if (library.contains(QFileInfo(name).completeBaseName())) {
            qDebug() << "Loaded playlist name equal to some from loaded before";
            emit loadedPlaylistAlreadyExist(name);
        }
        else {
            library[QFileInfo(name).completeBaseName()] = playlist;
        }

    }
}

void ContentManager::rename(const QString &oldName, const QString &newName)
{
    library[newName] = library[oldName];
    library.remove(oldName);
}

void ContentManager::loadSucces()
{
    wasLoadedSuccesfull = true;
}

void ContentManager::loadFailed()
{
    wasLoadedSuccesfull = false;
}
