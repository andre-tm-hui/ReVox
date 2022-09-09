#include "devicesettings.h"
#include "ui_devicesettings.h"

DeviceSettings::DeviceSettings(device_data *d_data, AudioManager *audioManager, std::map<std::string, QCheckBox*> *checkboxes, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DeviceSettings)
{
    ui->setupUi(this);

    this->d_data = d_data;
    this->audioManager = audioManager;
    this->checkboxes = checkboxes;

    setDevices(input);
    setDevices(output);
    setDevices(stream);
    connect(&mapper, SIGNAL(mappedInt(int)), this, SLOT(onChange(int)));
    connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(apply()));
}

DeviceSettings::~DeviceSettings()
{
    audioManager->SetCheckboxes(this->checkboxes);
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
    case stream:
        devices = &d_data->streamDevices;
        idx = &d_data->streamIdx;
        t_idx = &t_streamOutput;
        qcb = ui->stream;
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
        std::cout<<dName.toStdString()<<std::endl;
        if (dName.contains(qcb->currentText()) && !dName.contains(QString("[Loopback]")))
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
    d_data->streamIdx = t_streamOutput;

    audioManager->Reset(d_data->inputIdx, d_data->outputIdx, d_data->streamIdx);
}

