#ifndef ONBOARDING_H
#define ONBOARDING_H

#include <QWidget>
#include <QGroupBox>
#include <soundboardmenu.h>
#include <fxmenu.h>

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
                        QWidget *parent = nullptr);
    ~Onboarding();

private slots:
    void next();
    void skip();

private:
    void hideSlide();
    void showSlide();

    Ui::Onboarding *ui;

    std::vector<QGroupBox*> slides;
    unsigned int currentSlide = 0;

    SoundboardMenu *soundboardMenu;
    FXMenu *fxMenu;
    QToolButton *fxMenuButton;
};

#endif // ONBOARDING_H
