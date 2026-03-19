#include "MainWindow.h"
#include <QApplication>
#include <QScreen>
#include <QMessageBox>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QFont>
#include <QGraphicsDropShadowEffect>
#include <QRandomGenerator>

namespace VoiceChat {

// ============ Login Dialog ============

LoginDialog::LoginDialog(QWidget *parent) : QDialog(parent) {
    setWindowTitle("VoiceChat - Login");
    setModal(true);
    setFixedSize(400, 300);
    
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    
    QLabel *titleLabel = new QLabel("VoiceChat Intercom", this);
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(18);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(titleLabel);
    
    QFormLayout *formLayout = new QFormLayout();
    formLayout->setSpacing(12);
    
    // Username
    usernameEdit = new QLineEdit(this);
    usernameEdit->setPlaceholderText("Enter your name");
    usernameEdit->setMaxLength(20);
    formLayout->addRow("Username:", usernameEdit);
    
    // Mode selection
    modeComboBox = new QComboBox(this);
    modeComboBox->addItem("🖥️  Server Mode");
    modeComboBox->addItem("👤  Client Mode");
    modeComboBox->setCurrentIndex(1); // Default to client
    formLayout->addRow("Mode:", modeComboBox);
    
    // Server address (for client mode)
    serverAddressEdit = new QLineEdit("127.0.0.1", this);
    serverAddressEdit->setPlaceholderText("Server IP address");
    formLayout->addRow("Server IP:", serverAddressEdit);
    
    // Port
    portEdit = new QLineEdit("5000", this);
    portEdit->setPlaceholderText("5000");
    formLayout->addRow("Port:", portEdit);
    
    mainLayout->addLayout(formLayout);
    mainLayout->addStretch();
    
    // Buttons
    QDialogButtonBox *buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    mainLayout->addWidget(buttonBox);
    
    // Update server address enable state
    connect(modeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            [this](int index) {
        serverAddressEdit->setEnabled(index == 1); // Enable for client mode
    });
    
    QString dialogStyle = R"(
        QDialog {
            background-color: #1a1a1a;
        }
        QLabel {
            color: #e0e0e0;
        }
        QLineEdit, QComboBox {
            background-color: #2d2d2d;
            border: 2px solid #3d3d3d;
            border-radius: 6px;
            padding: 8px;
            color: #e0e0e0;
            min-height: 25px;
        }
        QLineEdit:focus, QComboBox:focus {
            border: 2px solid #ff6b35;
        }
        QPushButton {
            background-color: #ff6b35;
            color: white;
            border: none;
            border-radius: 6px;
            padding: 10px 20px;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: #ff8555;
        }
    )";
    setStyleSheet(dialogStyle);
}

QString LoginDialog::getUsername() const {
    QString name = usernameEdit->text().trimmed();
    return name.isEmpty() ? "User" : name;
}

QString LoginDialog::getServerAddress() const {
    return serverAddressEdit->text().trimmed();
}

int LoginDialog::getPort() const {
    bool ok;
    int port = portEdit->text().toInt(&ok);
    return (ok && port > 0 && port <= 65535) ? port : 5000;
}

bool LoginDialog::isServerMode() const {
    return modeComboBox->currentIndex() == 0;
}

// ============ Main Window ============

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , isConnected(false)
    , isTalking(false)
    , isListening(true)
    , isServerMode(false)
{
    setupUI();
    applyDarkTheme();
    populateAudioDevices();
    
    // Setup statistics update timer
    statsTimer = new QTimer(this);
    connect(statsTimer, &QTimer::timeout, this, &MainWindow::updateStatistics);
    statsTimer->start(100); // Update every 100ms
    
    // Setup users list update timer
    usersTimer = new QTimer(this);
    connect(usersTimer, &QTimer::timeout, this, &MainWindow::updateConnectedUsers);
    usersTimer->start(1000); // Update every second
    
    setWindowTitle("VoiceChat - Professional Intercom System");
    setFixedSize(480, 720);
    
    // Center window on screen
    QScreen *screen = QApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();
    int x = (screenGeometry.width() - width()) / 2;
    int y = (screenGeometry.height() - height()) / 2;
    move(x, y);
    
    // Show login dialog immediately
    showLoginDialog();
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
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    
    // ========== HEADER SECTION ==========
    QWidget *headerWidget = new QWidget(centralWidget);
    headerWidget->setObjectName("headerWidget");
    headerWidget->setFixedHeight(80);
    QVBoxLayout *headerLayout = new QVBoxLayout(headerWidget);
    headerLayout->setContentsMargins(20, 15, 20, 15);
    headerLayout->setSpacing(5);
    
    QLabel *titleLabel = new QLabel("CLEARCOM INTERCOM", headerWidget);
    titleLabel->setObjectName("titleLabel");
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(16);
    titleFont.setBold(true);
    titleFont.setLetterSpacing(QFont::AbsoluteSpacing, 2);
    titleLabel->setFont(titleFont);
    titleLabel->setAlignment(Qt::AlignCenter);
    headerLayout->addWidget(titleLabel);
    
    usernameLabel = new QLabel("Not Connected", headerWidget);
    usernameLabel->setObjectName("usernameLabel");
    usernameLabel->setAlignment(Qt::AlignCenter);
    headerLayout->addWidget(usernameLabel);
    
    mainLayout->addWidget(headerWidget);
    
    // ========== MAIN CONTROL PANEL ==========
    QWidget *controlPanel = new QWidget(centralWidget);
    controlPanel->setObjectName("controlPanel");
    QVBoxLayout *controlLayout = new QVBoxLayout(controlPanel);
    controlLayout->setSpacing(15);
    controlLayout->setContentsMargins(20, 20, 20, 20);
    
    // Channel Selector
    QLabel *channelLabel = new QLabel("CHANNEL", controlPanel);
    channelLabel->setObjectName("sectionLabel");
    controlLayout->addWidget(channelLabel);
    
    channelComboBox = new QComboBox(controlPanel);
    channelComboBox->setObjectName("channelCombo");
    channelComboBox->setFixedHeight(50);
    channelComboBox->addItem("🔴  CHANNEL 1 - PRODUCTION");
    channelComboBox->addItem("🟢  CHANNEL 2 - STAGE");
    channelComboBox->addItem("🔵  CHANNEL 3 - TECH");
    channelComboBox->addItem("🟡  CHANNEL 4 - DIRECTOR");
    connect(channelComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onChannelChanged);
    controlLayout->addWidget(channelComboBox);
    
    controlLayout->addSpacing(20);
    
    // ========== TALK/LISTEN CONTROLS ==========
    QLabel *controlsLabel = new QLabel("CONTROLS", controlPanel);
    controlsLabel->setObjectName("sectionLabel");
    controlLayout->addWidget(controlsLabel);
    
    // Talk Button with Indicator
    QHBoxLayout *talkLayout = new QHBoxLayout();
    talkLayout->setSpacing(15);
    
    talkIndicator = new QLabel(controlPanel);
    talkIndicator->setObjectName("indicator");
    talkIndicator->setFixedSize(20, 20);
    talkIndicator->setProperty("active", false);
    talkLayout->addWidget(talkIndicator);
    
    talkButton = new QPushButton("TALK", controlPanel);
    talkButton->setObjectName("talkButton");
    talkButton->setFixedHeight(80);
    talkButton->setEnabled(false);
    talkButton->setCheckable(false);
    connect(talkButton, &QPushButton::pressed, this, &MainWindow::onTalkPressed);
    connect(talkButton, &QPushButton::released, this, &MainWindow::onTalkReleased);
    talkLayout->addWidget(talkButton, 1);
    
    controlLayout->addLayout(talkLayout);
    
    // Listen Button with Indicator
    QHBoxLayout *listenLayout = new QHBoxLayout();
    listenLayout->setSpacing(15);
    
    listenIndicator = new QLabel(controlPanel);
    listenIndicator->setObjectName("indicator");
    listenIndicator->setFixedSize(20, 20);
    listenIndicator->setProperty("active", true);
    listenLayout->addWidget(listenIndicator);
    
    listenButton = new QPushButton("LISTEN: ON", controlPanel);
    listenButton->setObjectName("listenButton");
    listenButton->setFixedHeight(70);
    listenButton->setEnabled(false);
    listenButton->setCheckable(true);
    listenButton->setChecked(true);
    connect(listenButton, &QPushButton::clicked, this, &MainWindow::onListenToggled);
    listenLayout->addWidget(listenButton, 1);
    
    controlLayout->addLayout(listenLayout);
    
    controlLayout->addSpacing(20);
    
    // ========== AUDIO LEVELS ==========
    QLabel *levelsLabel = new QLabel("AUDIO LEVELS", controlPanel);
    levelsLabel->setObjectName("sectionLabel");
    controlLayout->addWidget(levelsLabel);
    
    QHBoxLayout *inputLevelLayout = new QHBoxLayout();
    QLabel *inputLabel = new QLabel("TX", controlPanel);
    inputLabel->setFixedWidth(30);
    inputLabel->setObjectName("meterLabel");
    inputLevelBar = new QProgressBar(controlPanel);
    inputLevelBar->setObjectName("txMeter");
    inputLevelBar->setTextVisible(false);
    inputLevelBar->setMaximum(100);
    inputLevelBar->setValue(0);
    inputLevelBar->setFixedHeight(25);
    inputLevelLayout->addWidget(inputLabel);
    inputLevelLayout->addWidget(inputLevelBar);
    controlLayout->addLayout(inputLevelLayout);
    
    QHBoxLayout *outputLevelLayout = new QHBoxLayout();
    QLabel *outputLabel = new QLabel("RX", controlPanel);
    outputLabel->setFixedWidth(30);
    outputLabel->setObjectName("meterLabel");
    outputLevelBar = new QProgressBar(controlPanel);
    outputLevelBar->setObjectName("rxMeter");
    outputLevelBar->setTextVisible(false);
    outputLevelBar->setMaximum(100);
    outputLevelBar->setValue(0);
    outputLevelBar->setFixedHeight(25);
    outputLevelLayout->addWidget(outputLabel);
    outputLevelLayout->addWidget(outputLevelBar);
    controlLayout->addLayout(outputLevelLayout);
    
    controlLayout->addSpacing(15);
    
    // ========== CONNECTED USERS ==========
    QLabel *usersLabel = new QLabel("CONNECTED USERS", controlPanel);
    usersLabel->setObjectName("sectionLabel");
    controlLayout->addWidget(usersLabel);
    
    usersCountLabel = new QLabel("0 users online", controlPanel);
    usersCountLabel->setObjectName("usersCount");
    controlLayout->addWidget(usersCountLabel);
    
    usersList = new QListWidget(controlPanel);
    usersList->setObjectName("usersList");
    usersList->setFixedHeight(120);
    controlLayout->addWidget(usersList);
    
    controlLayout->addStretch();
    
    // ========== CONNECTION BUTTON ==========
    connectButton = new QPushButton("CONNECT TO NETWORK", controlPanel);
    connectButton->setObjectName("connectButton");
    connectButton->setFixedHeight(50);
    connect(connectButton, &QPushButton::clicked, this, &MainWindow::onConnectClicked);
    controlLayout->addWidget(connectButton);
    
    mainLayout->addWidget(controlPanel);
    
    // ========== FOOTER STATUS ==========
    QWidget *footerWidget = new QWidget(centralWidget);
    footerWidget->setObjectName("footerWidget");
    footerWidget->setFixedHeight(60);
    QVBoxLayout *footerLayout = new QVBoxLayout(footerWidget);
    footerLayout->setContentsMargins(15, 10, 15, 10);
    footerLayout->setSpacing(3);
    
    connectionLabel = new QLabel("● OFFLINE", footerWidget);
    connectionLabel->setObjectName("statusLabel");
    footerLayout->addWidget(connectionLabel);
    
    statsLabel = new QLabel("Ready", footerWidget);
    statsLabel->setObjectName("statsLabel");
    statsLabel->setWordWrap(true);
    footerLayout->addWidget(statsLabel);
    
    mainLayout->addWidget(footerWidget);
}

void MainWindow::applyDarkTheme() {
    QString professionalStyle = R"(
        QMainWindow {
            background-color: #0a0a0a;
        }
        
        QWidget#headerWidget {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                      stop:0 #1a1a1a, stop:1 #0f0f0f);
            border-bottom: 2px solid #ff6b35;
        }
        
        QLabel#titleLabel {
            color: #ff6b35;
            font-weight: bold;
        }
        
        QLabel#usernameLabel {
            color: #a0a0a0;
            font-size: 12px;
        }
        
        QWidget#controlPanel {
            background-color: #121212;
        }
        
        QLabel#sectionLabel {
            color: #ff6b35;
            font-weight: bold;
            font-size: 11px;
            letter-spacing: 1px;
            padding: 5px 0px;
        }
        
        QComboBox#channelCombo {
            background-color: #1a1a1a;
            border: 2px solid #2a2a2a;
            border-radius: 8px;
            padding: 12px;
            color: #e0e0e0;
            font-size: 14px;
            font-weight: bold;
        }
        
        QComboBox#channelCombo:hover {
            border: 2px solid #ff6b35;
        }
        
        QComboBox#channelCombo::drop-down {
            border: none;
            padding-right: 10px;
        }
        
        QComboBox#channelCombo::down-arrow {
            image: none;
            border-left: 6px solid transparent;
            border-right: 6px solid transparent;
            border-top: 6px solid #ff6b35;
            margin-right: 8px;
        }
        
        QComboBox#channelCombo QAbstractItemView {
            background-color: #1a1a1a;
            border: 2px solid #ff6b35;
            selection-background-color: #ff6b35;
            color: #e0e0e0;
            outline: none;
        }
        
        QPushButton#talkButton {
            background-color: #8b0000;
            color: white;
            border: 3px solid #4a0000;
            border-radius: 12px;
            font-size: 24px;
            font-weight: bold;
            letter-spacing: 3px;
        }
        
        QPushButton#talkButton:hover:!pressed {
            background-color: #a00000;
            border-color: #ff0000;
        }
        
        QPushButton#talkButton:pressed {
            background-color: #ff0000;
            border-color: #ff3333;
            color: #ffffff;
        }
        
        QPushButton#talkButton:disabled {
            background-color: #2a2a2a;
            border-color: #1a1a1a;
            color: #555555;
        }
        
        QPushButton#listenButton {
            background-color: #004d00;
            color: white;
            border: 3px solid #003300;
            border-radius: 12px;
            font-size: 18px;
            font-weight: bold;
            letter-spacing: 2px;
        }
        
        QPushButton#listenButton:checked {
            background-color: #00cc00;
            border-color: #00ff00;
        }
        
        QPushButton#listenButton:hover {
            background-color: #006600;
        }
        
        QPushButton#listenButton:disabled {
            background-color: #2a2a2a;
            border-color: #1a1a1a;
            color: #555555;
        }
        
        QLabel#indicator {
            background-color: #2a2a2a;
            border: 2px solid #1a1a1a;
            border-radius: 10px;
        }
        
        QLabel#indicator[active="true"] {
            background-color: #00ff00;
            border-color: #00cc00;
        }
        
        QLabel#meterLabel {
            color: #999999;
            font-weight: bold;
            font-size: 11px;
        }
        
        QProgressBar#txMeter {
            background-color: #1a1a1a;
            border: 2px solid #2a2a2a;
            border-radius: 6px;
        }
        
        QProgressBar#txMeter::chunk {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                                      stop:0 #00ff00, stop:0.7 #ffff00, stop:1 #ff0000);
            border-radius: 4px;
        }
        
        QProgressBar#rxMeter {
            background-color: #1a1a1a;
            border: 2px solid #2a2a2a;
            border-radius: 6px;
        }
        
        QProgressBar#rxMeter::chunk {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                                      stop:0 #0099ff, stop:1 #00ffff);
            border-radius: 4px;
        }
        
        QLabel#usersCount {
            color: #999999;
            font-size: 11px;
            padding: 3px 0px;
        }
        
        QListWidget#usersList {
            background-color: #0a0a0a;
            border: 2px solid #2a2a2a;
            border-radius: 6px;
            color: #e0e0e0;
            padding: 5px;
            font-size: 12px;
        }
        
        QListWidget#usersList::item {
            padding: 6px;
            border-bottom: 1px solid #1a1a1a;
        }
        
        QListWidget#usersList::item:hover {
            background-color: #1a1a1a;
        }
        
        QPushButton#connectButton {
            background-color: #ff6b35;
            color: white;
            border: none;
            border-radius: 10px;
            font-size: 14px;
            font-weight: bold;
            letter-spacing: 1px;
        }
        
        QPushButton#connectButton:hover {
            background-color: #ff8555;
        }
        
        QPushButton#connectButton:pressed {
            background-color: #ff5515;
        }
        
        QWidget#footerWidget {
            background-color: #0a0a0a;
            border-top: 1px solid #2a2a2a;
        }
        
        QLabel#statusLabel {
            color: #666666;
            font-size: 11px;
            font-weight: bold;
        }
        
        QLabel#statusLabel[connected="true"] {
            color: #00ff00;
        }
        
        QLabel#statsLabel {
            color: #666666;
            font-size: 10px;
        }
    )";
    
    setStyleSheet(professionalStyle);
}

void MainWindow::showLoginDialog() {
    LoginDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        myUsername = dialog.getUsername();
        serverAddress = dialog.getServerAddress();
        serverPort = dialog.getPort();
        isServerMode = dialog.isServerMode();
        
        usernameLabel->setText(myUsername + (isServerMode ? " (Server)" : " (Client)"));
        
        if (isServerMode) {
            connectButton->setText("START SERVER");
        } else {
            connectButton->setText("CONNECT TO SERVER");
        }
    } else {
        // User cancelled, close application
        QApplication::quit();
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
    VoiceChatConfig config;
    config.isServer = true;
    config.serverPort = serverPort;
    config.serverBindAddress = "0.0.0.0";
    config.inputDeviceIndex = inputDeviceCombo->currentData().toInt();
    config.outputDeviceIndex = outputDeviceCombo->currentData().toInt();
    config.channelNumber = channelComboBox->currentIndex();
    config.username = myUsername.toStdString();
    
    client = std::make_unique<VoiceChatClient>();
    
    if (!client->initialize(config)) {
        QMessageBox::critical(this, "Error", "Failed to initialize server");
        client.reset();
        return;
    }
    
    if (!client->start()) {
        QMessageBox::critical(this, "Error", "Failed to start server");
        client.reset();
        return;
    }
    
    isConnected = true;
    connectButton->setText("STOP SERVER");
    connectionLabel->setText("● ONLINE (SERVER)");
    connectionLabel->setProperty("connected", true);
    connectionLabel->style()->unpolish(connectionLabel);
    connectionLabel->style()->polish(connectionLabel);
    
    talkButton->setEnabled(true);
    listenButton->setEnabled(true);
    
    updateConnectionStatus();
}

void MainWindow::stopServer() {
    if (client) {
        client->stop();
        client.reset();
    }
    
    isConnected = false;
    isTalking = false;
    isListening = true;
    
    connectButton->setText("START SERVER");
    connectionLabel->setText("● OFFLINE");
    connectionLabel->setProperty("connected", false);
    connectionLabel->style()->unpolish(connectionLabel);
    connectionLabel->style()->polish(connectionLabel);
    
    talkButton->setEnabled(false);
    listenButton->setEnabled(false);
    listenButton->setChecked(true);
    listenButton->setText("LISTEN: ON");
    
    talkIndicator->setProperty("active", false);
    talkIndicator->style()->unpolish(talkIndicator);
    talkIndicator->style()->polish(talkIndicator);
    
    usersList->clear();
    usersCountLabel->setText("0 users online");
}

void MainWindow::connectToServer() {
    VoiceChatConfig config;
    config.isServer = false;
    config.serverAddress = serverAddress.toStdString();
    config.serverPort = serverPort;
    config.inputDeviceIndex = inputDeviceCombo->currentData().toInt();
    config.outputDeviceIndex = outputDeviceCombo->currentData().toInt();
    config.channelNumber = channelComboBox->currentIndex();
    config.username = myUsername.toStdString();
    
    client = std::make_unique<VoiceChatClient>();
    
    if (!client->initialize(config)) {
        QMessageBox::critical(this, "Error", "Failed to initialize client");
        client.reset();
        return;
    }
    
    if (!client->start()) {
        QMessageBox::critical(this, "Error", "Failed to connect to server");
        client.reset();
        return;
    }
    
    isConnected = true;
    connectButton->setText("DISCONNECT");
    connectionLabel->setText("● ONLINE");
    connectionLabel->setProperty("connected", true);
    connectionLabel->style()->unpolish(connectionLabel);
    connectionLabel->style()->polish(connectionLabel);
    
    talkButton->setEnabled(true);
    listenButton->setEnabled(true);
    
    updateConnectionStatus();
}

void MainWindow::disconnectFromServer() {
    if (client) {
        client->stop();
        client.reset();
    }
    
    isConnected = false;
    isTalking = false;
    isListening = true;
    
    connectButton->setText("CONNECT TO SERVER");
    connectionLabel->setText("● OFFLINE");
    connectionLabel->setProperty("connected", false);
    connectionLabel->style()->unpolish(connectionLabel);
    connectionLabel->style()->polish(connectionLabel);
    
    talkButton->setEnabled(false);
    listenButton->setEnabled(false);
    listenButton->setChecked(true);
    listenButton->setText("LISTEN: ON");
    
    talkIndicator->setProperty("active", false);
    talkIndicator->style()->unpolish(talkIndicator);
    talkIndicator->style()->polish(talkIndicator);
    
    usersList->clear();
    usersCountLabel->setText("0 users online");
}

void MainWindow::onTalkPressed() {
    if (!isConnected || !client) return;
    
    isTalking = true;
    client->setTalking(true);
    
    talkIndicator->setProperty("active", true);
    talkIndicator->style()->unpolish(talkIndicator);
    talkIndicator->style()->polish(talkIndicator);
}

void MainWindow::onTalkReleased() {
    if (!client) return;
    
    isTalking = false;
    client->setTalking(false);
    
    talkIndicator->setProperty("active", false);
    talkIndicator->style()->unpolish(talkIndicator);
    talkIndicator->style()->polish(talkIndicator);
}

void MainWindow::onListenToggled() {
    if (!isConnected || !client) return;
    
    isListening = listenButton->isChecked();
    client->setListening(isListening);
    
    listenButton->setText(isListening ? "LISTEN: ON" : "LISTEN: OFF");
    
    listenIndicator->setProperty("active", isListening);
    listenIndicator->style()->unpolish(listenIndicator);
    listenIndicator->style()->polish(listenIndicator);
}

void MainWindow::onChannelChanged(int index) {
    if (client && isConnected) {
        // Would need to update client channel - requires reconnection
        QMessageBox::information(this, "Channel Change", 
            "Please disconnect and reconnect to change channels.");
    }
}

void MainWindow::updateStatistics() {
    if (!client || !isConnected) {
        inputLevelBar->setValue(0);
        outputLevelBar->setValue(0);
        statsLabel->setText("Not connected");
        return;
    }
    
    auto stats = client->getStats();
    
    // Simulate audio levels (in real implementation, get from audio capture/playback)
    if (isTalking) {
        inputLevelBar->setValue(QRandomGenerator::global()->bounded(40) + 60); // 60-100 when talking
    } else {
        inputLevelBar->setValue(QRandomGenerator::global()->bounded(20)); // 0-20 when not talking
    }
    
    if (isListening) {
        outputLevelBar->setValue(QRandomGenerator::global()->bounded(30) + 10); // 10-40 when listening
    } else {
        outputLevelBar->setValue(0);
    }
    
    statsLabel->setText(QString("TX: %1 | RX: %2 | Loss: %3")
        .arg(stats.packetsSent)
        .arg(stats.packetsReceived)
        .arg(stats.packetsLost));
}

void MainWindow::updateConnectionStatus() {
    // Update connection status display
}

void MainWindow::updateConnectedUsers() {
    if (!client || !isConnected) {
        return;
    }
    
    auto users = client->getConnectedUsers();
    
    // Only update if list changed
    if (users.size() != static_cast<size_t>(usersList->count())) {
        usersList->clear();
        for (const auto& user : users) {
            QString userText = QString::fromStdString(user.username);
            if (user.isTalking) {
                userText = "🔴 " + userText + " (talking)";
            } else if (user.isListening) {
                userText = "🟢 " + userText;
            } else {
                userText = "⚪ " + userText;
            }
            usersList->addItem(userText);
        }
        
        usersCountLabel->setText(QString("%1 user%2 online")
            .arg(users.size())
            .arg(users.size() == 1 ? "" : "s"));
    }
}

void MainWindow::populateAudioDevices() {
    // This would require PortAudio device enumeration
    // For now, just add default option
    inputDeviceCombo = new QComboBox();
    outputDeviceCombo = new QComboBox();
    
    inputDeviceCombo->addItem("Default Input Device", -1);
    outputDeviceCombo->addItem("Default Output Device", -1);
}

} // namespace VoiceChat
