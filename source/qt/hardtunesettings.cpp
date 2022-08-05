#include "hardtunesettings.h"
#include "ui_hardtunesettings.h"

HardtuneSettings::HardtuneSettings(PitchShift *autotune, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::HardtuneSettings)
{
    ui->setupUi(this);

    this->autotune = autotune;
    currentNotes = this->autotune->getNotes();
    setCheckboxes();

    connect(&mapper, SIGNAL(mappedInt(int)), this, SLOT(toggleNote(int)));
    connect(ui->C, SIGNAL(stateChanged(int)), &mapper, SLOT(map()));
    mapper.setMapping(ui->C, 0);
    connect(ui->Db, SIGNAL(stateChanged(int)), &mapper, SLOT(map()));
    mapper.setMapping(ui->Db, 1);
    connect(ui->D, SIGNAL(stateChanged(int)), &mapper, SLOT(map()));
    mapper.setMapping(ui->D, 2);
    connect(ui->Eb, SIGNAL(stateChanged(int)), &mapper, SLOT(map()));
    mapper.setMapping(ui->Eb, 3);
    connect(ui->E, SIGNAL(stateChanged(int)), &mapper, SLOT(map()));
    mapper.setMapping(ui->E, 4);
    connect(ui->F, SIGNAL(stateChanged(int)), &mapper, SLOT(map()));
    mapper.setMapping(ui->F, 5);
    connect(ui->Gb, SIGNAL(stateChanged(int)), &mapper, SLOT(map()));
    mapper.setMapping(ui->Gb, 6);
    connect(ui->G, SIGNAL(stateChanged(int)), &mapper, SLOT(map()));
    mapper.setMapping(ui->G, 7);
    connect(ui->Ab, SIGNAL(stateChanged(int)), &mapper, SLOT(map()));
    mapper.setMapping(ui->Ab, 8);
    connect(ui->A, SIGNAL(stateChanged(int)), &mapper, SLOT(map()));
    mapper.setMapping(ui->A, 9);
    connect(ui->Bb, SIGNAL(stateChanged(int)), &mapper, SLOT(map()));
    mapper.setMapping(ui->Bb, 10);
    connect(ui->B, SIGNAL(stateChanged(int)), &mapper, SLOT(map()));
    mapper.setMapping(ui->B, 11);

    connect(ui->scale, SIGNAL(currentIndexChanged(int)), this, SLOT(setScale(int)));
}

HardtuneSettings::~HardtuneSettings()
{
    delete ui;
}

void HardtuneSettings::setCheckboxes()
{
    ui->C->blockSignals(true);
    ui->C->setCheckState(currentNotes[0] ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);
    ui->C->blockSignals(false);
    ui->Db->blockSignals(true);
    ui->Db->setCheckState(currentNotes[1] ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);
    ui->Db->blockSignals(false);
    ui->D->blockSignals(true);
    ui->D->setCheckState(currentNotes[2] ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);
    ui->D->blockSignals(false);
    ui->Eb->blockSignals(true);
    ui->Eb->setCheckState(currentNotes[3] ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);
    ui->Eb->blockSignals(false);
    ui->E->blockSignals(true);
    ui->E->setCheckState(currentNotes[4] ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);
    ui->E->blockSignals(false);
    ui->F->blockSignals(true);
    ui->F->setCheckState(currentNotes[5] ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);
    ui->F->blockSignals(false);
    ui->Gb->blockSignals(true);
    ui->Gb->setCheckState(currentNotes[6] ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);
    ui->Gb->blockSignals(false);
    ui->G->blockSignals(true);
    ui->G->setCheckState(currentNotes[7] ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);
    ui->G->blockSignals(false);
    ui->Ab->blockSignals(true);
    ui->Ab->setCheckState(currentNotes[8] ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);
    ui->Ab->blockSignals(false);
    ui->A->blockSignals(true);
    ui->A->setCheckState(currentNotes[9] ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);
    ui->A->blockSignals(false);
    ui->Bb->blockSignals(true);
    ui->Bb->setCheckState(currentNotes[10] ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);
    ui->Bb->blockSignals(false);
    ui->B->blockSignals(true);
    ui->B->setCheckState(currentNotes[11] ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);
    ui->B->blockSignals(false);
}

void HardtuneSettings::toggleNote(int note)
{
    currentNotes[note] = !currentNotes[note];
    autotune->setNotes(currentNotes);
    this->ui->scale->setCurrentText("Custom");
    for (auto const& [note, scale] : scales)
    {
        if (currentNotes == scale)
        {
            this->ui->scale->setCurrentIndex((int)note);
        }
    }
}

void HardtuneSettings::setScale(int note)
{
    if (note < 12)
    {
        currentNotes = scales[(Note)note];
        autotune->setNotes(currentNotes);
        setCheckboxes();
    }
}
