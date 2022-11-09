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
    QPushButton *button;
    int idx;
    int keycode;
    bool isSoundboard;
    QWidget *hotkeyItem;
    QWidget *mainWindow;
    QWidget *mainThread;
} cbData;

class HotkeyItem : public QWidget
{
    Q_OBJECT;
public:
    inline HotkeyItem(bool isSoundboard,
                      QString label,
                      int width,
                      int idx,
                      int keycode,
                      AudioManager *audioManager,
                      QWidget *mainWindow) : QWidget()
    {
        this->cb.isSoundboard = isSoundboard;
        this->cb.idx = idx;
        this->cb.keycode = keycode;
        this->cb.audioManager = audioManager;
        this->cb.hotkeyItem = this;
        this->cb.mainWindow = mainWindow;
        this->cb.mainThread = this;

        this->icon = new QWidget(this);
        this->icon->setStyleSheet("QWidget {image: url(:/icons/edit.png); background: transparent;}");
        this->icon->setFixedSize(QSize(16, 16));
        this->icon->move(QPoint(10, 10));

        this->hotkey = new QPushButton(this);
        this->cb.button = this->hotkey;
        this->label = new QLineEdit(label, this);

        this->label->move(QPoint(42, 3));
        this->label->setFixedSize(QSize(10 * width / 16, 30));

        this->label->setStyleSheet("QLineEdit {border-radius: 3px; background-color: #303030; color: #FFFFFF;}"
                                   "QLineEdit:hover {background-color: #404040;}");
        this->label->setAlignment(Qt::AlignVCenter);
        this->label->setTextMargins(10, 0, 10, 0);
        connect(this->label, SIGNAL(textChanged(QString)), this, SLOT(labelChanged()));

        this->hotkey->move(QPoint((12 * width / 16), 3));
        this->hotkey->setFixedSize(QSize(4 * width / 16 - 6, 30));
        this->hotkey->setStyleSheet("QPushButton {border-radius: 3px; background-color: #303030; color: #FFFFFF;}"
                                    "QPushButton:hover {background-color: #404040;}");

        json *hotkeys = isSoundboard ? &(audioManager->soundboardHotkeys) : &(audioManager->voiceFXHotkeys);
        connect(this->hotkey, SIGNAL(clicked()), this, SLOT(rebind()));

        if (keycode == -1)
        {
            rebind();
        }
        else if (keycode < 0)
        {
            this->hotkey->setText("None");
        }
        else
        {
            this->hotkey->setText(vkCodenames[keycode]);
            this->hotkey->setToolTip(QString::fromStdString((*hotkeys)[std::to_string(idx)]["deviceName"].get<std::string>()));
        }

        connect(this, SIGNAL(newKeySet(QString)), this, SLOT(cleanup(QString)), Qt::ConnectionType::QueuedConnection);
    }

    QWidget *icon;
    QPushButton *hotkey;
    QLineEdit *label;
    cbData cb;

signals:
    void newKeySet(QString qKeybind);

private slots:
    void rebind();
    void cleanup(QString qKeybind);
    void labelChanged();

private:
    static void WaitForKeyboardInput(void *data);
    QString GetKeyName(int keybind);
};

#endif // HOTKEYITEM_H
