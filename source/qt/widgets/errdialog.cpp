#include "errdialog.h"
#include "ui_errdialog.h"

ErrDialog::ErrDialog(int errCode, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ErrDialog)
{
    ui->setupUi(this);

    std::stringstream ss;
    ss<<"ReVox - Error Code: "<<errCode;
    this->setWindowTitle(QString::fromStdString(ss.str()));

    ui->label->setText(errTitles[errCode]);
    ui->label_2->setText(errMessages[errCode]);

    connect(ui->pushButton, SIGNAL(pressed()), this, SLOT(close()));
}

ErrDialog::~ErrDialog()
{
    emit errQuit();
    delete ui;
}
