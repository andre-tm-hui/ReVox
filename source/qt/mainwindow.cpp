#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

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

    inputDevices = ui->inputDevices;
    outputDevices = ui->outputDevices;
    loopbackDevices = ui->loopbackDevices;
    virtualInputDevices = ui->virtualInputDevices;
    virtualOutputDevices = ui->virtualOutputDevices;

    keybinds = ui->tableWidget;

    int currentDevice = 0;
    for (auto const& [dName, i] : audioManager->deviceList)
    {
        if (dName.find("VB-Audio") != std::string::npos)
        {
            if (dName.find("Input") != std::string::npos)
            {
                virtualInputDevices->addItem(QString::fromStdString(dName));
                if (dName.find(Pa_GetDeviceInfo(audioManager->virtualInputDevice)->name) != std::string::npos)
                {
                    virtualInputDevices->setCurrentIndex(currentDevice);
                }
            }
            else if (dName.find("Output") != std::string::npos)
            {
                virtualOutputDevices->addItem(QString::fromStdString(dName));
                if (dName.find(Pa_GetDeviceInfo(audioManager->virtualOutputDevice)->name) != std::string::npos)
                {
                    fprintf(stdout, "%s %s\n", dName.c_str(), Pa_GetDeviceInfo(audioManager->virtualOutputDevice)->name); fflush(stdout);
                    virtualOutputDevices->setCurrentIndex(currentDevice);
                }
            }
        }
    }

    currentDevice = 0;
    for (auto const& [dName, i] : audioManager->inputDevices)
    {
        inputDevices->addItem(QString::fromStdString(dName));
        if (dName.find(Pa_GetDeviceInfo(audioManager->inputDevice)->name) != std::string::npos)
        {
            inputDevices->setCurrentIndex(currentDevice);
        }
        currentDevice++;
    }

    currentDevice = 0;
    for (auto const& [dName, i] : audioManager->outputDevices)
    {
        outputDevices->addItem(QString::fromStdString(dName));
        if (dName.find(Pa_GetDeviceInfo(audioManager->outputDevice)->name) != std::string::npos)
        {
            outputDevices->setCurrentIndex(currentDevice);
        }
        currentDevice++;
    }

    currentDevice = 0;
    for (auto const& [dName, i] : audioManager->loopbackDevices)
    {
        loopbackDevices->addItem(QString::fromStdString(dName));
        if (dName.find(Pa_GetDeviceInfo(audioManager->loopbackDevice)->name) != std::string::npos)
        {
            loopbackDevices->setCurrentIndex(currentDevice);
        }

        currentDevice++;
    }

    keybinds->setColumnWidth(0, 256);
    keybinds->setColumnWidth(1, 5);

    for (auto const& [keybind, settings] : audioManager->keybinds)
    {
        int i = keybinds->rowCount();
        keybinds->insertRow(i);
        QString qKeybind = GetKeyName(keybind);

        QTableWidgetItem* item = new QTableWidgetItem();
        item->setText(qKeybind);
        item->setFlags(item->flags() ^ Qt::ItemIsEditable);

        QTableWidgetItem* settingItem = new QTableWidgetItem();
        settingItem->setIcon(QIcon(":/icons/settings.png"));
        settingItem->setFlags(settingItem->flags() ^ Qt::ItemIsEditable);

        keybinds->setItem(i, 0, item);
        keybinds->setItem(i, 1, settingItem);
        keycodeMap[qKeybind] = keybind;
    }

    setup = true;
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_inputDevices_currentIndexChanged(int index)
{
    if (setup)
    {
        audioManager->inputDevice = audioManager->deviceList[inputDevices->currentText().toStdString()];
        //audioManager->ResetInputRecorder();
        audioManager->ResetPassthrough();
    }
}


void MainWindow::on_outputDevices_currentIndexChanged(int index)
{
    if (setup)
    {
        audioManager->outputDevice = audioManager->deviceList[outputDevices->currentText().toStdString()];
        audioManager->ResetMonitor();
    }
}


void MainWindow::on_loopbackDevices_currentIndexChanged(int index)
{
    if (setup)
    {
        audioManager->loopbackDevice = audioManager->loopbackDevices[loopbackDevices->currentText().toStdString()].id;
        audioManager->ResetLoopbackRecorder();
    }
}


void MainWindow::on_virtualInputDevices_currentIndexChanged(int index)
{
    if (setup)
    {
        audioManager->virtualInputDevice = audioManager->deviceList[virtualInputDevices->currentText().toStdString()];
        audioManager->ResetPlayer();
        audioManager->ResetPassthrough();
    }
}

void MainWindow::on_virtualOutputDevices_currentIndexChanged(int index)
{
    if (setup)
    {
        audioManager->virtualOutputDevice = audioManager->deviceList[virtualOutputDevices->currentText().toStdString()];
        audioManager->ResetInputRecorder();
    }
}

KeyboardListener* MainWindow::keyboardListener = new KeyboardListener();

AudioManager* KeyboardListener::audioManager = new AudioManager();
AudioManager* MainWindow::audioManager = KeyboardListener::audioManager;
std::map<QString, int> MainWindow::keycodeMap = {};



void MainWindow::on_tableWidget_cellDoubleClicked(int row, int column)
{
    QTableWidgetItem* item = keybinds->item(row, 0);
    int keycode = keycodeMap[item->text()];
    if (column == 0)
    {
        audioManager->Rebind(keycode);
        keyboardListener->rebinding = true;

        item->setText("Listening...");

        std::thread t (WaitForKeyboardInput, item);
        t.detach();
    }
    else if (column == 1)
    {
        KeybindSettings popup (&audioManager->keybinds[keycode], audioManager);
        popup.setModal(true);
        popup.exec();
    }
}

void MainWindow::WaitForKeyboardInput(QTableWidgetItem* item)
{
    std::unique_lock<std::mutex> lck(*keyboardListener->mtx);
    keyboardListener->cv->wait(lck, keyboardListener->ready);

    QString qKeybind = GetKeyName(keyboardListener->rebindTo);
    item->setText(qKeybind);
    audioManager->SetNewBind(keyboardListener->rebindTo);

    keycodeMap[item->text()] = keyboardListener->rebindTo;
}




void MainWindow::on_addBind_clicked()
{
    int i = keybinds->rowCount();
    keybinds->insertRow(i);

    QTableWidgetItem* item = new QTableWidgetItem();
    item->setFlags(item->flags() ^ Qt::ItemIsEditable);
    item->setText("Listening...");
    keybinds->setItem(i, 0, item);
    keyboardListener->rebinding = true;
    keybinds->scrollToBottom();

    QTableWidgetItem* settingItem = new QTableWidgetItem();
    settingItem->setIcon(QIcon(":/icons/settings.png"));
    settingItem->setFlags(item->flags() ^ Qt::ItemIsEditable);
    keybinds->setItem(i, 1, settingItem);

    std::thread t (WaitForKeyboardInput, item);
    t.detach();
}


void MainWindow::on_removeBind_clicked()
{
    int i = keybinds->currentRow();
    QTableWidgetItem* item = keybinds->item(i, 0);
    int keycode = keycodeMap[item->text()];
    keybinds->removeRow(i);
    audioManager->RemoveBind(keycode);
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
        this->show();
        this->setWindowState(Qt::WindowState::WindowActive);
        break;
    default:
        ;
    }
}

QString MainWindow::GetKeyName(int keybind)
{
    QString qKeybind;
    if (vkCodenames.find(keybind) != vkCodenames.end())
    {
        qKeybind = QString::fromStdString(vkCodenames[keybind]);
    }
    else
    {
        WCHAR keybindAsStr[1024];
        UINT scanCode = MapVirtualKeyW(keybind, MAPVK_VK_TO_VSC);
        LONG lParamValue = (scanCode << 16);
        GetKeyNameTextW(lParamValue, keybindAsStr, 1024);
        qKeybind = QString::fromWCharArray(keybindAsStr);
    }

    return qKeybind;
}


