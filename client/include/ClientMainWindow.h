#pragma once

#include <QMainWindow>
#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QProgressBar>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QTimer>
#include <QSpinBox>
#include <QDialog>
#include <QCheckBox>
#include <vector>
#include <memory>
#include "VoiceChatClient.h"
#include "AudioDevice.h"

namespace VoiceChat {

class LoginDialog : public QDialog {
    Q_OBJECT

public:
    explicit LoginDialog(QWidget *parent = nullptr);
    QString getUsername() const;
    QString getPassword() const;
    QString getServerAddress() const;
    int getPort() const;

private:
    QLineEdit *usernameEdit_;
    QLineEdit *passwordEdit_;
    QLineEdit *serverAddressEdit_;
    QSpinBox *portSpinBox_;
};

class ChannelControl : public QWidget {
    Q_OBJECT

public:
    explicit ChannelControl(int channelNumber, QWidget *parent = nullptr);
    
    bool isTalkEnabled() const;
    bool isListenEnabled() const;
    int getChannelNumber() const { return channelNumber_; }

signals:
    void talkStateChanged(int channel, bool enabled);
    void listenStateChanged(int channel, bool enabled);

private slots:
    void onTalkPressed();
    void onTalkReleased();
    void onListenToggled(bool checked);

private:
    int channelNumber_;
    QPushButton *talkButton_;
    QCheckBox *listenCheckBox_;
    QLabel *statusLabel_;
    bool talkActive_;
};

class ClientMainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit ClientMainWindow(QWidget *parent = nullptr);
    ~ClientMainWindow();

private slots:
    void onConnectClicked();
    void onDisconnectClicked();
    void onSettingsClicked();
    void onChannelTalkStateChanged(int channel, bool enabled);
    void onChannelListenStateChanged(int channel, bool enabled);
    void updateMeters();
    void updateConnectionStatus();

private:
    void setupUI();
    void showLoginDialog();
    void updateDeviceList();
    void connectToServer();
    void disconnectFromServer();
    
    // UI Components
    std::vector<std::unique_ptr<ChannelControl>> channelControls_;
    QProgressBar *inputLevelMeter_;
    QProgressBar *outputLevelMeter_;
    QComboBox *inputDeviceCombo_;
    QComboBox *outputDeviceCombo_;
    QPushButton *connectButton_;
    QPushButton *disconnectButton_;
    QPushButton *settingsButton_;
    QLabel *connectionStatusLabel_;
    QLabel *usernameLabel_;
    QLabel *serverLabel_;
    
    // Timers
    QTimer *meterUpdateTimer_;
    QTimer *statusUpdateTimer_;
    
    // Client
    std::unique_ptr<VoiceChatClient> client_;
    std::vector<AudioDeviceInfo> inputDevices_;
    std::vector<AudioDeviceInfo> outputDevices_;
    
    // Connection info
    QString username_;
    QString password_;
    QString serverAddress_;
    int serverPort_;
    bool connected_;
};

} // namespace VoiceChat
