#include "bindablebutton.h"

BindableButton::BindableButton(std::shared_ptr<BaseManager> bm, std::string path, bool isNew, QWidget *parent) : QPushButton(parent), path(path), bm(bm)
{
    this->setCursor(QCursor(Qt::CursorShape::ArrowCursor));
    this->setStyleSheet("QPushButton {margin: 3px; border-radius: 3px; background-color: #303030; color: #D0D0D0;}"
                        "QPushButton:hover {background-color: #404040;}");

    connect(this, SIGNAL(pressed()), this, SLOT(startRebinding()));
    connect(this, SIGNAL(newHotkeySet()), this, SLOT(cleanUp()), Qt::ConnectionType::QueuedConnection);

    if (isNew) startRebinding();
    else updateSelf();
}

void BindableButton::setSettingPath(std::string path) {
    this->path = path;
    updateSelf();
}

void BindableButton::startRebinding() {
    if (bm.get() == nullptr) return;
    this->setText("Press any key");
    this->setToolTip("");
    this->setStyleSheet("QPushButton {margin: 3px; border-radius: 3px; background-color: #303030; color: #AA0000;}"
                        "QPushButton:hover {background-color: #404040;}");

    bm->StartRebinding(path);
    QWidget::window()->setEnabled(false);

    std::thread t(waitForRebind, this, bm);
    t.detach();
}

void BindableButton::cleanUp() {
    RebindData data = bm->GetRebindData();
    this->setStyleSheet("QPushButton {margin: 3px; border-radius: 3px; background-color: #303030; color: #D0D0D0;}"
                        "QPushButton:hover {background-color: #404040;}");

    if (data.keycode >= 0 && data.keycode < 256)
        this->setText(vkCodenames[data.keycode]);
    else
        this->setText("None");

    this->setToolTip(QString::fromStdString(data.deviceName));

    QWidget::window()->setEnabled(true);
}

void BindableButton::waitForRebind(BindableButton *hkb, std::shared_ptr<BaseManager> bm) {
    while (bm->IsRebinding()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    emit hkb->newHotkeySet();
}

void BindableButton::updateSelf() {
    json settings = bm->GetSetting(path);
    int keycode = settings["keycode"].get<int>();
    if (keycode >= 0 && keycode < 256)
        this->setText(vkCodenames[keycode]);
    else
        this->setText("None");
    this->setToolTip(QString::fromStdString(settings["deviceName"]));
}
