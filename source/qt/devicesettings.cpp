#include "devicesettings.h"
#include "ui_devicesettings.h"

DeviceSettings::DeviceSettings(device_data *d_data, AudioManager *audioManager, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DeviceSettings)
{
    ui->setupUi(this);

    this->d_data = d_data;
    this->audioManager = audioManager;

    setDevices(input);
    setDevices(output);
    setDevices(loopback);
    connect(&mapper, SIGNAL(mappedInt(int)), this, SLOT(onChange(int)));
    connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(apply()));
}

DeviceSettings::~DeviceSettings()
{
    delete ui;
}

void DeviceSettings::getDevice(deviceType type)
{
    switch (type)
    {
    case input:
        devices = &d_data->inputDevices;
        idx = &d_data->inputIdx;
        t_idx = &t_input;
        qcb = ui->input;
        break;
    case output:
        devices = &d_data->outputDevices;
        idx = &d_data->outputIdx;
        t_idx = &t_output;
        qcb = ui->output;
        break;
    case loopback:
        devices = &d_data->loopbackDevices;
        idx = &d_data->loopbackIdx;
        t_idx = &t_loopback;
        qcb = ui->loopback;
        break;
    default:
        return;
    }
}

void DeviceSettings::setDevices(deviceType type)
{
    getDevice(type);

    if (devices == nullptr) return;
    int i = 0;
    for (auto const& [id, dName] : *devices)
    {
        qcb->addItem(dName);
        if (id == *idx)
        {
            qcb->setCurrentIndex(i);
            *t_idx = id;
        }
        i++;
    }
    connect(qcb, SIGNAL(currentIndexChanged(int)), &mapper, SLOT(map()));
    mapper.setMapping(qcb, (int)type);
}

void DeviceSettings::onChange(int type)
{
    getDevice((deviceType)type);

    if (devices == nullptr) return;

    for (auto const& [id, dName] : *devices)
    {
        if (QString::compare(qcb->currentText(), dName) == 0)
        {
            *t_idx = id;
            break;

        }
    }
}

void DeviceSettings::apply()
{
    d_data->inputIdx = t_input;
    d_data->outputIdx = t_output;
    d_data->loopbackIdx = t_loopback;

    audioManager->Reset(d_data->inputIdx, d_data->outputIdx, d_data->loopbackIdx);
}

