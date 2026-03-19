#include "ServerMainWindow.h"
#include <QApplication>

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    
    VoiceChat::ServerMainWindow window;
    window.show();
    
    return app.exec();
}
