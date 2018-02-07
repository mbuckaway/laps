// csettings.cpp



#include "csettings.h"
#include "ui_csettings.h"

CSettings::CSettings(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CSettings)
{
    ui->setupUi(this);
}

CSettings::~CSettings() {
    delete ui;
}


