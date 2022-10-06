#ifndef ERRDIALOG_H
#define ERRDIALOG_H

#include <QDialog>
#include <sstream>

namespace Ui {
class ErrDialog;
}

class ErrDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ErrDialog(int errCode, QWidget *parent = nullptr);
    ~ErrDialog();

signals:
    void errQuit();

private:
    Ui::ErrDialog *ui;

    std::vector<QString> errTitles = {
        "No error",
        "VB-Cable Missing",
        "Default devices incorrectly configured"
    };

    std::vector<QString> errMessages = {
        "No error",
        "Virtual Audio Cable not detected. Please install from <a href=\"https://vb-audio.com/Cable/\">here</a>.",
        "The Virtual Audio Cable has been set as a default device. Please change this in your sound settings."
    };
};

#endif // ERRDIALOG_H
