#include "autotunetab.h"
#include "ui_autotunetab.h"

AutotuneTab::AutotuneTab(AudioManager *am, QTabWidget *tabWidget, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AutotuneTab)
{
    ui->setupUi(this);

    this->am = am;
    this->tabWidget = tabWidget;

    menu = new QMenu();
    menu->setStyleSheet("QMenu { background: #202020; font-family: Cascadia Code; color: #FFFFFF; }"
                        "QMenu::item:selected { background: #404040; }");
    ui->setScale->setMenu(menu);

    connect(ui->toggle, SIGNAL(stateChanged(int)), this, SLOT(toggle(int)));

    noteCheckboxes.push_back(ui->C);
    noteCheckboxes.push_back(ui->Db);
    noteCheckboxes.push_back(ui->D);
    noteCheckboxes.push_back(ui->Eb);
    noteCheckboxes.push_back(ui->E);
    noteCheckboxes.push_back(ui->F);
    noteCheckboxes.push_back(ui->Gb);
    noteCheckboxes.push_back(ui->G);
    noteCheckboxes.push_back(ui->Ab);
    noteCheckboxes.push_back(ui->A);
    noteCheckboxes.push_back(ui->Bb);
    noteCheckboxes.push_back(ui->B);

    mapper = new QSignalMapper(this), mapperAct = new QSignalMapper(this);
    QAction *a = nullptr;
    for (int i = 0; i < 12; i++)
    {
        a = new QAction(menuLabels[i],this);
        menu->addAction(a);
        mapperAct->setMapping(a, i);
        connect(a, SIGNAL(triggered()), mapperAct, SLOT(map()));
        mapper->setMapping(noteCheckboxes[i], i);
        connect(noteCheckboxes[i], SIGNAL(stateChanged(int)), mapper, SLOT(map()));
    }

    connect(mapperAct, SIGNAL(mappedInt(int)), this, SLOT(setScale(int)));
    connect(mapper, SIGNAL(mappedInt(int)), this, SLOT(toggleNote(int)));
}

AutotuneTab::~AutotuneTab()
{
    delete ui;
}

void AutotuneTab::SetValues(json *values, bool animate)
{
    this->values = values;
    this->blockSignals(true);
    ui->toggle->setCheckState((*values)["enabled"] ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);
    this->blockSignals(false);

    if (*this->values == nullptr) return;

    setFields(animate);
}

void AutotuneTab::setFields(bool animate)
{
    if ((*this->values)["enabled"])
    {
        ui->settings->setEnabled(true);
        ui->notes->setEnabled(true);

        if (animate)
        {
            // animate
        }
        else
        {

        }

        for (int i = 0; i < 12; i++)
        {
            noteCheckboxes[i]->setCheckState((*this->values)["notes"][i] ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);
        }
    }
    else
    {
        ui->settings->setEnabled(false);

        if (animate)
        {
            // animate
        }
        else
        {

        }

        for (int i = 0; i < 12; i++)
        {
            noteCheckboxes[i]->setCheckState(Qt::CheckState::Unchecked);
        }
    }
}

void AutotuneTab::toggle(int state)
{
    if (state == 0)
    {
        tabWidget->setTabIcon(1, QIcon(":/icons/FXOff.png"));
        am->passthrough->data.pitchShift->setAutotune(false);
        if (this->values != nullptr) (*this->values)["enabled"] = false;
        setFields();
        ui->overlay->show();
    }
    else
    {
        tabWidget->setTabIcon(1, QIcon(":/icons/FXOn.png"));
        am->passthrough->data.pitchShift->setAutotune(true);
        if (this->values != nullptr) (*this->values)["enabled"] = true;
        setFields();
        ui->overlay->hide();
    }
    am->SaveBinds();
}

void AutotuneTab::toggleNote(int note)
{
    if (ui->settings->isEnabled())
    {
        bool value = noteCheckboxes[note]->checkState() == Qt::CheckState::Checked ? true : false;
        if (this->values != nullptr) (*values)["notes"][note] = value;
        notes[note] = value;
        am->passthrough->data.pitchShift->setNotes(notes);
        am->SaveBinds();
    }
}

void AutotuneTab::setScale(int opt)
{
    for (int i = 0; i < 12; i++)
    {
        noteCheckboxes[i]->setCheckState(scales[(Note)opt][i] ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);
    }
}
