#ifndef CSettings_H
#define CSettings_H

#include <QWidget>

namespace Ui {
class CSettings;
}

class CSettings : public QWidget
{
    Q_OBJECT
public:
    explicit CSettings(QWidget *parent = 0);
    ~CSettings();
private:
    Ui::CSettings *ui;
public slots:
};

#endif // CSettings_H
