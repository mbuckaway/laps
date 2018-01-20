#include "cplotform.h"
#include "ui_cplotform.h"

CPlotForm::CPlotForm(QWidget *parent) :
    QTabWidget(parent),
    ui(new Ui::CPlotForm)
{
    ui->setupUi(this);
}

CPlotForm::~CPlotForm()
{
    delete ui;
}
