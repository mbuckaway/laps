#ifndef CMESSAGES_H
#define CMESSAGES_H

#include <QWidget>
#include <QFile>
#include <QTextStream>

namespace Ui {
class CMessages;
}

class CMessages : public QWidget
{
    Q_OBJECT
public:
    explicit CMessages(QWidget *parent = 0);
    ~CMessages();
    bool detailedMessagesIsChecked(void);
    QString errorText;
    int error;
    void startLogFile(const QString &fileName);
private:
    Ui::CMessages *ui;
    QFile *logFile;
    QTextStream *logTextStream;
public slots:
    void addMessage(const QString &text);
};

#endif // CMESSAGES_H
