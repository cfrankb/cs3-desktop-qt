#include <QShortcut>
#include <QMessageBox>
#include <QPainter>
#include <QKeyEvent>
#include <QStandardPaths>
#include <QDir>
#include "gamewidget.h"
#include "maparch.h"
#include "game.h"
#include "tilesdata.h"
#include "Frame.h"
#include "FrameSet.h"
#include "shared/qtgui/qfilewrap.h"
#include "shared/implementers/sn_sdl.h"
#include "shared/implementers/mu_sdl.h"

CGameWidget::CGameWidget(QWidget *parent)
    : QWidget{parent}, CGameMixin()
{
    m_mapArch = new CMapArch();
}

void CGameWidget::init()
{
    enableHiScore();
    m_zoom = true;
    QFileWrap file;
    const char filename[]{":/data/levels.mapz"} ;
    if (file.open(filename, "rb")) {
        if (m_mapArch->read(file)) {
            qDebug("reading succesful: %s \n", filename);
        }
        CGameMixin::init(m_mapArch, 0);
        file.close();
    } else {
        qDebug("can't read: %s\n", filename);
    }
    initSounds();
    initMusic();
    m_timer.setInterval(1000 / TICK_RATE);
    m_timer.start();
    connect(&m_timer, SIGNAL(timeout()), this, SLOT(mainLoop()));
    qDebug() << QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
}

void CGameWidget::exitGame()
{
    //reject();
}

void CGameWidget::mainLoop()
{
    CGameMixin::mainLoop();
    repaint();
    update();
}

void CGameWidget::changeZoom()
{
    CGameMixin::changeZoom();
}

void CGameWidget::sanityTest()
{
    CMap *map = m_maparch->at(m_game->level());
    const Pos pos = map->findFirst(TILES_ANNIE2);
    QStringList listIssues;
    if ((pos.x == CMap::NOT_FOUND ) && (pos.y == CMap::NOT_FOUND )) {
        listIssues.push_back(tr("No player on map"));
    }
    if (map->count(TILES_DIAMOND) == 0) {
        listIssues.push_back(tr("No diamond on map"));
    }
    if (listIssues.count() > 0) {
        QString msg = tr("Map %1 is incomplete:\n%2").arg(m_game->level() + 1).arg(listIssues.join("\n"));
        QMessageBox::warning(this, "", msg, QMessageBox::Button::Ok);
        exitGame();
    }
}

void CGameWidget::setZoom(bool zoom)
{
    CGameMixin::setZoom(zoom);
    int factor = m_zoom ? 2 : 1;
    this->setMaximumSize(QSize(WIDTH * factor, HEIGHT * factor));
    this->setMinimumSize(QSize(WIDTH * factor, HEIGHT * factor));
    this->resize(QSize(WIDTH * factor, HEIGHT * factor));
}

void CGameWidget::paintEvent(QPaintEvent *)
{
    CFrame bitmap(WIDTH, HEIGHT);
    switch (m_game->mode())
    {
    case CGame::MODE_INTRO:
    case CGame::MODE_RESTART:
    case CGame::MODE_GAMEOVER:
        drawLevelIntro(bitmap);
        break;
    case CGame::MODE_LEVEL:
        drawScreen(bitmap);
        break;
    case CGame::MODE_HISCORES:
        drawScores(bitmap);
        break;
    case CGame::MODE_CLICKSTART:
        drawPreScreen(bitmap);
        break;
    case CGame::MODE_HELP:
        drawHelpScreen(bitmap);
    }

    // show the screen
    QSize sz = size();
    const QImage & img = QImage(reinterpret_cast<uint8_t*>(bitmap.getRGB()), bitmap.len(), bitmap.hei(), QImage::Format_RGBX8888);
    const QPixmap & pixmap = QPixmap::fromImage(img.scaled(sz));
    QPainter p(this);
    p.drawPixmap(0, 0, pixmap);
    p.end();
}

void CGameWidget::keyPressEvent(QKeyEvent *event)
{
    keyReflector(event->key(), KEY_PRESSED);
    switch(event->key()) {
    case Qt::Key_Up:
        m_joyState[AIM_UP] = KEY_PRESSED;
        break;
    case Qt::Key_Down:
        m_joyState[AIM_DOWN] = KEY_PRESSED;
        break;
    case Qt::Key_Left:
        m_joyState[AIM_LEFT] = KEY_PRESSED;
        break;
    case Qt::Key_Right:
        m_joyState[AIM_RIGHT] = KEY_PRESSED;
        break;
    case Qt::Key_Escape:
        exitGame();
    }
}

void CGameWidget::keyReleaseEvent(QKeyEvent *event)
{
    keyReflector(event->key(), KEY_RELEASED);
    switch(event->key()) {
    case Qt::Key_Up:
        m_joyState[AIM_UP] = KEY_RELEASED;
        break;
    case Qt::Key_Down:
        m_joyState[AIM_DOWN] = KEY_RELEASED;
        break;
    case Qt::Key_Left:
        m_joyState[AIM_LEFT] = KEY_RELEASED;
        break;
    case Qt::Key_Right:
        m_joyState[AIM_RIGHT] = KEY_RELEASED;
    }
}

void CGameWidget::preloadAssets()
{
    QFileWrap file;


    typedef struct {
        const char *filename;
        CFrameSet **frameset;
    } asset_t;

    asset_t assets[] = {
                        {":/data/tiles.obl", &m_tiles},
                        {":/data/animz.obl", &m_animz},
                        {":/data/annie.obl", &m_annie},
                        };

    for (int i=0; i < 3; ++i) {
        asset_t & asset = assets[i];
        *(asset.frameset) = new CFrameSet();
        if (file.open(asset.filename, "rb")) {
            qDebug("reading %s", asset.filename);
            if ((*(asset.frameset))->extract(file)) {
                qDebug("extracted: %d", (*(asset.frameset))->getSize());
            }
            file.close();
        }
    }

    const char fontName [] = ":/data/bitfont.bin";
    int size = 0;
    if (file.open(fontName, "rb")) {
        size = file.getSize();
        m_fontData = new uint8_t[size];
        file.read(m_fontData, size);
        file.close();
        qDebug("loaded font: %d bytes", size);
    } else {
        qDebug("failed to open %s", fontName);
    }
}

void CGameWidget::keyReflector(int key, uint8_t keyState)
{
    auto range = [](auto keyCode, auto start, auto end)
    {
        return keyCode >= start && keyCode <= end;
    };

    uint16_t result;
    if (range(key, Qt::Key_0, Qt::Key_0))
    {
        result = key - Qt::Key_0 + Key_0;
    }
    else if (range(key, Qt::Key_A, Qt::Key_Z))
    {
        result = key - Qt::Key_A + Key_A;
    }
    else if (range(key, Qt::Key_F1, Qt::Key_F12))
    {
        result = key - Qt::Key_F1 + Key_F1;
    }
    else
    {
        switch (key)
        {
        case Qt::Key_Space:
            result = Key_Space;
            break;
        case Qt::Key_Backspace:
            result = Key_BackSpace;
            break;
        case Qt::Key_Enter:
        case Qt::Key_Return:
            result = Key_Enter;
            break;
        default:
            return;
        }
    }
    m_keyStates[result] = keyState;
}


QString CGameWidget::hiScorePath()
{
    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/hiscores.dat";
}

QString CGameWidget::saveGamePath()
{
    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/savegame.dat";
}

void CGameWidget::createPath()
{
    QString path{QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)};
    if (!QDir(path).exists()) {
        QDir().mkpath(path);
    }
}

bool CGameWidget::loadScores()
{
    std::string path{hiScorePath().toStdString()};
    qDebug("reading %s\n", path.c_str());
    QFileWrap file;
    if (file.open(path.c_str(), "rb"))
    {
        if (file.getSize() == sizeof(m_hiscores))
        {
            file.read(m_hiscores, sizeof(m_hiscores));
            file.close();
        }
        else
        {
            qDebug("size mismatch. resetting to default.\n");
            clearScores();
        }
        return true;
    }
    qDebug("can't read %s\n", path.c_str());
    return false;
}

bool CGameWidget::saveScores()
{
    createPath();
    std::string path{hiScorePath().toStdString()};
    qDebug("reading %s\n", path.c_str());
    QFileWrap file;
    if (file.open(path.c_str(), "wb"))
    {
        file.write(m_hiscores, sizeof(m_hiscores));
        file.close();
        return true;
    }
    qDebug("can't write %s\n", path.c_str());
    return false;
}

void CGameWidget::save()
{
    createPath();
    std::string path{saveGamePath().toStdString()};
    if (m_game->mode() != CGame::MODE_LEVEL)
    {
        qDebug("cannot save while not playing\n");
        return;
    }

    qDebug("writing: %s\n", path.c_str());
    std::string name{"Testing123"};
    FILE *tfile = fopen(path.c_str(), "wb");
    if (tfile)
    {
        write(tfile, name);
        fclose(tfile);
    }
    else
    {
        qDebug("can't write:%s\n", path.c_str());
    }
}

void CGameWidget::load()
{
    std::string path{saveGamePath().toStdString()};
    m_game->setMode(CGame::MODE_IDLE);
    std::string name;
    qDebug("reading: %s\n", path.c_str());
    FILE *sfile = fopen(path.c_str(), "rb");
    if (sfile)
    {
        if (!read(sfile, name))
        {
            qDebug("incompatible file\n");
        }
        fclose(sfile);
    }
    else
    {
        qDebug("can't read:%s\n", path.c_str());
    }
    m_game->setMode(CGame::MODE_LEVEL);
}

void CGameWidget::initSounds()
{
    constexpr const char *filelist[]{
        ":/data/sounds/gruup.wav",
        ":/data/sounds/key.ogg",
        ":/data/sounds/0009.ogg",
        ":/data/sounds/coin1.oga",
    };
    auto m_sound = new CSndSDL();
    QFileWrap file;
    for (size_t i = 0; i < sizeof(filelist) / sizeof(filelist[0]); ++i)
    {
        auto soundName = filelist[i];
        if (file.open(soundName, "rb"))
        {
            int size = file.getSize();
            auto sound = new uint8_t[size];
            file.read(sound, size);
            file.close();
            qDebug("loaded %s: %d bytes", soundName, size);
            m_sound->add(sound, size, i + 1);
        }
        else
        {
            qDebug("failed to open: %s", soundName);
        }
    }
    m_game->attach(m_sound);
}

void CGameWidget::initMusic()
{
    m_music = new CMusicSDL();
    // copy embedded music file to temp folder
    QString path = QDir::tempPath() + "/cs3idea_64.ogg";
    if (!QFile(path).exists()) {
        uint8_t *buf = nullptr;
        int size;
        QFileWrap file;
        if (file.open(":/data/cs3idea_64.ogg", "rb")) {
            size = file.getSize();
            buf = new uint8_t[size];
            file.read(buf, size);
            file.close();
        }
        if (buf && file.open(path.toStdString().c_str(), "wb")) {
            file.write(buf, size);
            file.close();
        }
    }

    if (m_music && m_music->open(path.toStdString().c_str()))
    {
        m_music->play();
    }
}

void CGameWidget::stopMusic()
{
    if (m_music)
    {
        m_music->stop();
    }
}

void CGameWidget::startMusic()
{
    if (m_music)
    {
        m_music->play();
    }
    else
    {
        initMusic();
    }
}
