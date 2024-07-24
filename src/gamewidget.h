#ifndef CGAMEWIDGET_H
#define CGAMEWIDGET_H

#include "gamemixin.h"
#include <QObject>
#include <QWidget>

class CMapArch;

class CGameWidget : public QWidget, public CGameMixin
{
    Q_OBJECT
public:
    explicit CGameWidget(QWidget *parent = nullptr);
    void init();

protected slots:
    virtual void mainLoop() override;
    void changeZoom();
    virtual void preloadAssets() override;

protected:
    CMapArch *m_mapArch = nullptr;

private:
    QTimer m_timer;
    virtual void keyPressEvent(QKeyEvent *event) override;
    virtual void keyReleaseEvent(QKeyEvent *event) override;
    virtual void paintEvent(QPaintEvent *) override;
    virtual void exitGame();
    virtual void sanityTest() override;
    virtual void setZoom(bool zoom) override;
    void keyReflector(int key, uint8_t keyState);

signals:
};

#endif // CGAMEWIDGET_H
