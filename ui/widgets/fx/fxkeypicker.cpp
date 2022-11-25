#include "fxkeypicker.h"
#include "ui_fxkeypicker.h"

FXKeyPicker::FXKeyPicker(std::shared_ptr<FXManager> fm,
                         std::string fx,
                         std::string param,
                         QWidget *parent) :
    FXParam(fm, fx, param, parent),
    ui(new Ui::FXKeyPicker)
{
    ui->setupUi(this);

    menu = new QMenu();
    menu->setStyleSheet("QMenu { background: #202020; color: #D0D0D0; }"
                        "QMenu::item:selected { background: #00A0A0; }");
    ui->keySelect->setMenu(menu);

    mapper = new QSignalMapper(this);
    mapperMenu = new QSignalMapper(this);

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

    for (int i = 0; i < 12; i++) {
        mapper->setMapping(noteCheckboxes[i], i);
        connect(noteCheckboxes[i], SIGNAL(pressed()), mapper, SLOT(map()));
        QAction *a = new QAction(labels[i], ui->keySelect);
        mapperMenu->setMapping(a, i);
        connect(a, SIGNAL(triggered()), mapperMenu, SLOT(map()));
        menu->addAction(a);
    }

    connect(mapper, SIGNAL(mappedInt(int)), this, SLOT(onKeyToggle(int)));
    connect(mapperMenu, SIGNAL(mappedInt(int)), this, SLOT(onScaleSet(int)));
}

FXKeyPicker::~FXKeyPicker()
{
    delete mapper;
    delete mapperMenu;
    delete ui;
}

void FXKeyPicker::setValue(int val) {
    for (int i = 11; i >= 0; i--) {
        int dec = pow(2, i);
        if (val >= dec) {
            val -= dec;
            noteCheckboxes[i]->setCheckState(Qt::CheckState::Checked);
        } else {
            noteCheckboxes[i]->setCheckState(Qt::CheckState::Unchecked);
        }
    }
}

void FXKeyPicker::resizeEvent(QResizeEvent *event) {
    ui->container->move((this->width() - ui->container->width()) / 2, 0);

    FXParam::resizeEvent(event);
}

void FXKeyPicker::onKeyToggle(int key) {
    std::string path = getPath();
    int newVal = fm->GetSetting(path), diff = pow(2, key);
    newVal = noteCheckboxes[key]->checkState() == Qt::CheckState::Checked ? newVal - diff : newVal + diff;
    fm->ApplyFXSettings({fx, {param, newVal}});
    fm->UpdateSettings(path, newVal);
}

void FXKeyPicker::onScaleSet(int scale) {
    std::string path = getPath();
    int newVal = scales[scale];
    fm->ApplyFXSettings({{fx, {{param, newVal}}}});
    fm->UpdateSettings(path, newVal);
    setValue(newVal);
}
