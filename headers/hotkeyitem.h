#ifndef HOTKEYITEM_H
#define HOTKEYITEM_H

#include <QWidget>
#include <QToolButton>
#include <QPushButton>
#include <QLineEdit>
#include <audiomanager.h>
#include <keyboardlistener.h>
#include <thread>
#include <vkcodenames.h>

typedef struct {
    AudioManager *audioManager;
    KeyboardListener *keyboardListener;
    QPushButton *button;
    int keycode;
    bool isSoundboard;
} cbData;

class HotkeyItem : public QWidget
{
    Q_OBJECT;
public:
    inline HotkeyItem(bool isSoundboard, QString label, int width, int keycode, AudioManager *audioManager, KeyboardListener *keyboardListener) : QWidget()
    {
        this->cb.isSoundboard = isSoundboard;

        this->cb.keycode = keycode;
        this->cb.audioManager = audioManager;
        this->cb.keyboardListener = keyboardListener;

        this->hotkey = new QPushButton(this);
        this->cb.button = this->hotkey;
        this->label = new QLineEdit(label, this);

        this->label->move(QPoint(10, 3));
        this->label->setFixedSize(QSize(10 * width / 16, 30));

        //this->label->setVerticalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
        //this->label->setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
        this->label->setStyleSheet("border-radius: 2px; background-color: #303030; color: #FFFFFF;");
        this->label->setAlignment(Qt::AlignVCenter);
        this->label->setTextMargins(10, 0, 10, 0);
        connect(this->label, SIGNAL(textChanged(QString)), this, SLOT(labelChanged()));

        this->hotkey->move(QPoint((12 * width / 16), 3));
        this->hotkey->setFixedSize(QSize(4 * width / 16 - 10, 30));
        this->hotkey->setStyleSheet("QPushButton {border-radius: 2px; background-color: #303030; color: #FFFFFF;}"
                                    "QPushButton:hover {background-color: #404040;}");
        connect(this->hotkey, SIGNAL(clicked()), this, SLOT(rebind()));

        if (keycode < 0)
        {
            rebind();
        }
        else
        {
            this->hotkey->setText(vkCodenames[keycode]);
        }
    }

    QPushButton *hotkey;
    QLineEdit *label;
    cbData cb;

private slots:
    void rebind();
    void labelChanged();


private:
    static void WaitForKeyboardInput(void *data);
    QString GetKeyName(int keybind);
};

#endif // HOTKEYITEM_H
