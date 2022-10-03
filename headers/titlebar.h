#ifndef TITLEBAR_H
#define TITLEBAR_H

#include <QWidget>
#include <QMouseEvent>
#include <QDesktopServices>
#include <QUrl>

namespace Ui {
class TitleBar;
}

class TitleBar : public QWidget
{
    Q_OBJECT

public:
    explicit TitleBar(QWidget *parent = nullptr);
    ~TitleBar();

signals:
    void windowMoved(QMouseEvent*, int x, int y);

private slots:
    void openHelp();

private:
    Ui::TitleBar *ui;

    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    int m_nMouseClick_X_Coordinate;
    int m_nMouseClick_Y_Coordinate;
};

#endif // TITLEBAR_H
