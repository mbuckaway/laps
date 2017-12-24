// mainwindow.cpp
//

#include <QTimer>
#include <QMessageBox>
#include <QTableWidget>
#include <QDateTime>
#include <QScrollBar>
#include <QThread>
#include <QPlainTextEdit>
#include <QTextCursor>
#include <QTableView>
#include <QToolBox>
#include <QComboBox>
#include <QDebug>
#include <QSettings>
#include <QStandardItemModel>

#include <stdio.h>
#include <unistd.h>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "ltkcpp.h"
#include "creader.h"
#include "cdbase.h"
#include "main.h"


// Columns in active riders table

#define AT_NAME             0
#define AT_LAPCOUNT         1
#define AT_LAPSPEED         2
#define AT_BESTLAPSPEED     3
#define AT_DISTANCE         4
#define AT_AVERAGESPEED     5
#define AT_DISTANCETHISMONTH 6
#define AT_AVERAGESPEEDTHISMONTH 7
#define AT_DISTANCELASTMONTH 8
#define AT_AVERAGESPEEDLASTMONTH 9
#define AT_DISTANCEALLTIME  10
#define AT_COMMENT          11


// Columns in lap table (listing of all laps)

#define LT_NAME             0
#define LT_LAPCOUNT         1
#define LT_DATETIME         2
#define LT_TIMESTAMP        3
#define LT_LAPTIME          4
#define LT_LAPSPEED         5
#define LT_COMMENT          6


// Column widths in each table

#define CW_NAME             150
#define CW_DATETIME         80
#define CW_TIMESTAMP        190
#define CW_LAPCOUNT         60
#define CW_SPEED            90
#define CW_DISTANCE         80



// Records to note:
// - personal best lap
// - personal best 10 laps
// - personal best 1k
// - personal best 10k
// - personal best 20k
// - personal best 40k
// - evening best lap
// - evening best 10 laps
// - evening best 1k
// - month best lap
// - month best 10 laps
// - month best 1k
// - alltime best lap
// - alltime best 10 laps
// - alltime best 1k
// - alltime best 10k
// - alltime best 20k
// - alltime best 40k









CNamesModel::CNamesModel(QObject *parent) : QAbstractTableModel(parent) {
    loadNames();
}



void CNamesModel::loadNames(void) {
    qDebug() << "loadNames";
    tagIdList.clear();
    firstNameList.clear();
    lastNameList.clear();

    QSqlQuery query;
    query.prepare("select * from names");
    if (!query.exec())
        return;

    int idTagId = query.record().indexOf("tagId");
    int idFirst = query.record().indexOf("firstName");
    int idLast = query.record().indexOf("lastName");
    while (query.next()) {
        tagIdList.append(query.value(idTagId).toString());
        firstNameList.append(query.value(idFirst).toString());
        lastNameList.append(query.value(idLast).toString());
    }
    QModelIndex topLeft = index(0, 0);
    QModelIndex bottomRight = index(tagIdList.size() - 1, 2);
    emit dataChanged(topLeft, bottomRight);
}



void CNamesModel::addName(QByteArray tagId, QByteArray firstName, QByteArray lastName) {
    qDebug() << "addName" << tagId << firstName << lastName;
    insertRows(rowCount(QModelIndex()), 1);
    setData(createIndex(rowCount() - 1, 0), tagId, Qt::EditRole);
    setData(createIndex(rowCount() - 1, 1), firstName, Qt::EditRole);
    setData(createIndex(rowCount() - 1, 2), lastName, Qt::EditRole);
//    QModelIndex topLeft = index(0, 0);
//    QModelIndex bottomRight = index(tagIdList.size() - 1, 2);
//    emit dataChanged(topLeft, bottomRight);
}



bool CNamesModel::insertRows(int position, int rows, const QModelIndex &/*parent*/) {
    beginInsertRows(QModelIndex(), position, position + rows - 1);
    for (int row = 0; row < rows; ++row) {
        tagIdList.insert(position, QString());
        firstNameList.insert(position, QString());
        lastNameList.insert(position, QString());
    }
    endInsertRows();
    return true;
}



bool CNamesModel::removeRows(int position, int rows, const QModelIndex &/*parent*/) {
    beginRemoveRows(QModelIndex(), position, position + rows - 1);
    for (int row = 0; row < rows; ++row) {
        tagIdList.removeAt(position);
        firstNameList.removeAt(position);
        lastNameList.removeAt(position);
    }
    endRemoveRows();
    return true;
}



int CNamesModel::rowCount(const QModelIndex &/*parent*/) const {
    return tagIdList.size();

}



int CNamesModel::columnCount(const QModelIndex &/*parent*/) const {
    return 3;
}



QVariant CNamesModel::data(const QModelIndex &index, int role) const {
    int row = index.row();
    int col = index.column();

    switch (role) {
    case Qt::DisplayRole:
        if (col == 0) return tagIdList[row];
        if (col == 1) return firstNameList[row];
        if (col == 2) return lastNameList[row];
        break;
    case Qt::FontRole:
        break;
    case Qt::BackgroundRole:
//        if (row == 1 && col == 2) {
//            QBrush redBackground(Qt::red);
//            return redBackground;
//        }
        break;
    case Qt::TextAlignmentRole:
        return Qt::AlignLeft + Qt::AlignVCenter;
        break;
//    case Qt::CheckStateRole:
//        return Qt::Checked;
//        break;
    }

    return QVariant();
}



QVariant CNamesModel::headerData(int section, Qt::Orientation orientation, int role) const {
    switch (role) {
    case Qt::DisplayRole:
        if (orientation == Qt::Horizontal) {
            switch (section) {
            case 0:
                return QString("Tag Id");
            case 1:
                return QString("First Name");
            case 2:
                return QString ("Last Name");

            }
        }
    }
    return QVariant();
}



bool CNamesModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    if (role == Qt::EditRole) {
        switch (index.column()) {
        case 0:
            tagIdList[index.row()] = value.toString();
            return true;
        case 1:
            firstNameList[index.row()] = value.toString();
            return true;
        case 2:
            lastNameList[index.row()] = value.toString();
            return true;
        }
    }
    return false;
}



Qt::ItemFlags CNamesModel::flags(const QModelIndex &index) const {
    return QAbstractTableModel::flags(index);
    //return Qt::ItemIsEditable | QAbstractTableModel::flags(index);
    //return Qt::ItemIsSelectable | QAbstractTableModel::flags(index);
}



// **************************************************************************************************

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    QString s;
    ui->setupUi(this);
    trackReader = NULL;
    deskReader = NULL;
    QCoreApplication::setApplicationName("LLRPLaps");
    QCoreApplication::setApplicationVersion("0.1");


    initializeSettingsPanel();

    ui->mainTitleLabel->setText(QCoreApplication::applicationName() + " " + QCoreApplication::applicationVersion());
    ui->leftTitleLabel->setText(ui->trackNameLineEdit->text());
    ui->rightTitleLabel->setText(QString());
    setWindowTitle(QCoreApplication::applicationName() + ": " + ui->trackNameLineEdit->text());

    guiInformation("This application accepts tag-reader data and displays lap times and information.  Be sure to configure settings for your track in the Settings tab.");


    // Get track length at position of each antenna.  Lap speed is estimated from
    // these values assuming rider maintains same position on track for entire lap.

    trackLengthM.append(settings.value("trackLength1M").toFloat());
    trackLengthM.append(settings.value("trackLength2M").toFloat());
    trackLengthM.append(settings.value("trackLength3M").toFloat());
    trackLengthM.append(settings.value("trackLength4M").toFloat());

    ui->trackLength1LineEdit->setText(s.setNum(trackLengthM[0]));
    ui->trackLength2LineEdit->setText(s.setNum(trackLengthM[1]));
    ui->trackLength3LineEdit->setText(s.setNum(trackLengthM[2]));
    ui->trackLength4LineEdit->setText(s.setNum(trackLengthM[3]));
    for (int i=0; i<trackLengthM.size(); i++) {
        if (trackLengthM[i] == 0.) {
            guiCritical("Track length must be specified in settings tab");
            break;
        }
    }


    // Get tablePurgeIntervalSec, the interval on which tables are purged of innactive riders

    tablePurgeIntervalSec = settings.value("tablePurgeIntervalHours").toFloat() * 3600.;
    if (tablePurgeIntervalSec == 0.) tablePurgeIntervalSec = 6. * 3600.;
    if (tablePurgeIntervalSec < 10.) tablePurgeIntervalSec = 10.;
    ui->tablePurgeIntervalLineEdit->setText(s.setNum(tablePurgeIntervalSec / 3600.));


    // If laps table has more than this number of entries, disable sorting to ensure responsive operation

    lapsTableMaxSizeWithSort = 10000;


    // Get tag reader information from settings.  Leave IP empty for simulation (test) mode.

    trackReader = new CReader(ui->trackReaderIP->text(), 0, CReader::track);
    deskReader = new CReader(ui->deskReaderIP->text(), 1, CReader::desk);


    // Initialize member variables

    activeRidersTableSortingEnabled = true;
    lapsTableSortingEnabled = true;
    float nominalSpeedkmph = 32.0;
    float nominalLapSec = (trackLengthM[0] / 1000.) / nominalSpeedkmph * 3600.;
    maxAcceptableLapSec = nominalLapSec * 2.;           // max acceptable time for lap.  If greater, rider must have left and returned to track


    // Initialize 1-sec timer for panel dateTime and possibly other things

    connect(&clockTimer, SIGNAL(timeout()), this, SLOT(onClockTimerTimeout()));
    clockTimer.setInterval(1000);
    clockTimer.start();


    // Configure gui

    ui->lapsTableSortedCheckBox->setCheckable(false);
    ui->activeRidersTableSortedCheckBox->setCheckable(false);


    // Configure messages console

    QPlainTextEdit *m = ui->messagesPlainTextEdit;
    m->setReadOnly(true);


    // Configure active riders table

    QTableWidget *t = ui->activeRidersTableWidget;
    t->setColumnWidth(AT_NAME, CW_NAME);
    t->setColumnWidth(AT_LAPCOUNT, CW_LAPCOUNT);
    t->setColumnWidth(AT_LAPSPEED, CW_SPEED);
    t->setColumnWidth(AT_DISTANCE, CW_DISTANCE);
    t->setColumnWidth(AT_BESTLAPSPEED, CW_SPEED);
    t->setColumnWidth(AT_AVERAGESPEED, CW_SPEED);
    t->setColumnWidth(AT_AVERAGESPEEDTHISMONTH, CW_SPEED);
    t->setColumnWidth(AT_AVERAGESPEEDLASTMONTH, CW_SPEED);
    t->setColumnWidth(AT_DISTANCEALLTIME, CW_DISTANCE);
    t->setColumnWidth(AT_COMMENT, 300);

    t->setSortingEnabled(activeRidersTableSortingEnabled);
    t->setEnabled(false);
    connect(t->horizontalHeader(), SIGNAL(sectionClicked(int)), this, SLOT(onActiveRidersTableHorizontalHeaderSectionClicked(int)));
    connect(ui->activeRidersTableSortedCheckBox, SIGNAL(clicked(bool)), this, SLOT(onActiveRidersTableSortedCheckBoxClicked(bool)));


    // Configure laps table

    t = ui->lapsTableWidget;
    t->setColumnWidth(LT_NAME, CW_NAME);
    t->setColumnWidth(LT_LAPCOUNT, CW_LAPCOUNT);
    t->setColumnWidth(LT_DATETIME, CW_DATETIME);
    t->setColumnWidth(LT_TIMESTAMP, CW_TIMESTAMP);
    t->setColumnWidth(LT_LAPSPEED, CW_SPEED);

    t->setSortingEnabled(lapsTableSortingEnabled);
    t->setEnabled(false);

    connect(t->horizontalHeader(), SIGNAL(sectionClicked(int)), this, SLOT(onLapsTableHorizontalHeaderSectionClicked(int)));
    connect(ui->lapsTableSortedCheckBox, SIGNAL(clicked(bool)), this, SLOT(onLapsTableSortedCheckBoxClicked(bool)));


    // Default to showing messages during connection to reader

    ui->tabWidget->setCurrentIndex(2);


    // Populate antenna power comboBoxes (enabled when reader connects)

    ui->trackAntenna1PowerComboBox->setEnabled(false);
    ui->trackAntenna2PowerComboBox->setEnabled(false);
    ui->trackAntenna3PowerComboBox->setEnabled(false);
    ui->trackAntenna4PowerComboBox->setEnabled(false);

    ui->deskAntenna1PowerComboBox->setEnabled(false);
    ui->deskAntenna2PowerComboBox->setEnabled(false);
    ui->deskAntenna3PowerComboBox->setEnabled(false);
    ui->deskAntenna4PowerComboBox->setEnabled(false);

    connect(ui->trackAntenna1PowerComboBox, SIGNAL(activated(int)), this, SLOT(onTrackAntenna1PowerComboBoxActivated(int)));

    connect(trackReader, SIGNAL(newLogMessage(QString)), this, SLOT(onNewLogMessage(QString)));
    connect(trackReader, SIGNAL(connected(void)), this, SLOT(onReaderConnected(void)));
    connect(trackReader, SIGNAL(newTag(CTagInfo)), this, SLOT(onNewTrackTag(CTagInfo)));

    connect(deskReader, SIGNAL(newLogMessage(QString)), this, SLOT(onNewLogMessage(QString)));
    connect(deskReader, SIGNAL(connected(void)), this, SLOT(onReaderConnected(void)));
    connect(deskReader, SIGNAL(newTag(CTagInfo)), this, SLOT(onNewDeskTag(CTagInfo)));


    // Move CReader class to separate threads

    QThread *trackReaderThread = new QThread(this);
    trackReader->moveToThread(trackReaderThread);
    trackReader->thread = trackReaderThread;
    readerThreadList.append(trackReaderThread);
    connect(trackReaderThread, SIGNAL(started(void)), trackReader, SLOT(onStarted(void)));
    connect(trackReaderThread, SIGNAL(finished(void)), trackReaderThread, SLOT(deleteLater(void)));
    trackReaderThread->start();

    QThread *deskReaderThread = new QThread(this);
    deskReader->moveToThread(deskReaderThread);
    deskReader->thread = deskReaderThread;
    readerThreadList.append(deskReaderThread);
    connect(deskReaderThread, SIGNAL(started(void)), deskReader, SLOT(onStarted(void)));
    connect(deskReaderThread, SIGNAL(finished(void)), deskReaderThread, SLOT(deleteLater(void)));
    deskReaderThread->start();


    // Configure Dbase page

    connect(ui->deskClearPushButton, SIGNAL(clicked()), this, SLOT(onDbaseClearPushButtonClicked()));
    connect(ui->deskSearchPushButton, SIGNAL(clicked()), this, SLOT(onDbaseSearchPushButtonClicked()));
    connect(ui->deskAddPushButton, SIGNAL(clicked()), this, SLOT(onDbaseAddPushButtonClicked()));
    connect(ui->deskRemovePushButton, SIGNAL(clicked()), this, SLOT(onDbaseRemovePushButtonClicked()));
    connect(ui->deskUpdatePushButton, SIGNAL(clicked()), this, SLOT(onDbaseUpdatePushButtonClicked()));
    connect(ui->deskReadPushButton, SIGNAL(clicked(bool)), this, SLOT(onDbaseReadPushButtonClicked(bool)));
    connect(ui->deskTagIdLineEdit, SIGNAL(textChanged(QString)), this, SLOT(onDbaseTagIdTextChanged(QString)));
    connect(ui->deskFirstNameLineEdit, SIGNAL(textChanged(QString)), this, SLOT(onDbaseFirstNameTextChanged(QString)));
    connect(ui->deskLastNameLineEdit, SIGNAL(textChanged(QString)), this, SLOT(onDbaseLastNameTextChanged(QString)));
    updateDbaseButtons();

    // Initialize names table

    int rc = dbase.open("test", "abc", "def");
    if (rc != 0)
        guiCritical("Error opening database file: " + dbase.errorText() + ".\n\nWe will continue but rider names are not available and results are not being recorded.");

    namesModel = new CNamesModel(this);
    namesModel->loadNames();
    ui->namesTableView->setModel(namesModel);
    ui->namesTableView->setColumnWidth(0, 200);
    ui->namesTableView->setColumnWidth(1, 200);
    ui->namesTableView->horizontalHeader()->setStretchLastSection(true);
    ui->namesTableView->setSortingEnabled(true);

//    ui->namesTableView->setColumnWidth(2, 300);
//    ui->namesTableView->setSortingEnabled(true);


    // Start timer that will purge old riders from activeRidersTable

    connect(&purgeActiveRidersListTimer, SIGNAL(timeout(void)), this, SLOT(onPurgeActiveRidersList(void)));
    purgeActiveRidersListTimer.setInterval(tablePurgeIntervalSec * 1000);
    purgeActiveRidersListTimer.start();


    connect(ui->applySettingsPushButton, SIGNAL(clicked()), this, SLOT(onApplySettingsPushButtonClicked()));
    connect(ui->saveSettingsPushButton, SIGNAL(clicked()), this, SLOT(onSaveSettingsPushButtonClicked()));


    QStandardItemModel *model = new QStandardItemModel(2,3,this); //2 Rows and 3 Columns
    model->setHorizontalHeaderItem(0, new QStandardItem(QString("Column1 Header")));
    model->setHorizontalHeaderItem(1, new QStandardItem(QString("Column2 Header")));
    model->setHorizontalHeaderItem(2, new QStandardItem(QString("Column3 Header")));

    ui->tableView->setModel(model);
    ui->tableView->hide();

    QStandardItem *firstRow = new QStandardItem(QString("ColumnValue"));
    model->setItem(0,0,firstRow);
}






MainWindow::~MainWindow() {
    qDebug() << "closing...";
    dbase.close();
    for (int i=0; i<readerThreadList.size(); i++) {
        readerThreadList[i]->requestInterruption();
        readerThreadList[i]->wait();
        delete readerThreadList[i];
    }
    readerThreadList.clear();
    delete ui;
}



// ***********************************************************************************
//
// Member functions related to settings panel

void MainWindow::initializeSettingsPanel(void) {
    ui->trackNameLineEdit->setText(settings.value("trackName").toString());


}



void MainWindow::onTrackAntenna1PowerComboBoxActivated(int antennaIndex) {
    qDebug().nospace() << "onAntenna1ComboBoxActivated()" << antennaIndex;
}



void MainWindow::onSaveSettingsPushButtonClicked(void) {
    onApplySettingsPushButtonClicked();

    settings.setValue("trackName", ui->trackNameLineEdit->text());
    settings.setValue("trackLength1M", trackLengthM[0]);
    settings.setValue("trackLength2M", trackLengthM[1]);
    settings.setValue("trackLength3M", trackLengthM[2]);
    settings.setValue("trackLength4M", trackLengthM[3]);
    settings.setValue("tablePurgeIntervalHours", tablePurgeIntervalSec / 3600.);
    settings.setValue("transmitPower1", 0.);
    settings.setValue("transmitPower2", 0.);
    settings.setValue("transmitPower3", 0.);
    settings.setValue("transmitPower4", 0.);
}



void MainWindow::onApplySettingsPushButtonClicked(void) {
    ui->leftTitleLabel->setText(ui->trackNameLineEdit->text());
    trackLengthM.clear();
    trackLengthM.append(ui->trackLength1LineEdit->text().toFloat());
    trackLengthM.append(ui->trackLength2LineEdit->text().toFloat());
    trackLengthM.append(ui->trackLength3LineEdit->text().toFloat());
    trackLengthM.append(ui->trackLength4LineEdit->text().toFloat());
    tablePurgeIntervalSec = ui->tablePurgeIntervalLineEdit->text().toFloat() * 3600.;
    if (tablePurgeIntervalSec == 0.) tablePurgeIntervalSec = 6. * 3600.;
    if (tablePurgeIntervalSec < 10.) tablePurgeIntervalSec = 10.;
}



void MainWindow::onClockTimerTimeout(void) {
    ui->rightTitleLabel->setText(QDateTime::currentDateTime().toString("ddd MMMM d yyyy  hh:mm:ss"));
}



// *********************************************************************************************
// Member functions related to processing tags received
//

// onPurgeRidersList
// Loop through riders in activeRidersList and remove any that are getting old.
// Remove from activeRidersTable also.
// Then loop through lapsTable and remove old entries.
//
void MainWindow::onPurgeActiveRidersList(void) {
    unsigned long long currentTimeUSec = QDateTime::currentMSecsSinceEpoch() * 1000;

    // Loop through all active riders and see which are geting old

    activeRidersTableMutex.lock();
    ui->activeRidersTableWidget->setSortingEnabled(false);

    for (int i=activeRidersList.size()-1; i>=0; i--) {
        long long inactiveSec = (currentTimeUSec - activeRidersList[i].previousTimeStampUSec) / 1000000;
        if (inactiveSec >= (2*tablePurgeIntervalSec)) {
            bool riderRemoved = false;
            QString nameToRemove = activeRidersList[i].name;
            qDebug() << "Removing" << nameToRemove;
            for (int j=ui->activeRidersTableWidget->rowCount()-1; j>=0; j--) {
                if (ui->activeRidersTableWidget->item(j, AT_NAME)->text() == nameToRemove) {
                    activeRidersList.removeAt(i);
                    ui->activeRidersTableWidget->removeRow(j);
                    riderRemoved = true;
                    break;
                }
            }
            if (!riderRemoved) {
                printf("Rider not found in activeRidersTable in onPurgeActiveRidersList\n");
                fflush(stdout);
            }
        }
    }

    activeRidersTableMutex.unlock();
    ui->activeRidersTableWidget->setSortingEnabled(activeRidersTableSortingEnabled);

    // Loop through lapsTable and remove any entries where the timeStamp is older than tablePurgeIntervalSec

    lapsTableMutex.lock();
    ui->lapsTableWidget->setSortingEnabled(false);

    bool scrollToBottomRequired = false;
    if (ui->lapsTableWidget->verticalScrollBar()->sliderPosition() == ui->lapsTableWidget->verticalScrollBar()->maximum()) {
        scrollToBottomRequired = true;
    }
    for (int i=ui->lapsTableWidget->rowCount()-1; i>=0; i--) {  // ignore last entry
        unsigned long long timeStampUSec = ui->lapsTableWidget->item(i, LT_TIMESTAMP)->text().toULongLong();
        long long timeStampAgeSec = (currentTimeUSec - timeStampUSec) / 1000000;
        if (timeStampAgeSec >= (2*tablePurgeIntervalSec)) {
            ui->lapsTableWidget->removeRow(i);
        }
    }
    if (scrollToBottomRequired) {
        ui->lapsTableWidget->scrollToBottom();
    }

    lapsTableMutex.unlock();
    ui->lapsTableWidget->setSortingEnabled(lapsTableSortingEnabled);
}



void MainWindow::onReaderConnected(void) {
    CReader *sendingReader = (CReader *)sender();

    // If deskReader, block signals until we want to read from reader

    if (sendingReader == deskReader)
        deskReader->blockSignals(true);

    QString s;
    ui->tabWidget->setCurrentIndex(0);
    onNewLogMessage(s.sprintf("Connected to reader %d", sendingReader->readerId));


    // Populate comboBox with available power settings for each reader (WIP).
    // Reader has been configured to select lowest power setting when connected.

    QList<int> *transmitPowerList = sendingReader->getTransmitPowerList();
    if (sendingReader == trackReader) {
        ui->trackAntenna1PowerComboBox->clear();
        for (int i=0; i<transmitPowerList->size(); i++) {
            ui->trackAntenna1PowerComboBox->addItem(s.setNum(transmitPowerList->at(i)));
        }
        ui->trackAntenna1PowerComboBox->setEnabled(true);
        ui->trackAntenna2PowerComboBox->clear();
        for (int i=0; i<transmitPowerList->size(); i++) {
            ui->trackAntenna2PowerComboBox->addItem(s.setNum(transmitPowerList->at(i)));
        }
        ui->trackAntenna2PowerComboBox->setEnabled(true);
        ui->trackAntenna3PowerComboBox->clear();
        for (int i=0; i<transmitPowerList->size(); i++) {
            ui->trackAntenna3PowerComboBox->addItem(s.setNum(transmitPowerList->at(i)));
        }
        ui->trackAntenna3PowerComboBox->setEnabled(true);
        ui->trackAntenna4PowerComboBox->clear();
        for (int i=0; i<transmitPowerList->size(); i++) {
            ui->trackAntenna4PowerComboBox->addItem(s.setNum(transmitPowerList->at(i)));
        }
        ui->trackAntenna4PowerComboBox->setEnabled(true);
    }
    else if (sendingReader == deskReader) {
        ui->deskAntenna1PowerComboBox->clear();
        for (int i=0; i<transmitPowerList->size(); i++) {
            ui->deskAntenna1PowerComboBox->addItem(s.setNum(transmitPowerList->at(i)));
        }
        ui->deskAntenna1PowerComboBox->setEnabled(true);
        ui->deskAntenna2PowerComboBox->clear();
        for (int i=0; i<transmitPowerList->size(); i++) {
            ui->deskAntenna2PowerComboBox->addItem(s.setNum(transmitPowerList->at(i)));
        }
        ui->deskAntenna2PowerComboBox->setEnabled(true);
        ui->deskAntenna3PowerComboBox->clear();
        for (int i=0; i<transmitPowerList->size(); i++) {
            ui->deskAntenna3PowerComboBox->addItem(s.setNum(transmitPowerList->at(i)));
        }
        ui->deskAntenna3PowerComboBox->setEnabled(true);
        ui->deskAntenna4PowerComboBox->clear();
        for (int i=0; i<transmitPowerList->size(); i++) {
            ui->deskAntenna4PowerComboBox->addItem(s.setNum(transmitPowerList->at(i)));
        }
        ui->deskAntenna4PowerComboBox->setEnabled(true);
    }


    ui->lapsTableWidget->setEnabled(true);
    ui->activeRidersTableWidget->setEnabled(true);
}



void MainWindow::onLapsTableHorizontalHeaderSectionClicked(int /*section*/) {
    if (ui->lapsTableWidget->isSortingEnabled()) {
        ui->lapsTableSortedCheckBox->setCheckable(true);
        ui->lapsTableSortedCheckBox->setChecked(true);
    }
}



void MainWindow::onActiveRidersTableHorizontalHeaderSectionClicked(int /*section*/) {
    if (ui->activeRidersTableWidget->isSortingEnabled()) {
        ui->activeRidersTableSortedCheckBox->setCheckable(true);
        ui->activeRidersTableSortedCheckBox->setChecked(true);
    }
}


void MainWindow::onLapsTableSortedCheckBoxClicked(bool state) {
    if (!state) {
        ui->lapsTableSortedCheckBox->setChecked(false);
        ui->lapsTableSortedCheckBox->setCheckable(false);
        ui->lapsTableWidget->setSortingEnabled(true);
        ui->lapsTableWidget->sortByColumn(LT_TIMESTAMP);
        ui->lapsTableWidget->setSortingEnabled(false);
    }
}


void MainWindow::onActiveRidersTableSortedCheckBoxClicked(bool state) {
    if (!state) {
        ui->activeRidersTableSortedCheckBox->setChecked(false);
        ui->activeRidersTableSortedCheckBox->setCheckable(false);
        ui->activeRidersTableWidget->setSortingEnabled(true);
        ui->activeRidersTableWidget->sortByColumn(LT_TIMESTAMP);
        ui->activeRidersTableWidget->setSortingEnabled(false);
    }
}



// Process new tag
//
void MainWindow::onNewTrackTag(CTagInfo tagInfo) {
    QString s;
    QTableWidget *lapsTable = ui->lapsTableWidget;
    QTableWidget *activeRidersTable = ui->activeRidersTableWidget;
    static int tagCount = 0;

    tagCount++;

    // Add string to messages window

    onNewLogMessage(s.sprintf("readerId=%d antennaId=%d timeStampUSec=%llu tagData=%s", tagInfo.readerId, tagInfo.antennaId, tagInfo.timeStampUSec, tagInfo.tagId.data()));


    // Turn off table sorting and lock mutex while we update tables

    lapsTableMutex.lock();
    activeRidersTableMutex.lock();
    ui->lapsTableWidget->setSortingEnabled(false);
    ui->activeRidersTableWidget->setSortingEnabled(false);


    // ActiveRidersList is the main list containing information from each active rider.  Use it for all calculations
    // and then put information to be displayed into activeRidersTable and/or lapsTable.

    try {

        // Check to see if tag is in activeRidersList and get index if it is.  If not in list, set firstLap
        // to indicate rider is riding first lap of workout.

        int activeRidersListIndex = -1;
        for (int i=0; i<activeRidersList.size(); i++) {
            if (tagInfo.tagId == activeRidersList[i].tagId) {
                activeRidersListIndex = i;
                break;
            }
        }

        // firstCrossing is set true if this is the first time this tag is seen in this workout

        bool firstCrossing = false;
        if (activeRidersListIndex < 0)
            firstCrossing = true;
        else
            firstCrossing = false;


        // If this is a new rider in this workout, try getting name from dbase

        QString name;
        if (activeRidersListIndex < 0) {   // New rider, so get name from dBase and calculate best times in each category
            QString firstName;
            QString lastName;
            int rc = dbase.findNameFromTagId(tagInfo.tagId, &firstName, &lastName);
            if (rc == 0)
                name = firstName + " " + lastName;

            if (name.isEmpty())
                name = tagInfo.tagId;
        }
        else {
            name = activeRidersList[activeRidersListIndex].name;
        }


        // firstCrossingAfterBreak will be set true if this is the first time this tag is seen after taking a break

        bool firstCrossingAfterBreak = false;


        // Set rider to point to CRider entry in activeRidersList

        CRider *rider = NULL;

        // If firstCrossing, get best times and add new entry to activeRidersList

        if (firstCrossing) {
            rider = new CRider();

            rider->name = name;
            rider->tagId = tagInfo.tagId;
            rider->previousTimeStampUSec = tagInfo.timeStampUSec;


            // Get stats for this rider

            dbase.getStats(tagInfo.tagId, rider);

            // Add to active riders list

            activeRidersList.append(*rider);
            activeRidersListIndex = activeRidersList.size() - 1;
            rider = &activeRidersList[activeRidersListIndex];
        }

        // else we have completed at least one full lap so update lap stats and thisMonth stats

        else {
            rider = &activeRidersList[activeRidersListIndex];

            // Calculate lap time.  If lap time is greater than maxAcceptableLapSec, rider must have taken a break so do not
            // calculate lap time

            float lapSec = (double)(tagInfo.timeStampUSec - rider->previousTimeStampUSec) / 1.e6;
            if (lapSec > maxAcceptableLapSec) {
                firstCrossingAfterBreak = true;

                rider->lapSec = 0.;
                rider->lapM = 0.;
            }
            else {
                firstCrossingAfterBreak = false;

                rider->lapCount++;
                rider->lapSec = lapSec;
                rider->lapM = trackLengthM[tagInfo.antennaId - 1];
                if ((rider->bestLapSec == 0.) || (rider->lapSec < rider->bestLapSec)) {
                    rider->bestLapSec = rider->lapSec;
                    rider->bestLapM = rider->lapM;
                }
                rider->totalSec += rider->lapSec;
                rider->totalM += rider->lapM;

                rider->thisMonth.lapCount++;
                rider->thisMonth.totalSec += rider->lapSec;
                rider->thisMonth.totalM += rider->lapM;

                rider->allTime.lapCount++;
                rider->allTime.totalM += rider->lapM;
            }
            rider->previousTimeStampUSec = tagInfo.timeStampUSec;
        }


        // Update currentTime string

        QString currentTimeString(QDateTime::fromMSecsSinceEpoch(tagInfo.timeStampUSec / 1000).toString("hh:mm:ss"));


        // Append new entry (row) to lapsTable

        int r = lapsTable->rowCount();
        bool scrollToBottomRequired = false;
        if (lapsTable->verticalScrollBar()->sliderPosition() == lapsTable->verticalScrollBar()->maximum()) {
            scrollToBottomRequired = true;
        }
        lapsTable->insertRow(r);
        lapsTable->setRowHeight(r, 20);
        if (scrollToBottomRequired) {
            lapsTable->scrollToBottom();
        }

        QTableWidgetItem *item = new QTableWidgetItem;
        item->setData(Qt::DisplayRole, name);
        lapsTable->setItem(r, LT_NAME, item);
        lapsTable->item(r, LT_NAME)->setTextAlignment(Qt::AlignLeft);

        item = new QTableWidgetItem;
        item->setData(Qt::DisplayRole, rider->lapCount);
        lapsTable->setItem(r, LT_LAPCOUNT, item);
        lapsTable->item(r, LT_LAPCOUNT)->setTextAlignment(Qt::AlignHCenter);

        item = new QTableWidgetItem;
        item->setData(Qt::DisplayRole, currentTimeString);
        lapsTable->setItem(r, LT_DATETIME, item);
        lapsTable->item(r, LT_DATETIME)->setTextAlignment(Qt::AlignHCenter);

        item = new QTableWidgetItem;
        item->setData(Qt::DisplayRole, tagInfo.timeStampUSec);
        lapsTable->setItem(r, LT_TIMESTAMP, item);
        lapsTable->item(r, LT_TIMESTAMP)->setTextAlignment(Qt::AlignHCenter);

        if (firstCrossing) {
            item = new QTableWidgetItem;
            item->setData(Qt::DisplayRole, "First lap of workout");
            lapsTable->setItem(r, LT_COMMENT, item);
            lapsTable->item(r, LT_COMMENT)->setTextAlignment(Qt::AlignLeft);
        }
        else {
            if (firstCrossingAfterBreak) {
                item = new QTableWidgetItem;
                item->setData(Qt::DisplayRole, "First lap after break");
                lapsTable->setItem(r, LT_COMMENT, item);
                lapsTable->item(r, LT_COMMENT)->setTextAlignment(Qt::AlignLeft);
            }
            else {
                item = new QTableWidgetItem;
                item->setData(Qt::DisplayRole, rider->lapSec);
                lapsTable->setItem(r, LT_LAPTIME, item);
                lapsTable->item(r, LT_LAPTIME)->setTextAlignment(Qt::AlignHCenter);

                if (rider->lapSec > 0.) {
                    item = new QTableWidgetItem;
                    item->setData(Qt::DisplayRole, lapSpeed(rider->lapSec, rider->lapM));
                    lapsTable->setItem(r, LT_LAPSPEED, item);
                    lapsTable->item(r, LT_LAPSPEED)->setTextAlignment(Qt::AlignHCenter);
                }
            }
        }


        // Populate activeRidersTable entries
        // If new active rider, append new blank line to activeRidersTable and set activeRidersTableIndex
        // and add previous best times

        r = activeRidersTable->rowCount();
        int activeRidersTableIndex = -1;
        if (firstCrossing) {
            bool scrollToBottomRequired = false;
            if (activeRidersTable->verticalScrollBar()->sliderPosition() == activeRidersTable->verticalScrollBar()->maximum()) {
                scrollToBottomRequired = true;
            }
            activeRidersTable->insertRow(r);
            activeRidersTableIndex = r;
            activeRidersTable->setRowHeight(activeRidersTableIndex, 20);
            if (scrollToBottomRequired) {
                activeRidersTable->scrollToBottom();
            }

            activeRidersTable->setItem(r, AT_NAME, new QTableWidgetItem());
            activeRidersTable->item(r, AT_NAME)->setText(rider->name);
            activeRidersTable->item(r, AT_NAME)->setTextAlignment(Qt::AlignLeft);

            QTableWidgetItem *item = new QTableWidgetItem;
            activeRidersTable->setItem(r, AT_LAPCOUNT, item);
            activeRidersTable->item(r, AT_LAPCOUNT)->setTextAlignment(Qt::AlignHCenter);

            item = new QTableWidgetItem;
            activeRidersTable->setItem(r, AT_LAPSPEED, item);
            activeRidersTable->item(r, AT_LAPSPEED)->setTextAlignment(Qt::AlignHCenter);

            item = new QTableWidgetItem;
            activeRidersTable->setItem(r, AT_BESTLAPSPEED, item);
            activeRidersTable->item(r, AT_BESTLAPSPEED)->setTextAlignment(Qt::AlignHCenter);

            item = new QTableWidgetItem;
            activeRidersTable->setItem(r, AT_DISTANCE, item);
            activeRidersTable->item(r, AT_DISTANCE)->setTextAlignment(Qt::AlignHCenter);

            item = new QTableWidgetItem;
            activeRidersTable->setItem(r, AT_AVERAGESPEED, item);
            activeRidersTable->item(r, AT_AVERAGESPEED)->setTextAlignment(Qt::AlignHCenter);

            item = new QTableWidgetItem;
            activeRidersTable->setItem(r, AT_DISTANCELASTMONTH, item);
            activeRidersTable->item(r, AT_DISTANCELASTMONTH)->setTextAlignment(Qt::AlignHCenter);

            item = new QTableWidgetItem;
            activeRidersTable->setItem(r, AT_AVERAGESPEEDLASTMONTH, item);
            activeRidersTable->item(r, AT_AVERAGESPEEDLASTMONTH)->setTextAlignment(Qt::AlignHCenter);

            item = new QTableWidgetItem;
            activeRidersTable->setItem(r, AT_DISTANCETHISMONTH, item);
            activeRidersTable->item(r, AT_DISTANCETHISMONTH)->setTextAlignment(Qt::AlignHCenter);

            item = new QTableWidgetItem;
            activeRidersTable->setItem(r, AT_AVERAGESPEEDTHISMONTH, item);
            activeRidersTable->item(r, AT_AVERAGESPEEDTHISMONTH)->setTextAlignment(Qt::AlignHCenter);

            item = new QTableWidgetItem;
            activeRidersTable->setItem(r, AT_DISTANCEALLTIME, item);
            activeRidersTable->item(r, AT_DISTANCEALLTIME)->setTextAlignment(Qt::AlignHCenter);

            item = new QTableWidgetItem;
            activeRidersTable->setItem(r, AT_COMMENT, item);
            activeRidersTable->item(r, AT_COMMENT)->setTextAlignment(Qt::AlignLeft);


            // Set initial this month values

            if (rider->thisMonth.lapCount > 0) {
                activeRidersTable->item(r, AT_DISTANCETHISMONTH)->setData(Qt::DisplayRole, rider->thisMonth.totalM / 1000.);
                activeRidersTable->item(r, AT_AVERAGESPEEDTHISMONTH)->setData(Qt::DisplayRole, lapSpeed(rider->thisMonth.totalSec, rider->thisMonth.totalM));
            }

            // Set initial last month values

            if (rider->lastMonth.lapCount > 0) {
                activeRidersTable->item(r, AT_DISTANCELASTMONTH)->setData(Qt::DisplayRole, rider->lastMonth.totalM / 1000.);
                activeRidersTable->item(r, AT_AVERAGESPEEDLASTMONTH)->setData(Qt::DisplayRole, lapSpeed(rider->lastMonth.totalSec, rider->lastMonth.totalM));
            }

            // Set initial all time values

            activeRidersTable->item(r, AT_DISTANCEALLTIME)->setData(Qt::DisplayRole, rider->allTime.totalM / 1000.);

            // Set initial comment

//            activeRidersTable->item(r, AT_COMMENT)->setText("First lap");
        }

        // Otherwise get activeRidersTableIndex corresponding to this rider already in table

        else {
            for (int i=0; i<ui->activeRidersTableWidget->rowCount(); i++) {
                if (ui->activeRidersTableWidget->item(i, AT_NAME)->text() == rider->name) {
                    activeRidersTableIndex = i;
                    break;
                }
            }

            // Set comment

//            activeRidersTable->item(r, AT_COMMENT)->setText("");
        }

        // If activeRidersTableIndex is still < 0 (should never happen), append blank entry with name ???

        if (activeRidersTableIndex < 0) {
//            int r = activeRidersTable->rowCount();
            bool scrollToBottomRequired = false;
            if (activeRidersTable->verticalScrollBar()->sliderPosition() == activeRidersTable->verticalScrollBar()->maximum()) {
                scrollToBottomRequired = true;
            }
            activeRidersTable->insertRow(r);
            activeRidersTable->setRowHeight(r, 20);
            if (scrollToBottomRequired) {
                activeRidersTable->scrollToBottom();
            }
            activeRidersTable->setItem(r, AT_NAME, new QTableWidgetItem());
            activeRidersTable->item(r, AT_NAME)->setText("????");

            activeRidersTable->setItem(r, AT_LAPCOUNT, new QTableWidgetItem());
            activeRidersTable->setItem(r, AT_LAPSPEED, new QTableWidgetItem());
            activeRidersTable->setItem(r, AT_BESTLAPSPEED, new QTableWidgetItem());
            activeRidersTable->setItem(r, AT_DISTANCE, new QTableWidgetItem());
            activeRidersTable->setItem(r, AT_AVERAGESPEED, new QTableWidgetItem());
        }

        // activeRidersListIndex now points to new entry in activeRidersList and
        // activeRidersTableIndex points to entry in activeRidersTable


        // Update activeRidersTable entries

        if (!firstCrossing && !firstCrossingAfterBreak && (activeRidersTableIndex >= 0)) {
            int r = activeRidersTableIndex;

            // Lap count and speed

            activeRidersTable->item(r, AT_LAPCOUNT)->setData(Qt::DisplayRole, rider->lapCount);
            activeRidersTable->item(r, AT_LAPSPEED)->setData(Qt::DisplayRole, lapSpeed(rider->lapSec, rider->lapM));

            // Best lap speed

            if (rider->bestLapSec > 0.)
                activeRidersTable->item(r, AT_BESTLAPSPEED)->setData(Qt::DisplayRole, lapSpeed(rider->bestLapSec, rider->bestLapM));
            else
                activeRidersTable->item(r, AT_BESTLAPSPEED)->setData(Qt::DisplayRole, "");

            // Distance

            activeRidersTable->item(r, AT_DISTANCE)->setData(Qt::DisplayRole, rider->totalM / 1000.);
            if (rider->thisMonth.lapCount > 0) {
                activeRidersTable->item(r, AT_DISTANCETHISMONTH)->setData(Qt::DisplayRole, rider->thisMonth.totalM / 1000.);
                activeRidersTable->item(r, AT_AVERAGESPEEDTHISMONTH)->setData(Qt::DisplayRole, lapSpeed(rider->thisMonth.totalSec, rider->thisMonth.totalM));
            }

            // Average speed

            if (rider->lapCount > 0)
                activeRidersTable->item(r, AT_AVERAGESPEED)->setData(Qt::DisplayRole, lapSpeed(rider->totalSec, rider->totalM));
            else
                activeRidersTable->item(r, AT_AVERAGESPEED)->setData(Qt::DisplayRole, "");

            // All time distance

            activeRidersTable->item(r, AT_DISTANCEALLTIME)->setData(Qt::DisplayRole, rider->allTime.totalM / 1000.);

        }


        // Add lap to database

        int lapmsec = (int)(rider->lapSec * 1000.);
        dbase.addLap(rider->tagId.toLatin1(), QDateTime::currentDateTime().date().year(), QDateTime::currentDateTime().date().month(), QDateTime::currentDateTime().date().day(), QTime::currentTime().hour(), QTime::currentTime().minute(), QTime::currentTime().second(), lapmsec, rider->lapM);


        // Loop through entries in activeRiders table and flag riders on break

//        for (int i=0; i<ui->activeRidersTableWidget)


        // Re-enable sorting on lapsTable if enabled and lapsTable is not really large

        if (ui->lapsTableWidget->rowCount() < lapsTableMaxSizeWithSort) {
            ui->lapsTableWidget->setSortingEnabled(lapsTableSortingEnabled);
        }
        else if (ui->lapsTableWidget->rowCount() == lapsTableMaxSizeWithSort) {
            ui->lapsTableWidget->sortByColumn(LT_TIMESTAMP, Qt::AscendingOrder);
            ui->lapsTableWidget->setSortingEnabled(false);
            ui->lapsTableSortedCheckBox->setChecked(false);
            ui->lapsTableSortedCheckBox->setEnabled(false);
        }

        // Re-enable sorting on activeRidersTable if enabled

        ui->activeRidersTableWidget->setSortingEnabled(activeRidersTableSortingEnabled);
    }

    catch (const char *p) {
        qDebug() << "Exception caught:" << QString(p);
    }

    catch (QString s) {
        qDebug() << "Exception caught:" << s;
    }

    catch (...) {
        qDebug() << "Exception caught";
    }


    // Unlock tables mutex

    lapsTableMutex.unlock();
    activeRidersTableMutex.unlock();


    // lapCount is total laps all riders

    ui->lapCountLineEdit->setText(s.setNum(tagCount));
    ui->riderCountLineEdit->setText(s.setNum(ui->activeRidersTableWidget->rowCount()));
}



float MainWindow::lapSpeed(float lapSec, float lapM) {
    double lapSpeed = 0.;
    if (lapSec > 0.)
        lapSpeed = lapM / (lapSec / 3600.) / 1000.;
    return lapSpeed;
}



void MainWindow::onNewLogMessage(QString s) {
    ui->messagesPlainTextEdit->appendPlainText(QDateTime::currentDateTime().toString("yyyy-MM-dd_hh:mm:ss ") + s);
}



// **********************************************************************************************************

void MainWindow::onDbaseSearchPushButtonClicked(void) {
    QByteArray tagId = ui->deskTagIdLineEdit->text().toLatin1();
    QString firstName = ui->deskFirstNameLineEdit->text();
    QString lastName = ui->deskLastNameLineEdit->text();

    // If tagId contains entry, search based on only that.
    // If found, enter name.  Otherwise clear name.

    if (!tagId.isEmpty()) {
        int rc = dbase.findNameFromTagId(tagId, &firstName, &lastName);
        if (rc == 0) {
            ui->deskFirstNameLineEdit->setText(firstName);
            ui->deskLastNameLineEdit->setText(lastName);
        }
        else {
            ui->deskFirstNameLineEdit->clear();
            ui->deskLastNameLineEdit->clear();
        }
    }

    // Else if first or last name givem, search on that.  Don't clear names on search fail.

    else if (!lastName.isEmpty() || !firstName.isEmpty()) {
        int id = dbase.getIdFromName(firstName, lastName);
        if (id > 0) {
            dbase.getAllFromId(id, &tagId, &firstName, &lastName);
            ui->deskTagIdLineEdit->setText(tagId);
            ui->deskFirstNameLineEdit->setText(firstName);
            ui->deskLastNameLineEdit->setText(lastName);
        }
    }

    updateDbaseButtons();
}



void MainWindow::onDbaseAddPushButtonClicked(void) {
    QByteArray tagId = ui->deskTagIdLineEdit->text().toLatin1();
    QByteArray firstName = ui->deskFirstNameLineEdit->text().toLatin1();
    QByteArray lastName = ui->deskLastNameLineEdit->text().toLatin1();

    int rc = dbase.addTagId(tagId, firstName, lastName);
    if (rc != 0) {
        guiCritical(dbase.errorText());
        return;
    }

    onDbaseClearPushButtonClicked();

    namesModel->addName(tagId, firstName, lastName);    // todo: check for success

    updateDbaseButtons();
}



void MainWindow::onDbaseClearPushButtonClicked(void) {
    ui->deskTagIdLineEdit->clear();
    ui->deskFirstNameLineEdit->clear();
    ui->deskLastNameLineEdit->clear();

    updateDbaseButtons();
}



void MainWindow::onDbaseRemovePushButtonClicked(void) {
    QMessageBox::StandardButtons b = guiQuestion("You are about to remove this tag from the database.  Press Ok to continue.", QMessageBox::Ok | QMessageBox::Abort);
    if (b == QMessageBox::Ok) {
        // Remove from table, get index into tagIdList, remove
        int rc = dbase.removeTagId(ui->deskTagIdLineEdit->text().toLatin1());
        if (rc == 0) {
            ui->deskTagIdLineEdit->clear();
            ui->deskFirstNameLineEdit->clear();
            ui->deskLastNameLineEdit->clear();
        }
    }

    updateDbaseButtons();
}



void MainWindow::onDbaseUpdatePushButtonClicked(void) {
    QByteArray tagId = ui->deskTagIdLineEdit->text().toLatin1();
    QByteArray firstName = ui->deskFirstNameLineEdit->text().toLatin1();
    QByteArray lastName = ui->deskLastNameLineEdit->text().toLatin1();
    int rc = dbase.updateTagId(tagId, firstName, lastName);
    if (rc == 0)
        onDbaseClearPushButtonClicked();
    else
        guiCritical(dbase.errorText());

//    namesModel->loadNames();
    updateDbaseButtons();
}



void MainWindow::onDbaseReadPushButtonClicked(bool state) {
    if (state) {
        ui->deskReadPushButton->setChecked(true);
        ui->deskTagIdLineEdit->clear();
        ui->deskFirstNameLineEdit->clear();
        ui->deskLastNameLineEdit->clear();
        if (deskReader)
            deskReader->blockSignals(false);
    }
    else {
        if (deskReader)
            deskReader->blockSignals(true);
        ui->deskReadPushButton->setChecked(false);
    }

    updateDbaseButtons();
}


void MainWindow::onNewDeskTag(CTagInfo tagInfo) {
    deskReader->blockSignals(true);
    ui->deskTagIdLineEdit->setText(tagInfo.tagId);
    QString firstName;
    QString lastName;
    if (!tagInfo.tagId.isEmpty()) {
        int rc = dbase.findNameFromTagId(tagInfo.tagId, &firstName, &lastName);
        if (rc == 0) {
            ui->deskFirstNameLineEdit->setText(firstName);
            ui->deskLastNameLineEdit->setText(lastName);
        }
        else {
            ui->deskFirstNameLineEdit->clear();
            ui->deskLastNameLineEdit->clear();
        }
    }
    ui->deskReadPushButton->setChecked(false);

    updateDbaseButtons();
}



void MainWindow::onDbaseTagIdTextChanged(QString) {
    updateDbaseButtons();
}



void MainWindow::onDbaseFirstNameTextChanged(QString) {
    updateDbaseButtons();
}



void MainWindow::onDbaseLastNameTextChanged(QString) {
    updateDbaseButtons();
}



void MainWindow::updateDbaseButtons(void) {

//    qDebug() << ui->deskTagIdLineEdit->text();

    // If ReadTag pushed, all others should be disabled

    if (ui->deskReadPushButton->isChecked()) {
        ui->deskSearchPushButton->setEnabled(false);
        ui->deskAddPushButton->setEnabled(false);
        ui->deskClearPushButton->setEnabled(false);
        ui->deskRemovePushButton->setEnabled(false);
        ui->deskUpdatePushButton->setEnabled(false);
        return;
    }

    // Search is enabled when TagId, FirstName or LastName is filled

    if (!ui->deskTagIdLineEdit->text().isEmpty() || !ui->deskFirstNameLineEdit->text().isEmpty() || !ui->deskLastNameLineEdit->text().isEmpty())
        ui->deskSearchPushButton->setEnabled(true);
    else
        ui->deskSearchPushButton->setEnabled(false);

    // Add is enabled when TagId, FirstName and LastName are filled

    if (!ui->deskTagIdLineEdit->text().isEmpty() && !ui->deskFirstNameLineEdit->text().isEmpty() && !ui->deskLastNameLineEdit->text().isEmpty())
        ui->deskAddPushButton->setEnabled(true);
    else
        ui->deskAddPushButton->setEnabled(false);

    // Clear is enabled when any of TagId, FirstName or LastName are filled

    if (!ui->deskTagIdLineEdit->text().isEmpty() || !ui->deskFirstNameLineEdit->text().isEmpty() || !ui->deskLastNameLineEdit->text().isEmpty())
        ui->deskClearPushButton->setEnabled(true);
    else
        ui->deskClearPushButton->setEnabled(false);

    // Remove is enabled when TagId, FirstName and LastName are filled

    if (!ui->deskTagIdLineEdit->text().isEmpty() && !ui->deskFirstNameLineEdit->text().isEmpty() && !ui->deskLastNameLineEdit->text().isEmpty())
        ui->deskRemovePushButton->setEnabled(true);
    else
        ui->deskRemovePushButton->setEnabled(false);

    // Update is enabled when TagId, FirstName and LastName are filled

    if (!ui->deskTagIdLineEdit->text().isEmpty() && !ui->deskFirstNameLineEdit->text().isEmpty() && !ui->deskLastNameLineEdit->text().isEmpty())
        ui->deskUpdatePushButton->setEnabled(true);
    else
        ui->deskUpdatePushButton->setEnabled(false);

}



void MainWindow::guiCritical(QString s) {
    QMessageBox::critical(this, "llrplaps Critical Error", s, QMessageBox::Ok);
}


void MainWindow::guiInformation(QString s) {
    QMessageBox::information(this, QCoreApplication::applicationName() + "\n\n", s, QMessageBox::Ok);
}


QMessageBox::StandardButtons MainWindow::guiQuestion(QString s, QMessageBox::StandardButtons b) {
    return QMessageBox::question(this, "llrplaps Question", s, b);
}


