/********************************
 * This is CLIENT main file
 ********************************/

#include <QtGui/QApplication>
#include "../declarations.h"
#include "../gameui.h"

const quint32 version=1000040;
const char *versionString="prealpha-0.0.4";

int main(int argc, char *argv[])                                                                                                                                // powered by Norrius in 2011-2012
{
    try {
#ifdef SNS_UI_ONLY
        qDebug() << "UI only";
#else
        qDebug() << "All modules";
#endif
        Q_INIT_RESOURCE(qt);
        Q_INIT_RESOURCE(ui);

        QApplication app(argc, argv);

        //app.addLibraryPath("lib/");

        gameUI ui;

        return app.exec();
    }
    catch (const char *ex) {
        cerr << "The main cycle caught unhandled exception:\n" << ex << "\n";
        return 1;
    }
    catch (...) {
        cerr << "The main cycle caught unknown unhandled exception.\n";
        return 1;
    }
}
