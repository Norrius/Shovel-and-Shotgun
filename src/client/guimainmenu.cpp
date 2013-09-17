#include "guimainmenu.h"
#include "ui_guimainmenu.h"

guiMainMenu::guiMainMenu(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::guiMainMenu)
{
    ui->setupUi(this);
}

guiMainMenu::~guiMainMenu()
{
    delete ui;
}
