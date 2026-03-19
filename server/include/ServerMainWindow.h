#pragma once

#include <QMainWindow>
#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QSpinBox>
#include <QListWidget>
#include <memory>
#include "UserManager.h"
#include "NetworkManager.h"

namespace VoiceChat {

class ServerMainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit ServerMainWindow(QWidget *parent = nullptr);
    ~ServerMainWindow();

private slots:
    void onStartServerClicked();
    void onStopServerClicked();
    void onCreateUserClicked();
    void onDeleteUserClicked();
    void onRefreshUsersClicked();
    void onRefreshClientsClicked();
    void onUserListUpdated(const std::vector<ClientInfo>& clients);

private:
    void setupUI();
    void updateUserTable();
    void updateClientList();
    void updateServerStatus();

    // User Management
    std::unique_ptr<UserManager> userManager_;
    QTableWidget *userTableWidget_;
    QLineEdit *newUsernameEdit_;
    QLineEdit *newPasswordEdit_;
    QPushButton *createUserButton_;
    QPushButton *deleteUserButton_;
    QPushButton *refreshUsersButton_;

    // Server Control
    std::unique_ptr<NetworkManager> networkManager_;
    QSpinBox *portSpinBox_;
    QLineEdit *bindAddressEdit_;
    QPushButton *startServerButton_;
    QPushButton *stopServerButton_;
    QLabel *serverStatusLabel_;

    // Connected Clients
    QListWidget *connectedClientsWidget_;
    QPushButton *refreshClientsButton_;
    QLabel *clientCountLabel_;

    bool serverRunning_;
};

} // namespace VoiceChat
