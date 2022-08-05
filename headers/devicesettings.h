#ifndef DEVICESETTINGS_H
#define DEVICESETTINGS_H

#include <QDialog>
#include <QComboBox>
#include <QSignalMapper>
#include <map>
#include <audiomanager.h>

typedef struct
{
    int inputIdx;
    std::map<int, QString> inputDevices;
    int outputIdx;
    std::map<int, QString> outputDevices;
    int streamIdx;
    std::map<int, QString> streamDevices;

    int sampleRate;
    int bufferSize;
} device_data;

enum deviceType {input, output, stream };

namespace Ui {
class DeviceSettings;
}

class DeviceSettings : public QDialog
{
    Q_OBJECT

public:
    explicit DeviceSettings(device_data *d_data, AudioManager *audioManager,  QWidget *parent = nullptr);
    ~DeviceSettings();

private slots:
    void onChange(int type);
    void apply();

private:
    void getDevice(deviceType type);
    void setDevices(deviceType type);

private:
    Ui::DeviceSettings *ui;
    device_data *d_data;
    AudioManager *audioManager;

    std::map<int, QString> *devices;
    int *idx, *t_idx, t_input, t_output, t_loopback, t_vInput, t_vOutput;
    QComboBox *qcb;

    deviceType selectedType;
    QSignalMapper mapper;
};

#endif // DEVICESETTINGS_H
