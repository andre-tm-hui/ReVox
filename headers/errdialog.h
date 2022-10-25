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
        "Audio Devices Misconfigured"
    };

    std::vector<QString> errMessages = {
        "No error",
        "Virtual Audio Cable not detected. Please install from <a href=\"https://vb-audio.com/Cable/\">here</a>.",
        "Make sure the VB-Audio Cable devices are not set as default audio devices."
    };
};

#endif // ERRDIALOG_H
