#include <QtGui/QApplication>
#include "editor.h"

int main(int argc, char *argv[])
{
    Q_INIT_RESOURCE(editor);

    QApplication a(argc, argv);
    editor w;
    
    return a.exec();
}
