#include "fxmenu.h"
#include "ui_fxmenu.h"

FXMenu::FXMenu(json *hotkeys,
               AudioManager *am,
               KeyboardListener *kl,
               HUD *hud,
               QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FXMenu)
{
    ui->setupUi(this);

    this->hotkeys = hotkeys;
    this->am = am;
    this->kl = kl;
    this->hud = hud;

    reverbTab = new ReverbTab(am, ui->tabWidget, ui->reverb);
    autotuneTab = new AutotuneTab(am, ui->tabWidget, ui->autotune);
    pitchTab = new PitchTab(am, ui->tabWidget, ui->pitch);

    connect(ui->hotkeysList, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(onHotkeySelect()));

    connect(ui->slider, SIGNAL(sliderMoved(int)), this, SLOT(onSliderChanged(int)));
    connect(ui->slider, SIGNAL(valueChanged(int)), this, SLOT(onSliderChanged(int)));
    ui->slider->setValue(am->settings["monitorMic"]);

    connect(ui->add, SIGNAL(clicked()), this, SLOT(addBind()));
    connect(ui->remove, SIGNAL(clicked()), this, SLOT(removeBind()));

    for (auto& [keybind, settings] : (*hotkeys).items())
    {
        addBind(settings["keycode"].get<int>(), QString::fromStdString(settings["label"].get<std::string>()));
    }

    for (int i = 0; i < ui->tabWidget->count(); i++)
    {
        ui->tabWidget->widget(i)->setEnabled(false);
    }
}

FXMenu::~FXMenu()
{
    delete ui;
}

void FXMenu::DisableTabWidget()
{
    for (int i = 0; i < ui->tabWidget->count(); i++)
    {
        ui->tabWidget->widget(i)->setEnabled(false);
        ui->tabWidget->setTabIcon(i, QIcon(":/icons/FXOff.png"));
    }

    reverbTab->SetValues(&(*(am->GetFXOffPointer()))["reverb"],
                         ui->tabWidget->currentIndex() == 0 ? true : false);

    autotuneTab->SetValues(&(*(am->GetFXOffPointer()))["autotune"],
                           ui->tabWidget->currentIndex() == 1 ? true : false);

    pitchTab->SetValues(&(*(am->GetFXOffPointer()))["pitch"],
                        ui->tabWidget->currentIndex() == 2 ? true : false);

    ui->hotkeysList->clearSelection();
    currHotkey = -1;
    ui->overlay->show();
    ui->remove->setEnabled(false);
}

void FXMenu::OnHotkeyToggle(int idx, bool clear)
{
    if (hotkeys->find(std::to_string(idx)) == hotkeys->end()) return;
    currHotkey = idx;
    json *settings = &(*hotkeys)[std::to_string(currHotkey)];
    for (int i = 0; i < ui->tabWidget->count(); i++)
    {
        ui->tabWidget->widget(i)->setEnabled(!clear);
    }

    am->passthrough->SetFX(*settings);

    reverbTab->SetValues(&(*settings)["reverb"],
                         ui->tabWidget->currentIndex() == 0 ? true : false);


    autotuneTab->SetValues(&(*settings)["autotune"],
                           ui->tabWidget->currentIndex() == 1 ? true : false);


    pitchTab->SetValues(&(*settings)["pitch"],
                        ui->tabWidget->currentIndex() == 2 ? true : false);


    if (clear)
    {
        ui->hotkeysList->clearSelection();
    }
    else
    {
        ui->overlay->hide();
    }
}

void FXMenu::onHotkeySelect()
{
    HotkeyItem *item = qobject_cast<HotkeyItem *>(ui->hotkeysList->itemWidget(ui->hotkeysList->currentItem()));
    if (item->cb.idx == this->currHotkey) return;
    OnHotkeyToggle(item->cb.idx, false);
    ui->remove->setEnabled(true);

    emit itemSelected();
}

void FXMenu::addBind(int keybind, QString label)
{
    HotkeyItem *item = new HotkeyItem(false,
                                      label == "" ? "Bind " + QString::number(ui->hotkeysList->count() + 1) : label,
                                      ui->hotkeysList->width(),
                                      ui->hotkeysList->count(),
                                      keybind,
                                      am,
                                      this->parentWidget());
    QListWidgetItem *orig = new QListWidgetItem();
    orig->setFlags(Qt::ItemIsEnabled | Qt::ItemIsEditable | Qt::ItemIsSelectable);
    ui->hotkeysList->addItem(orig);
    ui->hotkeysList->setItemWidget(orig, item);
    orig->setSizeHint(QSize(0, 36));

    ui->hotkeysList->scrollToBottom();
    ui->hotkeysList->resize(ui->hotkeysList->width(), __min(36 * ui->hotkeysList->count() + 2, 221));
}

void FXMenu::removeBind()
{
    HotkeyItem *item = qobject_cast<HotkeyItem *>(ui->hotkeysList->itemWidget(ui->hotkeysList->currentItem()));
    am->RemoveBind(item->cb.idx, bindType::voicefx);
    ui->hotkeysList->removeItemWidget(ui->hotkeysList->currentItem());
    delete ui->hotkeysList->currentItem();
    DisableTabWidget();

    for (int i = item->cb.idx; i < ui->hotkeysList->count(); i++) {
        HotkeyItem *item = qobject_cast<HotkeyItem *>(ui->hotkeysList->itemWidget(ui->hotkeysList->item(i)));
        item->cb.idx--;
    }

    ui->hotkeysList->resize(ui->hotkeysList->width(), __min(36 * ui->hotkeysList->count() + 2, 221));
}

void FXMenu::onSliderChanged(int value)
{
    am->SetMicMonitor(value);
    ui->grooveOff->resize(ui->grooveOff->width(), 343 - 343 * value / ui->slider->maximum());
}
