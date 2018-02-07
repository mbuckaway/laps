// **************************************************************************************************
//
// mainwindow.cpp
//
// llrplaps
//
// The working directory must be set to the bin directory containing the executable and start-up script.
// Ensure the log and data directories exist
//
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
#include <QClipboard>
#include <QCloseEvent>



#include <cplot.h>
#include <stdio.h>
#include <unistd.h>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <ltkcpp.h>
#include "creader.h"
#include "cdbase.h"
#include "main.h"
#include "csmtp.h"
#include "cusers.h"


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
#define AT_COMMENT              11


// Columns in lap table (listing of all laps)

#define LT_TIMESTAMP        0
#define LT_NAME             1
#define LT_DATETIME         2
#define LT_LAPCOUNT         3
#define LT_READER           4
#define LT_ANTENNA          5
#define LT_LAPTIME          6
#define LT_LAPSPEED         7
#define LT_COMMENT          8


// Column widths in each table

#define CW_NAME             125
#define CW_DATETIME         80
#define CW_TIMESTAMP        190
#define CW_LAPCOUNT         60
#define CW_SEC              70
#define CW_SPEED            95
#define CW_KM               80
#define CW_MEMBERSHIPNUMBER 80
#define CW_CAREGISTRATION   80
#define CW_EMAIL            80
#define CW_INDEX            50
#define CW_ANTENNA          60
#define CW_READER           60


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




MainWindow *mainWindow = NULL;
CMessages *messagesWindow = NULL;
CPreferences *preferencesWindow = NULL;
CUsers *usersWindow = NULL;






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
    return 9;
}



bool CLapsTableModel::insertRows(int position, int count, const QModelIndex &/*parent*/) {
    beginInsertRows(QModelIndex(), position, position + count - 1);

    for (int row=0; row<count; row++) {
        nameList.insert(position, QString());
        lapList.insert(position, 0);
        readerIdList.insert(position, 0);
        antennaIdList.insert(position, 0);
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
        readerIdList.removeAt(position);
        antennaIdList.removeAt(position);
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
        case LT_READER:
            return readerIdList[row];
        case LT_ANTENNA:
            return antennaIdList[row];
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
            case LT_READER:
                return QString("Reader");
            case LT_ANTENNA:
                return QString("Antenna");
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



void CLapsTableModel::newTag(CRider rider) {
    QString time = QTime::currentTime().toString();

    bool scrollToBottomRequired = false;
    if (mainWindow->ui->lapsTableView->verticalScrollBar()->sliderPosition() == mainWindow->ui->lapsTableView->verticalScrollBar()->maximum())
        scrollToBottomRequired = true;

    int row = nameList.size();
    insertRows(row, 1);

    if (scrollToBottomRequired)
        mainWindow->ui->lapsTableView->scrollToBottom();


    if (!rider.name.isEmpty())
        nameList[row] = rider.name;
    else
        nameList[row] = rider.tagId;

    if (rider.lapType == CRider::regularCrossing) {
        readerIdList[row] = rider.readerId;
        antennaIdList[row] = rider.antennaId;
        lapList[row] = rider.lapCount;
        timeList[row] = time;
        timeStampList[row] = rider.previousTimeStampUSec;
        lapSecList[row] = rider.lapSec;
        lapSpeedList[row] = rider.lapKmph;
    }
    else {
        readerIdList[row] = rider.readerId;
        antennaIdList[row] = rider.antennaId;
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
        float purgeInterval = preferencesWindow->tablePurgeIntervalHours;
        if (preferencesWindow && (inactiveHours >= purgeInterval))
            removeRows(i, 1);
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
    return 12;
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
            if (!rider->name.isEmpty())
                return rider->name;
            else
                return "???";
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
            if (rider->lapKmph > 0.)
                return rider->lapKmph;
            else
                return QString();
        case AT_BESTLAPSPEED:
            if (rider->bestLapKmph > 0.)
                return rider->bestLapKmph;
            else
                return QString();
        case AT_AVERAGESPEED:
            if (rider->averageKmph > 0.)
                return rider->averageKmph;
            else
                return QString();
        case AT_KMTHISMONTH:
            if (rider->thisMonth.totalM > 0.)
                return rider->thisMonth.totalM / 1000.;
            else
                return QString();
        case AT_AVERAGESPEEDTHISMONTH:
            if (rider->thisMonth.averageKmph > 0.)
                return rider->thisMonth.averageKmph;
            else
                return QString();
        case AT_KMLASTMONTH:
            if (rider->lastMonth.totalM > 0.)
                return rider->lastMonth.totalM / 1000.;
            else
                return QString();
        case AT_AVERAGESPEEDLASTMONTH:
            if (rider->lastMonth.averageKmph > 0.)
                return rider->lastMonth.averageKmph;
            else
                return QString();
//        case AT_LAPCOUNTALLTIME:
//            if (rider->allTime.lapCount > 0)
//                return rider->allTime.lapCount;
//            else
//                return QString();
//        case AT_KMALLTIME:
//            if (rider->allTime.totalM > 0.)
//                return rider->allTime.totalM / 1000.;
//            else
//                return QString();
        case AT_COMMENT:
            switch (rider->lapType) {
            case CRider::firstCrossing:
                return QString("first lap");
            case CRider::regularCrossing:
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
//        if (col == AT_COMMENT) {
//                return QFont();
//            else
//                return QFont();
//        }
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
//            case AT_LAPCOUNTALLTIME:
//                return QString("Total Laps");
//            case AT_KMALLTIME:
//                return QString("Total km");
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
// This routine processes each track tag as it arrives, calculating lap times, speed etc.
// Tags are assumed to be from valid read events (already filtered for unused antennas), but may be
// a null tag (used to update tables).
// Returns CRider pointer to entry in activeRidersList.
//
CRider *CActiveRidersTableModel::newTag(const CTagInfo &tagInfo) {
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
            return NULL;
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

            // On first crossing, try getting name from dbases

            id = mainWindow->membershipDbase.getIdFromTagId(tagInfo.tagId);
            if (id > 0) {
                rider->inDbase = true;
                mainWindow->membershipDbase.getAllFromId(id, &info);
                rider->tagId = info.tagId;
                if (info.firstName.isEmpty())
                    rider->name = info.lastName;
                else
                    rider->name = info.firstName + " " + info.lastName;
                if (info.sendReports && !info.eMail.isEmpty())
                    rider->reportStatus = 1;
            }
            else {
                rider->inDbase = false;
                rider->name.clear();
            }

            rider->lapCount = 0;
            rider->lapSec = 0.;
            rider->lapM = 0.;
            rider->lapKmph = 0.;
            rider->bestLapKmph = 0.;
            rider->totalSec = 0.;
            rider->totalM = 0.;
            rider->averageKmph = 0.;
            rider->tagId = tagInfo.tagId;

            // If rider is in dBase, get past stats

            if (rider->inDbase)
                mainWindow->lapsDbase.getStats(tagInfo.tagId, rider);


            // Any laps already in dBase from current session (will happen if application is stopped and restarted)
            // will be included in the above calculation of past stats but must now be added to current session stats

            {
                QList<CLapInfo> lapsInSession;
                mainWindow->lapsDbase.getLapInfo(rider->tagId, QDateTime::currentDateTime().addSecs(-3 * 3600), QDateTime::currentDateTime(), &lapsInSession);

                // Loop through laps for current session

                for (int i=0; i<lapsInSession.size(); i++) {
                    float lapKmph = mainWindow->kph(lapsInSession[i].lapM, lapsInSession[i].lapSec);

                    rider->lapCount++;
                    rider->totalM += lapsInSession[i].lapM;
                    rider->totalSec += lapsInSession[i].lapSec;
                    rider->lapKmph = lapKmph;

                    // Find best lap kph

                    if (rider->lapKmph > rider->bestLapKmph)
                        rider->bestLapKmph = lapKmph;

                    // Find best k kph

                    rider->lapMList.append(lapsInSession[i].lapM);
                    rider->lapSecList.append(lapsInSession[i].lapSec);
                    rider->lapMSum += lapsInSession[i].lapM;
                    rider->lapSecSum += lapsInSession[i].lapSec;
                    float kKmph = 0.;
                    if (rider->lapMSum >= 1000.) {
                        float sec = rider->lapSecSum;
                        float m = rider->lapMSum;
                        do {
                            rider->lapSecSum -= rider->lapSecList[0];
                            rider->lapMSum -= rider->lapMList[0];
                            rider->lapSecList.removeFirst();
                            rider->lapMList.removeFirst();
                        } while (rider->lapMSum >= 1000.);
                        kKmph = mainWindow->kph(m, sec);
                        if (kKmph > rider->bestKKmph)
                            rider->bestKKmph = kKmph;
                    }

                }

                rider->averageKmph = mainWindow->kph(rider->totalM, rider->totalSec);

//                if (rider->bestLapKmph > mainWindow->trackSessionBestLapKmph) {
//                    mainWindow->trackSessionBestLapKmph = rider->bestLapKmph;
//                    mainWindow->trackSessionBestLapKmphName = rider->name;
//                }

//                if (rider->bestKKmph > mainWindow->trackSessionBestKKmph) {
//                    mainWindow->trackSessionBestKKmph = rider->bestKKmph;
//                    mainWindow->trackSessionBestKKmphName = rider->name;
//                }

            }

            rider->nextLapType = CRider::regularCrossing;
            break;

        case CRider::regularCrossing:

            // On regular crossing, update lap stats and thisMonth stats

            lapSec = (float)(tagInfo.timeStampUSec - rider->previousTimeStampUSec) / 1.e6;

            // If lap time is greater than maxAcceptableLapSec, rider must have stopped for a break and returned
            // to track

            if (lapSec > mainWindow->maxAcceptableLapSec) {
                rider->lapSec = 0.;
                rider->lapM = 0.;
                rider->lapKmph = 0.;
                rider->lapType = CRider::firstCrossingAfterBreak;
                rider->nextLapType = CRider::regularCrossing;
            }
            else {
                rider->lapCount++;
                rider->lapSec = lapSec;
                if (preferencesWindow)
                    rider->lapM = preferencesWindow->trackLengthM[tagInfo.antennaId - 1];
                rider->lapKmph = mainWindow->kph(rider->lapM, rider->lapSec);

                rider->totalSec += rider->lapSec;
                rider->totalM += rider->lapM;
                rider->averageKmph = mainWindow->kph(rider->totalM, rider->totalSec);

                if (rider->lapKmph > rider->bestLapKmph)
                    rider->bestLapKmph = rider->lapKmph;

                // Update monthly and all-time stats only if rider name is in dbase

                if (rider->inDbase) {

                    // Update thisMonth stats

                    rider->thisMonth.lapCount++;
                    rider->thisMonth.totalSec += rider->lapSec;
                    rider->thisMonth.totalM += rider->lapM;
                    rider->thisMonth.averageKmph = mainWindow->kph(rider->thisMonth.totalM, rider->thisMonth.totalSec);
                    if (rider->lapKmph > rider->thisMonth.bestLapKmph)
                        rider->thisMonth.bestLapKmph = rider->lapKmph;

                    // Update allTime stats

                    rider->allTime.lapCount++;
                    rider->allTime.totalSec += rider->lapSec;
                    rider->allTime.totalM += rider->lapM;
                    rider->allTime.averageKmph = mainWindow->kph(rider->allTime.totalM, rider->allTime.totalSec);
                    if (rider->lapKmph > rider->allTime.bestLapKmph)
                        rider->allTime.bestLapKmph = rider->lapKmph;

                    // Find best k kph

                    rider->lapMList.append(rider->lapM);
                    rider->lapSecList.append(rider->lapSec);
                    rider->lapMSum += rider->lapM;
                    rider->lapSecSum += rider->lapSec;
                    if (rider->lapMSum >= 1000.) {
                        float sec = rider->lapSecSum;
                        float m = rider->lapMSum;
                        do {
                            rider->lapSecSum -= rider->lapSecList[0];
                            rider->lapMSum -= rider->lapMList[0];
                            rider->lapSecList.removeFirst();
                            rider->lapMList.removeFirst();
                        } while (rider->lapMSum >= 1000.);
                        float kKmph = mainWindow->kph(m, sec);
                        if (kKmph > rider->bestKKmph)
                            rider->bestKKmph = kKmph;
                    }



                }

                rider->nextLapType = CRider::regularCrossing;
            }
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


        // Update members of rider that change with each lap

        rider->readerId = tagInfo.readerId;
        rider->antennaId = tagInfo.antennaId;
        rider->previousTimeStampUSec = tagInfo.timeStampUSec;


        // Populate activeRidersTableView entries

        setData(createIndex(activeRiderIndex, 0), 0, Qt::EditRole);


        // Add lap to dbase if this is a regular ridingLap

        if (rider->lapType == CRider::regularCrossing) {
            QDateTime currentDateTime(QDateTime::currentDateTime());
            mainWindow->lapsDbase.addLap(*rider, currentDateTime);
        }


        mainWindow->lapsTableModel->newTag(*rider);


        // lapCount is total laps all riders

        QString s;
        mainWindow->ui->activeRiderCountLineEdit->setText(s.setNum(activeRidersList.size()));

        return rider;
    }

    catch (const QString &s) {
        mainWindow->guiCritical(s);
        return NULL;
    }
    catch (const char *p) {
        mainWindow->guiCritical(QString(*p));
        return NULL;
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
        float purgeInterval = preferencesWindow->tablePurgeIntervalHours;
        if (preferencesWindow && (inactiveHours >= purgeInterval))
            removeRows(i, 1);
    }

    if (scrollToBottomRequired)
        mainWindow->ui->activeRidersTableView->scrollToBottom();

    mainWindow->ui->activeRiderCountLineEdit->setText(s.setNum(activeRidersList.size()));

    return purgedRiders;
}



//float MainWindow::getBestKKmph(CRider *rider, const QList<CLapInfo> &lapList) {
//    rider->lapMList.append(lapList.last().lapM);
//    rider->lapSecList.append(lapList.last().lapSec);
//    rider->lapMSum += lapList.last().lapM;
//    rider->lapSecSum += lapList.last().lapSec;
//    float kKmph = 0.;
//    float bestKKmph = 0.;
//    if (rider->lapMSum >= 1000.) {
//        float sec = rider->lapSecSum;
//        float m = rider->lapMSum;
//        do {
//            rider->lapSecSum -= rider->lapSecList[0];
//            rider->lapMSum -= rider->lapMList[0];
//            rider->lapSecList.removeFirst();
//            rider->lapMList.removeFirst();
//        } while (rider->lapMSum >= 1000.);
//        kKmph = kph(m, sec);
//        if (kKmph > bestKKmph)
//            bestKKmph = kKmph;
//    }
//    return kKmph;
//}


// **************************************************************************************************

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    QString s;
    ui->setupUi(this);
    trackReader = NULL;
    deskReader = NULL;
    trackReaderThread = NULL;
    deskReaderThread = NULL;
    //membershipTableModel = NULL;
    //membershipProxyModel = NULL;
    lapsTableModel = NULL;
    lapsProxyModel = NULL;
    activeRidersTableModel = NULL;
    activeRidersProxyModel = NULL;
    trayIcon = NULL;

    trackSessionBestLapKmph = 0.;
    trackSessionBestKKmph = 0.;
    trackThisMonthBestLapKmph = 0.;
    trackThisMonthBestKKmph = 0.;
    trackAllTimeBestLapKmph = 0.;
    trackAllTimeBestKKmph = 0.;

    QCoreApplication::setApplicationVersion("0.8");
    setWindowIcon(QIcon(":/images/cycle3.png"));


    // Is there another instance of program running?

    QStringList listOfPids;
#if defined(Q_OS_WIN)
    // Get the list of process identifiers.
    DWORD aProcesses[1024], cbNeeded, cProcesses;
    unsigned int i;

    if (!EnumProcesses(aProcesses, sizeof(aProcesses), &cbNeeded))
    {
        return 0;
    }

    // Calculate how many process identifiers were returned.
    cProcesses = cbNeeded / sizeof(DWORD);

    // Search for a matching name for each process
    for (i = 0; i < cProcesses; i++)
    {
        if (aProcesses[i] != 0)
        {
            char szProcessName[MAX_PATH] = {0};

            DWORD processID = aProcesses[i];

            // Get a handle to the process.
            HANDLE hProcess = OpenProcess( PROCESS_QUERY_INFORMATION |
                                           PROCESS_VM_READ,
                                           FALSE, processID);

            // Get the process name
            if (NULL != hProcess)
            {
                HMODULE hMod;
                DWORD cbNeeded;

                if (EnumProcessModules(hProcess, &hMod, sizeof(hMod), &cbNeeded))
                {
                    GetModuleBaseNameA(hProcess, hMod, szProcessName, sizeof(szProcessName)/sizeof(char));
                }

                // Release the handle to the process.
                CloseHandle(hProcess);

                if (*szProcessName != 0 && strcmp(processName, szProcessName) == 0)
                {
                    listOfPids.append(QString::number(processID));
                }
            }
        }
    }

#else

    // Run pgrep, which looks through the currently running processses and lists the process IDs
    // which match the selection criteria to stdout.
    QProcess process;
    QString processName(QCoreApplication::applicationName());
    process.start("pgrep",  QStringList() << processName);
    process.waitForReadyRead();

    QByteArray bytes = process.readAllStandardOutput();

    process.terminate();
    process.waitForFinished();
    process.kill();

    // Output is something like "2472\n2323\n" for multiple instances

    listOfPids = QString(bytes).split("\n", QString::SkipEmptyParts);
#endif

    qDebug() << listOfPids.size();
    if (listOfPids.size() > 1) {
        guiCritical("The " + QCoreApplication::applicationName() + " application is already running.\n\nClick on the " + QCoreApplication::applicationName() + " icon in the system tray below to view lap data.");
        exit(0);
    }


    tagInDbase = false;
    entryEdited = false;

    ui->mainTitleLabel->setText(QCoreApplication::applicationName() + " " + QCoreApplication::applicationVersion());
    ui->rightTitleLabel->setText(QString());
    //logoImage = new QPixmap(":/images/cycle2.png");

    //ui->logoImageLabel->setPixmap(*logoImage);
    //ui->logoImageLabel->setScaledContents(false);
    //ui->logoImageLabel->setFrameStyle(QFrame::NoFrame);
    //ui->logoImageLabel->show();



    // Open log file
    // Start by moving the existing log file into a backup

    QDir logDir;
    logDir.setPath("../log");
    if (!logDir.mkpath(logDir.absolutePath())) {
        guiCritical("Could not create directory " + logDir.absolutePath());
        exit(1);
    }
    QStringList filter{QCoreApplication::applicationName() + "*.log"};
    logDir.setNameFilters(filter);
    QString s2;
    QFile::rename(logDir.absolutePath() + s.sprintf("/%s.log", QApplication::applicationName().toLatin1().data()), logDir.absolutePath() + s2.sprintf("/%s%03d.log",  QCoreApplication::applicationName().toLatin1().data(), logDir.entryInfoList().size() - 1));


    // Configure messages console

    messagesWindow = new CMessages();
    if (!messagesWindow) {
        guiCritical("Fatal internal error: could not create messagesWindow");
        exit(1);
    }
    messagesWindow->show();
    messagesWindow->startLogFile(logDir.absolutePath() + s.sprintf("/%s.log", QCoreApplication::applicationName().toLatin1().data()));


    // Configure preferences window

    preferencesWindow = new CPreferences();
    if (!preferencesWindow) {
        guiCritical("Fatal internal error: could not create preferencesWindow");
        exit(1);
    }
    setWindowTitle(QCoreApplication::applicationName() + ": " + preferencesWindow->trackName);
    ui->leftTitleLabel->setText(preferencesWindow->trackName);


    // Make backup of membership.db

    QDir dataDir;
    dataDir.setPath("../data");
    if (!dataDir.mkpath(dataDir.absolutePath())) {
        guiCritical("Could not create directory " + dataDir.absolutePath());
        exit(1);
    }
    filter = QStringList{"membership*.db"};
    dataDir.setNameFilters(filter);

    if (dataDir.entryList().size() > 0)
        QFile::copy(dataDir.absolutePath() + "/membership.db", dataDir.absolutePath() + s.sprintf("/membership-%03d.db", dataDir.entryList().size() - 1));


    // Make backups of lapsyyyy.db database

    QDate currentDate(QDate::currentDate());
    QString year = s.setNum(currentDate.year());
    filter = QStringList{"laps" + year + "*.db"};
    dataDir.setNameFilters(filter);
    if (dataDir.entryList().size() > 0)
        QFile::copy(dataDir.absolutePath() + "/laps" + year + ".db", dataDir.absolutePath() + "/laps" + year + s.sprintf("-%03d.db", dataDir.entryList().size() - 1));


    // Initialize member variables

    activeRidersTableSortingEnabled = true;
    lapsTableSortingEnabled = true;
    float nominalSpeedkmph = 30.0;              // Approximate, used to identify riders taking a break


    // Initialize 1-sec timer for panel dateTime and possibly other things

    connect(&clockTimer, SIGNAL(timeout()), this, SLOT(onClockTimerTimeout()));
    clockTimer.setInterval(1000);
    clockTimer.start();


    // Two databases are used.
    // membershipDbase contains track membership info for each rider.
    // This should be opened before lapsdbase because the membershipdbase is used when a new lapsdbase is created
    // at the start of a new year.

    QString membershipDbaseRootName(dataDir.absolutePath() + "/membership");
    QString membershipDbaseUserName("fcv");
    QString membershipDbasePassword("fcv");
    int rc = membershipDbase.open(membershipDbaseRootName, membershipDbaseUserName, membershipDbasePassword);
    if ((rc != 0) || !membershipDbase.isOpen())
        guiCritical(s.sprintf("Error %d opening membership database file \"%s\": %s.\n\nRider names will not be displayed and new tags cannot be added.", rc, membershipDbase.absoluteFilePath().toLatin1().data(), membershipDbase.errorText().toLatin1().data()));
    else
        messagesWindow->addMessage(s.sprintf("Opened membership database file \"%s\"", membershipDbase.absoluteFilePath().toLatin1().data()));


    // lapsDbase contains a record of all laps for all riders.
    // Each file contains data for one year.  Each file consists of two database tables.  The first is
    // lapsTable and keeps track of laps as they are recorded.  The second is priorsTable and contains a
    // summary of totals from all previous years, exclusive of laps in lapsTable.

    QString lapsDbaseRootName(dataDir.absolutePath() + "/laps");
    QString lapsDbaseUserName("fcv");
    QString lapsDbasePassword("fcv");
    rc = lapsDbase.open(lapsDbaseRootName, lapsDbaseUserName, lapsDbasePassword);
    if ((rc != 0) || !lapsDbase.isOpen())
        guiCritical(s.sprintf("Error %d opening laps database file \"%s\": %s.\n\nWe will continue but lap times and statistics are not being recorded.", rc, lapsDbase.absoluteFilePath().toLatin1().data(), lapsDbase.errorText().toLatin1().data()));
    else
        messagesWindow->addMessage(s.sprintf("Opened laps database file \"%s\"", lapsDbase.absoluteFilePath().toLatin1().data()));


    // Set value of mainWindow pointer so we can refer to dbase from usersWindow

    mainWindow = this;


    // Configure users window - must come after membershipDbase is initialized

    usersWindow = new CUsers();
    if (!usersWindow) {
        guiCritical("Fatal internal error: could not create usersWindow");
        exit(1);
    }





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
    ui->lapsTableView->setColumnWidth(LT_READER, CW_READER);
    ui->lapsTableView->setColumnWidth(LT_ANTENNA, CW_ANTENNA);
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
    ui->activeRidersTableView->setColumnWidth(AT_LAPSEC, CW_SEC);
    ui->activeRidersTableView->setColumnWidth(AT_LAPSPEED, CW_SPEED);
    ui->activeRidersTableView->setColumnWidth(AT_BESTLAPSPEED, CW_SPEED);
    ui->activeRidersTableView->setColumnWidth(AT_AVERAGESPEED, CW_SPEED);
    ui->activeRidersTableView->setColumnWidth(AT_KMTHISMONTH, CW_KM);
    ui->activeRidersTableView->setColumnWidth(AT_AVERAGESPEEDTHISMONTH, CW_SPEED);
    ui->activeRidersTableView->setColumnWidth(AT_KMLASTMONTH, CW_KM);
    ui->activeRidersTableView->setColumnWidth(AT_AVERAGESPEEDLASTMONTH, CW_SPEED);
//    ui->activeRidersTableView->setColumnWidth(AT_LAPCOUNTALLTIME, CW_LAPCOUNT);
//    ui->activeRidersTableView->setColumnWidth(AT_KMALLTIME, CW_KM);

    ui->activeRidersTableView->setAlternatingRowColors(true);
    ui->activeRidersTableView->horizontalHeader()->setStretchLastSection(true);
    ui->activeRidersTableView->horizontalHeader()->setStyleSheet("QHeaderView{font: bold;}");
    ui->activeRidersTableView->sortByColumn(1, Qt::DescendingOrder);     // must come before call to setSortingEnabled()
    ui->activeRidersTableView->setSortingEnabled(true);
    ui->activeRidersTableSortEnableCheckBox->setChecked(true);
    ui->activeRidersTableView->setEnabled(false);   // will be enabled when connected to reader

    ui->bestsLabel->clear();

//    ui->activeRidersTableSortEnableCheckBox->hide();
    connect(ui->activeRidersTableSortEnableCheckBox, SIGNAL(clicked(bool)), this, SLOT(onActiveRidersTableSortEnableCheckBoxClicked(bool)));

    connect(ui->activeRidersTableView, SIGNAL(clicked(const QModelIndex &)), this, SLOT(onActiveRidersTableClicked(const QModelIndex &)));
    connect(ui->activeRidersTableView, SIGNAL(doubleClicked(const QModelIndex &)), this, SLOT(onActiveRidersTableDoubleClicked(const QModelIndex &)));



    connect(ui->lapsTableView, SIGNAL(clicked(const QModelIndex &)), this, SLOT(onLapsTableClicked(const QModelIndex &)));
    connect(ui->lapsTableView, SIGNAL(doubleClicked(const QModelIndex &)), this, SLOT(onLapsTableDoubleClicked(const QModelIndex &)));

//    connect(ui->namesTableView, SIGNAL(clicked(const QModelIndex &)), this, SLOT(onNamesTableClicked(const QModelIndex &)));
//    connect(ui->namesTableView, SIGNAL(doubleClicked(const QModelIndex &)), this, SLOT(onNamesTableDoubleClicked(const QModelIndex &)));



    // Set track length corresponding to position of each antenna.  Lap speed is estimated from
    // these values assuming rider maintains same position on track for entire lap (black / blue line).
    // If this value is not set correctly, cycling speed and distance will be in error.

    messagesWindow->addMessage(s.sprintf("Track lengths for each antenna (m): %f %f %f %f", preferencesWindow->trackLengthM[0], preferencesWindow->trackLengthM[1], preferencesWindow->trackLengthM[2], preferencesWindow->trackLengthM[3]));


    // Check that track length values are reasonable and warn user that application will not start functioning
    // until configured

    bool initialized = true;
    float maxTrackLength = 0.;
    for (int i=0; i<preferencesWindow->trackLengthM.size(); i++) {
        if (preferencesWindow->trackReaderAntennaEnable[i] && preferencesWindow->trackLengthM[i] <= 0.)
            initialized = false;

        if (preferencesWindow->trackLengthM[i] > maxTrackLength)
            maxTrackLength = preferencesWindow->trackLengthM[i];
    }
    if (maxTrackLength <= 0.)
        initialized = false;

    if (!initialized)
        guiCritical("One or more track-length values are not configured properly in Preferences.  Enable at least one antenna, disable unused antennas, and set track length correctly for each enabled antenna. The application will not operate until properly configured.");

    float nominalLapSec = maxTrackLength / 1000. / nominalSpeedkmph * 3600.;
    maxAcceptableLapSec = nominalLapSec * 4.;   // max acceptable time for lap.  If greater, rider must have left and returned to track
    minAcceptableLapSec = nominalLapSec / 4.;   // min acceptable time for lap.  If less, rider must be passing tag over antenna manually to cheat


    connect(ui->actionPreferences, SIGNAL(triggered()), this, SLOT(onActionPreferences()));
    connect(ui->actionMessages, SIGNAL(triggered()), this, SLOT(onActionMessages()));
    connect(ui->actionUsers, SIGNAL(triggered()), this, SLOT(onActionUsers()));
    connect(ui->actionHelpAbout, SIGNAL(triggered()), this, SLOT(onActionHelpAbout()));
    connect(ui->actionExit, SIGNAL(triggered()), this, SLOT(onActionExit()));


    // Create tray icon

    trayIcon = new QSystemTrayIcon(QIcon(":/images/cycle3.png"), this);

    connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(onShowHide(QSystemTrayIcon::ActivationReason)));

    QAction *quitAction = new QAction("Exit", trayIcon);
    connect(quitAction, SIGNAL(triggered()), this, SLOT(onActionExit()));

    QAction *showHideAction = new QAction("Show/Hide", trayIcon);
    connect(showHideAction, SIGNAL(triggered()), this, SLOT(onShowHide()));

    QMenu *trayIconMenu = new QMenu;
    trayIconMenu->addAction(showHideAction);
    trayIconMenu->addAction(quitAction);

    trayIcon->setContextMenu(trayIconMenu);
    trayIcon->setToolTip("Click to show/hide the " + QCoreApplication::applicationName() + " application window");
    trayIcon->show();


//    connect(ui->eMailTestPushButton, SIGNAL(clicked()), this, SLOT(onEMailTestPushButtonClicked()));
//    connect(ui->saveSessionsPushButton, SIGNAL(clicked()), this, SLOT(onSaveSessionsPushButtonClicked()));

    // Get a list of laps already in dBase that could be from current session (will happen if application stopped and restarted).

    QList<int> lapsInSession;
    lapsDbase.getLaps(NULL, QDateTime::currentDateTime().addSecs(-3 * 3600), QDateTime::currentDateTime(), CLapsDbase::reportAny, &lapsInSession);


    // Create CReader objects for each physical reader device.  At this point the code accepts only 1 track reader
    // and 1 desk reader.

    int readerCounter = 0;
    trackReader = new CReader(preferencesWindow->trackReaderIp, ++readerCounter, CReader::track);

    deskReader = new CReader(preferencesWindow->deskReaderIp, ++readerCounter, CReader::desk);


    // Move CReader objects to separate threads and start

    if (trackReader) {
        trackReaderThread = new QThread(this);
        trackReader->moveToThread(trackReaderThread);
        trackReader->thread = trackReaderThread;
        //readerThreadList.append(trackReaderThread);
        connect(trackReaderThread, SIGNAL(started(void)), trackReader, SLOT(onStarted(void)));
        connect(trackReaderThread, SIGNAL(finished(void)), trackReaderThread, SLOT(deleteLater(void)));
        connect(trackReader, SIGNAL(newLogMessage(QString)), messagesWindow, SLOT(addMessage(QString)));
        connect(trackReader, SIGNAL(connected(void)), this, SLOT(onReaderConnected(void)));
        connect(trackReader, SIGNAL(newTag(CTagInfo)), this, SLOT(onNewTrackTag(CTagInfo)));
        connect(trackReader, SIGNAL(status(QString)), preferencesWindow, SLOT(onTrackReaderStatus(QString)));
//        if (initialized)
            trackReaderThread->start();
    }

    if (deskReader) {
        deskReaderThread = new QThread(this);
        deskReader->moveToThread(deskReaderThread);
        deskReader->thread = deskReaderThread;
        //readerThreadList.append(deskReaderThread);
        connect(deskReaderThread, SIGNAL(started(void)), deskReader, SLOT(onStarted(void)));
        connect(deskReaderThread, SIGNAL(finished(void)), deskReaderThread, SLOT(deleteLater(void)));
        connect(deskReader, SIGNAL(newLogMessage(QString)), messagesWindow, SLOT(addMessage(QString)));
        connect(deskReader, SIGNAL(connected(void)), this, SLOT(onReaderConnected(void)));
        connect(deskReader, SIGNAL(newTag(CTagInfo)), this, SLOT(onNewDeskTag(CTagInfo)));
        connect(deskReader, SIGNAL(status(QString)), preferencesWindow, SLOT(onDeskReaderStatus(QString)));
        deskReaderThread->start();
    }


    // Start timer that will purge old riders from activeRidersTable

    connect(&purgeActiveRidersListTimer, SIGNAL(timeout(void)), this, SLOT(onPurgeActiveRidersList(void)));
    int intervalMSec = (int)(preferencesWindow->tablePurgeIntervalHours * 3600. * 1000. / 4.);
    if (intervalMSec < 1000)
        intervalMSec = 1000;
    purgeActiveRidersListTimer.setInterval(intervalMSec);
    purgeActiveRidersListTimer.start();

//    mainWindow = this;
}




MainWindow::~MainWindow() {
    cleanExit();
}



// Override close event to minimize only
//
void MainWindow::closeEvent (QCloseEvent *event) {
    if (messagesWindow)
        messagesWindow->hide();
    if (preferencesWindow)
        preferencesWindow->hide();
    hide();
//    showMinimized();
    event->ignore();
}



// Get confirmation from user before exiting
//
void MainWindow::onActionExit(void) {
    int rc = guiQuestion("Closing " + QCoreApplication::applicationName() + " will stop recording of lap events.  Are you sure you want to exit?", QMessageBox::Yes | QMessageBox::No);
    if (rc == QMessageBox::Yes)
        cleanExit();
}



void MainWindow::onActiveRidersTableClearPushButtonClicked(bool) {
    if (activeRidersTableModel)
        activeRidersTableModel->purgeTable();
    ui->bestsLabel->clear();
    trackSessionBestLapKmph = 0.;
    trackSessionBestLapKmphName.clear();
}




void MainWindow::cleanExit(bool /*flag*/) {
    if (messagesWindow)
        messagesWindow->addMessage("Clean exit requested");

    // Wait for event queue to clear

//    QTimer::singleShot(0, this, SLOT(hide()));

    membershipDbase.close();
    lapsDbase.close();

    if (trackReaderThread) {
        trackReaderThread->requestInterruption();
        trackReaderThread->wait();
        delete trackReaderThread;
    }

    if (deskReaderThread) {
        deskReaderThread->requestInterruption();
        deskReaderThread->wait();
        delete deskReaderThread;
    }

    for (int i=0; i<plotList.size(); i++) {
        plotList[i]->close();
    }

    if (preferencesWindow) {
        preferencesWindow->close();
        delete preferencesWindow;
        preferencesWindow = NULL;
    }

    if (messagesWindow) {
        messagesWindow->close();
        delete messagesWindow;
        messagesWindow = NULL;
    }

    delete ui;
    exit(0);
}



void MainWindow::onActionHelpAbout(void) {
    QMessageBox::about(this, QCoreApplication::applicationName(), "Text");
}



void MainWindow::onActionPreferences(void) {
    if (preferencesWindow)
        preferencesWindow->show();
}


void MainWindow::onActionMessages(void) {
    if (messagesWindow)
        messagesWindow->show();
}


void MainWindow::onActionUsers(void) {
    if (usersWindow)
        usersWindow->show();
}



void MainWindow::onShowHide(QSystemTrayIcon::ActivationReason reason) {
    switch (reason) {
    case QSystemTrayIcon::Unknown:
    case QSystemTrayIcon::Context:
    case QSystemTrayIcon::MiddleClick:
        break;
    case QSystemTrayIcon::Trigger:
    case QSystemTrayIcon::DoubleClick:
        if (isVisible()) {
            hide();
        }
        else {
            show();
            raise();
            setFocus();
        }
    }
}




void MainWindow::onActiveRidersTableClicked(const QModelIndex &index) {
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(index.data().toString(), QClipboard::Clipboard);
}



void MainWindow::onActiveRidersTableDoubleClicked(const QModelIndex &index) {

    // Check whether name is in activeRidersTable - if yes, get tagId and show plot

    QString tagId;
    QString name = index.data().toString();
    int tableIndex = -1;
    for (int i=0; i<activeRidersTableModel->activeRidersList.size(); i++) {
        if (name == activeRidersTableModel->activeRidersList[i].name) {
            tableIndex = i;
            break;
        }
    }
    if (tableIndex >= 0) {
        tagId = activeRidersTableModel->activeRidersList[tableIndex].tagId;

//        CPlotForm *plotForm = new CPlotForm();
//        plotForm->show();

        QList<CLapInfo> laps;
        lapsDbase.getLapInfo(tagId, QDateTime::currentDateTime().addYears(-100), QDateTime::currentDateTime(), &laps);
        cplot *plotLapSpeed = new cplot(activeRidersTableModel->activeRidersList[tableIndex].name + " Lap Speed", cplot::enableAll, NULL);
        plotLapSpeed->addPoints(laps);
        plotLapSpeed->addHiddenPoint(QDateTime::currentDateTime(), 0.);
        plotLapSpeed->addHiddenPoint(QDateTime::currentDateTime(), 30.);
        plotLapSpeed->show();
        plotList.append(plotLapSpeed);

//        cplot *plotAverageSpeed = new cplot(activeRidersTableModel->activeRidersList[tableIndex].name + " Average Speed", cplot::enableAll, NULL);
//        plotAverageSpeed->addPoints(laps);
//        plotAverageSpeed->show();
//        plotList.append(plotAverageSpeed);
    }

}



void MainWindow::onLapsTableClicked(const QModelIndex &index) {
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(index.data().toString(), QClipboard::Clipboard);
}



void MainWindow::onLapsTableDoubleClicked(const QModelIndex &index) {
    QString tagId;

    // Check whether name is in activeRidersTable - if yes, get tagId

    QString name = index.data().toString();
    int tableIndex = -1;
    for (int i=0; i<activeRidersTableModel->activeRidersList.size(); i++) {
        if (name == activeRidersTableModel->activeRidersList[i].name) {
            tableIndex = i;
            if (tagId >= 0)
                tagId = activeRidersTableModel->activeRidersList[tableIndex].tagId;
            break;
        }
    }

    // If tagId not found, treat name as tagId

    if (tagId.isEmpty())
        tagId = index.data().toString();

    // Search dbase for tagId and plot if found

    QList<CLapInfo> laps;
    lapsDbase.getLapInfo(tagId, QDateTime::currentDateTime().addYears(-100), QDateTime::currentDateTime(), &laps);
    if (!laps.isEmpty()) {
        cplot *plotLapSpeed = new cplot(name + " Lap Speed", cplot::enableAll, NULL);
        plotLapSpeed->addPoints(laps);
        plotLapSpeed->addHiddenPoint(QDateTime::currentDateTime(), 0.);
        plotLapSpeed->addHiddenPoint(QDateTime::currentDateTime(), 30.);
        plotLapSpeed->show();
        plotList.append(plotLapSpeed);
    }
}




//void MainWindow::onNamesTableDoubleClicked(const QModelIndex &index) {
//    QString tagId = index.data().toString();
//    int id = membershipDbase.getIdFromTagId(tagId);
//    if (id < 0)
//        return;

//    CMembershipInfo info;
//    membershipDbase.getAllFromId(id, &info);

//    QList<CLapInfo> laps;
//    lapsDbase.getLapInfo(tagId, QDateTime::currentDateTime().addYears(-100), QDateTime::currentDateTime(), &laps);
//    cplot *plot = new cplot(info.firstName + " " + info.lastName);
//    plot->addPoints(laps);
//    plot->addHiddenPoint(QDateTime::currentDateTime(), 0.);
//    plot->addHiddenPoint(QDateTime::currentDateTime(), 30.);
//    plot->show();
//    plotList.append(plot);
//}





// ***********************************************************************************
//
// Member functions related to settings panel




//void MainWindow::onSaveSessionsPushButtonClicked(void) {
//    QString s;
//    for (int row=0; row<ui->sessionsTableWidget->rowCount(); row++) {
//        settings.setValue(s.sprintf("scheduleItem%dDay", row), ui->sessionsTableWidget->item(row, 0)->text());
//        settings.setValue(s.sprintf("scheduleItem%dSession", row), ui->sessionsTableWidget->item(row, 1)->text());
//        settings.setValue(s.sprintf("scheduleItem%dStartTime", row), ui->sessionsTableWidget->item(row, 2)->text());
//        settings.setValue(s.sprintf("scheduleItem%dEndTime", row), ui->sessionsTableWidget->item(row, 3)->text());
//    }
//}




//void MainWindow::onEMailTestPushButtonClicked(void) {
//    CSmtp *smtp = new CSmtp(ui->smtpUsernameLineEdit->text(), ui->smtpPasswordLineEdit->text(), ui->smtpServerLineEdit->text(), ui->smtpPortLineEdit->text().toInt());
//    connect(smtp, SIGNAL(completed(int)), this, SLOT(onTestMailSent(int)));
//    connect(smtp, SIGNAL(newLogMessage(QString)), messagesWindow, SLOT(addMessage(QString)));

//    QString body("This is test email message.");

//    body.append("\n\nReport generated by " + QCoreApplication::applicationName() + " " + QCoreApplication::applicationVersion());

//    smtp->sendMail(ui->emailFromLineEdit->text(), ui->emailTestToLineEdit->text(), ui->emailTestSubjectLineEdit->text(), body.toLatin1().data());
//}



//void MainWindow::onTestMailSent(void) {
//    QMessageBox::information(this, "EMail Test", "Email message sent");
//}



void MainWindow::onClockTimerTimeout(void) {
    QDateTime currentDateTime(QDateTime::currentDateTime());
    ui->rightTitleLabel->setText(QDateTime::currentDateTime().toString("ddd MMMM d yyyy  hh:mm:ss"));

    // Emit null tag every 10 sec

    if ((currentDateTime.time().second() % 10) == 0) {
        CTagInfo tagInfo;
        tagInfo.readerId = 0;
        tagInfo.antennaId = 0;
        tagInfo.timeStampUSec = QDateTime::currentMSecsSinceEpoch() * 1000;
        emit onNewTrackTag(tagInfo);
    }


    // Check to see if this is the first timeout after a specified time (midnight) and send email reports

//    static bool sentInThisInterval = false;
//    if ((currentDateTime.time().hour() % preferencesWindow->emailReportLatencyHours) == 0) {
//        if (!sentInThisInterval) {
//            sendInactiveRiderReports();
//            sentInThisInterval = true;
//        }
//    }
//    else {
//        sentInThisInterval = false;
//    }


    // Update session.  If changed, clear session bests

    QString session = getSession(currentDateTime);
    if (ui->scheduledSessionLineEdit->text() != session) {
        ui->scheduledSessionLineEdit->setText(session);
    }

}



// get session for specified dateTime

QString MainWindow::getSession(const QDateTime &dateTime) {
    QString session;
//    QString day = QDate::longDayName(dateTime.date().dayOfWeek());
//    for (int i=0; i<ui->sessionsTableWidget->rowCount(); i++) {
//        qDebug() << ui->sessionsTableWidget->item(i, 0)->text();
//        QString sessionDay = ui->sessionsTableWidget->item(i, 0)->text();
//        QTime sessionStartTime(QTime::fromString(ui->sessionsTableWidget->item(i, 2)->text(), "hh:mm"));
//        QTime sessionEndTime(QTime::fromString(ui->sessionsTableWidget->item(i, 3)->text(), "hh:mm"));
//        if ((day == sessionDay) && (dateTime.time() >= sessionStartTime) && (dateTime.time() < sessionEndTime)) {
//            session = ui->sessionsTableWidget->item(i, 1)->text();
//            break;
//        }
//    }
    return session;
}



// *********************************************************************************************
//
// sendInactiveRiderReports()
// This routing is called on a regular basis whether previous attempts to send reports were
// successful or not.
// Look through lapsDbase for laps within the last week with pending reports

void MainWindow::sendInactiveRiderReports(void) {
//    if (!preferencesWindow->emailSendReportsCheckBox->isChecked())
//        return;

    if (!lapsDbase.isOpen())
        return;

    // Get a copy of membershipDbase

    QList<CMembershipInfo> infoList;
    membershipDbase.getAllList(&infoList);

    // Create a list of membershipInfo of riders who have unreported laps in the past week

    membershipInfoNotReported.clear();
    QDate currentDate(QDate::currentDate());
    QDate dateStart = currentDate.addDays(-7);
    dateTimeOfReportStart = QDateTime(dateStart, QTime(0, 0, 0));
    dateTimeOfReportEnd = QDateTime(currentDate, QTime(24, 0, 0));

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
            int rc = lapsDbase.getLaps(infoList[i].tagId, dateTimeOfReportStart, dateTimeOfReportEnd, CLapsDbase::reportPending, &lapsNotReported);
            if (rc != 0) {
                qDebug() << "Error from lapsDbase.getLapsInPeriod()";
                return;
            }
            for (int j=0; j<lapsNotReported.size(); j++) {

                // Get dateTime for lap and append to tagIdNotReported and dateTimeNotReported

                QString tagId;
                CLapInfo lap;
                rc = lapsDbase.getLap(lapsNotReported[j], &tagId);//, &lap);
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

    QString body("This is an automatic email report describing recent cycling activity at the " + preferencesWindow->trackName + ".  Do not reply to this message.\n\n");
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

        QDateTime reportPeriodStart(reportDate, QTime(0, 0, 0));
        QDateTime reportPeriodEnd(reportDate.addDays(1), QTime(0, 0, 0));
        CStats statsForDay;
        int rc = lapsDbase.getStats(memberToReport->tagId, reportPeriodStart, reportPeriodEnd, CLapsDbase::reportAny, &statsForDay);
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
                    QDateTime sessionStart(reportDate, sessionStartTime);
                    QDateTime sessionEnd(reportDate, sessionEndTime);
                    CStats stats;
                    rc = lapsDbase.getStats(memberToReport->tagId, sessionStart, sessionEnd, CLapsDbase::reportAny, &stats);
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
        if (statsForDay.lapCount > 0.)
            averageLapSec = statsForDay.totalSec / (float)statsForDay.lapCount;
        if (statsForDay.totalSec > 0.)
            speed = statsForDay.totalM / statsForDay.totalSec / 1000. * 3600.;
//        if (statsForDay.bestLapSec > 0.)
//            bestSpeed = statsForDay.bestLapM / statsForDay.bestLapSec / 1000. * 3600.;
//        if (statsForDay.bestLapSec > 0.)
//            bestLapSec = statsForDay.bestLapSec;
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
//                if (statsForSession[i].bestLapSec > 0.) bestSpeed = statsForSession[i].bestLapM / statsForSession[i].bestLapSec / 1000. * 3600.;
//                if (statsForSession[i].bestLapSec > 0.) bestLapSec = statsForSession[i].bestLapSec;
                body.append(s.sprintf("%26s %4d  %7.3f %7.2f      %6.2f      %6.2f      %6.2f\n", sessionList[i].toLatin1().data(), statsForSession[i].lapCount, statsForSession[i].totalM/1000., averageLapSec, speed, bestLapSec, bestSpeed));
            }
        }

    }
    body.append("\n\nReport generated by " + QCoreApplication::applicationName() + " " + QCoreApplication::applicationVersion());

    sendReport(*memberToReport, body);
}



void MainWindow::sendReport(const CMembershipInfo &info, const QString &body) {
    messagesWindow->addMessage("Sending email report to " + info.firstName + " " + info.lastName + " at " + info.eMail);

//    qDebug() << ui->emailFromLineEdit->text() << info.eMail << ui->emailSubjectLineEdit->text() << body.toLatin1().data();

    // Create smtp client

//    smtp = new CSmtp(ui->smtpUsernameLineEdit->text(), ui->smtpPasswordLineEdit->text(), ui->smtpServerLineEdit->text(), ui->smtpPortLineEdit->text().toInt());
//    connect(smtp, SIGNAL(completed(int)), this, SLOT(onMailSent(int)));
//    connect(smtp, SIGNAL(newLogMessage(QString)), messagesWindow, SLOT(addMessage(QString)));
//    smtp->sendMail(ui->emailFromLineEdit->text(), info.eMail, ui->emailSubjectLineEdit->text(), body.toLatin1().data());
}



//void MainWindow::onMailSent(int error) {

//    // If there was an error sending reports, do not clear list or update reportStatus in dbase

//    if (error != 0) {
//        messagesWindow->addMessage("  Email report not sent");
//        return;
//    }


//    // Remove all entries from notReported lists for first rider on list

//    QString tagIdBeingRemoved = membershipInfoNotReported[0].tagId;
//    for (int i=membershipInfoNotReported.size()-1; i>=0; i--) {
//        if (membershipInfoNotReported[i].tagId == tagIdBeingRemoved) {
//            membershipInfoNotReported.removeAt(i);

//            int rc = lapsDbase.setReportStatus(CLapsDbase::reportCompleted, tagIdBeingRemoved, dateTimeOfReportStart, dateTimeOfReportEnd);
//            messagesWindow->addMessage("  Email report sent");
//            if (rc != 0) {
//                qDebug() << "Error from lapsDbase.setReported";
//                return;
//            }

//        }
//    }

//    emit prepareNextReport();
//}


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

    if (activeRidersTableModel && activeRidersTableModel->activeRidersList.isEmpty()) {
        ui->bestsLabel->clear();
        trackSessionBestLapKmph = 0.;
        trackSessionBestLapKmphName.clear();
        trackSessionBestKKmph = 0.;
        trackSessionBestKKmphName.clear();
    }
}



void MainWindow::onReaderConnected(void) {
    CReader *sendingReader = (CReader *)sender();

    // If deskReader, block signals until we want to read from reader

    if (sendingReader == deskReader)
        deskReader->blockSignals(true);

    QString s;
    ui->tabWidget->setCurrentIndex(0);
    messagesWindow->addMessage(s.sprintf("Connected to reader %d", sendingReader->readerId));

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
        activeRidersProxyModel->setDynamicSortFilter(false);
        ui->activeRidersTableView->setSortingEnabled(false);
    }
}



// Process new tag
//
void MainWindow::onNewTrackTag(CTagInfo tagInfo) {
    QString s;
    static int tagCount = 0;

    // Add string to messages window if detailedMessages is checked

    if (messagesWindow && messagesWindow->detailedMessagesIsChecked())
        messagesWindow->addMessage(s.sprintf("readerId=%d antennaId=%d timeStampUSec=%llu tagData=%s", tagInfo.readerId, tagInfo.antennaId, tagInfo.timeStampUSec, tagInfo.tagId.toLatin1().data()));

    // Tags from antennas not enabled are ignored

    if (!preferencesWindow || !preferencesWindow->trackReaderAntennaEnable[tagInfo.antennaId - 1])
        return;

    // Tags from antenna 0 are ignored

    if (tagInfo.antennaId == 0)
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

    // Confirm track length for the antenna triggering this read event is valid (> 0)

    if (!tagInfo.tagId.isEmpty() && (preferencesWindow->trackLengthM[tagInfo.antennaId - 1] == 0.)) {
        static bool warned = false;
        if (!warned) {
            warned = true;
            guiCritical(s.sprintf("Track length value for the antenna triggering this event (%d) is zero.  Please set track length in Settings panel.", tagInfo.antennaId));
        }
        return;
    }

    // Process tag (lapsTable updated from activeRidersTable)

    CRider *rider = activeRidersTableModel->newTag(tagInfo);        // this adds tag to lapsTable also
    if (rider && !rider->name.isEmpty()) {
        float lapKph = kph(rider->lapM, rider->lapSec);

        // Ignore lap if speed is really high (manually waving tag over antenna)

        if (lapKph > 100.) {
            messagesWindow->addMessage(s.sprintf("TagId=%s name=%s lapSpeed=%f Speed too high, reading ignored", rider->tagId.toLatin1().data(), rider->name.toLatin1().data(), lapKph));
            return;
        }

        // Best session lap kph

        bool updateBestLap = false;
        if (lapKph > trackSessionBestLapKmph) {
            trackSessionBestLapKmph = lapKph;
            trackSessionBestLapKmphName = rider->name;
            updateBestLap = true;
        }

        // Best session km

        bool updateBestKm = false;
        if (rider->bestKKmph > trackSessionBestKKmph) {
            trackSessionBestKKmph = rider->bestKKmph;
            trackSessionBestKKmphName = rider->name;
            updateBestKm = true;
        }

        // Best thisMonth lap

        if (rider->thisMonth.bestLapKmph > trackThisMonthBestLapKmph) {
            trackThisMonthBestLapKmph = rider->thisMonth.bestLapKmph;
            trackThisMonthBestLapKmphName = rider->name;
            updateBestKm = true;
        }

        // Best thisMonth km

        if (rider->thisMonth.bestKKmph > trackThisMonthBestKKmph) {
            trackThisMonthBestKKmph = rider->thisMonth.bestKKmph;
            trackThisMonthBestKKmphName = rider->name;
            updateBestKm = true;
        }

        if ((trackSessionBestLapKmph > 0.)  && (updateBestLap || updateBestKm))
            ui->bestsLabel->setText(s.sprintf("Session Fastest Lap: %s (%.2f km/h)\nSession Fastest km: %s (%.2f km/h)\nMonth Fastest Lap: %s (%.2f km/h)\nMonth Fastest km: %s (%.2f km/h)", trackSessionBestLapKmphName.toLatin1().data(), trackSessionBestLapKmph, trackSessionBestKKmphName.toLatin1().data(), trackSessionBestKKmph, trackThisMonthBestLapKmphName.toLatin1().data(), trackThisMonthBestLapKmph, trackThisMonthBestKKmphName.toLatin1().data(), trackThisMonthBestKKmph));


        // Best allTime lap kph

        if (rider->allTime.bestLapKmph > trackAllTimeBestLapKmph) {
            trackAllTimeBestLapKmph = rider->allTime.bestLapKmph;
            trackAllTimeBestLapKmphName = rider->name;
            updateBestKm = true;
        }

        // Best allTime km

        if (rider->allTime.bestKKmph > trackAllTimeBestKKmph) {
            trackAllTimeBestKKmph = rider->allTime.bestKKmph;
            trackAllTimeBestKKmphName = rider->name;
            updateBestKm = true;
        }

    }
}



float MainWindow::kph(float lapM, float lapSec) {
    float kph = 0.;
    if (lapSec > 0.)
        kph = lapM / 1000. / lapSec * 3600.;
    return kph;
}






void MainWindow::guiCritical(QString s) {
    if (messagesWindow)
        messagesWindow->addMessage("Critical: " + s);
    QMessageBox::critical(this, QCoreApplication::applicationName() + " Critical Error", s, QMessageBox::Ok);
}


void MainWindow::guiInformation(QString s) {
    if (messagesWindow)
        messagesWindow->addMessage("Information: " + s);
    QMessageBox::information(this, QCoreApplication::applicationName() + "\n\n", s, QMessageBox::Ok);
}


QMessageBox::StandardButtons MainWindow::guiQuestion(QString s, QMessageBox::StandardButtons b) {
    return QMessageBox::question(this, QCoreApplication::applicationName() + " Question", s, b);
}


