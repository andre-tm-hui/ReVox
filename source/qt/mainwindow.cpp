#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->actionDevice_Setup, SIGNAL(triggered()), SLOT(openDeviceSetup()));

    keybinds = ui->keybindList;

    restoreAction = new QAction(tr("&Open"), this);
    connect(restoreAction, &QAction::triggered, this, &QWidget::showNormal);

    quitAction = new QAction(tr("&Quit"), this);
    connect(quitAction, &QAction::triggered, qApp, &QCoreApplication::quit);

    trayIconMenu = new QMenu(this);
    trayIconMenu->addAction(restoreAction);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(quitAction);

    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setContextMenu(trayIconMenu);
    trayIcon->setIcon(QIcon(":/icons/icon.png"));

    this->hide();
    trayIcon->show();

    this->setWindowIcon(QIcon(":/icons/icon.png"));
    connect(trayIcon, &QSystemTrayIcon::activated, this, &MainWindow::iconActivated);

    d_data = new device_data();
    d_data->inputDevices = std::map<int, QString>();
    d_data->inputIdx = 0;
    d_data->outputDevices = std::map<int, QString>();
    d_data->outputIdx = 0;
    d_data->streamDevices = std::map<int, QString>();
    d_data->streamIdx = 0;

    int currentDevice = 0;
    for (auto const& [dName, i] : audioManager->inputDevices)
    {
        d_data->inputDevices[i.id] = QString::fromStdString(dName);
        if (dName.find(audioManager->settings["inputDevice"]) != std::string::npos)
        {
            d_data->inputIdx = i.id;
        }
        currentDevice++;
    }

    currentDevice = 0;
    for (auto const& [dName, i] : audioManager->outputDevices)
    {
        d_data->outputDevices[i.id] = QString::fromStdString(dName);
        if (dName.find(audioManager->settings["outputDevice"]) != std::string::npos)
        {
            d_data->outputIdx = i.id;
        }
        currentDevice++;
    }

    currentDevice = 0;
    for (auto const& [dName, i] : audioManager->loopbackDevices)
    {
        d_data->streamDevices[i.id] = QString::fromStdString(dName);
        if (dName.find(audioManager->settings["streamOutputDevice"]) != std::string::npos)
        {
            d_data->streamIdx = i.id;
        }

        currentDevice++;
    }

    for (auto& [keybind, settings] : audioManager->soundboardHotkeys.items())
    {
        QString qKeybind = vkCodenames[std::stoi(keybind)];
        addBind(0, std::stoi(keybind), QString::fromStdString(settings["label"].get<std::string>()));
    }

    for (auto& [keybind, settings] : audioManager->voiceFXHotkeys.items())
    {
        QString qKeybind = vkCodenames[std::stoi(keybind)];
        addBind(1, std::stoi(keybind), QString::fromStdString(settings["label"].get<std::string>()));
    }

    connect(&mapper_add, SIGNAL(mappedInt(int)), this, SLOT(addBind(int)));
    connect(ui->addBind, SIGNAL(clicked()), &mapper_add, SLOT(map()));
    mapper_add.setMapping(ui->addBind, 0);
    connect(ui->addBind_2, SIGNAL(clicked()), &mapper_add, SLOT(map()));
    mapper_add.setMapping(ui->addBind_2, 1);

    connect(&mapper_remove, SIGNAL(mappedInt(int)), this, SLOT(removeBind(int)));
    connect(ui->removeBind, SIGNAL(clicked()), &mapper_remove, SLOT(map()));
    mapper_remove.setMapping(ui->removeBind, 0);
    connect(ui->removeBind_2, SIGNAL(clicked()), &mapper_remove, SLOT(map()));
    mapper_remove.setMapping(ui->removeBind_2, 1);

    connect(ui->state, SIGNAL(stateChanged(int)), this, SLOT(toggleReverb(int)));
    connect(ui->state_2, SIGNAL(stateChanged(int)), this, SLOT(toggleAutotune(int)));
    connect(ui->state_4, SIGNAL(stateChanged(int)), this, SLOT(togglePitchshift(int)));

    connect(&mapper_fx, SIGNAL(mappedInt(int)), this, SLOT(openFXSettings(int)));
    connect(ui->settings, SIGNAL(clicked()), &mapper_fx, SLOT(map()));
    mapper_fx.setMapping(ui->settings, 0);
    connect(ui->settings_2, SIGNAL(clicked()), &mapper_fx, SLOT(map()));
    mapper_fx.setMapping(ui->settings_2, 1);
    connect(ui->settings_4, SIGNAL(clicked()), &mapper_fx, SLOT(map()));
    mapper_fx.setMapping(ui->settings_4, 2);


    checkboxes["reverb"] = ui->state;
    checkboxes["autotune"] = ui->state_2;
    checkboxes["pitchshift"] = ui->state_4;
    audioManager->SetCheckboxes(&checkboxes);

    keyPressEater = new KeyPressEater(this);
    ui->keybindList->installEventFilter(keyPressEater);
    ui->fxToggleList->installEventFilter(keyPressEater);
    ui->addBind->installEventFilter(keyPressEater);
    ui->addBind_2->installEventFilter(keyPressEater);

    ui->maxOverlappingSounds->setValue(audioManager->settings["maxNumberOfSounds"].get<int>());
    ui->maxPlaybackLength->setValue(audioManager->settings["maxFileLength"].get<int>());

    ui->horizontalSlider->setValue(audioManager->settings["monitorSamples"].get<int>());
    ui->horizontalSlider_2->setValue(audioManager->settings["monitorMic"].get<int>());

    devices = new QMediaDevices();
    connect(devices, SIGNAL(audioInputsChanged()), this, SLOT(devicesChanged()));
    connect(devices, SIGNAL(audioOutputsChanged()), this, SLOT(devicesChanged()));

    setup = true;
}

MainWindow::~MainWindow()
{
    delete ui;
}

KeyboardListener* MainWindow::keyboardListener = new KeyboardListener();
AudioManager* KeyboardListener::audioManager = new AudioManager();
AudioManager* MainWindow::audioManager = KeyboardListener::audioManager;
std::map<QString, int> MainWindow::keycodeMap = {};


void MainWindow::addBind(int type, int keybind, QString label)
{
    QListWidget *list = type == 0 ? ui->keybindList : ui->fxToggleList;
    //std::cout << "adding bind" << std::endl;
    //QListWidget *list = keybinds;
    HotkeyItem *item = new HotkeyItem(
                type == 0,
                label == "" ? "Bind " + QString::number(list->count() + 1) : label,
                list->width(),
                keybind,
                audioManager,
                keyboardListener
                );
    QListWidgetItem *orig = new QListWidgetItem();
    orig->setFlags(Qt::ItemIsEnabled | Qt::ItemIsEditable | Qt::ItemIsSelectable);
    list->addItem(orig);
    list->setItemWidget(orig, item);
    orig->setSizeHint(QSize(0, 32));

    list->scrollToBottom();
}

void MainWindow::removeBind(int type)
{
    QListWidget *list = type == 0 ? ui->keybindList : ui->fxToggleList;
    HotkeyItem *item = qobject_cast<HotkeyItem *>(list->itemWidget(list->currentItem()));
    audioManager->RemoveBind(item->cb.keycode);
    list->removeItemWidget(list->currentItem());
    delete list->currentItem();
}

void MainWindow::openDeviceSetup()
{
    DeviceSettings popup (d_data, audioManager, &checkboxes);
    popup.setModal(true);
    popup.exec();
}


bool MainWindow::event(QEvent *event)
{
    switch (event->type())
    {
    case QEvent::WindowStateChange:
        if (isMinimized())
        {
            this->hide();
            event->ignore();
        }
        break;
    case QEvent::Close:
        this->hide();
        event->ignore();
        break;
    default:
        return QMainWindow::event(event);
    }
    return true;
}

void MainWindow::setVisible(bool visible)
{
    restoreAction->setEnabled(!visible);
    QMainWindow::setVisible(visible);
}

void MainWindow::iconActivated(QSystemTrayIcon::ActivationReason reason)
{
    switch (reason)
    {
    case QSystemTrayIcon::DoubleClick:
        this->audioManager->WaitForReady();
        this->show();
        this->setWindowState(Qt::WindowState::WindowActive);
        break;
    default:
        ;
    }
}

void MainWindow::toggleReverb(int state)
{
    audioManager->passthrough->data.reverb->setEnabled(state == 0 ? false : true);
}

void MainWindow::toggleAutotune(int state)
{
    audioManager->passthrough->data.pitchShift->setAutotune(state == 0 ? false : true);
}

void MainWindow::togglePitchshift(int state)
{
    audioManager->passthrough->data.pitchShift->setPitchshift(state == 0 ? false : true);
}

void MainWindow::openFXSettings(int type)
{
    switch (type)
    {
    case 0: {// reverb
        ReverbSettings rv(audioManager->passthrough->data.reverb);
        rv.setModal(true);
        rv.exec();
        break;
    }
    case 1: {
        HardtuneSettings ht(audioManager->passthrough->data.pitchShift);
        ht.setModal(true);
        ht.exec();
        break;
    }
    case 2: {
        PitchSettings pt(audioManager->passthrough->data.pitchShift);
        pt.setModal(true);
        pt.exec();
        break;
    }
    default:
        break;
    }
}

void MainWindow::on_maxPlaybackLength_valueChanged(int arg1)
{
    audioManager->SetPlaybackLength(arg1);
}


void MainWindow::on_maxOverlappingSounds_valueChanged(int arg1)
{
    audioManager->SetNumberOfSounds(arg1);
}


void MainWindow::on_horizontalSlider_valueChanged(int value)
{
    audioManager->SetSampleMonitor(value);
}


void MainWindow::on_horizontalSlider_2_valueChanged(int value)
{
    audioManager->SetMicMonitor(value);
}

void MainWindow::devicesChanged()
{
    audioManager->Reset(-1, -1, -1);
    std::cout<<"device changed"<<std::endl;
}
