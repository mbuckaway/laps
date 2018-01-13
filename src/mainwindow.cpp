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
#include <ltkcpp.h>
#include "creader.h"
#include "cdbase.h"
#include "main.h"
#include "csmtp.h"


// Columns in active riders table

#define AT_NAME                 0
#define AT_LAPCOUNT             1
#define AT_KM                   2
#define AT_LAPSEC               3
#define AT_LAPSPEED             4
#define AT_BESTLAPSPEED         5
#define AT_AVERAGESPEED         6
#define AT_KMTHISMONTH          7
#define AT_AVERAGESPEEDTHISMONTH 8
#define AT_KMLASTMONTH          9
#define AT_AVERAGESPEEDLASTMONTH 10
#define AT_KMALLTIME            11
#define AT_COMMENT              12


// Columns in lap table (listing of all laps)

#define LT_TIMESTAMP        0
#define LT_NAME             1
#define LT_DATETIME         2
#define LT_LAPCOUNT         3
#define LT_LAPTIME          4
#define LT_LAPSPEED         5
#define LT_COMMENT          6


// Column widths in each table

#define CW_NAME             150
#define CW_DATETIME         80
#define CW_TIMESTAMP        190
#define CW_LAPCOUNT         60
#define CW_SEC              80
#define CW_SPEED            90
#define CW_KM               80
#define CW_MEMBERSHIPNUMBER 80
#define CW_CAREGISTRATION   80
#define CW_EMAIL            80
#define CW_INDEX            50


// Personal records to note (?):
// - personal best lap
// - personal best 10 laps
// - personal best 1k
// - personal best 10k
// - personal best 20k
// - personal best 40k
// - session best lap
// - session best 10 laps
// - session best 1k
// - month best lap
// - month best 10 laps
// - month best 1k
// - alltime best lap
// - alltime best 10 laps
// - alltime best 1k
// - alltime best 10k
// - alltime best 20k
// - alltime best 40k


int testMode = false;






// *****************************************************************************
// CMembershipTableModel
//
CMembershipTableModel::CMembershipTableModel(QObject *parent) : QAbstractTableModel(parent) {
    mainWindow = (MainWindow *)parent;
    membershipInfoList.clear();

    QSqlQuery query(mainWindow->membershipDbase.dBase);
    query.prepare("select * from membershipTable");
    if (!query.exec())
        return;

    int idTagId = query.record().indexOf("tagId");
    int idFirst = query.record().indexOf("firstName");
    int idLast = query.record().indexOf("lastName");
    int idMembership = query.record().indexOf("membershipNumber");
    int idCaRegistration = query.record().indexOf("caRegistration");
    int idEmail = query.record().indexOf("eMail");
    int idSendReports = query.record().indexOf("sendReports");
    while (query.next()) {
        CMembershipInfo info;
        info.tagId = query.value(idTagId).toString();
        info.firstName = query.value(idFirst).toString();
        info.lastName = query.value(idLast).toString();
        info.membershipNumber = query.value(idMembership).toString();
        info.caRegistration = query.value(idCaRegistration).toString();
        info.eMail = query.value(idEmail).toString();
        info.sendReports = query.value(idSendReports).toInt();
        membershipInfoList.append(info);
    }
    QModelIndex topLeft = index(0, 0);
    QModelIndex bottomRight = index(membershipInfoList.size() - 1, 0);
    emit dataChanged(topLeft, bottomRight);
}



bool CMembershipTableModel::add(const CMembershipInfo &info) {
    int row = rowCount();
    insertRows(row, 1);
    setData(index(row, 0), info.tagId, Qt::EditRole);
    setData(index(row, 1), info.firstName, Qt::EditRole);
    setData(index(row, 2), info.lastName, Qt::EditRole);
    setData(index(row, 3), info.membershipNumber, Qt::EditRole);
    setData(index(row, 4), info.caRegistration.toUpper(), Qt::EditRole);
    setData(index(row, 5), info.eMail, Qt::EditRole);
    setData(index(row, 6), info.sendReports, Qt::EditRole);
    QModelIndex topLeft = index(row, 0);
    QModelIndex bottomRight = index(row, 5);
    emit dataChanged(topLeft, bottomRight);
    return true;
}



bool CMembershipTableModel::update(const CMembershipInfo &info) {
    int row = -1;
    for (int i=0; i<membershipInfoList.size(); i++) {
        if (membershipInfoList[i].tagId == info.tagId) {
            row = i;
            break;
        }
    }
    if (row < 0) {
        mainWindow->guiCritical("Could not find tagId in CMembershipInfoList in CMembershipTableModel::update");
        return false;
    }
    setData(index(row, 0), info.tagId, Qt::EditRole);
    setData(index(row, 1), info.firstName, Qt::EditRole);
    setData(index(row, 2), info.lastName, Qt::EditRole);
    setData(index(row, 3), info.membershipNumber, Qt::EditRole);
    setData(index(row, 4), info.caRegistration.toUpper(), Qt::EditRole);
    setData(index(row, 5), info.eMail, Qt::EditRole);
    setData(index(row, 6), info.sendReports, Qt::EditRole);
    QModelIndex topLeft = index(row, 0);
    QModelIndex bottomRight = index(row, 5);
    emit dataChanged(topLeft, bottomRight);
    return true;
}



bool CMembershipTableModel::remove(const QString &tagId) {
    int row = -1;
    for (int i=0; i<membershipInfoList.size(); i++) {
        if (membershipInfoList[i].tagId == tagId) {
            row = i;
            break;
        }
    }
    if (row < 0) {
        mainWindow->guiCritical("Could not find tagId in CMembershipInfoList in CMembershipTableModel::remove");
        return false;
    }
    if (!removeRows(row, 1))
        return false;
    QModelIndex topLeft = index(row, 0);
    QModelIndex bottomRight = index(rowCount() - 1, 2);
    emit dataChanged(topLeft, bottomRight);
    return true;
}



bool CMembershipTableModel::insertRows(int position, int count, const QModelIndex &/*parent*/) {
    beginInsertRows(QModelIndex(), position, position + count - 1);
    for (int row=0; row<count; row++)
        membershipInfoList.insert(position, CMembershipInfo());
    endInsertRows();
    QModelIndex topLeft = index(position, 0);
    QModelIndex bottomRight = index(position, 0);
    emit dataChanged(topLeft, bottomRight);
    return true;
}



bool CMembershipTableModel::removeRows(int position, int count, const QModelIndex &/*parent*/) {
    beginRemoveRows(QModelIndex(), position, position + count - 1);
    for (int row = 0; row < count; row++) {
        membershipInfoList.removeAt(position);
    }
    endRemoveRows();
    return true;
}



int CMembershipTableModel::rowCount(const QModelIndex &/*parent*/) const {
    return membershipInfoList.size();
}



int CMembershipTableModel::columnCount(const QModelIndex &/*parent*/) const {
    return 7;
}



QVariant CMembershipTableModel::data(const QModelIndex &index, int role) const {
    int row = index.row();
    int col = index.column();

    switch (role) {
    case Qt::DisplayRole:
        switch (col) {
        case 0:
            return membershipInfoList[row].tagId;
        case 1:
            return membershipInfoList[row].firstName;
        case 2:
            return membershipInfoList[row].lastName;
        case 3:
            return membershipInfoList[row].membershipNumber;
        case 4:
            return membershipInfoList[row].caRegistration;
        case 5:
            return membershipInfoList[row].eMail;
        case 6:
            return membershipInfoList[row].sendReports;
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
                return QString("Track Membership N");
            case 4:
                return QString("CA Registration");
            case 5:
                return QString("eMail");
            case 6:
                return QString("EmailReports");
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
            membershipInfoList[row].tagId = value.toString();
            return true;
        case 1:
            membershipInfoList[row].firstName = value.toString();
            return true;
        case 2:
            membershipInfoList[row].lastName = value.toString();
            return true;
        case 3:
            membershipInfoList[row].membershipNumber = value.toString();
            return true;
        case 4:
            membershipInfoList[row].caRegistration = value.toString();
            return true;
        case 5:
            membershipInfoList[row].eMail = value.toString();
            return true;
        case 6:
            membershipInfoList[row].sendReports = value.toBool();
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



bool CLapsTableModel::insertRows(int position, int count, const QModelIndex &/*parent*/) {
    beginInsertRows(QModelIndex(), position, position + count - 1);

    for (int row=0; row<count; row++) {
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



bool CLapsTableModel::removeRows(int position, int count, const QModelIndex &/*parent*/) {
    beginRemoveRows(QModelIndex(), position, position + count - 1);

    for (int i=0; i<count; i++) {
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
        case LT_TIMESTAMP:
            return timeStampList[row];
        case LT_NAME:
            return nameList[row];
        case LT_DATETIME:
            return timeList[row];
        case LT_LAPCOUNT:
            return lapList[row];
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
            case LT_TIMESTAMP:
                return QString("TimeStamp");
            case LT_NAME:
                return QString("Name");
            case LT_DATETIME:
                return QString("Time");
            case LT_LAPCOUNT:
                return QString("Lap");
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



bool CLapsTableModel::add(CRider rider) {
    QString time = QTime::currentTime().toString();
    float speed = 0.;
    if (rider.lapSec > 0.) speed = rider.lapM / rider.lapSec / 1000. * 3600.;

    bool scrollToBottomRequired = false;
    if (mainWindow->ui->lapsTableView->verticalScrollBar()->sliderPosition() == mainWindow->ui->lapsTableView->verticalScrollBar()->maximum())
        scrollToBottomRequired = true;

    int row = nameList.size();
    insertRows(row, 1);

    if (scrollToBottomRequired)
        mainWindow->ui->lapsTableView->scrollToBottom();

    nameList[row] = rider.name;
    if (rider.lapType == CRider::regularCrossing) {
        lapList[row] = rider.lapCount;
        timeList[row] = time;
        timeStampList[row] = rider.previousTimeStampUSec;
        lapSecList[row] = rider.lapSec;
        lapSpeedList[row] = speed;
    }
    else {
        lapList[row] = rider.lapCount;
        timeList[row] = time;
        timeStampList[row] = rider.previousTimeStampUSec;
        lapSecList[row] = 0.;
        lapSpeedList[row] = 0.;
    }

    // Assign a comment - this is just a duplicate of what is shown in activeRidersTable at this point, but could be
    // used to identify other conditions for debugging

    switch (rider.lapType) {
    case CRider::firstCrossing:
        commentList[row] = QString("first lap");
        break;
    case CRider::regularCrossing:
        commentList[row].clear();
        break;
    case CRider::onBreak:
        commentList[row] = QString("on break");
        break;
    case CRider::firstCrossingAfterBreak:
        commentList[row] = QString("first lap after break");
        break;
    case CRider::unknown:
        commentList[row] = QString("unknown lap type");
        break;
    }

    return true;
}




// Loop through all active riders and see which are geting old.
//
void CLapsTableModel::purgeTable(void) {
    unsigned long long currentTimeUSec = QDateTime::currentMSecsSinceEpoch() * 1000;

    bool scrollToBottomRequired = false;
    if (mainWindow->ui->lapsTableView->verticalScrollBar()->sliderPosition() == mainWindow->ui->lapsTableView->verticalScrollBar()->maximum())
        scrollToBottomRequired = true;

    for (int i=timeStampList.size()-1; i>=0; i--) {
        float inactiveHours = (float)(currentTimeUSec - timeStampList[i]) / 1000000. / 3600.;
        if (inactiveHours >= mainWindow->ui->tablePurgeIntervalDoubleSpinBox->value()) {
            removeRows(i, 1);
        }
    }

    if (scrollToBottomRequired)
        mainWindow->ui->lapsTableView->scrollToBottom();
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
    return 13;
}



bool CActiveRidersTableModel::insertRows(int position, int count, const QModelIndex &/*parent*/) {
    beginInsertRows(QModelIndex(), position, position + count - 1);
    for (int i=0; i<count; i++)
        activeRidersList.append(CRider());
    endInsertRows();
    return true;
}



bool CActiveRidersTableModel::removeRows(int position, int count, const QModelIndex &/*parent*/) {
    beginRemoveRows(QModelIndex(), position, position + count - 1);
    for (int i=0; i<count; i++)
        activeRidersList.removeAt(position);
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
            if (rider->lapCount > 0)
                return rider->lapCount;
            else
                return QString();
        case AT_LAPSEC:
            if (rider->lapSec > 0.)
                return rider->lapSec;
            else
                return QString();
        case AT_KM:
            if (rider->totalM > 0.)
                return rider->totalM / 1000.;
            else
                return QString();
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
        case AT_KMTHISMONTH:
            if (rider->thisMonth.totalM > 0.)
                return rider->thisMonth.totalM / 1000.;
            else
                return QString();
        case AT_AVERAGESPEEDTHISMONTH:
            if (rider->thisMonth.totalSec > 0.)
                return rider->thisMonth.totalM / rider->thisMonth.totalSec * 3600. / 1000.;
            else
                return QString();
        case AT_KMLASTMONTH:
            if (rider->lastMonth.totalM > 0.)
                return rider->lastMonth.totalM / 1000.;
            else
                return QString();
        case AT_AVERAGESPEEDLASTMONTH:
            if (rider->lastMonth.totalSec > 0.)
                return rider->lastMonth.totalM / rider->lastMonth.totalSec * 3600. / 1000.;
            else
                return QString();
        case AT_KMALLTIME:
            if (rider->allTime.totalM > 0.)
                return rider->allTime.totalM / 1000.;
            else
                return QString();
        case AT_COMMENT:
            switch (rider->lapType) {
            case CRider::firstCrossing:
                return QString("first lap");
            case CRider::regularCrossing:
//                if (rider->lapCount >= mainWindow->bestLapCountInSession)
//                    return QString("Best lap count in session");
                return rider->comment;
            case CRider::onBreak:
                return QString("on break");
            case CRider::firstCrossingAfterBreak:
                return QString("first lap after break");
            case CRider::unknown:
                return QString("unknown lap status");
            }
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
            case AT_LAPSEC:
                return QString("L Sec");
            case AT_KM:
                return QString("km");
            case AT_LAPSPEED:
                return QString("L km/h");
            case AT_BESTLAPSPEED:
                return QString("Best L km/h");
            case AT_AVERAGESPEED:
                return QString("Ave km/h");
            case AT_KMTHISMONTH:
                return QString("This M km");
            case AT_AVERAGESPEEDTHISMONTH:
                return QString("This M km/h");
            case AT_KMLASTMONTH:
                return QString("Last M km");
            case AT_AVERAGESPEEDLASTMONTH:
                return QString("Last M km/h");
            case AT_KMALLTIME:
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



// newTrackTag()
// This routine processes each tag as it arrives, calculating lap times, speed etc.

void CActiveRidersTableModel::newTrackTag(const CTagInfo &tagInfo) {
    try {
        bool nullTag = tagInfo.tagId.isEmpty();


        // Process nullTag, generated periodically to update table display

        if (nullTag) {
            for (int i=0; i<activeRidersList.size(); i++) {
                CRider *rider = &activeRidersList[i];
                float lapSec = (double)(tagInfo.timeStampUSec - rider->previousTimeStampUSec) / 1.e6;
                if (lapSec > mainWindow->maxAcceptableLapSec) {
                    rider->lapType = CRider::onBreak;
                    setData(createIndex(i, 0), 0, Qt::EditRole);
                }
            }
            return;
        }


        // ActiveRidersList is the main list containing information of each active rider
        // Set rider to point to appropriate entry if in list

        CRider *rider = NULL;
        int activeRiderIndex = -1;
        for (int i=0; i<activeRidersList.size(); i++) {
            if (tagInfo.tagId == activeRidersList[i].tagId) {
                rider = &activeRidersList[i];
                activeRiderIndex = i;
                break;
            }
        }

        // If new rider, append row to table which also adds blank entry to activeRidersList

        if (!rider) {
            bool scrollToBottomRequired = false;
            if (mainWindow->ui->activeRidersTableView->verticalScrollBar()->sliderPosition() == mainWindow->ui->activeRidersTableView->verticalScrollBar()->maximum())
                scrollToBottomRequired = true;
            insertRows(activeRidersList.size(), 1);
            if (scrollToBottomRequired)
                mainWindow->ui->activeRidersTableView->scrollToBottom();

            activeRiderIndex = activeRidersList.size() - 1;
            rider = &activeRidersList[activeRiderIndex];
            rider->lapType = CRider::firstCrossing;
        }
        else {

            // Init lapType to nextLapType from previous tag read

            rider->lapType = rider->nextLapType;
        }


        CMembershipInfo info;
        int id = 0;
        float lapSec = 0.;

        switch (rider->lapType) {

        case CRider::firstCrossing:

            // In first lap try getting name and prior stats from two dbases

            id = mainWindow->membershipDbase.getIdFromTagId(tagInfo.tagId);
            if (id > 0) {
                rider->inDbase = true;
                mainWindow->membershipDbase.getAllFromId(id, &info);
                rider->name = info.firstName + " " + info.lastName;
                if (info.sendReports && !info.eMail.isEmpty())
                    rider->reportStatus = 1;
                mainWindow->lapsDbase.getStats(tagInfo.tagId, rider);
            }
            else {
                rider->inDbase = false;
                rider->name = "???";
            }
            rider->lapCount = 0;
            rider->lapSec = 0.;
            rider->lapM = 0.;
            rider->bestLapSec = 0.;
            rider->bestLapM = 0.;
            rider->totalSec = 0.;
            rider->totalM = 0.;
            rider->tagId = tagInfo.tagId;
            rider->nextLapType = CRider::regularCrossing;
            break;

        case CRider::regularCrossing:

            // In riding lap update lap stats and thisMonth stats

            lapSec = (float)(tagInfo.timeStampUSec - rider->previousTimeStampUSec) / 1.e6;

            // If lap time is greater than maxAcceptableLapSec, rider must have stopped for a break and returned
            // to track

            if (lapSec > mainWindow->maxAcceptableLapSec) {
                rider->lapSec = 0.;
                rider->lapM = 0.;
                rider->lapType = CRider::firstCrossingAfterBreak;
                rider->nextLapType = CRider::regularCrossing;
            }
            else {
                rider->lapSec = lapSec;
                rider->lapM = mainWindow->trackLengthM[tagInfo.antennaId - 1];
                if ((rider->bestLapSec == 0.) || (rider->lapSec < rider->bestLapSec)) {
                    rider->bestLapSec = rider->lapSec;
                    rider->bestLapM = rider->lapM;
                }
                rider->lapCount++;
                rider->totalSec += rider->lapSec;
                rider->totalM += rider->lapM;

                // Update monthly and all-time stats only if rider is in dbase

                if (rider->inDbase) {
                    rider->thisMonth.lapCount++;
                    rider->thisMonth.totalSec += rider->lapSec;
                    rider->thisMonth.totalM += rider->lapM;

                    rider->allTime.lapCount++;
                    rider->allTime.totalM += rider->lapM;
                }
                rider->nextLapType = CRider::regularCrossing;
            }

            // Check for session bests

            if (rider->lapSec > 0.) {
                float lapSpeed = rider->lapM / 1000. / rider->lapSec * 3600.;
                if (lapSpeed > mainWindow->bestLapSpeedInSession)
                    mainWindow->bestLapSpeedInSession = lapSpeed;
            }
            if (rider->lapCount > mainWindow->bestLapCountInSession)
                mainWindow->bestLapCountInSession = rider->lapCount;
            break;

        case CRider::onBreak:

            // Do nothing while on break

            rider->nextLapType = CRider::firstCrossingAfterBreak;
            break;

        case CRider::firstCrossingAfterBreak:

            rider->lapSec = 0.;
            rider->lapM = 0.;
            rider->nextLapType = CRider::regularCrossing;
            break;

        case CRider::unknown:

            // Should never happen

            rider->nextLapType = CRider::unknown;
            rider->comment = "unknown";
            break;
        }

        rider->previousTimeStampUSec = tagInfo.timeStampUSec;


        // Populate activeRidersTableView entries

        setData(createIndex(activeRiderIndex, 0), 0, Qt::EditRole);


        // Add lap to dbase if rider is in dbase and this is a regular ridingLap

        if (rider->inDbase && (rider->lapType == CRider::regularCrossing)) {
            QDateTime currentDateTime(QDateTime::currentDateTime());
            unsigned int dateTime = CLapsDbase::dateTime2Int(currentDateTime.date().year(), currentDateTime.date().month(), currentDateTime.date().day(), currentDateTime.time().hour(), currentDateTime.time().minute(), currentDateTime.time().second());
            mainWindow->lapsDbase.addLap(*rider, dateTime);
        }


        // Add to lapsTableView

        mainWindow->lapsTableModel->add(*rider);

        // lapCount is total laps all riders

        QString s;
        mainWindow->ui->activeRiderCountLineEdit->setText(s.setNum(activeRidersList.size()));
    }

    catch (const QString &s) {
        mainWindow->guiCritical(s);
    }
    catch (const char *p) {
        mainWindow->guiCritical(QString(*p));
    }

}



// Loop through all active riders and see which are geting old
// Return a list of riders removed.
//
QList<CRider> CActiveRidersTableModel::purgeTable(void) {
    QString s;
    unsigned long long currentTimeUSec = QDateTime::currentMSecsSinceEpoch() * 1000;

    bool scrollToBottomRequired = false;
    if (mainWindow->ui->activeRidersTableView->verticalScrollBar()->sliderPosition() == mainWindow->ui->activeRidersTableView->verticalScrollBar()->maximum())
        scrollToBottomRequired = true;

    QList<CRider> purgedRiders;
    for (int i=activeRidersList.size()-1; i>=0; i--) {
        float inactiveHours = (float)(currentTimeUSec - activeRidersList[i].previousTimeStampUSec) / 1000000. / 3600.;
        if (inactiveHours >= mainWindow->ui->tablePurgeIntervalDoubleSpinBox->value()) {
            removeRows(i, 1);
        }
    }

    if (scrollToBottomRequired)
        mainWindow->ui->activeRidersTableView->scrollToBottom();

    mainWindow->ui->activeRiderCountLineEdit->setText(s.setNum(activeRidersList.size()));
    return purgedRiders;
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
    membershipProxyModel = NULL;
    lapsTableModel = NULL;
    lapsProxyModel = NULL;
    activeRidersTableModel = NULL;
    activeRidersProxyModel = NULL;
    logFile = NULL;
    QCoreApplication::setApplicationName("LLRPLaps");
    if (testMode)
        QCoreApplication::setApplicationVersion("0.1-TestMode");
    else
        QCoreApplication::setApplicationVersion("0.1");

    initializeSettingsPanel();
    bool initialized = true;
    tagInDbase = false;
    bestLapSpeedInSession = 0.;
    bestLapCountInSession = 0;

    ui->mainTitleLabel->setText(QCoreApplication::applicationName() + " " + QCoreApplication::applicationVersion());
    ui->leftTitleLabel->setText(ui->trackNameLineEdit->text());
    ui->rightTitleLabel->setText(QString());
    setWindowTitle(QCoreApplication::applicationName() + ": " + ui->trackNameLineEdit->text());


    // Open log file

    logFile = new QFile;
    if (!logFile)
        qDebug() << "Error creating log QFile";
    logFile->setFileName("llrplaps.log");
    int rc = logFile->open(QIODevice::Append | QIODevice::Text);
    if (!rc) {
        qDebug() << "log file not opened";
    }
    logTextStream = new QTextStream(logFile);
    if (!logTextStream)
        qDebug() << "Error creating log QTextStream";
    logTextStream->setCodec("UTF-8");


    // Get track length at position of each antenna.  Lap speed is estimated from
    // these values assuming rider maintains same position on track for entire lap.
    // If this value is not set correctly, speed and distance cycled will be in error.

    trackLengthM.append(settings.value("trackLength1M").toFloat());
    trackLengthM.append(settings.value("trackLength2M").toFloat());
    trackLengthM.append(settings.value("trackLength3M").toFloat());
    trackLengthM.append(settings.value("trackLength4M").toFloat());


    // tablePurgeInterval is the interval on which tables are purged of inactive riders
    // emailReportLatency is the time after being purged before an email report is sent

    ui->tablePurgeIntervalDoubleSpinBox->setMinimum(0.01);
    ui->tablePurgeIntervalDoubleSpinBox->setValue(settings.value("tablePurgeIntervalHours").toFloat());
    ui->emailReportLatencySpinBox->setMinimum(1);

    if (!initialized)
        guiCritical("Track configuration must be initialized (\"Settings\" tab) before application will work");


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
    connect(ui->deskCaRegistrationLineEdit, SIGNAL(textChanged(QString)), this, SLOT(onDbaseCaRegistrationTextChanged(QString)));
    connect(ui->deskEMailLineEdit, SIGNAL(textChanged(QString)), this, SLOT(onDbaseEMailTextChanged(QString)));

    updateDbaseButtons();


    // Two databases are used.
    // membershipDbase contains track membership info for each rider.
    // lapsDbase contains a record of all laps for all riders.

    QString membershipDbaseFileName;
    if (testMode) membershipDbaseFileName = "membershipTest.db";
    else membershipDbaseFileName = "membership.db";
    QString membershipDbaseUserName = "fcv";
    QString membershipDbasePassword = "fcv";
    rc = membershipDbase.open(membershipDbaseFileName, membershipDbaseUserName, membershipDbasePassword);
    if ((rc != 0) || !membershipDbase.isOpen())
        guiCritical(s.sprintf("Error %d opening membership database file \"%s\": %s.\n\nRider names will not be displayed and new tags cannot be added.", rc, membershipDbaseFileName.toLatin1().data(), membershipDbase.errorText().toLatin1().data()));


    QDate currentDate(QDate::currentDate());
    QString lapsDbaseFileName;
    if (testMode) lapsDbaseFileName = s.sprintf("lapsTest%d.db", currentDate.year());
    else lapsDbaseFileName = s.sprintf("laps%d.db", currentDate.year());
    QString lapsDbaseUserName = "fcv";
    QString lapsDbasePassword = "fcv";
    rc = lapsDbase.open(lapsDbaseFileName, lapsDbaseUserName, lapsDbasePassword);
    if ((rc != 0) || !lapsDbase.isOpen())
        guiCritical(s.sprintf("Error %d opening laps database file \"%s\": %s.\n\nWe will continue but lap times and statistics are not being recorded.", rc, lapsDbaseFileName.toLatin1().data(), lapsDbase.errorText().toLatin1().data()));

    // If this is January, open dbase from previous year if it exists

//    if (lapsDbase.isOpen() && (currentDate.month() == 1)) {
//        if (testMode) lapsDbaseFileName = s.sprintf("lapsTest%d.db", currentDate.year() - 1);
//        else lapsDbaseFileName = s.sprintf("laps%d.db", currentDate.year() - 1);
//        rc = lapsDbase.openLastYear(lapsDbaseFileName, lapsDbaseUserName, lapsDbasePassword);
//        if ((rc != 0) || !lapsDbase.isOpen())
//            guiCritical(s.sprintf("Error %d opening laps database file \"%s\": %s.\n\nWe will continue but lap times and statistics are not being recorded.", rc, lapsDbaseFileName.toLatin1().data(), lapsDbase.errorText().toLatin1().data()));

    // Initialize membership table

    membershipTableModel = new CMembershipTableModel(this);
    membershipProxyModel = new QSortFilterProxyModel;
    membershipProxyModel->setSourceModel(membershipTableModel);
    membershipProxyModel->setSortCaseSensitivity(Qt::CaseInsensitive);

    ui->namesTableView->setModel(membershipProxyModel);
    ui->namesTableView->setColumnWidth(0, 150);
    ui->namesTableView->setColumnWidth(1, 150);
    ui->namesTableView->setColumnWidth(2, 200);
    ui->namesTableView->setColumnWidth(3, 200);
    ui->namesTableView->setColumnWidth(4, 200);
    ui->namesTableView->setColumnWidth(5, 200);

    ui->namesTableView->setAlternatingRowColors(true);
    ui->namesTableView->horizontalHeader()->setStretchLastSection(true);
    ui->namesTableView->horizontalHeader()->setStyleSheet("QHeaderView{font: bold;}");
    ui->namesTableView->sortByColumn(2, Qt::AscendingOrder);     // must come before call to setSortingEnabled()
    ui->namesTableView->setSortingEnabled(true);

    // Disable membershipDbase tab if database not open

    if (!membershipDbase.isOpen())
        ui->tabWidget->setTabEnabled(3, false);


    // Initialize laps table

    lapsTableModel = new CLapsTableModel(this);
    lapsProxyModel = new QSortFilterProxyModel;
    lapsProxyModel->setSourceModel(lapsTableModel);
    lapsProxyModel->setSortCaseSensitivity(Qt::CaseInsensitive);
//    lapsProxyModel->setDynamicSortFilter(true);

    ui->lapsTableView->setModel(lapsProxyModel);
    ui->lapsTableView->setColumnWidth(LT_TIMESTAMP, CW_TIMESTAMP);
    ui->lapsTableView->setColumnWidth(LT_NAME, CW_NAME);
    ui->lapsTableView->setColumnWidth(LT_LAPCOUNT, CW_LAPCOUNT);
    ui->lapsTableView->setColumnWidth(LT_DATETIME, CW_DATETIME);
    ui->lapsTableView->setColumnWidth(LT_LAPTIME, CW_SPEED);
    ui->lapsTableView->setColumnWidth(LT_LAPSPEED, CW_SPEED);

    ui->lapsTableView->hideColumn(LT_TIMESTAMP);

    ui->lapsTableView->setAlternatingRowColors(true);
    ui->lapsTableView->horizontalHeader()->setStretchLastSection(true);
    ui->lapsTableView->horizontalHeader()->setStyleSheet("QHeaderView{font: bold;}");
//    ui->lapsTableView->sortByColumn(LT_TIMESTAMP, Qt::DescendingOrder);     // must come before call to setSortingEnabled()
//    ui->lapsTableView->setSortingEnabled(true);
    ui->lapsTableView->setEnabled(false);   // will be enabled when connected to reader

    ui->lapsTableSortEnableCheckBox->hide();
//    connect(ui->lapsTableSortEnableCheckBox, SIGNAL(clicked(bool)), this, SLOT(onLapsTableSortEnableCheckBoxClicked(bool)));


    // Configure active riders table

    activeRidersTableModel = new CActiveRidersTableModel(this);
    activeRidersProxyModel = new QSortFilterProxyModel;
    activeRidersProxyModel->setSourceModel(activeRidersTableModel);
    activeRidersProxyModel->setSortCaseSensitivity(Qt::CaseInsensitive);

    ui->activeRidersTableView->setModel(activeRidersProxyModel);
    ui->activeRidersTableView->setColumnWidth(AT_NAME, CW_NAME);
    ui->activeRidersTableView->setColumnWidth(AT_LAPCOUNT, CW_LAPCOUNT);
    ui->activeRidersTableView->setColumnWidth(AT_KM, CW_KM);
    ui->activeRidersTableView->setColumnWidth(AT_LAPSPEED, CW_SPEED);
    ui->activeRidersTableView->setColumnWidth(AT_BESTLAPSPEED, CW_SPEED);
    ui->activeRidersTableView->setColumnWidth(AT_AVERAGESPEED, CW_SPEED);
    ui->activeRidersTableView->setColumnWidth(AT_KMTHISMONTH, CW_KM);
    ui->activeRidersTableView->setColumnWidth(AT_AVERAGESPEEDTHISMONTH, CW_SPEED);
    ui->activeRidersTableView->setColumnWidth(AT_KMLASTMONTH, CW_KM);
    ui->activeRidersTableView->setColumnWidth(AT_AVERAGESPEEDLASTMONTH, CW_SPEED);
    ui->activeRidersTableView->setColumnWidth(AT_KMALLTIME, CW_KM);

    ui->activeRidersTableView->setAlternatingRowColors(true);
    ui->activeRidersTableView->horizontalHeader()->setStretchLastSection(true);
    ui->activeRidersTableView->horizontalHeader()->setStyleSheet("QHeaderView{font: bold;}");
    ui->activeRidersTableView->sortByColumn(1, Qt::DescendingOrder);     // must come before call to setSortingEnabled()
    ui->activeRidersTableView->setSortingEnabled(true);
    ui->activeRidersTableSortEnableCheckBox->setChecked(true);
    ui->activeRidersTableView->setEnabled(false);   // will be enabled when connected to reader

//    ui->activeRidersTableSortEnableCheckBox->hide();
    connect(ui->activeRidersTableSortEnableCheckBox, SIGNAL(clicked(bool)), this, SLOT(onActiveRidersTableSortEnableCheckBoxClicked(bool)));


    // Start timer that will purge old riders from activeRidersTable

    connect(&purgeActiveRidersListTimer, SIGNAL(timeout(void)), this, SLOT(onPurgeActiveRidersList(void)));
    purgeActiveRidersListTimer.setInterval((int)(ui->tablePurgeIntervalDoubleSpinBox->value() * 3600. * 1000. / 4.));
    purgeActiveRidersListTimer.start();


    connect(ui->saveSettingsPushButton, SIGNAL(clicked()), this, SLOT(onSaveSettingsPushButtonClicked()));
    connect(ui->eMailTestPushButton, SIGNAL(clicked()), this, SLOT(onEMailTestPushButtonClicked()));
    connect(ui->saveSessionsPushButton, SIGNAL(clicked()), this, SLOT(onSaveSessionsPushButtonClicked()));


    // Get tag reader information from settings.  Leave IP empty for simulation (test) mode.

    int readerCounter = 0;
    if (initialized) {
        trackReader = new CReader(ui->trackReaderIpLineEdit->text(), readerCounter++, CReader::track);
        deskReader = new CReader(ui->deskReaderIpLineEdit->text(), readerCounter++, CReader::desk);
    }


    // Move CReader objects to separate threads and start

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

}




MainWindow::~MainWindow() {
    membershipDbase.close();
    lapsDbase.close();
    for (int i=0; i<readerThreadList.size(); i++) {
        readerThreadList[i]->requestInterruption();
        readerThreadList[i]->wait();
        delete readerThreadList[i];
    }
    readerThreadList.clear();
    logFile->close();

    delete ui;
}



// ***********************************************************************************
//
// Member functions related to settings panel

void MainWindow::initializeSettingsPanel(void) {
    ui->trackNameLineEdit->setText(settings.value("trackName").toString());

    ui->trackLength1LineEdit->setText(settings.value("trackLength1M").toString());
    ui->trackLength2LineEdit->setText(settings.value("trackLength2M").toString());
    ui->trackLength3LineEdit->setText(settings.value("trackLength3M").toString());
    ui->trackLength4LineEdit->setText(settings.value("trackLength4M").toString());

    ui->trackReaderIpLineEdit->setText(settings.value("trackReaderIp").toString());

    ui->deskReaderIpLineEdit->setText(settings.value("deskReaderIp").toString());

    ui->smtpUsernameLineEdit->setText(settings.value("smtpUsername").toString());
    ui->smtpPasswordLineEdit->setText(settings.value("smtpPassword").toString());
    ui->smtpServerLineEdit->setText(settings.value("smtpServer").toString());
    ui->smtpPortLineEdit->setText(settings.value("smtpPort").toString());
    ui->emailSendReportsCheckBox->setChecked(settings.value("emailSendReports").toBool());
    ui->emailFromLineEdit->setText(settings.value("emailFrom").toString());
    ui->emailSubjectLineEdit->setText(settings.value("emailSubject").toString());
    ui->emailReportLatencySpinBox->setValue(settings.value("emailReportLatency").toInt());

    for (int row=0; row<ui->sessionsTableWidget->rowCount(); row++) {
        for (int col=0; col<ui->sessionsTableWidget->columnCount(); col++) {
            ui->sessionsTableWidget->setItem(row, col, new QTableWidgetItem());
        }
        QString s;
        ui->sessionsTableWidget->item(row, 0)->setText(settings.value(s.sprintf("scheduleItem%dDay", row)).toString());
        ui->sessionsTableWidget->item(row, 1)->setText(settings.value(s.sprintf("scheduleItem%dSession", row)).toString());
        ui->sessionsTableWidget->item(row, 2)->setText(settings.value(s.sprintf("scheduleItem%dStartTime", row)).toString());
        ui->sessionsTableWidget->item(row, 3)->setText(settings.value(s.sprintf("scheduleItem%dEndTime", row)).toString());
    }

}



void MainWindow::onTrackAntenna1PowerComboBoxActivated(int antennaIndex) {
    qDebug().nospace() << "onAntenna1ComboBoxActivated()" << antennaIndex;
}



void MainWindow::onSaveSettingsPushButtonClicked(void) {
    settings.setValue("trackName", ui->trackNameLineEdit->text());
    settings.setValue("trackLength1M", ui->trackLength1LineEdit->text());
    settings.setValue("trackLength2M", ui->trackLength2LineEdit->text());
    settings.setValue("trackLength3M", ui->trackLength3LineEdit->text());
    settings.setValue("trackLength4M", ui->trackLength4LineEdit->text());
    settings.setValue("tablePurgeIntervalHours", ui->tablePurgeIntervalDoubleSpinBox->value());
    settings.setValue("trackReaderIp", ui->trackReaderIpLineEdit->text());
    settings.setValue("trackTransmitPower1", ui->trackAntenna1PowerComboBox->currentText());
    settings.setValue("trackTransmitPower2", ui->trackAntenna2PowerComboBox->currentText());
    settings.setValue("trackTransmitPower3", ui->trackAntenna3PowerComboBox->currentText());
    settings.setValue("trackTransmitPower4", ui->trackAntenna4PowerComboBox->currentText());

    settings.setValue("deskReaderIp", ui->deskReaderIpLineEdit->text());
    settings.setValue("trackTransmitPower1", ui->deskAntenna1PowerComboBox->currentText());
    settings.setValue("trackTransmitPower2", ui->deskAntenna2PowerComboBox->currentText());
    settings.setValue("trackTransmitPower3", ui->deskAntenna3PowerComboBox->currentText());
    settings.setValue("trackTransmitPower4", ui->deskAntenna4PowerComboBox->currentText());

    settings.setValue("smtpUsername", ui->smtpUsernameLineEdit->text());
    settings.setValue("smtpPassword", ui->smtpPasswordLineEdit->text());
    settings.setValue("smtpServer", ui->smtpServerLineEdit->text());
    settings.setValue("smtpPort", ui->smtpPortLineEdit->text().toInt());
    settings.setValue("emailSendReports", ui->emailSendReportsCheckBox->isChecked());
    settings.setValue("emailFrom", ui->emailFromLineEdit->text());
    settings.setValue("emailSubject", ui->emailSubjectLineEdit->text());
    settings.setValue("emailReportLatency", ui->emailReportLatencySpinBox->value());
}



void MainWindow::onSaveSessionsPushButtonClicked(void) {
    for (int row=0; row<ui->sessionsTableWidget->rowCount(); row++) {
        QString s;
        settings.setValue(s.sprintf("scheduleItem%dDay", row), ui->sessionsTableWidget->item(row, 0)->text());
        settings.setValue(s.sprintf("scheduleItem%dSession", row), ui->sessionsTableWidget->item(row, 1)->text());
        settings.setValue(s.sprintf("scheduleItem%dStartTime", row), ui->sessionsTableWidget->item(row, 2)->text());
        settings.setValue(s.sprintf("scheduleItem%dEndTime", row), ui->sessionsTableWidget->item(row, 3)->text());
    }
}



void MainWindow::onApplySettingsPushButtonClicked(void) {
}



void MainWindow::onEMailTestPushButtonClicked(void) {
    CSmtp *smtp = new CSmtp(ui->smtpUsernameLineEdit->text(), ui->smtpPasswordLineEdit->text(), ui->smtpServerLineEdit->text(), ui->smtpPortLineEdit->text().toInt());
    connect(smtp, SIGNAL(completed(int)), this, SLOT(onTestMailSent(int)));
    connect(smtp, SIGNAL(newLogMessage(QString)), this, SLOT(onNewLogMessage(QString)));

    QString body("This is test email message.");

    body.append("\n\nReport generated by llrpLaps " + QCoreApplication::applicationVersion());

    smtp->sendMail(ui->emailFromLineEdit->text(), ui->emailTestToLineEdit->text(), ui->emailTestSubjectLineEdit->text(), body.toLatin1().data());
}


void MainWindow::onTestMailSent(void) {
    QMessageBox::information(this, "EMail Test", "Email message sent");
}



void MainWindow::onClockTimerTimeout(void) {
    ui->rightTitleLabel->setText(QDateTime::currentDateTime().toString("ddd MMMM d yyyy  hh:mm:ss"));

    // Check to see if this is the first timeout after a specified time (midnight) and send email reports

    QDateTime currentDateTime(QDateTime::currentDateTime());

    static bool sentInThisInterval = false;
    if ((currentDateTime.time().hour() % ui->emailReportLatencySpinBox->value()) == 0) {
//    if ((currentDateTime.time().second() % 10) == 0) {
        if (!sentInThisInterval) {
            sendReports();
            sentInThisInterval = true;
        }
    }
    else {
        sentInThisInterval = false;
    }


    // Update session.  If changed, clear session bests

    QString session = getSession(currentDateTime);
    if (ui->scheduledSessionLineEdit->text() != session) {
        ui->scheduledSessionLineEdit->setText(session);
        bestLapSpeedInSession = 0.;
        bestLapCountInSession = 0;
    }

}



// get session for specified dateTime

QString MainWindow::getSession(const QDateTime &dateTime) {
    QString session;
    QString day = QDate::longDayName(dateTime.date().dayOfWeek());
    for (int i=0; i<ui->sessionsTableWidget->rowCount(); i++) {
        QString sessionDay = ui->sessionsTableWidget->item(i, 0)->text();
        QTime sessionStartTime(QTime::fromString(ui->sessionsTableWidget->item(i, 2)->text(), "hh:mm"));
        QTime sessionEndTime(QTime::fromString(ui->sessionsTableWidget->item(i, 3)->text(), "hh:mm"));
        if ((day == sessionDay) && (dateTime.time() >= sessionStartTime) && (dateTime.time() < sessionEndTime)) {
            session = ui->sessionsTableWidget->item(i, 1)->text();
            break;
        }
    }
    return session;
}




// *********************************************************************************************
//
// sendReports()
// This routing is called on a regular basis whether previous attempts to send reports were
// successful or not.
// Look through lapsDbase for laps within the last week with pending reports

void MainWindow::sendReports(void) {
    if (!ui->emailSendReportsCheckBox->isChecked())
        return;
    if (!lapsDbase.isOpen())
        return;

    // Get a copy of membershipDbase

    QList<CMembershipInfo> infoList;
    membershipDbase.getAllList(&infoList);

    // Create a list of membershipInfo of riders who have unreported laps in the past week

    membershipInfoNotReported.clear();
    QDate currentDate(QDate::currentDate());
    QDate dateStart = currentDate.addDays(-7);
    dateTimeOfReportStart = CLapsDbase::dateTime2Int(dateStart.year(), dateStart.month(), dateStart.day(), 0, 0, 0);
    dateTimeOfReportEnd = CLapsDbase::dateTime2Int(currentDate.year(), currentDate.month(), currentDate.day(), 24, 0, 0);

    for (int i=0; i<infoList.size(); i++) {
        bool tagInActiveRidersList = false;
        for (int j=0; j<activeRidersTableModel->activeRidersList.size(); j++) {
            if (infoList[i].tagId == activeRidersTableModel->activeRidersList[j].tagId) {
                tagInActiveRidersList = true;
                break;
            }
        }

        if (infoList[i].sendReports && !infoList[i].eMail.isEmpty() && !tagInActiveRidersList) {
            QList<int> lapsNotReported;
            int rc = lapsDbase.getLapsInPeriod(infoList[i].tagId, dateTimeOfReportStart, dateTimeOfReportEnd, CLapsDbase::reportPending, &lapsNotReported);
            if (rc != 0) {
                qDebug() << "Error from lapsDbase.getLapsInPeriod()";
                return;
            }
            for (int j=0; j<lapsNotReported.size(); j++) {

                // Get dateTime for lap and append to tagIdNotReported and dateTimeNotReported

                QString tagId;
                unsigned int dateTime;
                float lapSec;
                float lapM;
                int reportStatus;
                rc = lapsDbase.getLap(lapsNotReported[j], &tagId, &dateTime, &lapSec, &lapM, &reportStatus);
                if (rc != 0) {
                    qDebug() << "Error from lapsDbase.getLap()";
                    return;
                }
                bool inList = false;
                for (int k=0; k<membershipInfoNotReported.size(); k++) {
                    if (membershipInfoNotReported[k].tagId == tagId) {
                        inList = true;
                        break;
                    }
                }
                if (!inList)
                    membershipInfoNotReported.append(infoList[i]);
            }
        }
    }

    emit prepareNextReport();
}



// prepareNextReport()
// Sends report for day associated with next day on dateTimeNotReported/tagIdNotReported lists.
// Removes all laps from lists for the day reported and sets reportStatus flag in lapsDbase.
//
void MainWindow::prepareNextReport(void) {
    if (membershipInfoNotReported.isEmpty())
        return;
    if (!lapsDbase.isOpen())
        return;

    // Point to member to report on

    CMembershipInfo *memberToReport = &membershipInfoNotReported[0];

    QString body("This is an automatic email report describing recent cycling activity at the " + ui->trackNameLineEdit->text() + ".  Do not reply to this message.\n\n");
    body.append("Name: " + memberToReport->firstName + " " + memberToReport->lastName + "\n");
    body.append("TagId: " + memberToReport->tagId + "\n");
    body.append("MembershipNumber: " + memberToReport->membershipNumber + "\n");

//    QString body("<html>\n<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd\">\n<html xmlns=\"http://www.w3.org/1999/xhtml\">\n<head>\n<meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\" />\n<title>Demystifying Email Design</title>\n<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\"/>\n</head></html>\n");

//    QString body("<html>\n<p>Hello world</html>");

//    body.append("<html>\n<table width=\"600\">\n<tr>\n<th>Firstname</th><th>Lastname</th><th>Age</th></tr><tr><td>Jill</td><td>Smith</td><td>50</td></tr><tr><td>Eve</td><td>Jackson</td><td>94</td></tr></table></html>\n");

    body.append("\nDate               Session  Laps    km   AveLapSec  AveLapkm/hr  BestLapSec  BestLapkm/hr\n");

    // Get stats for each session and each day in past week for this rider

    QDate currentDate(QDate::currentDate());
    for (int i=6; i>=0; i--) {
        QDate reportDate = currentDate.addDays(-i);

        // Stats for entire day

        unsigned int reportPeriodStart = CLapsDbase::dateTime2Int(reportDate.year(), reportDate.month(), reportDate.day(), 0, 0, 0);
        unsigned int reportPeriodEnd = CLapsDbase::dateTime2Int(reportDate.year(), reportDate.month(), reportDate.day(), 24, 0, 0);
        CStats statsForDay;
        int rc = lapsDbase.getStatsForPeriod(memberToReport->tagId, reportPeriodStart, reportPeriodEnd, CLapsDbase::reportAny, &statsForDay);
        if (rc != 0) {
            qDebug() << "Error from lapsDbase.getStatsForPeriod in sendNextReport";
            return;
        }

        // Stats for each session in day

        QList<CStats> statsForSession;
        QStringList sessionList;    // must correspond to elements in statsForSession
        if (statsForDay.lapCount > 0) {
            QString reportDay = QDate::longDayName(reportDate.dayOfWeek());
            for (int i=0; i<ui->sessionsTableWidget->rowCount(); i++) {
                QString sessionDay = ui->sessionsTableWidget->item(i, 0)->text();
                if (sessionDay == reportDay) {
                    QTime sessionStartTime(QTime::fromString(ui->sessionsTableWidget->item(i, 2)->text(), "hh:mm"));
                    QTime sessionEndTime(QTime::fromString(ui->sessionsTableWidget->item(i, 3)->text(), "hh:mm"));
                    sessionEndTime.addSecs(-1);
                    unsigned int sessionStart = CLapsDbase::dateTime2Int(reportDate.year(), reportDate.month(), reportDate.day(), sessionStartTime.hour(), sessionStartTime.minute(), sessionStartTime.second());
                    unsigned int sessionEnd = CLapsDbase::dateTime2Int(reportDate.year(), reportDate.month(), reportDate.day(), sessionEndTime.hour(), sessionEndTime.minute(), sessionEndTime.second());
                    CStats stats;
                    rc = lapsDbase.getStatsForPeriod(memberToReport->tagId, sessionStart, sessionEnd, CLapsDbase::reportAny, &stats);
                    statsForSession.append(stats);
                    sessionList.append(ui->sessionsTableWidget->item(i, 1)->text());
                }
            }
        }

        // Report day

        QString s;
        float averageLapSec = 0.;
        float speed = 0.;
        float bestSpeed = 0.;
        float bestLapSec = 0.;
        if (statsForDay.lapCount > 0.) averageLapSec = statsForDay.totalSec / (float)statsForDay.lapCount;
        if (statsForDay.totalSec > 0.) speed = statsForDay.totalM / statsForDay.totalSec / 1000. * 3600.;
        if (statsForDay.bestLapSec > 0.) bestSpeed = statsForDay.bestLapM / statsForDay.bestLapSec / 1000. * 3600.;
        if (statsForDay.bestLapSec > 0.) bestLapSec = statsForDay.bestLapSec;
        body.append(s.sprintf("\n%-26s %4d  %7.3f %7.2f      %6.2f      %6.2f      %6.2f\n", reportDate.toString().toLatin1().data(), statsForDay.lapCount, statsForDay.totalM/1000., averageLapSec, speed, bestLapSec, bestSpeed));

        // Report sessions

        for (int i=0; i<statsForSession.size(); i++) {
            if (statsForSession[i].lapCount > 0) {
                QString s;
                float averageLapSec = 0.;
                float speed = 0.;
                float bestSpeed = 0.;
                float bestLapSec = 0.;
                if (statsForSession[i].lapCount > 0.) averageLapSec = statsForSession[i].totalSec / (float)statsForSession[i].lapCount;
                if (statsForSession[i].totalSec > 0.) speed = statsForSession[i].totalM / statsForSession[i].totalSec / 1000. * 3600.;
                if (statsForSession[i].bestLapSec > 0.) bestSpeed = statsForSession[i].bestLapM / statsForSession[i].bestLapSec / 1000. * 3600.;
                if (statsForSession[i].bestLapSec > 0.) bestLapSec = statsForSession[i].bestLapSec;
                body.append(s.sprintf("%26s %4d  %7.3f %7.2f      %6.2f      %6.2f      %6.2f\n", sessionList[i].toLatin1().data(), statsForSession[i].lapCount, statsForSession[i].totalM/1000., averageLapSec, speed, bestLapSec, bestSpeed));
            }
        }

    }
    body.append("\n\nReport generated by llrpLaps " + QCoreApplication::applicationVersion());

    sendReport(*memberToReport, body);
}



void MainWindow::sendReport(const CMembershipInfo &info, const QString &body) {
    emit onNewLogMessage("Sending email report to " + info.firstName + " " + info.lastName + " at " + info.eMail);

    qDebug() << ui->emailFromLineEdit->text() << info.eMail << ui->emailSubjectLineEdit->text() << body.toLatin1().data();

    // Create smtp client

    smtp = new CSmtp(ui->smtpUsernameLineEdit->text(), ui->smtpPasswordLineEdit->text(), ui->smtpServerLineEdit->text(), ui->smtpPortLineEdit->text().toInt());
    connect(smtp, SIGNAL(completed(int)), this, SLOT(onMailSent(int)));
    connect(smtp, SIGNAL(newLogMessage(QString)), this, SLOT(onNewLogMessage(QString)));
    smtp->sendMail(ui->emailFromLineEdit->text(), info.eMail, ui->emailSubjectLineEdit->text(), body.toLatin1().data());
}



void MainWindow::onMailSent(int error) {

    // If there was an error sending reports, do not clear list or update reportStatus in dbase

    if (error != 0) {
        emit onNewLogMessage("  Email report not sent");
        return;
    }


    // Remove all entries from notReported lists for first rider on list

    QString tagIdBeingRemoved = membershipInfoNotReported[0].tagId;
    for (int i=membershipInfoNotReported.size()-1; i>=0; i--) {
        if (membershipInfoNotReported[i].tagId == tagIdBeingRemoved) {
            membershipInfoNotReported.removeAt(i);

            int rc = lapsDbase.setReportStatus(CLapsDbase::reportCompleted, tagIdBeingRemoved, dateTimeOfReportStart, dateTimeOfReportEnd);
            emit onNewLogMessage("  Email report sent");
            if (rc != 0) {
                qDebug() << "Error from lapsDbase.setReported";
                return;
            }

        }
    }

    emit prepareNextReport();
}


// *********************************************************************************************
// Member functions related to processing tags received
//

// onPurgeRidersList
// Loop through riders in activeRidersList and remove any that are getting old.
// Remove from activeRidersTable also.
// Then loop through lapsTable and remove old entries.
// Send email to riders when removed from list.
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



void MainWindow::onLapsTableSortEnableCheckBoxClicked(bool state) {
    if (state) {
//        lapsProxyModel->setDynamicSortFilter(true);
//        ui->lapsTableView->sortByColumn(3, Qt::DescendingOrder);     // must come before call to setSortingEnabled()
//        ui->lapsTableView->setSortingEnabled(true);
    }
    else {
//        lapsProxyModel->setDynamicSortFilter(false);
//        ui->lapsTableView->setSortingEnabled(false);
//        ui->lapsTableView->sortByColumn(3, Qt::AscendingOrder);     // must come before call to setSortingEnabled()
    }
}



void MainWindow::onActiveRidersTableSortEnableCheckBoxClicked(bool state) {
    if (state) {
        activeRidersProxyModel->setDynamicSortFilter(true);
        ui->activeRidersTableView->sortByColumn(0, Qt::AscendingOrder);     // must come before call to setSortingEnabled()
        ui->activeRidersTableView->setSortingEnabled(true);
    }
    else {
//        ui->activeRidersTableView->sortByColumn(0, Qt::AscendingOrder);     // must come before call to setSortingEnabled()
        activeRidersProxyModel->setDynamicSortFilter(false);
        ui->activeRidersTableView->setSortingEnabled(false);
    }
}



// Process new tag
//
void MainWindow::onNewTrackTag(CTagInfo tagInfo) {
    QString s;
    static int tagCount = 0;

    // Add string to messages window

    onNewLogMessage(s.sprintf("readerId=%d antennaId=%d timeStampUSec=%llu tagData=%s", tagInfo.readerId, tagInfo.antennaId, tagInfo.timeStampUSec, tagInfo.tagId.toLatin1().data()));

    // Tags from antennas with track length set to -1 are ignored

    if (!tagInfo.tagId.isEmpty() && (trackLengthM[tagInfo.antennaId - 1] == -1.))
        return;

    // lapCount is total laps all riders

    if (!tagInfo.tagId.isEmpty()) {
        tagCount++;
        ui->lapCountLineEdit->setText(s.setNum(tagCount));
    }

    if (!activeRidersTableModel) {
        qDebug() << "No activeRidersTableModel";
        return;
    }

    // Confirm there is a valid track length for the antenna triggering this read event.  Antennas not used should have a length value set to -1.

    if (!tagInfo.tagId.isEmpty() && (trackLengthM[tagInfo.antennaId - 1] == 0.))
        guiCritical(s.sprintf("The track length value for the antenna triggering this event (%d) is zero.  Please set track length in Settings panel.", tagInfo.antennaId));

    // Process tag

    activeRidersTableModel->newTrackTag(tagInfo);
}



float MainWindow::lapSpeed(float lapSec, float lapM) {
    double lapSpeed = 0.;
    if (lapSec > 0.)
        lapSpeed = lapM / (lapSec / 3600.) / 1000.;
    return lapSpeed;
}



void MainWindow::onNewLogMessage(QString s) {
    QString text(QDateTime::currentDateTime().toString("yyyy-MM-dd_hh:mm:ss ") + s);
    ui->messagesPlainTextEdit->appendPlainText(text);

   if (logFile && logTextStream)
        *logTextStream << text << "\n";

}



// **********************************************************************************************************

void MainWindow::onDbaseSearchPushButtonClicked(void) {
    QString tagId = ui->deskTagIdLineEdit->text();
    QString firstName = ui->deskFirstNameLineEdit->text();
    QString lastName = ui->deskLastNameLineEdit->text();
    QString membershipNumber = ui->deskMembershipNumberLineEdit->text();
//    QString caRegistration = ui->deskCaRegistrationLineEdit->text();
//    QString eMail = ui->deskEMailLineEdit->text();
//    bool sendReports = ui->sendReportsCheckBox->isChecked();

    // If tagId contains entry, search based on only that.
    // If found, update fields.  Otherwise clear fields.

    CMembershipInfo info;

    if (!tagId.isEmpty() && firstName.isEmpty() && lastName.isEmpty() && membershipNumber.isEmpty()) {
        int id = membershipDbase.getIdFromTagId(ui->deskTagIdLineEdit->text().toLatin1());
        if (id > 0) {
            tagInDbase = true;
            membershipDbase.getAllFromId(id, &info);
            ui->deskFirstNameLineEdit->setText(info.firstName);
            ui->deskLastNameLineEdit->setText(info.lastName);
            ui->deskMembershipNumberLineEdit->setText(info.membershipNumber);
            ui->deskCaRegistrationLineEdit->setText(info.caRegistration);
            ui->deskEMailLineEdit->setText(info.eMail);
            ui->sendReportsCheckBox->setChecked(info.sendReports);
        }
        else {
            tagInDbase = false;
            ui->deskFirstNameLineEdit->clear();
            ui->deskLastNameLineEdit->clear();
            ui->deskMembershipNumberLineEdit->clear();
            ui->deskCaRegistrationLineEdit->clear();
            ui->deskEMailLineEdit->clear();
            ui->sendReportsCheckBox->setChecked(false);
        }
    }

    // Else if first or last name given, search on that.  Don't clear names on search fail.

    else if ((!lastName.isEmpty() || !firstName.isEmpty()) && membershipNumber.isEmpty()) {
        int id = membershipDbase.getIdFromName(firstName, lastName);
        if (id > 0) {
            tagInDbase = true;
            membershipDbase.getAllFromId(id, &info);
            ui->deskTagIdLineEdit->setText(info.tagId);
            ui->deskFirstNameLineEdit->setText(info.firstName);
            ui->deskLastNameLineEdit->setText(info.lastName);
            ui->deskMembershipNumberLineEdit->setText(info.membershipNumber);
            ui->deskCaRegistrationLineEdit->setText(info.caRegistration);
            ui->deskEMailLineEdit->setText(info.eMail);
            ui->sendReportsCheckBox->setChecked(info.sendReports);
        }
        else {
            tagInDbase = false;
        }
    }

    // Else if membership number is given, search on that.  Don't clear names on search fail.

    else if ((lastName.isEmpty() && firstName.isEmpty()) && !membershipNumber.isEmpty()) {
        int id = membershipDbase.getIdFromMembershipNumber(membershipNumber);
        if (id > 0) {
            tagInDbase = true;
            membershipDbase.getAllFromId(id, &info);
            ui->deskTagIdLineEdit->setText(info.tagId);
            ui->deskFirstNameLineEdit->setText(info.firstName);
            ui->deskLastNameLineEdit->setText(info.lastName);
            ui->deskMembershipNumberLineEdit->setText(info.membershipNumber);
            ui->deskCaRegistrationLineEdit->setText(info.caRegistration);
            ui->deskEMailLineEdit->setText(info.eMail);
            ui->sendReportsCheckBox->setChecked(info.sendReports);
        }
        else {
            tagInDbase = false;
        }
    }
    updateDbaseButtons();
}



void MainWindow::onDbaseAddPushButtonClicked(void) {

    // Check whether tagId is already in dbase

    int id = membershipDbase.getIdFromTagId(ui->deskTagIdLineEdit->text().toLatin1());
    if (id != 0) {
        guiCritical("Tag \"" + ui->deskTagIdLineEdit->text().toLatin1() + "\" already in database");
        return;
    }

    // Add entry to database

    CMembershipInfo info;
    info.tagId = ui->deskTagIdLineEdit->text();
    info.firstName = ui->deskFirstNameLineEdit->text();
    info.lastName = ui->deskLastNameLineEdit->text();
    info.membershipNumber = ui->deskMembershipNumberLineEdit->text();
    info.caRegistration = ui->deskCaRegistrationLineEdit->text();
    info.eMail = ui->deskEMailLineEdit->text();
    info.sendReports = ui->sendReportsCheckBox->isChecked();
    int rc = membershipDbase.add(info);
    if (rc != 0) {
        guiCritical(membershipDbase.errorText());
        return;
    }

    // Add to table

    if (!membershipTableModel->add(info)) {
        guiCritical("Could not add entry to membershipTable");
        return;
    }

    onDbaseClearPushButtonClicked();
}



void MainWindow::onDbaseClearPushButtonClicked(void) {
    tagInDbase = false;
    ui->deskTagIdLineEdit->clear();
    ui->deskFirstNameLineEdit->clear();
    ui->deskLastNameLineEdit->clear();
    ui->deskMembershipNumberLineEdit->clear();
    ui->deskCaRegistrationLineEdit->clear();
    ui->deskEMailLineEdit->clear();
    ui->sendReportsCheckBox->setChecked(false);
    updateDbaseButtons();
}



void MainWindow::onDbaseRemovePushButtonClicked(void) {
    QMessageBox::StandardButtons b = guiQuestion("You are about to remove this tag entry from the database.  Press Ok to continue.", QMessageBox::Ok | QMessageBox::Abort);
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

    onDbaseClearPushButtonClicked();
}



void MainWindow::onDbaseUpdatePushButtonClicked(void) {

    // Confirm we should be changing database

    if (guiQuestion("You are about to modify an existing tag entry in the database.  Press Ok to continue.", QMessageBox::Ok | QMessageBox::Abort) != QMessageBox::Ok)
        return;

    CMembershipInfo info;
    info.tagId = ui->deskTagIdLineEdit->text();
    info.firstName = ui->deskFirstNameLineEdit->text();
    info.lastName = ui->deskLastNameLineEdit->text();
    info.membershipNumber = ui->deskMembershipNumberLineEdit->text();
    info.caRegistration = ui->deskCaRegistrationLineEdit->text();
    info.eMail = ui->deskEMailLineEdit->text();
    info.sendReports = ui->sendReportsCheckBox->isChecked();

    // Update entry in database

    int rc = membershipDbase.update(info);
    if (rc != 0) {
        guiCritical("Could not update database: " + membershipDbase.errorText());
        return;
    }

    // Add to table

    if (!membershipTableModel->update(info)) {
        guiCritical("Could not update membershipTable");
        return;
    }

    onDbaseClearPushButtonClicked();
}



void MainWindow::onDbaseReadPushButtonClicked(bool state) {
    if (state) {
        ui->deskReadPushButton->setChecked(true);
        onDbaseClearPushButtonClicked();
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
    if (tagInfo.antennaId <= 0) return;
    deskReader->blockSignals(true);
    onDbaseClearPushButtonClicked();
    ui->deskTagIdLineEdit->setText(tagInfo.tagId);
    onDbaseSearchPushButtonClicked();
    ui->deskReadPushButton->setChecked(false);

    updateDbaseButtons();
}



void MainWindow::onDbaseTagIdTextChanged(QString) {
    tagInDbase = false;
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


void MainWindow::onDbaseCaRegistrationTextChanged(QString) {
    updateDbaseButtons();
}


void MainWindow::onDbaseEMailTextChanged(QString) {
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


    // Add is enabled when all fields are filled, tagInDbase is false, and with email optional

    if (!ui->deskTagIdLineEdit->text().isEmpty() && !ui->deskFirstNameLineEdit->text().isEmpty() && !ui->deskLastNameLineEdit->text().isEmpty() && !ui->deskMembershipNumberLineEdit->text().isEmpty() && !ui->deskCaRegistrationLineEdit->text().isEmpty() && !tagInDbase)
        ui->deskAddPushButton->setEnabled(true);

    else
        ui->deskAddPushButton->setEnabled(false);


    // Clear is enabled when any field is filled

    if (!ui->deskTagIdLineEdit->text().isEmpty() || !ui->deskFirstNameLineEdit->text().isEmpty() || !ui->deskLastNameLineEdit->text().isEmpty() || !ui->deskMembershipNumberLineEdit->text().isEmpty() || !ui->deskCaRegistrationLineEdit->text().isEmpty() || !ui->deskEMailLineEdit->text().isEmpty())
        ui->deskClearPushButton->setEnabled(true);

    else
        ui->deskClearPushButton->setEnabled(false);


    // Remove is enabled when tadId and first name and last name are filled, and tagInDbase is true

    if (!ui->deskTagIdLineEdit->text().isEmpty() && !ui->deskFirstNameLineEdit->text().isEmpty() && !ui->deskLastNameLineEdit->text().isEmpty() && !ui->deskMembershipNumberLineEdit->text().isEmpty() && tagInDbase)
        ui->deskRemovePushButton->setEnabled(true);

    else
        ui->deskRemovePushButton->setEnabled(false);


    // Update is enabled when all fields are filled, tagInDbase is true, and with email optional

    if (!ui->deskTagIdLineEdit->text().isEmpty() && !ui->deskFirstNameLineEdit->text().isEmpty() && !ui->deskLastNameLineEdit->text().isEmpty() && !ui->deskMembershipNumberLineEdit->text().isEmpty() && !ui->deskCaRegistrationLineEdit->text().isEmpty() && tagInDbase)
        ui->deskUpdatePushButton->setEnabled(true);

    else
        ui->deskUpdatePushButton->setEnabled(false);

}





void MainWindow::guiCritical(QString s) {
    QMessageBox::critical(NULL, "llrplaps Critical Error", s, QMessageBox::Ok);
}


void MainWindow::guiInformation(QString s) {
    QMessageBox::information(NULL, QCoreApplication::applicationName() + "\n\n", s, QMessageBox::Ok);
}


QMessageBox::StandardButtons MainWindow::guiQuestion(QString s, QMessageBox::StandardButtons b) {
    return QMessageBox::question(NULL, "llrplaps Question", s, b);
}


