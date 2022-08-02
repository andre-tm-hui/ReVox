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

void HotkeyItem::openSettings()
{
    if (cb.isSoundboard)
    {
        KeybindSettings popup (cb.keycode, &cb.audioManager->soundboardHotkeys[std::to_string(cb.keycode)], cb.audioManager);
        popup.setModal(true);
        popup.exec();
    }
}

void HotkeyItem::labelChanged()
{
    QTextCursor pos = this->label->textCursor();
    if (this->label->toPlainText().contains('\n'))
    {
        this->label->setText(this->label->toPlainText().remove('\n'));
        this->label->setAlignment(Qt::AlignCenter);
        if (pos.position() > this->label->toPlainText().length()) pos.setPosition(this->label->toPlainText().length());
    }
    if (this->label->toPlainText().length() > 18)
    {
        this->label->setPlainText(this->label->toPlainText().left(18));
        this->label->setAlignment(Qt::AlignCenter);
    }
    this->label->setTextCursor(pos);
    json *data = cb.isSoundboard ? &cb.audioManager->soundboardHotkeys[std::to_string(cb.keycode)] :
                                &cb.audioManager->voiceFXHotkeys[std::to_string(cb.keycode)];
    (*data)["label"] = this->label->toPlainText().toStdString();
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

void HotkeyItem::overrideConfig()
{
    cb.audioManager->OverrideConfig(std::to_string(cb.keycode));
}
