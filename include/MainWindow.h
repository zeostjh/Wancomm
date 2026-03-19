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
#include <QListWidget>
#include <QDialog>
#include <memory>
#include "VoiceChatClient.h"

namespace VoiceChat
{

    class LoginDialog : public QDialog
    {
        Q_OBJECT

    public:
        explicit LoginDialog(QWidget *parent = nullptr);
        QString getUsername() const;
        QString getServerAddress() const;
        int getPort() const;
        bool isServerMode() const;

    private:
        QLineEdit *usernameEdit;
        QLineEdit *serverAddressEdit;
        QLineEdit *portEdit;
        QComboBox *modeComboBox;
    };

    class MainWindow : public QMainWindow
    {
        Q_OBJECT

    public:
        explicit MainWindow(QWidget *parent = nullptr);
        ~MainWindow();

    private slots:
        void onConnectClicked();
        void onTalkPressed();
        void onTalkReleased();
        void onListenToggled();
        void onChannelChanged(int index);
        void updateStatistics();
        void updateConnectionStatus();
        void updateConnectedUsers();
        void populateAudioDevices();

    private:
        void setupUI();
        void applyDarkTheme();
        void connectToServer();
        void disconnectFromServer();
        void startServer();
        void stopServer();
        void showLoginDialog();

        // UI Components
        QWidget *centralWidget;

        // User info display  
        QLabel *usernameLabel;
        QLabel *connectionLabel;
        
        // Audio device selection
        QComboBox *inputDeviceCombo;
        QComboBox *outputDeviceCombo;
        
        // Channel selection
        QComboBox *channelComboBox;
        QLabel *channelNameLabels[4];

        // ClearCom-style controls
        QPushButton *talkButton;
        QPushButton *listenButton;
        
        // LED indicators
        QLabel *talkIndicator;
        QLabel *listenIndicator;
        QLabel *signalIndicator;

        // Audio level meters
        QProgressBar *inputLevelBar;
        QProgressBar *outputLevelBar;
        
        // Connected users list
        QListWidget *usersList;
        QLabel *usersCountLabel;

        // Statistics
        QLabel *statsLabel;
        QPushButton *connectButton;

        // Voice chat client
        std::unique_ptr<VoiceChatClient> client;
        QTimer *statsTimer;
        QTimer *usersTimer;

        // State
        bool isConnected;
        bool isTalking;
        bool isListening;
        bool isServerMode;
        QString myUsername;
        QString serverAddress;
        int serverPort;
    };

} // namespace VoiceChat
