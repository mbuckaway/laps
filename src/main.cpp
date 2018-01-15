#include "mainwindow.h"
#include <QApplication>


// Setup steps.
// 1. Set setupMode true.
// 2. Edit xml file (../readerConfig.xml) with antenna transmit power settings.
// 3. Start this application and look for printed info on tags in antenna zone.
// 4. Adjust antenna power and find minimum power that sees tags when held at waist height while standing on start line.



int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    MainWindow w;
    w.show();
    return a.exec();
}
