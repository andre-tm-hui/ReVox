#ifndef FXDIAL_H
#define FXDIAL_H

#include <QWidget>

namespace Ui {
class FXDial;
}

class FXDial : public QWidget
{
    Q_OBJECT

public:
    explicit FXDial(QWidget *parent = nullptr);
    ~FXDial();

private slots:
    void onValueChange(int value);

signals:
    void valueChanged(int value);

private:
    Ui::FXDial *ui;
};

#endif // FXDIAL_H
