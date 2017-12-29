// **************************************************************************************************
//
// mainwindow.cpp
//
// llrplaps
//
// ***************************************************************************************************

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
#define AT_DISTANCE         2
#define AT_LAPSPEED         3
#define AT_BESTLAPSPEED     4
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
#define CW_MEMBERSHIPNUMBER 80



// Personal records to note (?):
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







// *****************************************************************************
// CMembershipTableModel
//
CMembershipTableModel::CMembershipTableModel(QObject *parent) : QAbstractTableModel(parent) {
    tagIdList.clear();
    firstNameList.clear();
    lastNameList.clear();
    membershipList.clear();

    QSqlQuery query;
    query.prepare("select * from names");
    if (!query.exec())
        return;

    int idTagId = query.record().indexOf("tagId");
    int idFirst = query.record().indexOf("firstName");
    int idLast = query.record().indexOf("lastName");
    int idMembership = query.record().indexOf("membershipNumber");
    while (query.next()) {
        tagIdList.append(query.value(idTagId).toString());
        firstNameList.append(query.value(idFirst).toString());
        lastNameList.append(query.value(idLast).toString());
        membershipList.append(query.value(idMembership).toInt());
    }
    QModelIndex topLeft = index(0, 0);
    QModelIndex bottomRight = index(tagIdList.size() - 1, 0);
    emit dataChanged(topLeft, bottomRight);
}



bool CMembershipTableModel::add(const QByteArray &tagId, const QString &firstName, const QString &lastName, const int membershipNumber) {
    int row = rowCount();
    insertRows(row, 1);
    setData(index(row, 0), tagId, Qt::EditRole);
    setData(index(row, 1), firstName, Qt::EditRole);
    setData(index(row, 2), lastName, Qt::EditRole);
    setData(index(row, 3), membershipNumber, Qt::EditRole);
    QModelIndex topLeft = index(row, 0);
    QModelIndex bottomRight = index(row, 2);
    emit dataChanged(topLeft, bottomRight);
    return true;
}



bool CMembershipTableModel::remove(const QByteArray &tagId) {
    int row = tagIdList.indexOf(tagId);
    if (!removeRows(row, 1))
        return false;
    QModelIndex topLeft = index(row, 0);
    QModelIndex bottomRight = index(rowCount() - 1, 2);
    emit dataChanged(topLeft, bottomRight);
    return true;
}



bool CMembershipTableModel::insertRows(int position, int rows, const QModelIndex &/*parent*/) {
    beginInsertRows(QModelIndex(), position, position + rows - 1);
    for (int row = 0; row < rows; ++row) {
        tagIdList.insert(position, QString());
        firstNameList.insert(position, QString());
        lastNameList.insert(position, QString());
        membershipList.insert(position, 0);
    }
    endInsertRows();
    QModelIndex topLeft = index(position, 0);
    QModelIndex bottomRight = index(position, 2);
    emit dataChanged(topLeft, bottomRight);
    return true;
}



bool CMembershipTableModel::removeRows(int position, int rows, const QModelIndex &/*parent*/) {
    beginRemoveRows(QModelIndex(), position, position + rows - 1);
    for (int row = 0; row < rows; ++row) {
        tagIdList.removeAt(position);
        firstNameList.removeAt(position);
        lastNameList.removeAt(position);
        membershipList.removeAt(position);
    }
    endRemoveRows();
    return true;
}



int CMembershipTableModel::rowCount(const QModelIndex &/*parent*/) const {
    return tagIdList.size();

}



int CMembershipTableModel::columnCount(const QModelIndex &/*parent*/) const {
    return 4;
}



QVariant CMembershipTableModel::data(const QModelIndex &index, int role) const {
    int row = index.row();
    int col = index.column();

    switch (role) {
    case Qt::DisplayRole:
        switch (col) {
        case 0:
            return tagIdList[row];
        case 1:
            return firstNameList[row];
        case 2:
            return lastNameList[row];
        case 3:
            return membershipList[row];
        }
        break;
    case Qt::FontRole:
        break;
    case Qt::BackgroundRole:
        break;
    case Qt::TextAlignmentRole:
        return Qt::AlignLeft + Qt::AlignVCenter;
    }

    return QVariant();
}



QVariant CMembershipTableModel::headerData(int section, Qt::Orientation orientation, int role) const {
    switch (role) {
    case Qt::DisplayRole:
        if (orientation == Qt::Horizontal) {
            switch (section) {
            case 0:
                return QString("Tag Id");
            case 1:
                return QString("First Name");
            case 2:
                return QString("Last Name");
            case 3:
                return QString("Membership Number");
            }
        }
    }
    return QVariant();
}



bool CMembershipTableModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    int row = index.row();
    int col = index.column();
    if (role == Qt::EditRole) {
        switch (col) {
        case 0:
            tagIdList[row] = value.toString();
            return true;
        case 1:
            firstNameList[row] = value.toString();
            return true;
        case 2:
            lastNameList[row] = value.toString();
            return true;
        case 3:
            membershipList[row] = value.toInt();
            return true;
        }
    }
    return false;
}



Qt::ItemFlags CMembershipTableModel::flags(const QModelIndex &index) const {
    return QAbstractTableModel::flags(index);
}



// ********************************************************************************************
// CLapsTableModel
//
CLapsTableModel::CLapsTableModel(QObject *parent) : QAbstractTableModel(parent) {
    mainWindow = (MainWindow *)parent;
}



int CLapsTableModel::rowCount(const QModelIndex &/*parent*/) const {
    return nameList.size();

}



int CLapsTableModel::columnCount(const QModelIndex &/*parent*/) const {
    return 7;
}



bool CLapsTableModel::insertRows(int position, int rows, const QModelIndex &/*parent*/) {
    if (rows != 1) {
        mainWindow->guiCritical("Trying to insert more than one row in CLapsTable");
        return false;
    }

    beginInsertRows(QModelIndex(), position, position + rows - 1);
    for (int row = 0; row < rows; ++row) {
        nameList.insert(position, QString());
        lapList.insert(position, 0);
        timeList.insert(position, QString());
        timeStampList.insert(position, 0);
        lapSecList.insert(position, 0.);
        lapSpeedList.insert(position, 0.);
        commentList.insert(position, QString());
    }
    endInsertRows();
    return true;
}



bool CLapsTableModel::removeRows(int position, int rows, const QModelIndex &/*parent*/) {
    beginRemoveRows(QModelIndex(), position, position + rows - 1);
    for (int i=0; i<rows; i++) {
        nameList.removeAt(position);
        lapList.removeAt(position);
        timeList.removeAt(position);
        timeStampList.removeAt(position);
        lapSecList.removeAt(position);
        lapSpeedList.removeAt(position);
        commentList.removeAt(position);
    }
    endRemoveRows();
    return true;
}



QVariant CLapsTableModel::data(const QModelIndex &index, int role) const {
    QString s;
    int row = index.row();
    int col = index.column();
    if (row >= nameList.size()) {
        mainWindow->guiCritical(s.sprintf("Specified row (%d) too large for lapsTable data", row));
        return QVariant();
    }

    switch (role) {
    case Qt::DisplayRole:
        switch (col) {
        case LT_NAME:
            return nameList[row];
        case LT_LAPCOUNT:
            return lapList[row];
        case LT_DATETIME:
            return timeList[row];
        case LT_TIMESTAMP:
            return timeStampList[row];
        case LT_LAPTIME:
            if (lapSecList[row] == 0.) return QString();
            else return lapSecList[row];
        case LT_LAPSPEED:
            if (lapSpeedList[row] == 0.) return QString();
            else return lapSpeedList[row];
        case LT_COMMENT:
            return commentList[row];
        }
        break;
    case Qt::FontRole:
        break;
    case Qt::BackgroundRole:
        break;
    case Qt::TextAlignmentRole:
        switch (col) {
        case LT_NAME:
        case LT_COMMENT:
            return Qt::AlignLeft + Qt::AlignVCenter;
        default:
            return Qt::AlignHCenter + Qt::AlignVCenter;
        }
    }

    return QVariant();
}



QVariant CLapsTableModel::headerData(int section, Qt::Orientation orientation, int role) const {
    switch (role) {
    case Qt::DisplayRole:
        if (orientation == Qt::Horizontal) {
            switch (section) {
            case LT_NAME:
                return QString("Name");
            case LT_LAPCOUNT:
                return QString("Lap");
            case LT_DATETIME:
                return QString("Time");
            case LT_TIMESTAMP:
                return QString("TimeStamp");
            case LT_LAPTIME:
                return QString("Lap Sec");
            case LT_LAPSPEED:
                return QString("Lap km/h");
            case LT_COMMENT:
                return QString("Comment");
            }
        }
        break;
    }

    return QVariant();
}



Qt::ItemFlags CLapsTableModel::flags(const QModelIndex &index) const {
    return QAbstractTableModel::flags(index);
}



bool CLapsTableModel::addEntry(CRider rider) {
    QString time = QTime::currentTime().toString();
    float speed = 0.;
    if (rider.lapSec > 0.) speed = rider.lapM / rider.lapSec / 1000. * 3600.;

    bool scrollToBottomRequired = false;
    if (mainWindow->ui->lapsTableView->verticalScrollBar()->sliderPosition() == mainWindow->ui->lapsTableView->verticalScrollBar()->maximum())
        scrollToBottomRequired = true;

    int row = nameList.size();
    insertRows(row, 1);
    nameList[row] = rider.name;
    lapList[row] = rider.lapCount;
    timeList[row] = time;
    timeStampList[row] = rider.previousTimeStampUSec;
    lapSecList[row] = rider.lapSec;
    lapSpeedList[row] = speed;
    commentList[row] = rider.comment;

    if (scrollToBottomRequired)
        mainWindow->ui->lapsTableView->scrollToBottom();

    return true;
}




void CLapsTableModel::purgeTable(void) {
    unsigned long long currentTimeUSec = QDateTime::currentMSecsSinceEpoch() * 1000;

    // Loop through all active riders and see which are geting old

    for (int i=timeStampList.size()-1; i>=0; i--) {
        long long inactiveSec = (currentTimeUSec - timeStampList[i]) / 1000000;
        if (inactiveSec >= (2 * mainWindow->tablePurgeIntervalSec)) {
            removeRows(i, 1);
        }
    }
}



// ***********************************************************************************************
// CActiveRidersTable
//
CActiveRidersTableModel::CActiveRidersTableModel(QObject *parent) : QAbstractTableModel(parent) {
    mainWindow = (MainWindow *)parent;
}



int CActiveRidersTableModel::rowCount(const QModelIndex &/*parent*/) const {
    return activeRidersList.size();

}



int CActiveRidersTableModel::columnCount(const QModelIndex &/*parent*/) const {
    return 12;
}



bool CActiveRidersTableModel::insertRows(int position, int rows, const QModelIndex &/*parent*/) {
    if (rows != 1) {
        mainWindow->guiCritical("Trying to insert more than one row in CActiveRidersTable");
        return false;
    }
    beginInsertRows(QModelIndex(), position, position + rows - 1);
    activeRidersList.append(CRider());
    endInsertRows();
    return true;
}



bool CActiveRidersTableModel::removeRows(int position, int rows, const QModelIndex &/*parent*/) {
    beginRemoveRows(QModelIndex(), position, position + rows - 1);
    for (int i=0; i<rows; i++) {
        activeRidersList.removeAt(position);
    }
    endRemoveRows();
    return true;
}



QVariant CActiveRidersTableModel::data(const QModelIndex &index, int role) const {
    QString s;
    int row = index.row();
    int col = index.column();
    if (row >= activeRidersList.size()) {
        mainWindow->guiCritical(s.sprintf("Specified row (%d) too large for activeRidersList", row));
        return QString();
    }
    const CRider *rider = &(activeRidersList.at(row));
    switch (role) {
    case Qt::DisplayRole:
        switch (col) {
        case AT_NAME:
            return rider->name;
        case AT_LAPCOUNT:
            return rider->lapCount;
        case AT_DISTANCE:
            return rider->totalM / 1000.;
        case AT_LAPSPEED:
            if (rider->lapSec > 0.)
                return rider->lapM / rider->lapSec * 3600. / 1000.;
            else
                return QString();
        case AT_BESTLAPSPEED:
            if (rider->bestLapSec > 0.)
                return rider->bestLapM / rider->bestLapSec * 3600. / 1000.;
            else
                return QString();
        case AT_AVERAGESPEED:
            if (rider->totalSec > 0.)
                return rider->totalM / rider->totalSec * 3600. / 1000.;
            else
                return QString();
        case AT_DISTANCETHISMONTH:
            return rider->thisMonth.totalM / 1000.;
        case AT_AVERAGESPEEDTHISMONTH:
            if (rider->thisMonth.totalSec > 0.)
                return rider->thisMonth.totalM / rider->thisMonth.totalSec * 3600. / 1000.;
            else
                return QString();
        case AT_DISTANCELASTMONTH:
            return rider->lastMonth.totalM / 1000.;
        case AT_AVERAGESPEEDLASTMONTH:
            if (rider->lastMonth.totalSec > 0.)
                return rider->lastMonth.totalM / rider->lastMonth.totalSec * 3600. / 1000.;
            else
                return QString();
        case AT_DISTANCEALLTIME:
            return rider->allTime.totalM / 1000.;
        case AT_COMMENT:
            return rider->comment;
        }
        break;
    case Qt::FontRole:
        break;
    case Qt::BackgroundRole:
        break;
    case Qt::TextAlignmentRole:
        switch (col) {
        case AT_NAME:
        case AT_COMMENT:
            return Qt::AlignLeft + Qt::AlignVCenter;
        default:
            return Qt::AlignHCenter + Qt::AlignVCenter;
        }
    }

    return QVariant();
}



QVariant CActiveRidersTableModel::headerData(int section, Qt::Orientation orientation, int role) const {
    switch (role) {
    case Qt::DisplayRole:
        if (orientation == Qt::Horizontal) {
            switch (section) {
            case AT_NAME:
                return QString("Name");
            case AT_LAPCOUNT:
                return QString("Laps");
            case AT_DISTANCE:
                return QString("km");
            case AT_LAPSPEED:
                return QString("L km/h");
            case AT_BESTLAPSPEED:
                return QString("Best L km/h");
            case AT_AVERAGESPEED:
                return QString("Ave km/h");
            case AT_DISTANCETHISMONTH:
                return QString("This M km");
            case AT_AVERAGESPEEDTHISMONTH:
                return QString("This M km/h");
            case AT_DISTANCELASTMONTH:
                return QString("Last M km");
            case AT_AVERAGESPEEDLASTMONTH:
                return QString("Last M km/h");
            case AT_DISTANCEALLTIME:
                return QString("Total km");
            case AT_COMMENT:
                return QString("Comment");
            }
        }
        break;
    }

    return QVariant();
}



bool CActiveRidersTableModel::setData(const QModelIndex &index, const QVariant &/*value*/, int role) {
    int row = index.row();
    switch (role) {
    case Qt::EditRole:
        QModelIndex topLeft(createIndex(row, 1));
        QModelIndex bottomRight(createIndex(activeRidersList.size() - 1, columnCount()));
        emit dataChanged(topLeft, bottomRight);
    }
    return true;
}



Qt::ItemFlags CActiveRidersTableModel::flags(const QModelIndex &index) const {
    return QAbstractTableModel::flags(index);
}



void CActiveRidersTableModel::newTrackTag(CTagInfo tagInfo) {

    // ActiveRidersList is the main list containing information of each active rider

    int activeRiderIndex = -1;
    bool firstCrossing = false;
    bool firstCrossingAfterBreak = false;


    // Check to see if tag is in activeRidersList and append new empty entry if not,
    // else set firstCrossing

    CRider *rider = NULL;
    for (int i=0; i<activeRidersList.size(); i++) {
        if (tagInfo.tagId == activeRidersList[i].tagId) {
            rider = &activeRidersList[i];
            activeRiderIndex = i;
            break;
        }
    }
    if (!rider) {
        bool scrollToBottomRequired = false;
        if (mainWindow->ui->activeRidersTableView->verticalScrollBar()->sliderPosition() == mainWindow->ui->activeRidersTableView->verticalScrollBar()->maximum())
            scrollToBottomRequired = true;
        insertRows(activeRidersList.size()-1, 1);
        if (scrollToBottomRequired)
            mainWindow->ui->activeRidersTableView->scrollToBottom();

        activeRiderIndex = activeRidersList.size() - 1;
        rider = &activeRidersList[activeRiderIndex];
        firstCrossing = true;
    }
    else {
        firstCrossing = false;
    }


    // If this is a new active rider try getting name from dbase, or default to tagId provided on tag

    QString name;
    if (firstCrossing) {   // New rider, so get name from dBase and calculate best times in each category
        QByteArray tagId;
        QString firstName;
        QString lastName;
        int membershipNumber = 0;
        int id = mainWindow->membershipDbase.getIdFromTagId(tagInfo.tagId);
        if (id > 0) {
            mainWindow->membershipDbase.getAllFromId(id, &tagId, &firstName, &lastName, &membershipNumber);
            name = firstName + " " + lastName;
        }
        else {
            name = tagInfo.tagId;
        }
        rider->name = name;
        rider->tagId = tagInfo.tagId;
        rider->previousTimeStampUSec = tagInfo.timeStampUSec;

        // Get prior stats for this rider

        mainWindow->lapsDbase.getStats(tagInfo.tagId, rider);
    }


    // If we have completed at least one full lap, update lap stats and thisMonth stats

    if (!firstCrossing) {

        // Calculate lap time.  If lap time is greater than maxAcceptableLapSec, rider must have taken a break so do not
        // calculate lap time

        float lapSec = (double)(tagInfo.timeStampUSec - rider->previousTimeStampUSec) / 1.e6;
        if (lapSec > mainWindow->maxAcceptableLapSec) {
            firstCrossingAfterBreak = true;
            rider->lapSec = 0.;
            rider->lapM = 0.;
        }
        else {
            firstCrossingAfterBreak = false;
            rider->lapCount++;
            rider->lapSec = lapSec;
            rider->lapM = mainWindow->trackLengthM[tagInfo.antennaId - 1];
//            float lapSpeed = 0.;
//            if (rider->lapSec > 0.) {
//                lapSpeed = rider->lapM / rider->lapSec / 1000. * 3600.;
//            }
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


    // Add a comment

    if (firstCrossing)
        rider->comment = "First lap";
    else if (firstCrossingAfterBreak)
        rider->comment = "First lap after break";
    else {
//        float speed = 0.;
//        if (rider->lapSec > 0.) {
//            speed = rider->lapM / rider->lapSec / 1000. * 3600.;
//        }
        rider->comment.clear();
    }


    // Populate activeRidersTableView entries

    setData(createIndex(activeRiderIndex, 0), 0, Qt::EditRole);


    // Add lap to database

    int lapmsec = (int)(rider->lapSec * 1000.);
    mainWindow->lapsDbase.addLap(rider->tagId.toLatin1(), QDateTime::currentDateTime().date().year(), QDateTime::currentDateTime().date().month(), QDateTime::currentDateTime().date().day(), QTime::currentTime().hour(), QTime::currentTime().minute(), QTime::currentTime().second(), lapmsec, rider->lapM);


    // Add to lapsTableView

    mainWindow->lapsTableModel->addEntry(*rider);

    // lapCount is total laps all riders

    QString s;
    mainWindow->ui->riderCountLineEdit->setText(s.setNum(activeRidersList.size()));
}



void CActiveRidersTableModel::purgeTable(void) {
    unsigned long long currentTimeUSec = QDateTime::currentMSecsSinceEpoch() * 1000;

    // Loop through all active riders and see which are geting old

    for (int i=activeRidersList.size()-1; i>=0; i--) {
        long long inactiveSec = (currentTimeUSec - activeRidersList[i].previousTimeStampUSec) / 1000000;
        if (inactiveSec >= (2 * mainWindow->tablePurgeIntervalSec)) {
            removeRows(i, 1);
        }
    }
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
    membershipTableModel = NULL;
    lapsTableModel = NULL;
    activeRidersTableModel = NULL;
    QCoreApplication::setApplicationName("LLRPLaps");
    QCoreApplication::setApplicationVersion("0.1");


    initializeSettingsPanel();
    bool initialized = true;

    ui->mainTitleLabel->setText(QCoreApplication::applicationName() + " " + QCoreApplication::applicationVersion());
    ui->leftTitleLabel->setText(ui->trackNameLineEdit->text());
    ui->rightTitleLabel->setText(QString());
    setWindowTitle(QCoreApplication::applicationName() + ": " + ui->trackNameLineEdit->text());


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
            initialized = false;
        }
    }


    // Get tablePurgeIntervalSec, the interval on which tables are purged of inactive riders

    tablePurgeIntervalSec = settings.value("tablePurgeIntervalHours").toFloat() * 3600;
    if (tablePurgeIntervalSec == 0) tablePurgeIntervalSec = 6 * 3600;
    if (tablePurgeIntervalSec < 10) tablePurgeIntervalSec = 10;
    ui->tablePurgeIntervalLineEdit->setText(s.setNum(tablePurgeIntervalSec / 3600));


    // If laps table has more than this number of entries, disable sorting to ensure responsive operation

    lapsTableMaxSizeWithSort = 10000;


    if (!initialized)
        guiCritical("Track configuration must be initialized (\"Settings\" tab) before application will work");


    // Get tag reader information from settings.  Leave IP empty for simulation (test) mode.

    int readerCounter = 0;
    if (initialized) {
        trackReader = new CReader(ui->trackReaderIP->text(), readerCounter++, CReader::track);
        deskReader = new CReader(ui->deskReaderIP->text(), readerCounter++, CReader::desk);
    }


    // Initialize member variables

    activeRidersTableSortingEnabled = true;
    lapsTableSortingEnabled = true;
    float nominalSpeedkmph = 32.0;              // Approximate, used to identified riders taking a break
    float nominalLapSec = (trackLengthM[0] / 1000.) / nominalSpeedkmph * 3600.;
    maxAcceptableLapSec = nominalLapSec * 2.;   // max acceptable time for lap.  If greater, rider must have left and returned to track


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


    // Move CReader objects to separate threads

    if (trackReader) {
        QThread *trackReaderThread = new QThread(this);
        trackReader->moveToThread(trackReaderThread);
        trackReader->thread = trackReaderThread;
        readerThreadList.append(trackReaderThread);
        connect(trackReaderThread, SIGNAL(started(void)), trackReader, SLOT(onStarted(void)));
        connect(trackReaderThread, SIGNAL(finished(void)), trackReaderThread, SLOT(deleteLater(void)));
        connect(trackReader, SIGNAL(newLogMessage(QString)), this, SLOT(onNewLogMessage(QString)));
        connect(trackReader, SIGNAL(connected(void)), this, SLOT(onReaderConnected(void)));
        connect(trackReader, SIGNAL(newTag(CTagInfo)), this, SLOT(onNewTrackTag(CTagInfo)));
        trackReaderThread->start();
    }

    if (deskReader) {
        QThread *deskReaderThread = new QThread(this);
        deskReader->moveToThread(deskReaderThread);
        deskReader->thread = deskReaderThread;
        readerThreadList.append(deskReaderThread);
        connect(deskReaderThread, SIGNAL(started(void)), deskReader, SLOT(onStarted(void)));
        connect(deskReaderThread, SIGNAL(finished(void)), deskReaderThread, SLOT(deleteLater(void)));
        connect(deskReader, SIGNAL(newLogMessage(QString)), this, SLOT(onNewLogMessage(QString)));
        connect(deskReader, SIGNAL(connected(void)), this, SLOT(onReaderConnected(void)));
        connect(deskReader, SIGNAL(newTag(CTagInfo)), this, SLOT(onNewDeskTag(CTagInfo)));
        deskReaderThread->start();
    }


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
    connect(ui->deskMembershipNumberLineEdit, SIGNAL(textChanged(QString)), this, SLOT(onDbaseMembershipNumberTextChanged(QString)));

    updateDbaseButtons();


    // Two databases are used.
    // membershipDbase contains track membership info for each rider.
    // lapsDbase contains a record of all laps for all riders.

    QString membershipDbaseFileName = "membership.sqlite";
    QString membersgipDbaseUserName = "abc";
    QString membersgipDbasePassword = "def";
    int rc = membershipDbase.open(membershipDbaseFileName, membersgipDbaseUserName, membersgipDbasePassword);
    if (rc != 0)
        guiCritical("Error opening membership database file \"" + membershipDbaseFileName + "\": " + membershipDbase.errorText() + ".\n\nWe will continue but rider names will not be displayed and new tags cannot be added.");


    QString lapsDbaseFileName = "laps.sqlite";
    QString lapsDbaseUserName = "abc";
    QString lapsDbasePassword = "def";
    rc = lapsDbase.open(lapsDbaseFileName, lapsDbaseUserName, lapsDbasePassword);
    if (rc != 0)
        guiCritical("Error opening laps database file \"" + lapsDbaseFileName + "\": " + lapsDbase.errorText() + ".\n\nWe will continue but lap times and statistics are not being recorded.");


    // Initialize names table

    membershipTableModel = new CMembershipTableModel(this);
    ui->namesTableView->setModel(membershipTableModel);
    ui->namesTableView->setColumnWidth(0, 200);
    ui->namesTableView->setColumnWidth(1, 200);
    ui->namesTableView->setColumnWidth(2, 200);

    ui->namesTableView->horizontalHeader()->setStretchLastSection(true);
    ui->namesTableView->horizontalHeader()->setStyleSheet("QHeaderView{font: bold;}");
    ui->namesTableView->setSortingEnabled(true);


    // Initialize laps table

    lapsTableModel = new CLapsTableModel(this);
    ui->lapsTableView->setModel(lapsTableModel);
    ui->lapsTableView->setColumnWidth(LT_NAME, CW_NAME);
    ui->lapsTableView->setColumnWidth(LT_LAPCOUNT, CW_LAPCOUNT);
    ui->lapsTableView->setColumnWidth(LT_DATETIME, CW_DATETIME);
    ui->lapsTableView->setColumnWidth(LT_TIMESTAMP, CW_TIMESTAMP);
    ui->lapsTableView->setColumnWidth(LT_LAPTIME, CW_SPEED);
    ui->lapsTableView->setColumnWidth(LT_LAPSPEED, CW_SPEED);

    ui->lapsTableView->setAlternatingRowColors(true);
    ui->lapsTableView->horizontalHeader()->setStretchLastSection(true);
    ui->lapsTableView->horizontalHeader()->setStyleSheet("QHeaderView{font: bold;}");
    ui->lapsTableView->setSortingEnabled(true);
    ui->lapsTableView->setEnabled(false);


//    connect(ui->lapsTableSortedCheckBox, SIGNAL(clicked(bool)), this, SLOT(onLapsTableSortedCheckBoxClicked(bool)));


    // Configure active riders table

    activeRidersTableModel = new CActiveRidersTableModel(this);
    ui->activeRidersTableView->setModel(activeRidersTableModel);
    ui->activeRidersTableView->setColumnWidth(AT_NAME, CW_NAME);
    ui->activeRidersTableView->setColumnWidth(AT_LAPCOUNT, CW_LAPCOUNT);
    ui->activeRidersTableView->setColumnWidth(AT_DISTANCE, CW_DISTANCE);
    ui->activeRidersTableView->setColumnWidth(AT_LAPSPEED, CW_SPEED);
    ui->activeRidersTableView->setColumnWidth(AT_BESTLAPSPEED, CW_SPEED);
    ui->activeRidersTableView->setColumnWidth(AT_AVERAGESPEED, CW_SPEED);
    ui->activeRidersTableView->setColumnWidth(AT_DISTANCETHISMONTH, CW_DISTANCE);
    ui->activeRidersTableView->setColumnWidth(AT_AVERAGESPEEDTHISMONTH, CW_SPEED);
    ui->activeRidersTableView->setColumnWidth(AT_DISTANCELASTMONTH, CW_DISTANCE);
    ui->activeRidersTableView->setColumnWidth(AT_AVERAGESPEEDLASTMONTH, CW_SPEED);
    ui->activeRidersTableView->setColumnWidth(AT_DISTANCEALLTIME, CW_DISTANCE);

    ui->activeRidersTableView->setAlternatingRowColors(true);
    ui->activeRidersTableView->horizontalHeader()->setStretchLastSection(true);
    ui->activeRidersTableView->horizontalHeader()->setStyleSheet("QHeaderView{font: bold;}");
    ui->activeRidersTableView->setSortingEnabled(true);
    ui->activeRidersTableView->setEnabled(false);

//    TestModel model;
//        QSortFilterProxyModel proxyModel;
//        proxyModel.setSourceModel(activeRidersTableModel);
//    .setModel( &proxyModel );

//    connect(ui->lapsTableSortedCheckBox, SIGNAL(clicked(bool)), this, SLOT(onLapsTableSortedCheckBoxClicked(bool)));


    // Start timer that will purge old riders from activeRidersTable

    connect(&purgeActiveRidersListTimer, SIGNAL(timeout(void)), this, SLOT(onPurgeActiveRidersList(void)));
    purgeActiveRidersListTimer.setInterval(tablePurgeIntervalSec * 1000);
    purgeActiveRidersListTimer.start();


    connect(ui->applySettingsPushButton, SIGNAL(clicked()), this, SLOT(onApplySettingsPushButtonClicked()));
    connect(ui->saveSettingsPushButton, SIGNAL(clicked()), this, SLOT(onSaveSettingsPushButtonClicked()));

}






MainWindow::~MainWindow() {
    qDebug() << "closing...";
    membershipDbase.close();
    lapsDbase.close();
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
    if (activeRidersTableModel)
        activeRidersTableModel->purgeTable();

    if (lapsTableModel)
        lapsTableModel->purgeTable();
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


    ui->lapsTableView->setEnabled(true);
    ui->activeRidersTableView->setEnabled(true);
}



void MainWindow::onLapsTableSortedCheckBoxClicked(bool /*state*/) {
//    if (!state) {
//        ui->lapsTableSortedCheckBox->setChecked(false);
//        ui->lapsTableSortedCheckBox->setCheckable(false);
//        ui->lapsTableWidget->setSortingEnabled(true);
//        ui->lapsTableWidget->sortByColumn(LT_TIMESTAMP);
//        ui->lapsTableWidget->setSortingEnabled(false);
//    }
}


void MainWindow::onActiveRidersTableSortedCheckBoxClicked(bool /*state*/) {
//    if (!state) {
//        ui->activeRidersTableSortedCheckBox->setChecked(false);
//        ui->activeRidersTableSortedCheckBox->setCheckable(false);
//        ui->activeRidersTableWidget->setSortingEnabled(true);
//        ui->activeRidersTableWidget->sortByColumn(LT_TIMESTAMP);
//        ui->activeRidersTableWidget->setSortingEnabled(false);
//    }
}



// Process new tag
//
void MainWindow::onNewTrackTag(CTagInfo tagInfo) {
    static int tagCount = 0;
    tagCount++;

    QString s;

    // Add string to messages window

    onNewLogMessage(s.sprintf("readerId=%d antennaId=%d timeStampUSec=%llu tagData=%s", tagInfo.readerId, tagInfo.antennaId, tagInfo.timeStampUSec, tagInfo.tagId.data()));

    // lapCount is total laps all riders

    ui->lapCountLineEdit->setText(s.setNum(tagCount));

    if (activeRidersTableModel)
        activeRidersTableModel->newTrackTag(tagInfo);
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
    QString s;
    QByteArray tagId = ui->deskTagIdLineEdit->text().toLatin1();
    QString firstName = ui->deskFirstNameLineEdit->text();
    QString lastName = ui->deskLastNameLineEdit->text();
    int membershipNumber = ui->deskMembershipNumberLineEdit->text().toInt();

    // If tagId contains entry, search based on only that.
    // If found, update fields.  Otherwise clear fields.

    if (!tagId.isEmpty() && firstName.isEmpty() && lastName.isEmpty() && (membershipNumber == 0)) {
        int id = membershipDbase.getIdFromTagId(ui->deskTagIdLineEdit->text().toLatin1());
        if (id > 0) {
            membershipDbase.getAllFromId(id, &tagId, &firstName, &lastName, &membershipNumber);
            ui->deskFirstNameLineEdit->setText(firstName);
            ui->deskLastNameLineEdit->setText(lastName);
            ui->deskMembershipNumberLineEdit->setText(s.setNum(membershipNumber));
        }
        else {
            ui->deskFirstNameLineEdit->clear();
            ui->deskLastNameLineEdit->clear();
            ui->deskMembershipNumberLineEdit->clear();
        }
    }

    // Else if first or last name given, search on that.  Don't clear names on search fail.

    else if ((!lastName.isEmpty() || !firstName.isEmpty()) && (membershipNumber == 0)) {
        int id = membershipDbase.getIdFromName(firstName, lastName);
        if (id > 0) {
            membershipDbase.getAllFromId(id, &tagId, &firstName, &lastName, &membershipNumber);
            ui->deskTagIdLineEdit->setText(tagId);
            ui->deskFirstNameLineEdit->setText(firstName);
            ui->deskLastNameLineEdit->setText(lastName);
            ui->deskMembershipNumberLineEdit->setText(s.setNum(membershipNumber));
        }
    }

    // Else if membership number is given, search on that.  Don't clear names on search fail.

    else if ((lastName.isEmpty() && firstName.isEmpty()) && (membershipNumber > 0)) {
        int id = membershipDbase.getIdFromMembershipNumber(membershipNumber);
        if (id > 0) {
            membershipDbase.getAllFromId(id, &tagId, &firstName, &lastName, &membershipNumber);
            ui->deskTagIdLineEdit->setText(tagId);
            ui->deskFirstNameLineEdit->setText(firstName);
            ui->deskLastNameLineEdit->setText(lastName);
            ui->deskMembershipNumberLineEdit->setText(s.setNum(membershipNumber));
        }
    }
    updateDbaseButtons();
}



void MainWindow::onDbaseAddPushButtonClicked(void) {

    // Check whether tagId is already in dbase

    int rc = membershipDbase.getIdFromTagId(ui->deskTagIdLineEdit->text().toLatin1());
    if (rc != 0) {
        guiCritical("Tag \"" + ui->deskTagIdLineEdit->text().toLatin1() + "\" already in database");
        return;
    }

    // Check whether first and last name is already in dbase

    rc = membershipDbase.getIdFromName(ui->deskFirstNameLineEdit->text(), ui->deskLastNameLineEdit->text());
    if (rc != 0) {
        guiCritical("Name \"" + ui->deskFirstNameLineEdit->text() + " " + ui->deskLastNameLineEdit->text() + "\" already in database");
        return;
    }

    // Add entry to database

    rc = membershipDbase.add(ui->deskTagIdLineEdit->text().toLatin1(), ui->deskFirstNameLineEdit->text(), ui->deskLastNameLineEdit->text(), ui->deskMembershipNumberLineEdit->text().toInt(), 0, QByteArray());
    if (rc != 0) {
        guiCritical(membershipDbase.errorText());
        return;
    }

    // Add to table

    if (!membershipTableModel->add(ui->deskTagIdLineEdit->text().toLatin1(), ui->deskFirstNameLineEdit->text(), ui->deskLastNameLineEdit->text(), ui->deskMembershipNumberLineEdit->text().toInt())) {
        guiCritical("Could not add entry to membershipTable");
        return;
    }

    onDbaseClearPushButtonClicked();
}



void MainWindow::onDbaseClearPushButtonClicked(void) {
    ui->deskTagIdLineEdit->clear();
    ui->deskFirstNameLineEdit->clear();
    ui->deskLastNameLineEdit->clear();
    ui->deskMembershipNumberLineEdit->clear();
    updateDbaseButtons();
}



void MainWindow::onDbaseRemovePushButtonClicked(void) {
    QMessageBox::StandardButtons b = guiQuestion("You are about to remove this tag from the database.  Press Ok to continue.", QMessageBox::Ok | QMessageBox::Abort);
    if (b == QMessageBox::Ok) {
        if (membershipDbase.removeTagId(ui->deskTagIdLineEdit->text().toLatin1()) != 0) {
            guiCritical("Error removing name from database");
            return;
        }
        if (!membershipTableModel->remove(ui->deskTagIdLineEdit->text().toLatin1())) {
            guiCritical("Error removing name from namesTable");
            return;
        }
    }

    ui->deskTagIdLineEdit->clear();
    ui->deskFirstNameLineEdit->clear();
    ui->deskLastNameLineEdit->clear();
    updateDbaseButtons();
}



void MainWindow::onDbaseUpdatePushButtonClicked(void) {
    QByteArray tagId = ui->deskTagIdLineEdit->text().toLatin1();
    QByteArray firstName = ui->deskFirstNameLineEdit->text().toLatin1();
    QByteArray lastName = ui->deskLastNameLineEdit->text().toLatin1();
    int membershipNumber = ui->deskMembershipNumberLineEdit->text().toInt();
    int ccaRegistration = 0;
    QByteArray email;
    int rc = membershipDbase.update(tagId, firstName, lastName, membershipNumber, ccaRegistration, email);
    if (rc == 0)
        onDbaseClearPushButtonClicked();
    else
        guiCritical(membershipDbase.errorText());

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
        int rc = membershipDbase.findNameFromTagId(tagInfo.tagId, &firstName, &lastName);
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


void MainWindow::onDbaseMembershipNumberTextChanged(QString) {
    updateDbaseButtons();
}



void MainWindow::updateDbaseButtons(void) {

    // If ReadTag pushed, all others should be disabled

    if (ui->deskReadPushButton->isChecked()) {
        ui->deskSearchPushButton->setEnabled(false);
        ui->deskAddPushButton->setEnabled(false);
        ui->deskClearPushButton->setEnabled(false);
        ui->deskRemovePushButton->setEnabled(false);
        ui->deskUpdatePushButton->setEnabled(false);
        return;
    }


    // Search is enabled when one of TagId, FirstName and/or LastName, or membershipNumber is filled

    if (!ui->deskTagIdLineEdit->text().isEmpty() && ui->deskFirstNameLineEdit->text().isEmpty() && ui->deskLastNameLineEdit->text().isEmpty() && ui->deskMembershipNumberLineEdit->text().isEmpty())
        ui->deskSearchPushButton->setEnabled(true);

    else if (ui->deskTagIdLineEdit->text().isEmpty() && !ui->deskFirstNameLineEdit->text().isEmpty() && ui->deskLastNameLineEdit->text().isEmpty() && ui->deskMembershipNumberLineEdit->text().isEmpty())
        ui->deskSearchPushButton->setEnabled(true);

    else if (ui->deskTagIdLineEdit->text().isEmpty() && !ui->deskFirstNameLineEdit->text().isEmpty() && !ui->deskLastNameLineEdit->text().isEmpty() && ui->deskMembershipNumberLineEdit->text().isEmpty())
        ui->deskSearchPushButton->setEnabled(true);

    else if (ui->deskTagIdLineEdit->text().isEmpty() && ui->deskFirstNameLineEdit->text().isEmpty() && !ui->deskLastNameLineEdit->text().isEmpty() && ui->deskMembershipNumberLineEdit->text().isEmpty())
        ui->deskSearchPushButton->setEnabled(true);

    else if (ui->deskTagIdLineEdit->text().isEmpty() && ui->deskFirstNameLineEdit->text().isEmpty() && ui->deskLastNameLineEdit->text().isEmpty() && !ui->deskMembershipNumberLineEdit->text().isEmpty())
        ui->deskSearchPushButton->setEnabled(true);

    else
        ui->deskSearchPushButton->setEnabled(false);


    // Add is enabled when all fields are filled

    if (!ui->deskTagIdLineEdit->text().isEmpty() && !ui->deskFirstNameLineEdit->text().isEmpty() && !ui->deskLastNameLineEdit->text().isEmpty() && !ui->deskMembershipNumberLineEdit->text().isEmpty())
        ui->deskAddPushButton->setEnabled(true);

    else
        ui->deskAddPushButton->setEnabled(false);


    // Clear is enabled when any of TagId, FirstName or LastName are filled

    if (!ui->deskTagIdLineEdit->text().isEmpty() || !ui->deskFirstNameLineEdit->text().isEmpty() || !ui->deskLastNameLineEdit->text().isEmpty() || !ui->deskMembershipNumberLineEdit->text().isEmpty())
        ui->deskClearPushButton->setEnabled(true);

    else
        ui->deskClearPushButton->setEnabled(false);


    // Remove is enabled when all fields are filled

    if (!ui->deskTagIdLineEdit->text().isEmpty() && !ui->deskFirstNameLineEdit->text().isEmpty() && !ui->deskLastNameLineEdit->text().isEmpty() && !ui->deskMembershipNumberLineEdit->text().isEmpty())
        ui->deskRemovePushButton->setEnabled(true);

    else
        ui->deskRemovePushButton->setEnabled(false);


    // Update is enabled when all fields are filled

    if (!ui->deskTagIdLineEdit->text().isEmpty() && !ui->deskFirstNameLineEdit->text().isEmpty() && !ui->deskLastNameLineEdit->text().isEmpty() && !ui->deskMembershipNumberLineEdit->text().isEmpty())
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


