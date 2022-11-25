#include "onboarding.h"
#include "ui_onboarding.h"
#include <iostream>

Onboarding::Onboarding(SoundboardMenu *soundboardMenu,
                       FXMenu *fxMenu,
                       QToolButton *fxMenuButton,
                       MainInterface *mi,
                       QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Onboarding),
    soundboardMenu(soundboardMenu),
    fxMenu(fxMenu),
    fxMenuButton(fxMenuButton),
    mi(mi)
{
    ui->setupUi(this);

    for (int i = 1; i <= totalSlides; i++) {
        QString n = QString::number(i);
        if (i < 10) n = "0" + QString::number(i);
        QGroupBox *slide = this->findChild<QGroupBox*>("slide_" + n);
        slide->findChild<QGroupBox*>("instruction_" + n)->setStyleSheet(
                    "QGroupBox { background: #E0202020; border-radius: 10px; }"
                    );
        QPushButton *button = slide->findChild<QPushButton*>("next_" + n);
        if (button != nullptr)
            connect(button, SIGNAL(clicked()), this, SLOT(next()));
        slide->move(0,0);
        slide->hide();
        slides.push_back(slide);
    }

    mi->SetBlocked(true);

    connect(soundboardMenu, &SoundboardMenu::itemAdded, this, [=]() { next(); });
    connect(soundboardMenu, &SoundboardMenu::itemSelected, this, [=]() { next(); });
    connect(soundboardMenu, &SoundboardMenu::itemSelected, this, [=]() { mi->SetBlocked(false); });
    connect(soundboardMenu, &SoundboardMenu::clipRecorded, this, [=]() { if (currentSlide == 7) next(); });
    connect(fxMenuButton, &QToolButton::clicked, this, [=]() { next(); });
    connect(fxMenu, &FXMenu::itemSelected, this, [=]() { if (currentSlide <= 20) next(); });
    connect(ui->skip, SIGNAL(clicked()), this, SLOT(skip()));

    showSlide();
}

Onboarding::~Onboarding()
{
    delete ui;
}

void Onboarding::next()
{
    slides[currentSlide-1]->hide();
    currentSlide++;
    if (currentSlide == totalSlides) {
        ui->skip->hide();
    }
    if (currentSlide <= slides.size())
        showSlide();
    else
        delete this;
}

void Onboarding::skip()
{
    slides[currentSlide-1]->hide();
    currentSlide = slides.size();
    showSlide();
    ui->skip->hide();
}

void Onboarding::showSlide()
{
    QGroupBox *slide = slides[currentSlide-1];
    slide->move(0, 0);
    slide->show();
    QString url = ":/rc/onboarding/" + QString::number(currentSlide) + ".bmp";
    QBitmap bm = QBitmap(url);
    this->setMask(bm);
}
