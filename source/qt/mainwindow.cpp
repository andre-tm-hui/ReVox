#include "mainwindow.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Setup device monitor - checks for changes in connected devices and resets the software if changes occur
    devices = new QMediaDevices();
    bool vCablesFound = false;
    for (auto const& device : devices->audioInputs())
    {
        if (device.description().contains("VB-Audio Virtual Cable")) { vCablesFound = true; break; }
    }
    if (!vCablesFound)
    {
        ErrDialog *err = new ErrDialog(1);
        err->show();
        connect(err, SIGNAL(errQuit()), qApp, SLOT(quit()));
        return;
    }
    if (devices->defaultAudioInput().description().contains("VB-Audio Virtual Cable") ||
            devices->defaultAudioOutput().description().contains("VB-Audio Virtual Cable"))
    {
        ErrDialog *err = new ErrDialog(2);
        err->show();
        connect(err, SIGNAL(errQuit()), qApp, SLOT(quit()));
        return;
    }
    connect(devices, SIGNAL(audioInputsChanged()), this, SLOT(devicesChanged()));
    connect(devices, SIGNAL(audioOutputsChanged()), this, SLOT(devicesChanged()));

    qApp->setStyleSheet("QToolTip { color: #ffffff; background-color: #000000; border: 0px; }");

    setWindowFlags(Qt::FramelessWindowHint | Qt::Window);

    restoreAction = new QAction(tr("&Open"), this);
    connect(restoreAction, &QAction::triggered, this, &QWidget::showNormal);

    quitAction = new QAction(tr("&Quit"), this);
    connect(quitAction, &QAction::triggered, qApp, &QCoreApplication::quit);

    resetAction = new QAction(tr("&Reset Audio"), this);
    connect(resetAction, SIGNAL(triggered()), this, SLOT(resetAudio()));

    trayIconMenu = new QMenu(this);
    trayIconMenu->addAction(restoreAction);
    trayIconMenu->addAction(resetAction);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(quitAction);

    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setContextMenu(trayIconMenu);
    trayIcon->setIcon(QIcon(":/icons/icon.png"));

    if (audioManager->settings["startWithWindows"])
        this->hide();
    trayIcon->show();

    this->setWindowIcon(QIcon(":/icons/icon.png"));
    connect(trayIcon, &QSystemTrayIcon::activated, this, &MainWindow::iconActivated);

    keyPressEater = new KeyPressEater(this);
    connect(&mapper_menu, SIGNAL(mappedInt(int)), this, SLOT(openMenu(int)));

    hud = new HUD(&audioManager->passthrough->data);
    hud->SetPosition(audioManager->settings["hudPosition"]);
    audioManager->SetHUD(hud);

    soundboardMenu = new SoundboardMenu(&audioManager->soundboardHotkeys,
                                                        audioManager,
                                                        keyboardListener,
                                                        this);
    soundboardMenu->move(80, 31);
    activeMenu = soundboardMenu;
    connect(ui->soundboardButton, SIGNAL(clicked()), &mapper_menu, SLOT(map()));
    mapper_menu.setMapping(ui->soundboardButton, 0);

    fxMenu = new FXMenu(&audioManager->voiceFXHotkeys,
                                audioManager,
                                keyboardListener,
                                hud,
                                this);

    fxMenu->move(80, 31);
    fxMenu->setVisible(false);
    fxMenu->setEnabled(false);
    connect(ui->fxButton, SIGNAL(clicked()), &mapper_menu, SLOT(map()));
    mapper_menu.setMapping(ui->fxButton, 1);

    settingsMenu = new SettingsMenu(audioManager, hud, this);

    settingsMenu->move(80, 31);
    settingsMenu->setVisible(false);
    settingsMenu->setEnabled(false);
    connect(ui->settingsButton, SIGNAL(clicked()), &mapper_menu, SLOT(map()));
    mapper_menu.setMapping(ui->settingsButton, -1);
    settingsMenu->SetDevices(audioManager->outputDevices, audioManager->GetCurrentOutputDevice());

    audioManager->fxHotkey = &fxHotkey;
    audioManager->SetWaveform(soundboardMenu->GetWaveform());

    titleBar = new TitleBar(this);
    connect(titleBar, SIGNAL(windowMoved(QMouseEvent*,int,int)), this, SLOT(moveWindow(QMouseEvent*,int,int)));

    if (audioManager->settings["firstTime"]) {
        Onboarding *ob = new Onboarding(soundboardMenu, fxMenu, ui->fxButton, this);
        audioManager->settings["firstTime"] = false;
        audioManager->SaveSettings();
    }

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

bool MainWindow::event(QEvent *event)
{
    switch (event->type())
    {
    case QEvent::WindowStateChange:
        if (isMinimized())
        {
            this->hide();
            event->ignore();
            //audioManager->passthrough->SetFX(audioManager->GetFXOff());
            fxMenu->DisableTabWidget();
            soundboardMenu->DisableSettings();
        }
        break;
    case QEvent::Close:
        this->hide();
        event->ignore();
        //audioManager->passthrough->SetFX(audioManager->GetFXOff());
        fxMenu->DisableTabWidget();
        soundboardMenu->DisableSettings();
        break;
    default:
        return QMainWindow::event(event);
    }
    return true;
}

void MainWindow::close()
{
    this->hide();
    //event->ignore();
    //audioManager->passthrough->SetFX(audioManager->GetFXOff());
    fxMenu->DisableTabWidget();
    soundboardMenu->DisableSettings();
}

void MainWindow::setVisible(bool visible)
{
    restoreAction->setEnabled(!visible);
    QMainWindow::setVisible(visible);
    if (setup) fxMenu->OnHotkeyToggle(fxHotkey);
}

void MainWindow::iconActivated(QSystemTrayIcon::ActivationReason reason)
{
    switch (reason)
    {
    case QSystemTrayIcon::DoubleClick:
        this->audioManager->WaitForReady();
        this->show();
        this->setWindowState(Qt::WindowState::WindowActive);
        this->resize(960, 540);
        this->repaint();
        break;
    default:
        ;
    }
}

void MainWindow::devicesChanged()
{
    audioManager->Reset(true);
    settingsMenu->SetDevices(audioManager->outputDevices, audioManager->GetCurrentOutputDevice());
}

void MainWindow::openMenu(int menu)
{
    switch ((Menu)menu)
    {
    case settings:
        nextMenu = settingsMenu;
        ui->soundboardButton->setEnabled(true);
        ui->fxButton->setEnabled(true);
        ui->settingsButton->setEnabled(false);
        //ui->voiceButton->setEnabled(true);
        break;
    case soundboard:
        nextMenu = soundboardMenu;
        ui->soundboardButton->setEnabled(false);
        ui->fxButton->setEnabled(true);
        ui->settingsButton->setEnabled(true);
        //ui->voiceButton->setEnabled(true);
        //ui->soundboardMenu->show();
        break;
    case fx:
        //ui->fxMenu->show();
        nextMenu = fxMenu;
        ui->soundboardButton->setEnabled(true);
        ui->fxButton->setEnabled(false);
        ui->settingsButton->setEnabled(true);
        //ui->voiceButton->setEnabled(true);
        break;
    case voice:
        //ui->voiceMenu->show();
        ui->soundboardButton->setEnabled(false);
        ui->fxButton->setEnabled(true);
        ui->settingsButton->setEnabled(true);
        //ui->voiceButton->setEnabled(true);
        break;
    default:
        break;
    }
    QPropertyAnimation *a = fade(this, activeMenu, false, 200);

    connect(a, SIGNAL(finished()), this, SLOT(fadeInMenu()));
}

void MainWindow::fadeInMenu()
{
    nextMenu->setVisible(true);
    nextMenu->setEnabled(true);
    QPropertyAnimation *a = fade(this, nextMenu, true, 200);
    activeMenu->hide();
    activeMenu->setEnabled(false);
    activeMenu = nextMenu;
}

void MainWindow::resetAudio()
{
    audioManager->Reset();
}

void MainWindow::moveWindow(QMouseEvent *event, int x, int y)
{
    move(event->globalPosition().x() - x,
         event->globalPosition().y() - y);
}
