#include <QApplication>
#include <QString>
#include <QScreen>

#include "xmleditor.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    QCoreApplication::setOrganizationName("My QtProject");
    QCoreApplication::setApplicationName("XML Editor");
    QCoreApplication::setApplicationVersion("1.0");
    
    XmlEditor editor;
    editor.show();

    qApp->setStyle(QStyleFactory::create("Fusion"));
    
    return app.exec();
}

