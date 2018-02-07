// csessions.cpp



#include "csessions.h"
#include "ui_csessions.h"

CSessions::CSessions(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CSessions)
{
    ui->setupUi(this);
}

CSessions::~CSessions() {
    delete ui;
}


