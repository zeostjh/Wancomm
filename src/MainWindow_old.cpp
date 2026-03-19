#include "MainWindow.h"
#include <QApplication>
#include <QScreen>
#include <QMessageBox>

namespace VoiceChat {

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , isConnected(false)
    , isMuted(false)
    , isServerMode(true)
{
    setupUI();
    applyDarkTheme();
    populateAudioDevices();
    
    // Setup statistics update timer
    statsTimer = new QTimer(this);
    connect(statsTimer, &QTimer::timeout, this, &MainWindow::updateStatistics);
    statsTimer->start(100); // Update every 100ms
    
    setWindowTitle("VoiceChat - Real-time Voice Communication");
    resize(500, 600);
    
    // Center window on screen
    QScreen *screen = QApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();
    int x = (screenGeometry.width() - width()) / 2;
    int y = (screenGeometry.height() - height()) / 2;
    move(x, y);
}

MainWindow::~MainWindow() {
    if (client && isConnected) {
        client->stop();
    }
}

void MainWindow::setupUI() {
    centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setSpacing(20);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    
    // Connection Group
    QGroupBox *connectionGroup = new QGroupBox("Connection", centralWidget);
    QVBoxLayout *connectionLayout = new QVBoxLayout(connectionGroup);
    connectionLayout->setSpacing(12);
    
    // Mode selection
    QHBoxLayout *modeLayout = new QHBoxLayout();
    QLabel *modeLabel = new QLabel("Mode:", connectionGroup);
    modeComboBox = new QComboBox(connectionGroup);
    modeComboBox->addItem("Server");
    modeComboBox->addItem("Client");
    connect(modeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onModeChanged);
    modeLayout->addWidget(modeLabel);
    modeLayout->addWidget(modeComboBox, 1);
    connectionLayout->addLayout(modeLayout);
    
    // Server Bind IP (for server mode)
    QHBoxLayout *bindLayout = new QHBoxLayout();
    QLabel *bindLabel = new QLabel("Bind IP:", connectionGroup);
    serverBindEdit = new QLineEdit("0.0.0.0", connectionGroup);
    serverBindEdit->setPlaceholderText("0.0.0.0 for all interfaces");
    bindLayout->addWidget(bindLabel);
    bindLayout->addWidget(serverBindEdit, 1);
    connectionLayout->addLayout(bindLayout);
    
    // IP Address (for client mode)
    QHBoxLayout *ipLayout = new QHBoxLayout();
    QLabel *ipLabel = new QLabel("Server IP:", connectionGroup);
    ipAddressEdit = new QLineEdit("127.0.0.1", connectionGroup);
    ipAddressEdit->setEnabled(false); // Disabled in server mode by default
    ipLayout->addWidget(ipLabel);
    ipLayout->addWidget(ipAddressEdit, 1);
    connectionLayout->addLayout(ipLayout);
    
    // Port
    QHBoxLayout *portLayout = new QHBoxLayout();
    QLabel *portLabel = new QLabel("Port:", connectionGroup);
    portEdit = new QLineEdit("5000", connectionGroup);
    portLayout->addWidget(portLabel);
    portLayout->addWidget(portEdit, 1);
    connectionLayout->addLayout(portLayout);
    
    // Connect button
    connectButton = new QPushButton("Start Server", connectionGroup);
    connectButton->setMinimumHeight(40);
    connect(connectButton, &QPushButton::clicked, this, &MainWindow::onConnectClicked);
    connectionLayout->addWidget(connectButton);
    
    // Status
    statusLabel = new QLabel("Status: Disconnected", connectionGroup);
    statusLabel->setAlignment(Qt::AlignCenter);
    connectionLayout->addWidget(statusLabel);
    
    mainLayout->addWidget(connectionGroup);
    
    // Audio Device Group
    QGroupBox *deviceGroup = new QGroupBox("Audio Devices", centralWidget);
    QVBoxLayout *deviceLayout = new QVBoxLayout(deviceGroup);
    deviceLayout->setSpacing(12);
    
    // Input device selection
    QHBoxLayout *inputDevLayout = new QHBoxLayout();
    QLabel *inputDevLabel = new QLabel("Input Device:", deviceGroup);
    inputDeviceCombo = new QComboBox(deviceGroup);
    inputDevLayout->addWidget(inputDevLabel);
    inputDevLayout->addWidget(inputDeviceCombo, 1);
    deviceLayout->addLayout(inputDevLayout);
    
    // Output device selection
    QHBoxLayout *outputDevLayout = new QHBoxLayout();
    QLabel *outputDevLabel = new QLabel("Output Device:", deviceGroup);
    outputDeviceCombo = new QComboBox(deviceGroup);
    outputDevLayout->addWidget(outputDevLabel);
    outputDevLayout->addWidget(outputDeviceCombo, 1);
    deviceLayout->addLayout(outputDevLayout);
    
    mainLayout->addWidget(deviceGroup);
    
    // Channel Group
    QGroupBox *channelGroup = new QGroupBox("Intercom Channels", centralWidget);
    QVBoxLayout *channelLayout = new QVBoxLayout(channelGroup);
    channelLayout->setSpacing(12);
    
    // Channel selection
    QHBoxLayout *channelSelectLayout = new QHBoxLayout();
    QLabel *channelLabel = new QLabel("Active Channel:", channelGroup);
    channelComboBox = new QComboBox(channelGroup);
    
    // Initialize default channel names
    channelNames[0] = "Channel 1";
    channelNames[1] = "Channel 2";
    channelNames[2] = "Channel 3";
    channelNames[3] = "Channel 4";
    
    for (int i = 0; i < 4; i++) {
        channelComboBox->addItem(channelNames[i]);
    }
    
    connect(channelComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onChannelChanged);
    channelSelectLayout->addWidget(channelLabel);
    channelSelectLayout->addWidget(channelComboBox, 1);
    channelLayout->addLayout(channelSelectLayout);
    
    // Channel naming
    QLabel *nameLabel = new QLabel("Channel Names:", channelGroup);
    channelLayout->addWidget(nameLabel);
    
    for (int i = 0; i < 4; i++) {
        QHBoxLayout *nameLayout = new QHBoxLayout();
        QLabel *chNumLabel = new QLabel(QString("Ch %1:").arg(i + 1), channelGroup);
        chNumLabel->setMinimumWidth(50);
        channelNameEdits[i] = new QLineEdit(channelNames[i], channelGroup);
        channelNameEdits[i]->setPlaceholderText(QString("Channel %1 Name").arg(i + 1));
        
        // Update combo box when name changes
        connect(channelNameEdits[i], &QLineEdit::textChanged, [this, i](const QString &text) {
            channelNames[i] = text.isEmpty() ? QString("Channel %1").arg(i + 1) : text;
            channelComboBox->setItemText(i, channelNames[i]);
        });
        
        nameLayout->addWidget(chNumLabel);
        nameLayout->addWidget(channelNameEdits[i], 1);
        channelLayout->addLayout(nameLayout);
    }
    
    mainLayout->addWidget(channelGroup);
    
    // Audio Controls Group
    QGroupBox *audioGroup = new QGroupBox("Audio Controls", centralWidget);
    QVBoxLayout *audioLayout = new QVBoxLayout(audioGroup);
    audioLayout->setSpacing(12);
    
    // Mute button
    muteButton = new QPushButton("🎤 Unmuted", audioGroup);
    muteButton->setMinimumHeight(50);
    muteButton->setEnabled(false);
    connect(muteButton, &QPushButton::clicked, this, &MainWindow::onMuteClicked);
    audioLayout->addWidget(muteButton);
    
    // Input level
    QLabel *inputLabel = new QLabel("Input Level:", audioGroup);
    inputLevelBar = new QProgressBar(audioGroup);
    inputLevelBar->setTextVisible(false);
    inputLevelBar->setMaximum(100);
    inputLevelBar->setValue(0);
    audioLayout->addWidget(inputLabel);
    audioLayout->addWidget(inputLevelBar);
    
    // Output level
    QLabel *outputLabel = new QLabel("Output Level:", audioGroup);
    outputLevelBar = new QProgressBar(audioGroup);
    outputLevelBar->setTextVisible(false);
    outputLevelBar->setMaximum(100);
    outputLevelBar->setValue(0);
    audioLayout->addWidget(outputLabel);
    audioLayout->addWidget(outputLevelBar);
    
    mainLayout->addWidget(audioGroup);
    
    // Statistics Group
    QGroupBox *statsGroup = new QGroupBox("Statistics", centralWidget);
    QVBoxLayout *statsLayout = new QVBoxLayout(statsGroup);
    
    statsLabel = new QLabel("Not connected", statsGroup);
    statsLabel->setWordWrap(true);
    statsLayout->addWidget(statsLabel);
    
    mainLayout->addWidget(statsGroup);
    
    mainLayout->addStretch();
}

void MainWindow::applyDarkTheme() {
    QString darkStyle = R"(
        QMainWindow {
            background-color: #1e1e1e;
        }
        
        QWidget {
            background-color: #1e1e1e;
            color: #e0e0e0;
            font-size: 13px;
            font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Arial, sans-serif;
        }
        
        QGroupBox {
            background-color: #2d2d2d;
            border: 2px solid #3d3d3d;
            border-radius: 8px;
            margin-top: 12px;
            padding-top: 10px;
            font-weight: bold;
            color: #e0e0e0;
        }
        
        QGroupBox::title {
            subcontrol-origin: margin;
            subcontrol-position: top left;
            left: 10px;
            padding: 0 5px;
            color: #0d9488;
        }
        
        QPushButton {
            background-color: #0d9488;
            color: white;
            border: none;
            border-radius: 6px;
            padding: 8px 16px;
            font-weight: bold;
            font-size: 14px;
        }
        
        QPushButton:hover {
            background-color: #0f766e;
        }
        
        QPushButton:pressed {
            background-color: #115e59;
        }
        
        QPushButton:disabled {
            background-color: #3d3d3d;
            color: #666666;
        }
        
        QLineEdit {
            background-color: #2d2d2d;
            border: 2px solid #3d3d3d;
            border-radius: 6px;
            padding: 8px;
            color: #e0e0e0;
            selection-background-color: #0d9488;
        }
        
        QLineEdit:focus {
            border: 2px solid #0d9488;
        }
        
        QLineEdit:disabled {
            background-color: #252525;
            color: #666666;
        }
        
        QComboBox {
            background-color: #2d2d2d;
            border: 2px solid #3d3d3d;
            border-radius: 6px;
            padding: 8px;
            color: #e0e0e0;
        }
        
        QComboBox:hover {
            border: 2px solid #0d9488;
        }
        
        QComboBox::drop-down {
            border: none;
            padding-right: 8px;
        }
        
        QComboBox::down-arrow {
            image: none;
            border-left: 5px solid transparent;
            border-right: 5px solid transparent;
            border-top: 5px solid #e0e0e0;
            margin-right: 5px;
        }
        
        QComboBox QAbstractItemView {
            background-color: #2d2d2d;
            border: 2px solid #0d9488;
            border-radius: 6px;
            selection-background-color: #0d9488;
            color: #e0e0e0;
            padding: 4px;
        }
        
        QLabel {
            background-color: transparent;
            color: #e0e0e0;
        }
        
        QProgressBar {
            background-color: #2d2d2d;
            border: 2px solid #3d3d3d;
            border-radius: 6px;
            height: 20px;
        }
        
        QProgressBar::chunk {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                                      stop:0 #0d9488, stop:1 #14b8a6);
            border-radius: 4px;
        }
    )";
    
    setStyleSheet(darkStyle);
}

void MainWindow::onModeChanged(int index) {
    isServerMode = (index == 0);
    
    if (isServerMode) {
        ipAddressEdit->setEnabled(false);
        serverBindEdit->setEnabled(true);
        connectButton->setText("Start Server");
        statusLabel->setText("Status: Ready to start server");
    } else {
        ipAddressEdit->setEnabled(true);
        serverBindEdit->setEnabled(false);
        connectButton->setText("Connect");
        statusLabel->setText("Status: Ready to connect");
    }
}

void MainWindow::onConnectClicked() {
    if (!isConnected) {
        if (isServerMode) {
            startServer();
        } else {
            connectToServer();
        }
    } else {
        if (isServerMode) {
            stopServer();
        } else {
            disconnectFromServer();
        }
    }
}

void MainWindow::startServer() {
    int port = portEdit->text().toInt();
    if (port <= 0 || port > 65535) {
        QMessageBox::warning(this, "Invalid Port", "Please enter a valid port number (1-65535)");
        return;
    }
    
    QString bindAddress = serverBindEdit->text().trimmed();
    if (bindAddress.isEmpty()) {
        bindAddress = "0.0.0.0";
    }
    
    VoiceChatConfig config;
    config.isServer = true;
    config.serverPort = port;
    config.serverBindAddress = bindAddress.toStdString();
    config.inputDeviceIndex = inputDeviceCombo->currentData().toInt();
    config.outputDeviceIndex = outputDeviceCombo->currentData().toInt();
    config.channelNumber = channelComboBox->currentIndex();
    
    client = std::make_unique<VoiceChatClient>();
    
    if (!client->initialize(config)) {
        QMessageBox::critical(this, "Error", "Failed to initialize voice chat server");
        client.reset();
        return;
    }
    
    if (!client->start()) {
        QMessageBox::critical(this, "Error", "Failed to start voice chat server");
        client.reset();
        return;
    }
    
    isConnected = true;
    connectButton->setText("Stop Server");
    muteButton->setEnabled(true);
    modeComboBox->setEnabled(false);
    portEdit->setEnabled(false);
    serverBindEdit->setEnabled(false);
    inputDeviceCombo->setEnabled(false);
    outputDeviceCombo->setEnabled(false);
    channelComboBox->setEnabled(false);
    QString channelName = channelNames[config.channelNumber];
    statusLabel->setText(QString("Status: ✅ Server on port %1, %2").arg(port).arg(channelName));
    statusLabel->setStyleSheet("color: #10b981; font-weight: bold;");
}

void MainWindow::stopServer() {
    if (client) {
        client->stop();
        client.reset();
    }
    
    isConnected = false;
    connectButton->setText("Start Server");
    muteButton->setEnabled(false);
    muteButton->setText("🎤 Unmuted");
    isMuted = false;
    modeComboBox->setEnabled(true);
    portEdit->setEnabled(true);
    serverBindEdit->setEnabled(true);
    inputDeviceCombo->setEnabled(true);
    outputDeviceCombo->setEnabled(true);
    channelComboBox->setEnabled(true);
    statusLabel->setText("Status: Server stopped");
    statusLabel->setStyleSheet("color: #e0e0e0;");
    inputLevelBar->setValue(0);
    outputLevelBar->setValue(0);
    statsLabel->setText("Not connected");
}

void MainWindow::connectToServer() {
    QString ip = ipAddressEdit->text();
    int port = portEdit->text().toInt();
    
    if (ip.isEmpty()) {
        QMessageBox::warning(this, "Invalid IP", "Please enter a server IP address");
        return;
    }
    
    if (port <= 0 || port > 65535) {
        QMessageBox::warning(this, "Invalid Port", "Please enter a valid port number (1-65535)");
        return;
    }
    
    VoiceChatConfig config;
    config.isServer = false;
    config.serverAddress = ip.toStdString();
    config.serverPort = port;
    config.inputDeviceIndex = inputDeviceCombo->currentData().toInt();
    config.outputDeviceIndex = outputDeviceCombo->currentData().toInt();
    config.channelNumber = channelComboBox->currentIndex();
    
    client = std::make_unique<VoiceChatClient>();
    
    if (!client->initialize(config)) {
        QMessageBox::critical(this, "Error", "Failed to initialize voice chat client");
        client.reset();
        return;
    }
    
    if (!client->start()) {
        QMessageBox::critical(this, "Error", "Failed to connect to server");
        client.reset();
        return;
    }
    
    isConnected = true;
    connectButton->setText("Disconnect");
    muteButton->setEnabled(true);
    modeComboBox->setEnabled(false);
    ipAddressEdit->setEnabled(false);
    portEdit->setEnabled(false);
    inputDeviceCombo->setEnabled(false);
    outputDeviceCombo->setEnabled(false);
    channelComboBox->setEnabled(false);
    QString channelName = channelNames[config.channelNumber];
    statusLabel->setText(QString("Status: ✅ Connected to %1:%2, %3").arg(ip).arg(port).arg(channelName));
    statusLabel->setStyleSheet("color: #10b981; font-weight: bold;");
}

void MainWindow::disconnectFromServer() {
    if (client) {
        client->stop();
        client.reset();
    }
    
    isConnected = false;
    connectButton->setText("Connect");
    muteButton->setEnabled(false);
    muteButton->setText("🎤 Unmuted");
    isMuted = false;
    modeComboBox->setEnabled(true);
    ipAddressEdit->setEnabled(true);
    portEdit->setEnabled(true);
    inputDeviceCombo->setEnabled(true);
    outputDeviceCombo->setEnabled(true);
    channelComboBox->setEnabled(true);
    statusLabel->setText("Status: Disconnected");
    statusLabel->setStyleSheet("color: #e0e0e0;");
    inputLevelBar->setValue(0);
    outputLevelBar->setValue(0);
    statsLabel->setText("Not connected");
}

void MainWindow::onMuteClicked() {
    isMuted = !isMuted;
    
    if (isMuted) {
        muteButton->setText("🔇 Muted");
        muteButton->setStyleSheet(R"(
            QPushButton {
                background-color: #dc2626;
            }
            QPushButton:hover {
                background-color: #b91c1c;
            }
            QPushButton:pressed {
                background-color: #991b1b;
            }
        )");
    } else {
        muteButton->setText("🎤 Unmuted");
        muteButton->setStyleSheet("");
    }
    
    // TODO: Implement actual mute functionality in VoiceChatClient
}

void MainWindow::updateStatistics() {
    if (!isConnected || !client) {
        return;
    }
    
    // Simulate audio levels (you'll need to get these from your actual audio system)
    static int inputLevel = 0;
    static int outputLevel = 0;
    static int direction = 1;
    
    if (!isMuted) {
        inputLevel += direction * 5;
        if (inputLevel >= 100 || inputLevel <= 0) {
            direction *= -1;
        }
        outputLevel = (inputLevel + 20) % 100;
    } else {
        inputLevel = 0;
        outputLevel = 0;
    }
    
    inputLevelBar->setValue(inputLevel);
    outputLevelBar->setValue(outputLevel);
    
    // Update statistics
    // TODO: Get actual statistics from VoiceChatClient
    QString stats;
    if (isServerMode) {
        stats = "Server Mode\n";
        stats += "Clients connected: 0\n";
    } else {
        stats = "Client Mode\n";
    }
    stats += QString("Input level: %1%\n").arg(inputLevel);
    stats += QString("Output level: %1%\n").arg(outputLevel);
    stats += QString("Muted: %1").arg(isMuted ? "Yes" : "No");
    
    statsLabel->setText(stats);
}

void MainWindow::updateConnectionStatus() {
    // TODO: Check actual connection status
}

void MainWindow::populateAudioDevices() {
    // Get input devices
    auto inputDevices = AudioCapture::getInputDevices();
    inputDeviceCombo->clear();
    inputDeviceCombo->addItem("Default Input Device", -1);
    
    for (const auto& device : inputDevices) {
        QString deviceName = QString::fromStdString(device.name);
        if (device.isDefaultInput) {
            deviceName += " (Default)";
        }
        inputDeviceCombo->addItem(deviceName, device.index);
    }
    
    // Get output devices
    auto outputDevices = AudioPlayback::getOutputDevices();
    outputDeviceCombo->clear();
    outputDeviceCombo->addItem("Default Output Device", -1);
    
    for (const auto& device : outputDevices) {
        QString deviceName = QString::fromStdString(device.name);
        if (device.isDefaultOutput) {
            deviceName += " (Default)";
        }
        outputDeviceCombo->addItem(deviceName, device.index);
    }
}

void MainWindow::onChannelChanged(int index) {
    // Update the current channel
    // If connected, would need to reconnect with new channel
    if (isConnected) {
        statusLabel->setText(QString("Status: ✅ Active on %1").arg(channelNames[index]));
    }
}

} // namespace VoiceChat
