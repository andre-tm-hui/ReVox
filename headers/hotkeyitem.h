#ifndef HOTKEYITEM_H
#define HOTKEYITEM_H

#include <QWidget>
#include <QToolButton>
#include <QPushButton>
#include <QTextEdit>
#include <audiomanager.h>
#include <keyboardlistener.h>
#include <thread>
#include <keybindsettings.h>
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

        this->setFixedSize(width - 8, 40);
        this->hotkey = new QPushButton(this);
        this->cb.button = this->hotkey;
        this->settings = new QToolButton(this);
        this->label = new QTextEdit(label, this);

        this->label->move(QPoint(0, 0));
        this->label->setFixedSize(QSize(6 * width / 16, 24));
        this->label->setAlignment(Qt::AlignCenter);
        this->label->setVerticalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
        this->label->setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
        this->label->setStyleSheet("border: 0px; background-color: #303030; color: #FFFFFF;");
        connect(this->label, SIGNAL(textChanged()), this, SLOT(labelChanged()));

        this->hotkey->move(QPoint((7 * width / 16), 0));
        this->hotkey->setFixedSize(QSize(6 * width / 16, 24));
        this->hotkey->setStyleSheet("border: 0px; background-color: #303030; color: #FFFFFF;");
        connect(this->hotkey, SIGNAL(clicked()), this, SLOT(rebind()));

        this->settings->move(QPoint(width - 48, 0));
        this->settings->setFixedSize(QSize(24, 24));
        this->settings->setIcon(isSoundboard ? QIcon(":/icons/settings.png") :  QIcon(":/icons/save.png"));
        this->settings->setStyleSheet("border: 0px; background-color: transparent;");
        if (isSoundboard)
        {
            connect(this->settings, SIGNAL(clicked()), this, SLOT(openSettings()));
        }
        else
        {
            connect(this->settings, SIGNAL(clicked()), this, SLOT(overrideConfig()));
        }

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
    QToolButton *settings;
    QTextEdit *label;
    cbData cb;

private slots:
    void rebind();
    void openSettings();
    void overrideConfig();
    void labelChanged();


private:
    static void WaitForKeyboardInput(void *data);
    QString GetKeyName(int keybind);
};

#endif // HOTKEYITEM_H
