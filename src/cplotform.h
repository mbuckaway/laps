#ifndef CPLOTFORM_H
#define CPLOTFORM_H

#include <QTabWidget>

namespace Ui {
class CPlotForm;
}

class CPlotForm : public QTabWidget
{
    Q_OBJECT

public:
    explicit CPlotForm(QWidget *parent = 0);
    ~CPlotForm();

private:
    Ui::CPlotForm *ui;
};

#endif // CPLOTFORM_H
