#include "hotkeyitem.h"

void HotkeyItem::rebind()
{
    cb.audioManager->Rebind(cb.keycode);
    cb.keyboardListener->rebinding = true;

    hotkey->setText("Press a Key");
    this->hotkey->setStyleSheet("QPushButton {border-radius: 3px; background-color: #303030; color: #AA0000;}"
                                "QPushButton:hover {background-color: #404040;}");
    hotkey->blockSignals(true);
    this->setEnabled(false);

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
    json *data = cb.isSoundboard ? &cb.audioManager->soundboardHotkeys[std::to_string(cb.keycode)] :
                                &cb.audioManager->voiceFXHotkeys[std::to_string(cb.keycode)];
    (*data)["label"] = this->label->text().toStdString();
    cb.audioManager->SaveBinds();
}

void HotkeyItem::WaitForKeyboardInput(void *data)
{
    cbData *m_data = (cbData*) data;
    std::unique_lock<std::mutex> lck(*m_data->keyboardListener->mtx);
    m_data->keyboardListener->cv->wait(lck, m_data->keyboardListener->ready);

    int newKeycode = m_data->keyboardListener->rebindTo;
    QString qKeybind;

    if (newKeycode == -1 && m_data->keycode == -1) newKeycode--;

    if (newKeycode < -1)
    {
        while (!(m_data->audioManager->soundboardHotkeys.find(std::to_string(newKeycode)) == m_data->audioManager->soundboardHotkeys.end() &&
                 m_data->audioManager->voiceFXHotkeys.find(std::to_string(newKeycode)) == m_data->audioManager->voiceFXHotkeys.end()))
            newKeycode--;
        qKeybind = "None";
    }
    else
    {
        if (newKeycode == -1)
            newKeycode = m_data->keycode;

        if (newKeycode >= 0) {
            qKeybind = vkCodenames[newKeycode];
        }
        else
            qKeybind = "None";
    }

    m_data->keycode = newKeycode;
    m_data->button->setText(qKeybind);
    m_data->button->setStyleSheet("QPushButton {border-radius: 3px; background-color: #303030; color: #FFFFFF;}"
                                  "QPushButton:hover {background-color: #404040;}");
    m_data->button->blockSignals(false);
    m_data->hotkeyItem->setEnabled(true);

    m_data->audioManager->SetNewBind(m_data->keycode, m_data->isSoundboard);

    m_data->addButton->setEnabled(true);
    m_data->mainWindow->setEnabled(true);
}

QString HotkeyItem::GetKeyName(int keybind)
{
    return vkCodenames[keybind];
}

