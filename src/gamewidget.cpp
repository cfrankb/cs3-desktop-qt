#include "gamewidget.h"
#include <QShortcut>
#include <QMessageBox>
#include <QPainter>
#include <QKeyEvent>
#include "maparch.h"
#include "game.h"
#include "tilesdata.h"
#include "Frame.h"
#include "FrameSet.h"
#include "shared/qtgui/qfilewrap.h"

CGameWidget::CGameWidget(QWidget *parent)
    : QWidget{parent}, CGameMixin()
{
    qDebug("constructor\n");
    new QShortcut(QKeySequence(Qt::Key_F11), this, SLOT(changeZoom()));

    m_mapArch = new CMapArch();
}

void CGameWidget::init()
{
    //setZoom(true);
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
    m_timer.setInterval(1000 / TICK_RATE);
    m_timer.start();
    connect(&m_timer, SIGNAL(timeout()), this, SLOT(mainLoop()));
}

void CGameWidget::exitGame()
{
    //reject();
}

void CGameWidget::mainLoop()
{
    //qDebug("mainLoop\n");
    CGameMixin::mainLoop();
    repaint();
    update();
}

void CGameWidget::changeZoom()
{
    qDebug("changeZoom");
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
    //qDebug("paint\n");
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
    }

    // show the screen
    QSize sz = size();
    //QSize sz{WIDTH * 2, HEIGHT * 2};

    const QImage & img = QImage(reinterpret_cast<uint8_t*>(bitmap.getRGB()), bitmap.len(), bitmap.hei(), QImage::Format_RGBX8888);
    const QPixmap & pixmap = QPixmap::fromImage(m_zoom ? img.scaled(sz): img);
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
            result = Key_Enter;
            break;
        default:
            return;
        }
    }
    m_keyStates[result] = keyState;
}

