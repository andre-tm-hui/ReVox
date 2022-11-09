#include "hotkeyitem.h"

void HotkeyItem::rebind()
{
    cb.audioManager->StartRebind();

    hotkey->setText("Press a Key");
    this->hotkey->setStyleSheet("QPushButton {border-radius: 3px; background-color: #303030; color: #AA0000;}"
                                "QPushButton:hover {background-color: #404040;}");
    cb.mainWindow->setEnabled(false);

    std::thread t (WaitForKeyboardInput, &this->cb);
    t.detach();
}

void HotkeyItem::labelChanged()
{
    int pos = this->label->cursorPosition();
    if (this->label->text().length() > 30)
    {
        this->label->setText(this->label->text().left(30));
        this->label->setAlignment(Qt::AlignVCenter);
    }
    this->label->setCursorPosition(pos);
    json *data = cb.isSoundboard ? &cb.audioManager->soundboardHotkeys[std::to_string(cb.idx)] :
                                &cb.audioManager->voiceFXHotkeys[std::to_string(cb.idx)];
    (*data)["label"] = this->label->text().toStdString();
    cb.audioManager->SaveBinds();
}

void HotkeyItem::WaitForKeyboardInput(void *data)
{
    cbData *m_data = (cbData*) data;
    while (m_data->audioManager->IsRebinding())
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

    int newKeycode = m_data->audioManager->GetRebindDestination();
    QString qKeybind;

    if (newKeycode == -1 && m_data->keycode == -1) newKeycode--;

    if (newKeycode == -1)
        newKeycode = m_data->keycode;

    if (newKeycode >= 0)
        qKeybind = vkCodenames[newKeycode];
    else
        qKeybind = "None";

    m_data->keycode = newKeycode;

    HotkeyItem *hk = (HotkeyItem*)(m_data->mainThread);

    emit hk->newKeySet(qKeybind);
}

QString HotkeyItem::GetKeyName(int keybind)
{
    return vkCodenames[keybind];
}


void HotkeyItem::cleanup(QString qKeybind)
{
    cb.button->setText(qKeybind);
    cb.button->setStyleSheet("QPushButton {border-radius: 3px; background-color: #303030; color: #FFFFFF;}"
                                  "QPushButton:hover {background-color: #404040;}");
    QString qDeviceName = "";
    if (!qKeybind.contains("None"))
        qDeviceName = QString::fromStdString(cb.audioManager->GetRebindDevice());
    cb.button->setToolTip(qDeviceName);
    cb.audioManager->SetNewBind(cb.keycode, cb.audioManager->GetRebindDevice(), cb.idx, cb.isSoundboard ? bindType::soundboard : bindType::voicefx);
    cb.mainWindow->setEnabled(true);
}
