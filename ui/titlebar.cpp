#include "titlebar.h"
#include "ui_titlebar.h"

TitleBar::TitleBar(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TitleBar)
{
    ui->setupUi(this);

    connect(ui->exit, SIGNAL(pressed()), this->parent(), SLOT(close()));
    connect(ui->help, SIGNAL(pressed()), this, SLOT(openHelp()));
}

TitleBar::~TitleBar()
{
    delete ui;
}

void TitleBar::openHelp()
{
    QDesktopServices::openUrl(QUrl("https://github.com/andre-tm-hui/ReVox/wiki"));
}

void TitleBar::mousePressEvent(QMouseEvent *event) {
    m_nMouseClick_X_Coordinate = event->pos().x();
    m_nMouseClick_Y_Coordinate = event->pos().y();
}

void TitleBar::mouseMoveEvent(QMouseEvent *event) {
    //move(event->globalPosition().x()-m_nMouseClick_X_Coordinate,
    //     event->globalPosition().y()-m_nMouseClick_Y_Coordinate);
    emit windowMoved(event, m_nMouseClick_X_Coordinate, m_nMouseClick_Y_Coordinate);
}
