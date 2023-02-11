#ifndef FXSWITCH_H
#define FXSWITCH_H

#include <QWidget>
#include "fxparam.h"

namespace Ui {
class FXSwitch;
}

class FXSwitch : public FXParam
{
    Q_OBJECT

public:
    explicit FXSwitch(std::shared_ptr<FXManager> fm,
                      std::string fx,
                      std::string param,
                      QWidget *parent = nullptr);
    ~FXSwitch();

    void setValue(int val) override;

protected:
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void onToggled(int val);

private:
    Ui::FXSwitch *ui;
};

#endif // FXSWITCH_H
