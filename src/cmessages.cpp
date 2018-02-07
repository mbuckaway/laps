// cmessages.cpp

#include <QScrollBar>
#include <QPlainTextEdit>
#include <QTextCursor>
#include <QDateTime>
#include <QFile>
#include <QDebug>


#include "cmessages.h"
#include "ui_cmessages.h"



CMessages::CMessages(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CMessages)
{
    ui->setupUi(this);
    logTextStream = NULL;
    logFile = new QFile();

    ui->messagesPlainTextEdit->setReadOnly(true);
}



CMessages::~CMessages() {
    if (logFile)
        delete logFile;
    if (logTextStream)
        delete logTextStream;
    delete ui;
}



void CMessages::addMessage(const QString &text) {
    bool scrollToBottomRequired = false;
    if (ui->messagesPlainTextEdit->verticalScrollBar()->sliderPosition() == ui->messagesPlainTextEdit->verticalScrollBar()->maximum())
        scrollToBottomRequired = true;

    QString s(QDateTime::currentDateTime().toString("yyyy-MM-dd_hh:mm:ss ") + text);
    ui->messagesPlainTextEdit->insertPlainText(s + '\n');

//    if (scrollToBottomRequired)
//        ui->messagesPlainTextEdit->scrollToBottom();

    if (logTextStream)
         *logTextStream << s << "\n";
}



void CMessages::startLogFile(const QString &fileName) {
    if (logTextStream) {
        delete logTextStream;
        logTextStream = NULL;
    }

    logFile->setFileName(fileName);
    int rc = logFile->open(QIODevice::Append | QIODevice::Text);
    if (!rc) {
        qDebug() << "log file not opened";
    }

    logTextStream = new QTextStream(logFile);
    if (!logTextStream)
        addMessage("Error creating log QTextStream");
    logTextStream->setCodec("UTF-8");

    addMessage("Opened log file " + logFile->fileName());
}



bool CMessages::detailedMessagesIsChecked(void) {
    return ui->detailedMessagesCheckBox->isChecked();
}
