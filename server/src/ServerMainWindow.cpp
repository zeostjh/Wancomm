#include "ServerMainWindow.h"
#include <QMessageBox>
#include <QHeaderView>
#include <QDateTime>
#include <QApplication>
#include <QScreen>

namespace VoiceChat {

ServerMainWindow::ServerMainWindow(QWidget *parent)
    : QMainWindow(parent), serverRunning_(false) {
    
    userManager_ = std::make_unique<UserManager>();
    userManager_->loadFromFile("users.dat");
    
    setupUI();
    updateUserTable();
}

ServerMainWindow::~ServerMainWindow() {
    if (serverRunning_ && networkManager_) {
        networkManager_->stopReceiving();
    }
}

void ServerMainWindow::setupUI() {
    setWindowTitle("VoiceChat Server - Admin");
    resize(900, 700);

    // Center window on screen
    const QRect screenGeometry = QApplication::primaryScreen()->geometry();
    int x = (screenGeometry.width() - width()) / 2;
    int y = (screenGeometry.height() - height()) / 2;
    move(x, y);

    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    
    // Server Control Section
    QGroupBox *serverGroup = new QGroupBox("Server Control");
    QVBoxLayout *serverLayout = new QVBoxLayout(serverGroup);
    
    QHBoxLayout *serverConfigLayout = new QHBoxLayout();
    serverConfigLayout->addWidget(new QLabel("Bind Address:"));
    bindAddressEdit_ = new QLineEdit("0.0.0.0");
    serverConfigLayout->addWidget(bindAddressEdit_);
    
    serverConfigLayout->addWidget(new QLabel("Port:"));
    portSpinBox_ = new QSpinBox();
    portSpinBox_->setRange(1024, 65535);
    portSpinBox_->setValue(5000);
    serverConfigLayout->addWidget(portSpinBox_);
    serverLayout->addLayout(serverConfigLayout);
    
    QHBoxLayout *serverButtonLayout = new QHBoxLayout();
    startServerButton_ = new QPushButton("Start Server");
    stopServerButton_ = new QPushButton("Stop Server");
    stopServerButton_->setEnabled(false);
    serverButtonLayout->addWidget(startServerButton_);
    serverButtonLayout->addWidget(stopServerButton_);
    serverLayout->addLayout(serverButtonLayout);
    
    serverStatusLabel_ = new QLabel("Status: Stopped");
    serverStatusLabel_->setStyleSheet("QLabel { font-weight: bold; color: red; }");
    serverLayout->addWidget(serverStatusLabel_);
    
    mainLayout->addWidget(serverGroup);
    
    // User Management Section
    QGroupBox *userGroup = new QGroupBox("User Management");
    QVBoxLayout *userLayout = new QVBoxLayout(userGroup);
    
    QHBoxLayout *newUserLayout = new QHBoxLayout();
    newUserLayout->addWidget(new QLabel("Username:"));
    newUsernameEdit_ = new QLineEdit();
    newUserLayout->addWidget(newUsernameEdit_);
    
    newUserLayout->addWidget(new QLabel("Password:"));
    newPasswordEdit_ = new QLineEdit();
    newPasswordEdit_->setEchoMode(QLineEdit::Password);
    newUserLayout->addWidget(newPasswordEdit_);
    
    createUserButton_ = new QPushButton("Create User");
    newUserLayout->addWidget(createUserButton_);
    userLayout->addLayout(newUserLayout);
    
    userTableWidget_ = new QTableWidget();
    userTableWidget_->setColumnCount(4);
    userTableWidget_->setHorizontalHeaderLabels({"Username", "Active", "Created", "Last Login"});
    userTableWidget_->horizontalHeader()->setStretchLastSection(true);
    userTableWidget_->setSelectionBehavior(QAbstractItemView::SelectRows);
    userTableWidget_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    userLayout->addWidget(userTableWidget_);
    
    QHBoxLayout *userButtonLayout = new QHBoxLayout();
    deleteUserButton_ = new QPushButton("Delete Selected User");
    refreshUsersButton_ = new QPushButton("Refresh");
    userButtonLayout->addWidget(deleteUserButton_);
    userButtonLayout->addWidget(refreshUsersButton_);
    userButtonLayout->addStretch();
    userLayout->addLayout(userButtonLayout);
    
    mainLayout->addWidget(userGroup);
    
    // Connected Clients Section
    QGroupBox *clientsGroup = new QGroupBox("Connected Clients");
    QVBoxLayout *clientsLayout = new QVBoxLayout(clientsGroup);
    
    clientCountLabel_ = new QLabel("Connected: 0");
    clientsLayout->addWidget(clientCountLabel_);
    
    connectedClientsWidget_ = new QListWidget();
    clientsLayout->addWidget(connectedClientsWidget_);
    
    refreshClientsButton_ = new QPushButton("Refresh Clients");
    clientsLayout->addWidget(refreshClientsButton_);
    
    mainLayout->addWidget(clientsGroup);
    
    // Connect signals
    connect(startServerButton_, &QPushButton::clicked, this, &ServerMainWindow::onStartServerClicked);
    connect(stopServerButton_, &QPushButton::clicked, this, &ServerMainWindow::onStopServerClicked);
    connect(createUserButton_, &QPushButton::clicked, this, &ServerMainWindow::onCreateUserClicked);
    connect(deleteUserButton_, &QPushButton::clicked, this, &ServerMainWindow::onDeleteUserClicked);
    connect(refreshUsersButton_, &QPushButton::clicked, this, &ServerMainWindow::onRefreshUsersClicked);
    connect(refreshClientsButton_, &QPushButton::clicked, this, &ServerMainWindow::onRefreshClientsClicked);
}

void ServerMainWindow::onStartServerClicked() {
    if (serverRunning_) {
        return;
    }
    
    networkManager_ = std::make_unique<NetworkManager>();
    networkManager_->setAuthenticator(userManager_.get());
    
    int port = portSpinBox_->value();
    QString bindAddress = bindAddressEdit_->text();
    
    if (networkManager_->initializeServer(port, bindAddress.toStdString())) {
        if (networkManager_->startReceiving()) {
            serverRunning_ = true;
            startServerButton_->setEnabled(false);
            stopServerButton_->setEnabled(true);
            portSpinBox_->setEnabled(false);
            bindAddressEdit_->setEnabled(false);
            
            serverStatusLabel_->setText("Status: Running");
            serverStatusLabel_->setStyleSheet("QLabel { font-weight: bold; color: green; }");
            
            // Set up user list callback
            networkManager_->setUserListCallback([this](const std::vector<ClientInfo>& clients) {
                QMetaObject::invokeMethod(this, [this, clients]() {
                    onUserListUpdated(clients);
                }, Qt::QueuedConnection);
            });
            
            QMessageBox::information(this, "Server Started", 
                QString("Server started on %1:%2").arg(bindAddress).arg(port));
        } else {
            QMessageBox::critical(this, "Error", "Failed to start receiving");
            networkManager_.reset();
        }
    } else {
        QMessageBox::critical(this, "Error", 
            QString("Failed to initialize server on %1:%2").arg(bindAddress).arg(port));
        networkManager_.reset();
    }
}

void ServerMainWindow::onStopServerClicked() {
    if (!serverRunning_ || !networkManager_) {
        return;
    }
    
    networkManager_->stopReceiving();
    networkManager_.reset();
    
    serverRunning_ = false;
    startServerButton_->setEnabled(true);
    stopServerButton_->setEnabled(false);
    portSpinBox_->setEnabled(true);
    bindAddressEdit_->setEnabled(true);
    
    serverStatusLabel_->setText("Status: Stopped");
    serverStatusLabel_->setStyleSheet("QLabel { font-weight: bold; color: red; }");
    
    connectedClientsWidget_->clear();
    clientCountLabel_->setText("Connected: 0");
    
    QMessageBox::information(this, "Server Stopped", "Server has been stopped");
}

void ServerMainWindow::onCreateUserClicked() {
    QString username = newUsernameEdit_->text().trimmed();
    QString password = newPasswordEdit_->text();
    
    if (username.isEmpty()) {
        QMessageBox::warning(this, "Invalid Input", "Username cannot be empty");
        return;
    }
    
    if (password.isEmpty()) {
        QMessageBox::warning(this, "Invalid Input", "Password cannot be empty");
        return;
    }
    
    if (userManager_->createUser(username.toStdString(), password.toStdString())) {
        QMessageBox::information(this, "Success", 
            QString("User '%1' created successfully").arg(username));
        newUsernameEdit_->clear();
        newPasswordEdit_->clear();
        updateUserTable();
    } else {
        QMessageBox::warning(this, "Error", 
            QString("Failed to create user '%1'. User may already exist.").arg(username));
    }
}

void ServerMainWindow::onDeleteUserClicked() {
    int currentRow = userTableWidget_->currentRow();
    if (currentRow < 0) {
        QMessageBox::warning(this, "No Selection", "Please select a user to delete");
        return;
    }
    
    QString username = userTableWidget_->item(currentRow, 0)->text();
    
    QMessageBox::StandardButton reply = QMessageBox::question(this, "Confirm Delete",
        QString("Are you sure you want to delete user '%1'?").arg(username),
        QMessageBox::Yes | QMessageBox::No);
    
    if (reply == QMessageBox::Yes) {
        if (userManager_->deleteUser(username.toStdString())) {
            QMessageBox::information(this, "Success", 
                QString("User '%1' deleted successfully").arg(username));
            updateUserTable();
        } else {
            QMessageBox::warning(this, "Error", 
                QString("Failed to delete user '%1'").arg(username));
        }
    }
}

void ServerMainWindow::onRefreshUsersClicked() {
    updateUserTable();
}

void ServerMainWindow::onRefreshClientsClicked() {
    updateClientList();
}

void ServerMainWindow::onUserListUpdated(const std::vector<ClientInfo>& clients) {
    updateClientList();
}

void ServerMainWindow::updateUserTable() {
    userTableWidget_->setRowCount(0);
    
    std::vector<User> users = userManager_->getAllUsers();
    
    for (const auto& user : users) {
        int row = userTableWidget_->rowCount();
        userTableWidget_->insertRow(row);
        
        userTableWidget_->setItem(row, 0, new QTableWidgetItem(QString::fromStdString(user.username)));
        userTableWidget_->setItem(row, 1, new QTableWidgetItem(user.isActive ? "Yes" : "No"));
        
        QDateTime created = QDateTime::fromSecsSinceEpoch(user.createdAt);
        userTableWidget_->setItem(row, 2, new QTableWidgetItem(created.toString("yyyy-MM-dd HH:mm")));
        
        QString lastLogin = user.lastLogin > 0 ? 
            QDateTime::fromSecsSinceEpoch(user.lastLogin).toString("yyyy-MM-dd HH:mm") : "Never";
        userTableWidget_->setItem(row, 3, new QTableWidgetItem(lastLogin));
    }
}

void ServerMainWindow::updateClientList() {
    connectedClientsWidget_->clear();
    
    if (!serverRunning_ || !networkManager_) {
        clientCountLabel_->setText("Connected: 0");
        return;
    }
    
    std::vector<ClientInfo> clients = networkManager_->getConnectedUsers();
    clientCountLabel_->setText(QString("Connected: %1").arg(clients.size()));
    
    for (const auto& client : clients) {
        QString info = QString("%1 - Channel %2 - %3")
            .arg(QString::fromStdString(client.username))
            .arg(client.channel)
            .arg(client.isTalking ? "Talking" : (client.isListening ? "Listening" : "Idle"));
        
        connectedClientsWidget_->addItem(info);
    }
}

} // namespace VoiceChat
