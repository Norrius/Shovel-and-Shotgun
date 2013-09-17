/********************************
 * This is SERVER main file
 ********************************/

#include <QtGui/QApplication>
#include "../declarations.h"
#include "../gamecore.h"

const quint32 version=1000040;
const char *versionString="prealpha-0.0.4";

class SNSServerApp : public QApplication
{
public:
    SNSServerApp(int &argc, char* argv[]) : QApplication(argc, argv) {}
    virtual ~SNSServerApp() {}
    virtual bool notify(QObject * receiver, QEvent * event)
    {
        try
        {
            return QApplication::notify(receiver, event);
        }
        catch (const char *e)
        {
            cerr << "Unhandled exception thrown from an event handler: " << e << "\n";
            exit(1);
        }
        catch (std::exception& e)
        {
            cerr << "Unhandled exception thrown from an event handler: " << e.what() << "\n";
            exit(1);
        }
        catch (...)
        {
            cerr << "Unknown unhandled exception thrown from an event handler.\n";
            exit(1);
        }
    }
};

int main(int argc, char *argv[])                                                                                                                                // powered by Norrius in 2011-2012
{
    /*clog << "int: " << sizeof(int) << " char: " << sizeof(char)
           << "\nfloat: " << sizeof(float) << " double: " << sizeof(double) << "\n";*/
    try
    {
        SNSServerApp app(argc, argv);

        gameCore core;

        return app.exec();
    }
    catch (const char *e)
    {
        cerr << "Unhandled exception thrown from main(): " << e << "\n";
        exit(1);
    }
    catch (...)
    {
        cerr << "Unknown unhandled exception thrown from main().\n";
        exit(1);
    }
}
