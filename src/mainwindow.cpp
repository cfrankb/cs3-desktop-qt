#include "mainwindow.h"
#include "gamewidget.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    QCoreApplication::setApplicationName("cs3v2");
    QCoreApplication::setOrganizationName("cfrankb");

    setWindowTitle(tr("CS3v2 Runtime"));
    m_widget = new CGameWidget(this);
    m_widget->init();

    QPalette pal = QPalette();

    m_widget->update();

    setCentralWidget(m_widget);
    centralWidget()->show();
    centralWidget()->setFocus();
}

MainWindow::~MainWindow()
{
    delete ui;
}
