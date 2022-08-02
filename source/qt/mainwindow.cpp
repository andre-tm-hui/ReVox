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

    d_data = new device_data;
    d_data->inputDevices = std::map<int, QString>();
    d_data->inputIdx = 0;
    d_data->outputDevices = std::map<int, QString>();
    d_data->outputIdx = 0;
    d_data->loopbackDevices = std::map<int, QString>();
    d_data->loopbackIdx = 0;
    d_data->vInputDevices = std::map<int, QString>();
    d_data->vInputIdx = 0;
    d_data->vOutputDevices = std::map<int, QString>();
    d_data->vOutputIdx = 0;

    int currentDevice = 0;
    for (auto const& [dName, i] : audioManager->deviceList)
    {
        if (dName.find("VB-Audio") != std::string::npos)
        {
            if (dName.find("Input") != std::string::npos)
            {
                d_data->vInputDevices[i] = QString::fromStdString(dName);

                if (dName.find(Pa_GetDeviceInfo(audioManager->virtualInputDevice)->name) != std::string::npos)
                {
                    d_data->vInputIdx = i;
                }
            }
            else if (dName.find("Output") != std::string::npos)
            {
                d_data->vOutputDevices[i] = QString::fromStdString(dName);
                if (dName.find(Pa_GetDeviceInfo(audioManager->virtualOutputDevice)->name) != std::string::npos)
                {
                    d_data->vOutputIdx = i;
                }
            }
        }
    }

    currentDevice = 0;
    for (auto const& [dName, i] : audioManager->inputDevices)
    {
        d_data->inputDevices[i.id] = QString::fromStdString(dName);
        if (dName.find(Pa_GetDeviceInfo(audioManager->inputDevice)->name) != std::string::npos)
        {
            d_data->inputIdx = i.id;
        }
        currentDevice++;
    }

    currentDevice = 0;
    for (auto const& [dName, i] : audioManager->outputDevices)
    {
        d_data->outputDevices[i.id] = QString::fromStdString(dName);
        if (dName.find(Pa_GetDeviceInfo(audioManager->outputDevice)->name) != std::string::npos)
        {
            d_data->outputIdx = i.id;
        }
        currentDevice++;
    }

    currentDevice = 0;
    for (auto const& [dName, i] : audioManager->loopbackDevices)
    {
        d_data->loopbackDevices[i.id] = QString::fromStdString(dName);
        if (dName.find(Pa_GetDeviceInfo(audioManager->loopbackDevice)->name) != std::string::npos)
        {
            d_data->loopbackIdx = i.id;
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

    checkboxes["reverb"] = ui->state;
    checkboxes["autotune"] = ui->state_2;
    audioManager->SetCheckboxes(&checkboxes);

    keyPressEater = new KeyPressEater(this);
    ui->keybindList->installEventFilter(keyPressEater);
    ui->fxToggleList->installEventFilter(keyPressEater);
    ui->addBind->installEventFilter(keyPressEater);
    ui->addBind_2->installEventFilter(keyPressEater);

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
    DeviceSettings popup (d_data, audioManager);
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
    std::cout<<state<<std::endl;
    audioManager->passthrough->data.useReverb = state == 0 ? false : true;
}

void MainWindow::toggleAutotune(int state)
{
    audioManager->passthrough->data.useAutotune = state == 0 ? false : true;
}
