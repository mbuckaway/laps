#ifndef CSessions_H
#define CSessions_H

#include <QWidget>

namespace Ui {
class CSessions;
}

class CSessions : public QWidget
{
    Q_OBJECT
public:
    explicit CSessions(QWidget *parent = 0);
    ~CSessions();
private:
    Ui::CSessions *ui;
public slots:
};

#endif // CSessions_H
