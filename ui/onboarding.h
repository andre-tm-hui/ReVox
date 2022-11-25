#ifndef ONBOARDING_H
#define ONBOARDING_H

#include <QWidget>
#include <QGroupBox>
#include "../interface/maininterface.h"
#include "menus/soundboardmenu.h"
#include "menus/fxmenu.h"

namespace Ui {
class Onboarding;
}

class Onboarding : public QWidget
{
    Q_OBJECT

public:
    explicit Onboarding(SoundboardMenu *soundboardMenu,
                        FXMenu *fxMenu,
                        QToolButton *fxMenuButton,
                        MainInterface *mi,
                        QWidget *parent = nullptr);
    ~Onboarding();

private slots:
    void next();
    void skip();

private:
    void showSlide();

    Ui::Onboarding *ui;

    std::vector<QGroupBox*> slides;
    unsigned int currentSlide = 1;
    const unsigned int totalSlides = 22,
                       unblockAt = 10;

    SoundboardMenu *soundboardMenu;
    FXMenu *fxMenu;
    QToolButton *fxMenuButton;
    MainInterface *mi;
};

#endif // ONBOARDING_H
