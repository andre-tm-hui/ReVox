#include "onboarding.h"
#include "ui_onboarding.h"

Onboarding::Onboarding(SoundboardMenu *soundboardMenu,
                       FXMenu *fxMenu,
                       QToolButton *fxMenuButton,
                       QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Onboarding),
    soundboardMenu(soundboardMenu),
    fxMenu(fxMenu),
    fxMenuButton(fxMenuButton)
{
    ui->setupUi(this);

    slides.push_back(ui->slide);
    slides.push_back(ui->slide_2);
    slides.push_back(ui->slide_3);
    slides.push_back(ui->slide_4);
    slides.push_back(ui->slide_5);
    slides.push_back(ui->slide_6);
    slides.push_back(ui->slide_7);
    slides.push_back(ui->slide_8);
    slides.push_back(ui->slide_9);
    slides.push_back(ui->slide_10);
    slides.push_back(ui->slide_11);
    slides.push_back(ui->slide_12);
    slides.push_back(ui->slide_13);

    for (unsigned int i = 0; i < slides.size(); i++) {
        slides[i]->hide();
    }

    connect(ui->next_1, SIGNAL(clicked()), this, SLOT(next()));
    connect(ui->next_2, SIGNAL(clicked()), this, SLOT(next()));
    connect(ui->next_4, SIGNAL(clicked()), this, SLOT(next()));
    connect(ui->next_6, SIGNAL(clicked()), this, SLOT(next()));
    connect(ui->next_7, SIGNAL(clicked()), this, SLOT(next()));
    connect(ui->next_8, SIGNAL(clicked()), this, SLOT(next()));
    connect(ui->next_13, SIGNAL(clicked()), this, SLOT(next()));
    connect(ui->next_15, SIGNAL(clicked()), this, SLOT(next()));
    connect(ui->next_17, SIGNAL(clicked()), this, SLOT(next()));

    connect(soundboardMenu, SIGNAL(addBindPressed()), this, SLOT(next()));
    connect(soundboardMenu, SIGNAL(itemSelected()), this, SLOT(next()));
    connect(fxMenuButton, SIGNAL(clicked()), this, SLOT(next()));
    connect(fxMenu, SIGNAL(itemSelected()), this, SLOT(next()));

    showSlide();
}

Onboarding::~Onboarding()
{
    delete ui;
}

void Onboarding::next()
{
    hideSlide();
    currentSlide++;
    if (currentSlide == 12) {
        ui->skip->hide();
    }
    if (currentSlide < slides.size())
        showSlide();
    else
        delete this;
}

void Onboarding::skip()
{
    hideSlide();
    currentSlide = slides.size() - 1;
    showSlide();
    ui->skip->hide();
}

void Onboarding::hideSlide()
{
    slides[currentSlide]->hide();
}

void Onboarding::showSlide()
{
    QGroupBox *slide = slides[currentSlide];
    slide->move(0, 0);
    slide->show();
    QString url = ":/images/pixMasks/" + QString::number(currentSlide) + ".bmp";
    QBitmap bm = QBitmap(url);
    this->setMask(bm);
}
