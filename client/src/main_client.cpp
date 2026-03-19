#include "ClientMainWindow.h"
#include <QApplication>

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    
    VoiceChat::ClientMainWindow window;
    window.show();
    
    return app.exec();
}
