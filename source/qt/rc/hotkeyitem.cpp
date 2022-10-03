#include "hotkeyitem.h"

void HotkeyItem::rebind()
{
    cb.audioManager->Rebind(cb.keycode);
    cb.keyboardListener->rebinding = true;

    hotkey->setText("Listening...");
    hotkey->blockSignals(true);

    std::thread t (WaitForKeyboardInput, &this->cb);
    t.detach();
}

void HotkeyItem::labelChanged()
{
    int pos = this->label->cursorPosition();
    if (this->label->text().length() > 18)
    {
        this->label->setText(this->label->text().left(18));
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

    m_data->keycode = m_data->keyboardListener->rebindTo;
    QString qKeybind = vkCodenames[m_data->keycode];
    m_data->button->setText(qKeybind);
    m_data->button->blockSignals(false);

    m_data->audioManager->SetNewBind(m_data->keycode, m_data->isSoundboard);
}

QString HotkeyItem::GetKeyName(int keybind)
{
    return vkCodenames[keybind];
}

