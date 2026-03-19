#include "MainWindow.h"
#include <QApplication>

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    
    VoiceChat::MainWindow window;
    window.show();
    
    return app.exec();
}

