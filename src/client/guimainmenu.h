#ifndef GUIMAINMENU_H
#define GUIMAINMENU_H

#include <QWidget>

namespace Ui {
class guiMainMenu;
}

class guiMainMenu : public QWidget
{
    Q_OBJECT
    
public:
    explicit guiMainMenu(QWidget *parent = 0);
    ~guiMainMenu();
    
private:
    Ui::guiMainMenu *ui;
};

#endif // GUIMAINMENU_H
