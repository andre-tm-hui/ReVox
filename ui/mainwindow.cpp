#include "mainwindow.h"

KeyboardListener* MainWindow::keyboardListener = new KeyboardListener();
MainInterface* KeyboardListener::mi = new MainInterface();
MainInterface* MainWindow::mi = KeyboardListener::mi;

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
        QMessageBox::critical(nullptr, "Error", "VB-Audio Cables not installed.");
        QApplication::quit();
        return;
    }
    if (devices->defaultAudioInput().description().contains("VB-Audio Virtual Cable") ||
            devices->defaultAudioOutput().description().contains("VB-Audio Virtual Cable"))
    {
        QMessageBox::critical(nullptr, "Error", "VB-Audio Cables set to default devices. Please change them.");
        QApplication::quit();
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
    trayIcon->setIcon(QIcon(":/rc/icon/icon.png"));
    trayIcon->show();

    this->setWindowIcon(QIcon(":/rc/icon/icon.png"));
    connect(trayIcon, &QSystemTrayIcon::activated, this, &MainWindow::iconActivated);

    keyPressEater = new KeyPressEater(this);
    connect(&mapper_menu, SIGNAL(mappedInt(int)), this, SLOT(openMenu(int)));

    hud = new HUD();
    hud->SetPosition(mi->GetSetting("hudPosition").get<int>());

    soundboardMenu = new SoundboardMenu(mi->GetSoundboardManager(), this);
    activeMenu = soundboardMenu;
    soundboardMenu->move(ui->sidebar->width(), 0);
    mapper_menu.setMapping(ui->soundboardButton, 0);
    connect(ui->soundboardButton, SIGNAL(clicked()), &mapper_menu, SLOT(map()));

    fxMenu = new FXMenu(mi->GetFXManager(), this);
    mi->GetFXManager()->SetHUD(hud);
    fxMenu->move(ui->sidebar->width(), 0);
    fxMenu->hide();
    mapper_menu.setMapping(ui->fxButton, 1);
    connect(ui->fxButton, SIGNAL(clicked()), &mapper_menu, SLOT(map()));

    settingsMenu = new SettingsMenu(std::shared_ptr<MainInterface>(mi), hud, this);

    settingsMenu->move(80, 31);
    settingsMenu->hide();
    connect(ui->settingsButton, SIGNAL(clicked()), &mapper_menu, SLOT(map()));
    mapper_menu.setMapping(ui->settingsButton, -1);
    settingsMenu->SetDevices(mi->GetOutputDevices(), mi->GetCurrentOutputDevice());

    if (mi->GetSetting("firstTime").get<bool>()) {
        restartTutorial();
        mi->UpdateSettings<bool>("firstTime", false);
    }

    titleBar = new TitleBar(this);
    titleBar->move(ui->sidebar->width(), 0);
    titleBar->raise();
    connect(titleBar, SIGNAL(windowMoved(QMouseEvent*,int,int)), this, SLOT(moveWindow(QMouseEvent*,int,int)));

    if (!mi->GetSetting("startWithWindows").get<bool>()) {
        this->show();
    } else {
        this->hide();
    }

    setup = true;
}

MainWindow::~MainWindow()
{
    delete ui;
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

void MainWindow::close()
{
    this->hide();
    soundboardMenu->reset();
    fxMenu->reset();
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
    mi->Reset(true);
    settingsMenu->SetDevices(mi->GetOutputDevices(), mi->GetCurrentOutputDevice());
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
        break;
    case fx:
        nextMenu = fxMenu;
        ui->soundboardButton->setEnabled(true);
        ui->fxButton->setEnabled(false);
        ui->settingsButton->setEnabled(true);
        //ui->voiceButton->setEnabled(true);
        break;
    case voice:
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
    nextMenu->show();
    fade(this, nextMenu, true, 200);
    activeMenu = nextMenu;
}

void MainWindow::resetAudio()
{
    mi->Reset();
    soundboardMenu->reset();
    fxMenu->reset();
}

void MainWindow::moveWindow(QMouseEvent *event, int x, int y)
{
    move(event->globalPosition().x() - x - ui->sidebar->width(),
         event->globalPosition().y() - y);
}

void MainWindow::restartTutorial() {
    openMenu(0);
    onboarding = new Onboarding(soundboardMenu, fxMenu, ui->fxButton, mi, QWidget::window());
    onboarding->raise();
    fade(this, onboarding, true, 300);
}
