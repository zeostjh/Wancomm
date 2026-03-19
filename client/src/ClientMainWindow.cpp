#include "ClientMainWindow.h"
#include <QMessageBox>
#include <QApplication>
#include <QScreen>
#include <QGridLayout>
#include <QFormLayout>
#include <QPalette>
#include <QScrollArea>

namespace VoiceChat {

// LoginDialog Implementation
LoginDialog::LoginDialog(QWidget *parent) : QDialog(parent) {
    setWindowTitle("Login to VoiceChat");
    setModal(true);
    resize(400, 250);
    
    QVBoxLayout *layout = new QVBoxLayout(this);
    
    QFormLayout *formLayout = new QFormLayout();
    
    usernameEdit_ = new QLineEdit();
    formLayout->addRow("Username:", usernameEdit_);
    
    passwordEdit_ = new QLineEdit();
    passwordEdit_->setEchoMode(QLineEdit::Password);
    formLayout->addRow("Password:", passwordEdit_);
    
    serverAddressEdit_ = new QLineEdit("localhost");
    formLayout->addRow("Server Address:", serverAddressEdit_);
    
    portSpinBox_ = new QSpinBox();
    portSpinBox_->setRange(1024, 65535);
    portSpinBox_->setValue(5000);
    formLayout->addRow("Port:", portSpinBox_);
    
    layout->addLayout(formLayout);
    
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    QPushButton *loginButton = new QPushButton("Login");
    QPushButton *cancelButton = new QPushButton("Cancel");
    buttonLayout->addStretch();
    buttonLayout->addWidget(loginButton);
    buttonLayout->addWidget(cancelButton);
    layout->addLayout(buttonLayout);
    
    connect(loginButton, &QPushButton::clicked, this, &QDialog::accept);
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);
}

QString LoginDialog::getUsername() const {
    return usernameEdit_->text();
}

QString LoginDialog::getPassword() const {
    return passwordEdit_->text();
}

QString LoginDialog::getServerAddress() const {
    return serverAddressEdit_->text();
}

int LoginDialog::getPort() const {
    return portSpinBox_->value();
}

// ChannelControl Implementation
ChannelControl::ChannelControl(int channelNumber, QWidget *parent)
    : QWidget(parent), channelNumber_(channelNumber), talkActive_(false) {
    
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(5, 5, 5, 5);
    
    QLabel *channelLabel = new QLabel(QString("Channel %1").arg(channelNumber + 1));
    channelLabel->setAlignment(Qt::AlignCenter);
    QFont font = channelLabel->font();
    font.setBold(true);
    channelLabel->setFont(font);
    layout->addWidget(channelLabel);
    
    talkButton_ = new QPushButton("TALK");
    talkButton_->setMinimumHeight(60);
    talkButton_->setCheckable(false);
    talkButton_->setStyleSheet(
        "QPushButton { background-color: #4CAF50; color: white; font-size: 14px; font-weight: bold; }"
        "QPushButton:pressed { background-color: #45a049; }"
    );
    layout->addWidget(talkButton_);
    
    listenCheckBox_ = new QCheckBox("Listen");
    listenCheckBox_->setChecked(true);
    layout->addWidget(listenCheckBox_);
    
    statusLabel_ = new QLabel("Idle");
    statusLabel_->setAlignment(Qt::AlignCenter);
    statusLabel_->setStyleSheet("QLabel { color: gray; font-size: 10px; }");
    layout->addWidget(statusLabel_);
    
    connect(talkButton_, &QPushButton::pressed, this, &ChannelControl::onTalkPressed);
    connect(talkButton_, &QPushButton::released, this, &ChannelControl::onTalkReleased);
    connect(listenCheckBox_, &QCheckBox::toggled, this, &ChannelControl::onListenToggled);
}

bool ChannelControl::isTalkEnabled() const {
    return talkActive_;
}

bool ChannelControl::isListenEnabled() const {
    return listenCheckBox_->isChecked();
}

void ChannelControl::onTalkPressed() {
    talkActive_ = true;
    talkButton_->setStyleSheet(
        "QPushButton { background-color: #ff4444; color: white; font-size: 14px; font-weight: bold; }"
    );
    statusLabel_->setText("Talking");
    statusLabel_->setStyleSheet("QLabel { color: red; font-size: 10px; font-weight: bold; }");
    emit talkStateChanged(channelNumber_, true);
}

void ChannelControl::onTalkReleased() {
    talkActive_ = false;
    talkButton_->setStyleSheet(
        "QPushButton { background-color: #4CAF50; color: white; font-size: 14px; font-weight: bold; }"
        "QPushButton:pressed { background-color: #45a049; }"
    );
    statusLabel_->setText(listenCheckBox_->isChecked() ? "Listening" : "Idle");
    statusLabel_->setStyleSheet("QLabel { color: gray; font-size: 10px; }");
    emit talkStateChanged(channelNumber_, false);
}

void ChannelControl::onListenToggled(bool checked) {
    if (!talkActive_) {
        statusLabel_->setText(checked ? "Listening" : "Idle");
    }
    emit listenStateChanged(channelNumber_, checked);
}

// ClientMainWindow Implementation
ClientMainWindow::ClientMainWindow(QWidget *parent)
    : QMainWindow(parent), connected_(false) {
    
    setupUI();
    
    meterUpdateTimer_ = new QTimer(this);
    connect(meterUpdateTimer_, &QTimer::timeout, this, &ClientMainWindow::updateMeters);
    meterUpdateTimer_->start(50); // Update meters at 20Hz
    
    statusUpdateTimer_ = new QTimer(this);
    connect(statusUpdateTimer_, &QTimer::timeout, this, &ClientMainWindow::updateConnectionStatus);
    statusUpdateTimer_->start(1000); // Update status once per second
    
    showLoginDialog();
}

ClientMainWindow::~ClientMainWindow() {
    if (connected_ && client_) {
        client_->stop();
    }
}

void ClientMainWindow::setupUI() {
    setWindowTitle("VoiceChat Client");
    resize(800, 700);
    
    // Center window on screen
    const QRect screenGeometry = QApplication::primaryScreen()->geometry();
    int x = (screenGeometry.width() - width()) / 2;
    int y = (screenGeometry.height() - height()) / 2;
    move(x, y);
    
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    
    // Connection Status Section
    QGroupBox *statusGroup = new QGroupBox("Connection Status");
    QVBoxLayout *statusLayout = new QVBoxLayout(statusGroup);
    
    QHBoxLayout *statusInfoLayout = new QHBoxLayout();
    connectionStatusLabel_ = new QLabel("Not Connected");
    connectionStatusLabel_->setStyleSheet("QLabel { font-weight: bold; color: red; }");
    statusInfoLayout->addWidget(new QLabel("Status:"));
    statusInfoLayout->addWidget(connectionStatusLabel_);
    statusInfoLayout->addStretch();
    statusLayout->addLayout(statusInfoLayout);
    
    QHBoxLayout *userInfoLayout = new QHBoxLayout();
    usernameLabel_ = new QLabel("-");
    serverLabel_ = new QLabel("-");
    userInfoLayout->addWidget(new QLabel("User:"));
    userInfoLayout->addWidget(usernameLabel_);
    userInfoLayout->addWidget(new QLabel("Server:"));
    userInfoLayout->addWidget(serverLabel_);
    userInfoLayout->addStretch();
    statusLayout->addLayout(userInfoLayout);
    
    QHBoxLayout *connectionButtonLayout = new QHBoxLayout();
    connectButton_ = new QPushButton("Connect");
    disconnectButton_ = new QPushButton("Disconnect");
    disconnectButton_->setEnabled(false);
    settingsButton_ = new QPushButton("Settings");
    connectionButtonLayout->addWidget(connectButton_);
    connectionButtonLayout->addWidget(disconnectButton_);
    connectionButtonLayout->addWidget(settingsButton_);
    connectionButtonLayout->addStretch();
    statusLayout->addLayout(connectionButtonLayout);
    
    mainLayout->addWidget(statusGroup);
    
    // Channel Controls Section
    QGroupBox *channelsGroup = new QGroupBox("Channels");
    QHBoxLayout *channelsLayout = new QHBoxLayout(channelsGroup);
    
    for (int i = 0; i < 4; i++) {
        auto channelControl = std::make_unique<ChannelControl>(i);
        connect(channelControl.get(), &ChannelControl::talkStateChanged,
                this, &ClientMainWindow::onChannelTalkStateChanged);
        connect(channelControl.get(), &ChannelControl::listenStateChanged,
                this, &ClientMainWindow::onChannelListenStateChanged);
        channelsLayout->addWidget(channelControl.get());
        channelControls_.push_back(std::move(channelControl));
    }
    
    mainLayout->addWidget(channelsGroup);
    
    // Audio Level Meters Section
    QGroupBox *metersGroup = new QGroupBox("Audio Levels");
    QFormLayout *metersLayout = new QFormLayout(metersGroup);
    
    inputLevelMeter_ = new QProgressBar();
    inputLevelMeter_->setRange(0, 100);
    inputLevelMeter_->setValue(0);
    inputLevelMeter_->setTextVisible(false);
    inputLevelMeter_->setStyleSheet(
        "QProgressBar { border: 1px solid gray; border-radius: 3px; }"
        "QProgressBar::chunk { background-color: #4CAF50; }"
    );
    metersLayout->addRow("Input Level:", inputLevelMeter_);
    
    outputLevelMeter_ = new QProgressBar();
    outputLevelMeter_->setRange(0, 100);
    outputLevelMeter_->setValue(0);
    outputLevelMeter_->setTextVisible(false);
    outputLevelMeter_->setStyleSheet(
        "QProgressBar { border: 1px solid gray; border-radius: 3px; }"
        "QProgressBar::chunk { background-color: #2196F3; }"
    );
    metersLayout->addRow("Output Level:", outputLevelMeter_);
    
    mainLayout->addWidget(metersGroup);
    
    // Audio Device Selection Section
    QGroupBox *deviceGroup = new QGroupBox("Audio Devices");
    QFormLayout *deviceLayout = new QFormLayout(deviceGroup);
    
    inputDeviceCombo_ = new QComboBox();
    deviceLayout->addRow("Input Device:", inputDeviceCombo_);
    
    outputDeviceCombo_ = new QComboBox();
    deviceLayout->addRow("Output Device:", outputDeviceCombo_);
    
    mainLayout->addWidget(deviceGroup);
    
    mainLayout->addStretch();
    
    // Connect button signals
    connect(connectButton_, &QPushButton::clicked, this, &ClientMainWindow::onConnectClicked);
    connect(disconnectButton_, &QPushButton::clicked, this, &ClientMainWindow::onDisconnectClicked);
    connect(settingsButton_, &QPushButton::clicked, this, &ClientMainWindow::onSettingsClicked);
    
    // Load audio devices
    updateDeviceList();
}

void ClientMainWindow::showLoginDialog() {
    LoginDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        username_ = dialog.getUsername();
        password_ = dialog.getPassword();
        serverAddress_ = dialog.getServerAddress();
        serverPort_ = dialog.getPort();
        
        usernameLabel_->setText(username_);
        serverLabel_->setText(QString("%1:%2").arg(serverAddress_).arg(serverPort_));
    } else {
        QApplication::quit();
    }
}

void ClientMainWindow::updateDeviceList() {
    inputDeviceCombo_->clear();
    outputDeviceCombo_->clear();
    
    inputDevices_ = AudioDevice::getInputDevices();
    outputDevices_ = AudioDevice::getOutputDevices();
    
    for (const auto& device : inputDevices_) {
        inputDeviceCombo_->addItem(QString::fromStdString(device.name));
    }
    
    for (const auto& device : outputDevices_) {
        outputDeviceCombo_->addItem(QString::fromStdString(device.name));
    }
}

void ClientMainWindow::onConnectClicked() {
    if (connected_) {
        return;
    }
    
    connectToServer();
}

void ClientMainWindow::onDisconnectClicked() {
    if (!connected_) {
        return;
    }
    
    disconnectFromServer();
}

void ClientMainWindow::onSettingsClicked() {
    showLoginDialog();
}

void ClientMainWindow::connectToServer() {
    client_ = std::make_unique<VoiceChatClient>();
    
    VoiceChatConfig config;
    config.serverAddress = serverAddress_.toStdString();
    config.serverPort = serverPort_;
    config.username = username_.toStdString();
    config.password = password_.toStdString();
    config.isServer = false;
    
    // Get selected devices
    int inputIndex = inputDeviceCombo_->currentIndex();
    int outputIndex = outputDeviceCombo_->currentIndex();
    if (inputIndex >= 0 && inputIndex < (int)inputDevices_.size()) {
        config.inputDeviceIndex = inputDevices_[inputIndex].index;
    }
    if (outputIndex >= 0 && outputIndex < (int)outputDevices_.size()) {
        config.outputDeviceIndex = outputDevices_[outputIndex].index;
    }
    
    if (client_->initialize(config)) {
        if (client_->start()) {
            connected_ = true;
            connectButton_->setEnabled(false);
            disconnectButton_->setEnabled(true);
            connectionStatusLabel_->setText("Connected");
            connectionStatusLabel_->setStyleSheet("QLabel { font-weight: bold; color: green; }");
            
            // Enable channel controls
            for (auto& control : channelControls_) {
                control->setEnabled(true);
            }
            
            QMessageBox::information(this, "Connected", "Successfully connected to server");
        } else {
            QMessageBox::critical(this, "Connection Failed", "Failed to start voice chat client");
            client_.reset();
        }
    } else {
        QMessageBox::critical(this, "Connection Failed", 
            "Failed to initialize connection to server. Check your credentials and network settings.");
        client_.reset();
    }
}

void ClientMainWindow::disconnectFromServer() {
    if (client_) {
        client_->stop();
        client_.reset();
    }
    
    connected_ = false;
    connectButton_->setEnabled(true);
    disconnectButton_->setEnabled(false);
    connectionStatusLabel_->setText("Not Connected");
    connectionStatusLabel_->setStyleSheet("QLabel { font-weight: bold; color: red; }");
    
    // Disable channel controls
    for (auto& control : channelControls_) {
        control->setEnabled(false);
    }
    
    inputLevelMeter_->setValue(0);
    outputLevelMeter_->setValue(0);
}

void ClientMainWindow::onChannelTalkStateChanged(int channel, bool enabled) {
    if (!connected_ || !client_) {
        return;
    }
    
    client_->setTalking(channel, enabled);
}

void ClientMainWindow::onChannelListenStateChanged(int channel, bool enabled) {
    if (!connected_ || !client_) {
        return;
    }
    
    client_->setListening(channel, enabled);
}

void ClientMainWindow::updateMeters() {
    if (!connected_ || !client_) {
        inputLevelMeter_->setValue(0);
        outputLevelMeter_->setValue(0);
        return;
    }
    
    auto levels = client_->getAudioLevels();
    inputLevelMeter_->setValue(static_cast<int>(levels.inputLevel * 100));
    outputLevelMeter_->setValue(static_cast<int>(levels.outputLevel * 100));
}

void ClientMainWindow::updateConnectionStatus() {
    if (!connected_ || !client_) {
        return;
    }
    
    if (!client_->isActive()) {
        QMessageBox::warning(this, "Connection Lost", "Connection to server was lost");
        disconnectFromServer();
    }
}

} // namespace VoiceChat
