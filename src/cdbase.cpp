// cdbase.cpp


#include <QMessageBox>

#include "cdbase.h"
#include "crider.h"




// ************************************************************************************************

CMembershipDbase::CMembershipDbase() {
    errorTextVal.clear();
    if (!QSqlDatabase::drivers().contains("QSQLITE"))
        qDebug() << "QSqlDatabase drivers:" << QSqlDatabase::drivers() << "does not contain QSQLITE";

    dBase = QSqlDatabase::addDatabase("QSQLITE", "membership");
}



int CMembershipDbase::open(const QString &rootName, const QString &username, const QString &password) {
    errorTextVal.clear();
    setFile(rootName + ".db");
    dBase.setUserName(username);
    dBase.setPassword(password);
    dBase.setDatabaseName(absoluteFilePath());

    if (!dBase.open()) {
        errorTextVal = dBase.lastError().text();
        qDebug() << "Error opening membership database:" << errorText();
        return 1;
    }

    bool showContents = false;

    // Make sure table exists and create as necessary

    QStringList tableList = dBase.tables();
    if (!tableList.contains("membershipTable")) {
        QSqlQuery query(dBase);
        qDebug() << "Creating new membershipTable in" << absoluteFilePath();
        query.prepare("create table membershipTable (id INTEGER PRIMARY KEY AUTOINCREMENT, tagId VARCHAR(20) UNIQUE, firstName VARCHAR(20), lastName VARCHAR(20), membershipNumber INTEGER, caRegistration VARCHAR(20), eMail VARCHAR(20), sendReports INTEGER)");
        if (!query.exec()) {
            errorTextVal = query.lastError().text();
            qDebug() << "Error creating new membershipTable:" << errorTextVal;
            return 2;
        }
        qDebug() << "  Created new membershipTable";
    }

    if (showContents) {
        QSqlQuery query(dBase);
        qDebug() << "List of membershipTable...";
        query.prepare("select * from membershipTable");
        if (!query.exec()) {
            errorTextVal = query.lastError().text();
            qDebug() << errorTextVal;
            return 3;
        }
        int id = query.record().indexOf("id");
        int idTagId = query.record().indexOf("tagId");
        int idFirst = query.record().indexOf("firstName");
        int idLast = query.record().indexOf("lastName");
        int idMembership = query.record().indexOf("membershipNumber");
        int idcaRegistration = query.record().indexOf("caRegistration");
        int idEmail = query.record().indexOf("eMail");
        int idSendReports = query.record().indexOf("sendReports");
        while (query.next()) {
            qDebug("id=%d tagId=%s name=%s %s membershipNumber=%s caRegistration=%s eMail=%s sendReports=%d", query.value(id).toString().toInt(), query.value(idTagId).toString().toLatin1().data(), query.value(idFirst).toString().toLatin1().data(), query.value(idLast).toString().toLatin1().data(), query.value(idMembership).toString().toLatin1().data(), query.value(idcaRegistration).toString().toLatin1().data(), query.value(idEmail).toString().toLatin1().data(), query.value(idSendReports).toInt());
        }
    }

    return 0;
}



void CMembershipDbase::close(void) {
    if (dBase.isOpen())
        dBase.close();
}




// add
// Add entry to membership database.  TagId and first-last name must be unique.
//
int CMembershipDbase::add(const CMembershipInfo &info) {
    if (!dBase.isOpen())
        return 1;

    QSqlQuery query(dBase);
    query.prepare("INSERT INTO membershipTable (tagId, firstName, lastName, membershipNumber, caRegistration, eMail, sendReports) VALUES (:tagId, :firstName, :lastName, :membershipNumber, :caRegistration, :eMail, :sendReports)");
    query.bindValue(":tagId", info.tagId);
    query.bindValue(":firstName", info.firstName);
    query.bindValue(":lastName", info.lastName);
    query.bindValue(":membershipNumber", info.membershipNumber);
    query.bindValue(":caRegistration", info.caRegistration.toUpper());
    query.bindValue(":eMail", info.eMail);
    query.bindValue(":sendReports", info.sendReports);
    if (!query.exec()) {
        errorTextVal = "Could not add to database.  Check that tagId, name, track membership number anc cycling association registration number are unique.";
        return 2;
    }
    return 0;
}



// update
//
int CMembershipDbase::update(const CMembershipInfo &info) {
    if (!dBase.isOpen())
        return 1;

    QSqlQuery query(dBase);
    query.prepare("UPDATE membershipTable SET firstName = :firstName, lastName = :lastName, membershipNumber = :membershipNumber, caRegistration = :caRegistration, email = :eMail, sendReports = :sendReports WHERE tagId = :tagId");
    query.bindValue(":tagId", info.tagId);
    query.bindValue(":firstName", info.firstName);
    query.bindValue(":lastName", info.lastName);
    query.bindValue(":membershipNumber", info.membershipNumber);
    query.bindValue(":caRegistration", info.caRegistration.toUpper());
    query.bindValue(":eMail", info.eMail);
    query.bindValue(":sendReports", info.sendReports);
    if (!query.exec()) {
        errorTextVal = "Could not update database.  Check that tagId, name, track membership number and cycling association registration are unique.";
        return 2;
    }

    return 0;
}


// removeTagId
// Remove entry with specified tagId
//
int CMembershipDbase::removeTagId(const QString &tagId) {
    if (!dBase.isOpen())
        return 1;

    QSqlQuery query(dBase);
    query.prepare("DELETE FROM membershipTable WHERE tagId = (:tagId)");
    query.bindValue(":tagId", tagId);
    if (!query.exec()) {
        errorTextVal = query.lastError().text();
        return 2;
    }
    return 0;
}



int CMembershipDbase::findTagIdFromName(const QString &firstName, const QString &lastName, QString *tagId) {
    if (!dBase.isOpen())
        return 1;

    QSqlQuery query(dBase);
    if (!lastName.isEmpty() && !firstName.isEmpty()) {
        query.prepare("SELECT tagId FROM membershipTable WHERE lastName = (:lastName) AND firstName = (:firstName)");
        query.bindValue(":lastName", lastName);
        query.bindValue(":firstName", firstName);
    }
    else if (!lastName.isEmpty()) {
        query.prepare("SELECT tagId FROM membershipTable WHERE lastName = (:lastName)");
        query.bindValue(":lastName", lastName);
    }
    else if (!firstName.isEmpty()) {
        query.prepare("SELECT tagId FROM membershipTable WHERE firstName = (:firstName)");
        query.bindValue(":firstName", firstName);
    }
    else return 2;

    if (!query.exec()) {
        errorTextVal = query.lastError().text();
        qDebug() << errorTextVal;
        return 3;
    }

    if (query.next()) {
        *tagId = query.value(0).toString();
        errorTextVal.clear();
        return 0;
    }
    errorTextVal = "Name not found";
    return 4;
}



// getIdFromTagId()
// Return 0 on error
//
int CMembershipDbase::getIdFromTagId(const QString &tagId) {
    if (!dBase.isOpen())
        return 0;

    if (tagId.isEmpty())
        return 0;

    QSqlQuery query(dBase);
    query.prepare("SELECT id FROM membershipTable WHERE tagId = (:tagId)");
    query.bindValue(":tagId", tagId);

    if (!query.exec()) {
        errorTextVal = query.lastError().text();
        qDebug() << errorTextVal;
        return 0;
    }

    int id = 0;
    if (query.next()) {
        id = query.value(0).toInt();
        errorTextVal.clear();
        return id;
    }
    errorTextVal = "TagId not found";
    return 0;
}



// getIdFromName()
// Return 0 on error
//
int CMembershipDbase::getIdFromName(const QString &firstName, const QString &lastName) {
    if (!dBase.isOpen())
        return 0;

    QSqlQuery query(dBase);
    if (!lastName.isEmpty() && !firstName.isEmpty()) {
        query.prepare("SELECT id FROM membershipTable WHERE lastName = (:lastName) AND firstName = (:firstName) COLLATE NOCASE");
        query.bindValue(":lastName", lastName);
        query.bindValue(":firstName", firstName);
    }
    else if (!lastName.isEmpty()) {
        query.prepare("SELECT id FROM membershipTable WHERE lastName = (:lastName) COLLATE NOCASE");
        query.bindValue(":lastName", lastName);
    }
    else if (!firstName.isEmpty()) {
        query.prepare("SELECT id FROM membershipTable WHERE firstName = (:firstName) COLLATE NOCASE");
        query.bindValue(":firstName", firstName);
    }
    else return 0;

    if (!query.exec()) {
        errorTextVal = query.lastError().text();
        qDebug() << errorTextVal;
        return 0;
    }

    int id = 0;
    if (query.next()) {
        id = query.value(0).toInt();
        errorTextVal.clear();
        return id;
    }
    errorTextVal = "Name not found";
    return 0;
}


// getIdFromMembershipNumber()
// Return 0 on error
//
int CMembershipDbase::getIdFromMembershipNumber(const QString &membershipNumber) {
    if (!dBase.isOpen())
        return 0;

    QSqlQuery query(dBase);
    query.prepare("SELECT id FROM membershipTable WHERE membershipNumber = (:membershipNumber)");
    query.bindValue(":membershipNumber", membershipNumber);

    if (!query.exec()) {
        errorTextVal = query.lastError().text();
        qDebug() << errorTextVal;
        return 0;
    }
    int id = 0;
    if (query.next()) {
        id = query.value(0).toInt();
        errorTextVal.clear();
        return id;
    }
    errorTextVal = "Name not found";
    return 0;
}



int CMembershipDbase::getAllFromId(int id, CMembershipInfo *info) {
    if (!dBase.isOpen())
        return 1;

    QSqlQuery query(dBase);
    query.prepare("SELECT * FROM membershipTable WHERE id = (:id)");
    query.bindValue(":id", id);
    if (!query.exec()) {
        errorTextVal = query.lastError().text();
        qDebug() << errorTextVal;
        return 2;
    }

    int tagIdIndex = query.record().indexOf("tagId");
    int firstNameIndex = query.record().indexOf("firstName");
    int lastNameIndex = query.record().indexOf("lastName");
    int membershipNumberIndex = query.record().indexOf("membershipNumber");
    int caRegistrationIndex = query.record().indexOf("caRegistration");
    int eMailIndex = query.record().indexOf("eMail");
    int sendReportsIndex = query.record().indexOf("sendReports");
    if ((tagIdIndex < 0) || (firstNameIndex < 0) || (lastNameIndex < 0) || (membershipNumberIndex < 0) || (caRegistrationIndex < 0) || (eMailIndex < 0) || (sendReportsIndex < 0)) {
        errorTextVal = "Could not find index";
        qDebug() << errorTextVal;
        return 3;
    }

    if (query.next()) {
        info->tagId = query.value(tagIdIndex).toString();
        info->firstName = query.value(firstNameIndex).toString();
        info->lastName = query.value(lastNameIndex).toString();
        info->membershipNumber = query.value(membershipNumberIndex).toString();
        info->caRegistration = query.value(caRegistrationIndex).toString();
        info->eMail = query.value(eMailIndex).toString();
        info->sendReports = query.value(sendReportsIndex).toBool();
        errorTextVal.clear();
        return 0;
    }
    errorTextVal = "Id not found";
    return 4;
}



int CMembershipDbase::getAllList(QList<CMembershipInfo> *infoList) {
    if (!dBase.isOpen())
        return 1;

    QSqlQuery query(dBase);
    query.prepare("SELECT * FROM membershipTable");
    if (!query.exec()) {
        errorTextVal = query.lastError().text();
        qDebug() << errorTextVal;
        return 2;
    }

    int tagIdIndex = query.record().indexOf("tagId");
    int firstNameIndex = query.record().indexOf("firstName");
    int lastNameIndex = query.record().indexOf("lastName");
    int membershipNumberIndex = query.record().indexOf("membershipNumber");
    int caRegistrationIndex = query.record().indexOf("caRegistration");
    int eMailIndex = query.record().indexOf("eMail");
    int sendReportsIndex = query.record().indexOf("sendReports");
    if ((tagIdIndex < 0) || (firstNameIndex < 0) || (lastNameIndex < 0) || (membershipNumberIndex < 0) || (caRegistrationIndex < 0) || (eMailIndex < 0) || (sendReportsIndex < 0)) {
        errorTextVal = "Could not find index";
        qDebug() << errorTextVal;
        return 3;
    }

    infoList->clear();

    errorTextVal.clear();
    while (query.next()) {
        infoList->append(CMembershipInfo());
        infoList->last().tagId = query.value(tagIdIndex).toString();
        infoList->last().firstName = query.value(firstNameIndex).toString();
        infoList->last().lastName = query.value(lastNameIndex).toString();
        infoList->last().membershipNumber = query.value(membershipNumberIndex).toString();
        infoList->last().caRegistration = query.value(caRegistrationIndex).toString();
        infoList->last().eMail = query.value(eMailIndex).toString();
        infoList->last().sendReports = query.value(sendReportsIndex).toBool();
    }
    return 0;
}



int CMembershipDbase::findNameFromTagId(const QString &tagId, QString *firstName, QString *lastName) {
    if (!dBase.isOpen())
        return 1;

    QSqlQuery query(dBase);
    query.prepare("SELECT * FROM membershipTable WHERE tagId = (:tagId)");
    query.bindValue(":tagId", tagId);
    if (!query.exec()) {
        errorTextVal = query.lastError().text();
        qDebug() << errorTextVal;
        return 1;
    }

    int firstNameIndex = query.record().indexOf("firstName");
    int lastNameIndex = query.record().indexOf("lastName");
    if ((firstNameIndex < 0) || (lastNameIndex < 0)) {
        errorTextVal = "Could not find index";
        qDebug() << errorTextVal;
        return 2;
    }

    if (query.next()) {
        *firstName = query.value(firstNameIndex).toString();
        *lastName = query.value(lastNameIndex).toString();
        errorTextVal.clear();
        return 0;
    }
    errorTextVal = "tag not found";
    return 3;
}




int CMembershipDbase::namesRowCount(void) {
    if (!dBase.isOpen())
        return 0;

    QSqlQuery query(dBase);
    query.prepare("SELECT Count(*) FROM membershipTable");
    if (!query.exec()) {
        errorTextVal = query.lastError().text();
        return -1;
    }

    if (query.next()) {
        int count = query.value(0).toInt();
        return count;
    }
    return 0;
}



QString CMembershipDbase::errorText(void) {
    return errorTextVal;
}



bool CMembershipDbase::isOpen(void) {
    return dBase.isOpen();
}











CLapInfo::CLapInfo(unsigned int dateTime, float lapSec, float lapM) {
    this->dateTime = dateTime;
    this->lapSec = lapSec;
    this->lapM = lapM;
    this->reportStatus = 0;
}



// CDateTime()
// Compact unsigned int value with dateTime stamp information to write to dBase.  Non-standard, but
// preserves chronological order and is very light weight.
//
// seconds (0 - 63) - 6 bits (0 - 5)
// minutes (0 - 63) - 6 bits (6 - 11)
// hours   (0 -31)  - 5 bits (12 - 16)
// day     (0 - 31) - 5 bits (17 - 21)
// month   (0 - 15) - 4 bits (22 - 25)
// year    (0 - 63) - 6 bits (26 - 31)
//
CDateTime::CDateTime(int year, int month, int day, int hour, int minute, int second) {
    if (year < 2000)
        yearVal = 2000;
    else
        yearVal = year;

    if (month < 0)
        monthVal = 0;
    else if (month > 15)
        monthVal = 15;
    else
        monthVal = month;

    if (day < 0)
        dayVal = 0;
    else if (day > 31)
        dayVal = 31;
    else
        dayVal = day;

    if (hour < 0)
        hourVal = 0;
    else if (hour > 15)
        hourVal = 15;
    else
        hourVal = hour;

    if (minute < 0)
        minuteVal = 0;
    else if (minute > 63)
        minuteVal = 63;
    else
        minuteVal = minute;

    if (second < 0)
        secondVal = 0;
    else if (second > 63)
        secondVal = 63;
    else
        secondVal = second;

    calculateUIntVal();
}



CDateTime::CDateTime(unsigned int dateTime) {
    yearVal = ((dateTime >> 26) & 0x3f) + 2000;
    monthVal = (dateTime >> 22) & 0x0f;
    dayVal = (dateTime >> 17) & 0x1f;
    hourVal = (dateTime >> 12) & 0x1f;
    minuteVal = (dateTime >> 6) & 0x3f;
    secondVal = (dateTime >> 0) & 0x3f;

    calculateUIntVal();
}



CDateTime::CDateTime(const QDateTime &dateTime) {
    yearVal = dateTime.date().year();
    if (yearVal < 2000)
        yearVal = 2000;

    monthVal = dateTime.date().month();
    dayVal = dateTime.date().day();
    hourVal = dateTime.time().hour();
    minuteVal = dateTime.time().minute();
    secondVal = dateTime.time().second();

    calculateUIntVal();
}



CDateTime::CDateTime(const QDate &date, int hour, int minute, int second) {
    yearVal = date.year();
    if (yearVal < 2000)
        yearVal = 2000;

    monthVal = date.month();
    dayVal = date.day();

    if (hour < 0)
        hourVal = 0;
    else if (hour > 15)
        hourVal = 15;
    else
        hourVal = hour;

    if (minute < 0)
        minuteVal = 0;
    else if (minute > 63)
        minuteVal = 63;
    else
        minuteVal = minute;

    if (second < 0)
        secondVal = 0;
    else if (second > 63)
        secondVal = 63;
    else
        secondVal = second;

    calculateUIntVal();
}



int CDateTime::year(void) const {
    return yearVal;
}



int CDateTime::month(void) const {
    return monthVal;
}



int CDateTime::day(void) const {
    return dayVal;
}



int CDateTime::hour(void) const {
    return hourVal;
}



int CDateTime::minute(void) const {
    return minuteVal;
}



int CDateTime::second(void) const {
    return secondVal;
}



unsigned int CDateTime::toUInt(void) const {
    return intVal;
}



QDate CDateTime::toQDate(void) const {
    return QDate(yearVal, monthVal, dayVal);
}



QTime CDateTime::toQTime(void) const {
    return QTime(hourVal, minuteVal, secondVal);
}



QDateTime CDateTime::toQDateTime(void) const {
    return QDateTime(QDate(yearVal, monthVal, dayVal), QTime(hourVal, minuteVal, secondVal));
}



void CDateTime::calculateUIntVal(void) {
    intVal = ((yearVal-2000) & 0x3f) << 26 | (monthVal & 0x0f) << 22 | (dayVal & 0x1f) << 17 | (hourVal & 0x1f) << 12 | (minuteVal & 0x3f) << 6 | (secondVal & 0x3f);
}





// ***************************************************************************************************
// Database files are associated with one year of data, with names like laps2018.db etc.
// Each file contains two tables.  The first, lapsTable, is a raw listing of each lap:
//   id: index id
//   tagId: character string showing tagId
//   dateTime: unsigned integer with timestamp
//   lapSec: time duration (s) of completed lap
//   lapM: distance of completed lap (m)
//   reportStatus: 0=not reported in email (yet), 1=reported
//
// The second table, priorsTable, is a summary of all laps in previous years:
//   id:
//   tagId:
//   lapSecTotal: sum of all lap times (s) completed before start of lapsTable
//   lapMTotal: sum of all lap distances (m) completed before start of lapsTable
//

CLapsDbase::CLapsDbase(void) {
    errorVal = 0;
    if (!QSqlDatabase::drivers().contains("QSQLITE"))
        qDebug() << "QSqlDatabase drivers:" << QSqlDatabase::drivers() << "does not contain QSQLITE";

    allTimeBestLapKph = 0.;
}



int CLapsDbase::open(const QString &rootName, const QString &username, const QString &password) {
    errorTextVal.clear();
    errorVal = 0;
    QString s;

    bool showContents = false;
    QDate currentDate(QDate::currentDate());

    // Close dbase if already open

    close();


    // Determine database file name (rootName + year + ".db")

    QString connectionName = rootName + s.setNum(currentDate.year());
    setFile(connectionName + ".db");


    // Make sure dbase for current year exists and create if necessary.

    dBase = QSqlDatabase::addDatabase("QSQLITE", connectionName);
    dBase.setUserName(username);
    dBase.setPassword(password);
    dBase.setDatabaseName(absoluteFilePath());

    if (!dBase.open()) {
        errorTextVal = dBase.lastError().text();
        errorVal = 1;
        return errorVal;
    }

    QSqlQuery query(dBase);


    // Make sure lapsTable exists in dbase

    if (!dBase.tables().contains("lapsTable")) {
//        qDebug() << "Creating new lapsTable in " + absoluteFilePath();
//        emit newLogMessage(QString("Creating new lapsTable"));
        query.prepare("create table lapsTable (id INTEGER PRIMARY KEY AUTOINCREMENT, tagId VARCHAR(20), dateTime UNSIGNED INTEGER, lapsec FLOAT, lapm FLOAT, reportStatus INTEGER)");
        if (!query.exec()) {
            errorTextVal = query.lastError().text();
            errorVal = 3;
            return errorVal;
        }
        qDebug() << "  Created new lapsTable";
    }

    if (showContents) {
        qDebug() << "List of lapsTable...";
        query.prepare("select * from lapsTable");
        if (!query.exec()) {
            errorTextVal = query.lastError().text();
            errorVal = 4;
            return errorVal;
        }
        int id = query.record().indexOf("id");
        int idTagId = query.record().indexOf("tagId");
        int idDateTime = query.record().indexOf("dateTime");
        int idLapSec = query.record().indexOf("lapsec");
        int idLapM = query.record().indexOf("lapm");
        int idReportStatus = query.record().indexOf("reportStatus");
        while (query.next()) {
            CDateTime dateTime(query.value(idDateTime).toUInt());
            float lapSec = query.value(idLapSec).toFloat();
            float lapM = query.value(idLapM).toFloat();
            int reportStatus = query.value(idReportStatus).toInt();
            if (query.value(idTagId).toString() == "201600000002") {
                qDebug("id=%d tagId=%s dateTime=%u (%d %d %d %d %d %d) lapSec=%f lapM=%f reportStatus=%d", query.value(id).toInt(), query.value(idTagId).toString().toLatin1().data(), dateTime.toUInt(), dateTime.year(), dateTime.month(), dateTime.day(), dateTime.hour(), dateTime.minute(), dateTime.second(), lapSec, lapM, reportStatus);
            }
        }
    }


    // Make sure priorsTable exists in dbase and create if not

    bool calculatePriorsRequired = true;//false;
    if (!dBase.tables().contains("priorsTable")) {
        qDebug() << "Creating new priorsTable in " + prior.absoluteFilePath();
        query.prepare("create table priorsTable (id INTEGER PRIMARY KEY AUTOINCREMENT, tagId VARCHAR(20) UNIQUE, name VARCHAR(20), lapCount INTEGER, lapSecTotal FLOAT, lapMTotal FLOAT)");
        if (!query.exec()) {
            errorTextVal = query.lastError().text();
            errorVal = 5;
            return errorVal;
        }
        calculatePriorsRequired = true;
        qDebug() << "  Created new priorsTable";
    }

    if (showContents) {
        qDebug() << "List of priorsTable...";
        query.prepare("select * from priorsTable");
        if (!query.exec()) {
            errorTextVal = query.lastError().text();
            errorVal = 6;
            return errorVal;
        }
        int idid = query.record().indexOf("id");
        int idTagId = query.record().indexOf("tagId");
        int idName = query.record().indexOf("name");
        int idLapCount = query.record().indexOf("lapCount");
        int idLapSecTotal = query.record().indexOf("lapSecTotal");
        int idLapMTotal = query.record().indexOf("lapMTotal");
        while (query.next()) {
            int id = query.value(idid).toInt();
            QString tagId = query.value(idTagId).toString();
            QString name = query.value(idName).toString();
            int lapCount = query.value(idLapCount).toInt();
            float lapSecTotal = query.value(idLapSecTotal).toFloat();
            float lapMTotal = query.value(idLapMTotal).toFloat();
            qDebug("id=%d tagId=%s name=%s lapCount=%d lapSecTotal=%f lapMTotal=%f", id, tagId.toLatin1().data(), name.toLatin1().data(), lapCount, lapSecTotal, lapMTotal);
        }
    }


    // Make a list of QSqlDatabase structures, one for each prior database file

    QFileInfo fileInfo;
    for (int year=currentDate.year() - 1; year>=2000; year--) {
        connectionName = rootName + s.setNum(year);
        fileInfo.setFile(connectionName + ".db");
        if (QFile::exists(fileInfo.absoluteFilePath())) {
            qDebug() << fileInfo.absoluteFilePath();
            dBasePriorList.append(QSqlDatabase());
            dBasePriorList.last() = QSqlDatabase::addDatabase("QSQLITE", connectionName);
            dBasePriorList.last().setUserName(username);
            dBasePriorList.last().setPassword(password);
            dBasePriorList.last().setDatabaseName(fileInfo.absoluteFilePath());
        }
        else {
            break;
        }

    }

    // If previous year dbase (first entry in dBasePriorList) exists, open and calculate totals if required

    if (dBasePriorList.size() > 0) {

        if (!dBasePriorList[0].isOpen()) {
            if (!dBasePriorList[0].open()) {
                errorTextVal = "Could not open previous year database";
                errorVal = 1;
                return errorVal;
            }
        }

        // If required, calculate totals from dBasePriorList[0]:lapsTable and put into dBase:priorsTable

        if (calculatePriorsRequired) {
            qDebug() << "Calculating priors";

            QSqlQuery queryPrior(dBasePriorList[0]);
            queryPrior.prepare("SELECT tagId FROM lapsTable");
            if (!queryPrior.exec()) {
                errorTextVal = queryPrior.lastError().text();
                errorVal = 7;
                return errorVal;
            }
            QStringList tagIdList;
            int idTagId = queryPrior.record().indexOf("tagId");
            while (queryPrior.next()) {
                QString tagId = queryPrior.value(idTagId).toString();
                if (!tagIdList.contains(tagId))
                    tagIdList.append(tagId);
            }
            qDebug() << tagIdList;

            // For each tagId, calculate lapCount, lapSecTotal and lapMTotal, including priors

            QList<float> lapSecTotal;
            QList<float> lapMTotal;
            QList<int> lapCount;
            for (int i=0; i<tagIdList.size(); i++) {
                queryPrior.prepare("SELECT lapSec, lapM FROM lapsTable WHERE tagId = :tagId");
                queryPrior.bindValue(":tagId", tagIdList[i]);
                if (!queryPrior.exec()) {
                    errorTextVal = queryPrior.lastError().text();
                    errorVal = 8;
                    return errorVal;
                }
                int idLapSec = queryPrior.record().indexOf("lapSec");
                int idLapM = queryPrior.record().indexOf("lapM");
                lapSecTotal.append(0.);
                lapMTotal.append(0.);
                lapCount.append(0);
                while (queryPrior.next()) {
                    lapSecTotal[i] += queryPrior.value(idLapSec).toFloat();
                    lapMTotal[i] += queryPrior.value(idLapM).toFloat();
                    lapCount[i]++;
                }

                // Add in priors

                int lapCountPrior = 0;
                float lapSecPrior = 0.;
                float lapMPrior = 0.;
                int rc = getPriors(dBasePriorList[0], tagIdList[i], &lapCountPrior, &lapSecPrior, &lapMPrior);
                if (rc) {
                    return errorVal;
                }
                lapCount[i] += lapCountPrior;
                lapSecTotal[i] += lapSecPrior;
                lapMTotal[i] += lapMPrior;
            }

            // Write these priors to the current-year priorsTable

            for (int i=0; i<tagIdList.size(); i++) {
                query.prepare("INSERT INTO priorsTable (tagId, name, lapCount, lapSecTotal, lapMTotal) VALUES (:tagId, :name, :lapCount, :lapSecTotal, :lapMTotal)");
                query.bindValue(":tagId", tagIdList[i]);
                query.bindValue(":name", QString());
                query.bindValue(":lapCount", lapCount[i]);
                query.bindValue(":lapSecTotal", lapSecTotal[i]);
                query.bindValue(":lapMTotal", lapMTotal[i]);
                if (!query.exec()) {
                    errorTextVal = "Could not insert data into priorsTable of laps";
                    errorVal = 9;
                    return errorVal;
                }
            }
        }
    }

    // Read through all databases and calculate bests

    QList<CLapInfo> laps;
    getLapInfo(QString(), QDateTime::currentDateTime().addYears(-100), QDateTime::currentDateTime(), &laps);
//    qDebug() << laps.size();
//    for (int i=0; i<laps.size(); i++) {
//        qDebug() << laps[i].;
//    }
    return 0;
}



void CLapsDbase::close(void) {
    dBase.close();
    for (int i=0; i<dBasePriorList.size(); i++) {
        if (dBasePriorList[i].isOpen()) {
            dBasePriorList[i].close();
        }
    }
    dBasePriorList.clear();
}




// addLap
// Add lap database
//
int CLapsDbase::addLap(const CRider &rider, const QDateTime &dateTime) {
    errorTextVal.clear();
    errorVal = 0;

    if (!dBase.isOpen()) {
        errorTextVal = "CLapsDbase is closed";
        errorVal = 2;
        return errorVal;
    }

    QSqlQuery query(dBase);
    query.prepare("INSERT INTO lapsTable (tagId, dateTime, lapsec, lapm, reportStatus) VALUES (:tagId, :dateTime, :lapsec, :lapm, :reportStatus)");
    query.bindValue(":tagId", rider.tagId);
    query.bindValue(":dateTime", CDateTime(dateTime).toUInt());
    query.bindValue(":lapsec", rider.lapSec);
    query.bindValue(":lapm", rider.lapM);
    query.bindValue(":reportStatus", rider.reportStatus);
    if (!query.exec()) {
        errorTextVal = "Could not add to laps table";
        errorVal = 3;
        return errorVal;
    }
    return 0;
}



// getLapInfo()
// Appends CLapInfo list with information from each lap within specified period from specfied database for
// specified tagId
//
int CLapsDbase::getLapInfo(const QSqlDatabase &dBase, const QString &tagId, const QDateTime &start, const QDateTime &end, QList<CLapInfo> *laps) {
    if (!dBase.isOpen()) {
        errorTextVal = dBase.databaseName() + " closed in getLapInfo";
        errorVal = 2;
        return errorVal;
    }

    QSqlQuery query(dBase);
    if (tagId.isEmpty()) {
        query.prepare("SELECT * FROM lapsTable WHERE dateTime BETWEEN :dateTimeStart AND :dateTimeEnd");
        query.bindValue(":dateTimeStart", CDateTime(start).toUInt());
        query.bindValue(":dateTimeEnd", CDateTime(end).toUInt());
    }
    else {
        query.prepare("SELECT * FROM lapsTable WHERE tagId = :tagId AND dateTime BETWEEN :dateTimeStart AND :dateTimeEnd");
        query.bindValue(":tagId", tagId);
        query.bindValue(":dateTimeStart", CDateTime(start).toUInt());
        query.bindValue(":dateTimeEnd", CDateTime(end).toUInt());
    }
    if (!query.exec()) {
        errorTextVal = query.lastError().text();
        errorVal = 3;
        return errorVal;
    }

    int dateTimeIndex = query.record().indexOf("dateTime");
    if (dateTimeIndex < 0) {
        errorTextVal = "Could not find dateTime index in getLap";
        errorVal = 5;
        return errorVal;
    }

    int lapsecIndex = query.record().indexOf("lapsec");
    if (lapsecIndex < 0) {
        errorTextVal = "Could not find lapsec index in getLap";
        errorVal = 6;
        return errorVal;
    }

    int lapmIndex = query.record().indexOf("lapm");
    if (lapmIndex < 0) {
        errorTextVal = "Could not find lapm index in getLap";
        errorVal = 7;
        return errorVal;
    }

    while (query.next()) {
        laps->append(CLapInfo(query.value(dateTimeIndex).toUInt(), query.value(lapsecIndex).toFloat(), query.value(lapmIndex).toFloat()));
    }

    return 0;
}



// getLapInfo()
// Fill ClapsInfo list for all laps in all database files within specified period for specfied tagId
//
int CLapsDbase::getLapInfo(const QString &tagId, const QDateTime &start, const QDateTime &end, QList<CLapInfo> *laps) {
    laps->clear();

    // Look through list of dBases and get lap info from any within time period

    int rc;
    for (int i=dBasePriorList.size()-1; i>=0; i--) {
        rc = getLapInfo(dBasePriorList[i], tagId, start, end, laps);
    }
    rc = getLapInfo(dBase, tagId, start, end, laps);
    return rc;
}




int CLapsDbase::getLap(int id, QString *tagId, CLapInfo *lapInfo) {
    errorTextVal.clear();
    errorVal = 0;

    if (!dBase.isOpen()) {
        errorTextVal = "CLapsDbase closed";
        errorVal = 2;
        return errorVal;
    }

    QSqlQuery query(dBase);
    query.prepare("SELECT * FROM lapsTable WHERE id = :id");
    query.bindValue(":id", id);
    if (!query.exec()) {
        errorTextVal = query.lastError().text();
        errorVal = 3;
        return errorVal;
    }

    int tagIdIndex = query.record().indexOf("tagId");
    int dateTimeIndex = query.record().indexOf("dateTime");
    int lapsecIndex = query.record().indexOf("lapsec");
    int lapmIndex = query.record().indexOf("lapm");
    int reportStatusIndex = query.record().indexOf("reportStatus");

    if (tagIdIndex < 0) {
        errorTextVal = "Could not find tagId index in getLap";
        errorVal = 4;
        return errorVal;
    }

    if (lapInfo) {
        if (dateTimeIndex < 0) {
            errorTextVal = "Could not find dateTime index in getLap";
            errorVal = 5;
            return errorVal;
        }

        if (lapsecIndex < 0) {
            errorTextVal = "Could not find lapsec index in getLap";
            errorVal = 6;
            return errorVal;
        }

        if (lapmIndex < 0) {
            errorTextVal = "Could not find lapm index in getLap";
            errorVal = 7;
            return errorVal;
        }

        if (reportStatusIndex < 0) {
            errorTextVal = "Could not find reportStatus index in getLap";
            errorVal = 8;
            return errorVal;
        }
    }
    if (query.next()) {
        *tagId = query.value(tagIdIndex).toString();
        if (lapInfo) {
            lapInfo->dateTime = query.value(dateTimeIndex).toUInt();
            lapInfo->lapSec = query.value(lapsecIndex).toFloat();
            lapInfo->lapM = query.value(lapmIndex).toFloat();
            lapInfo->reportStatus = query.value(reportStatusIndex).toInt();
        }
    }

    return 0;
}



// Calculate stats for specified tagId
//
int CLapsDbase::getStats(const QString &tagId, CRider *rider) {
    errorTextVal.clear();
    errorVal = 0;

    if (!dBase.isOpen()) {
        errorTextVal = "CLapsDbase is closed";
        errorVal = 2;
        return errorVal;
    }

    rider->thisMonth.clear();
    rider->lastMonth.clear();
    rider->allTime.clear();

    QDate thisMonth(QDate::currentDate());
    QDate lastMonth(thisMonth.addMonths(-1));

    // Get stats for this month

    QDateTime dateTimeStart(QDate(thisMonth.year(), thisMonth.month(), 1), QTime(0, 0, 0));

    errorVal = getStats(tagId, dateTimeStart, QDateTime::currentDateTime(), CLapsDbase::reportAny, &rider->thisMonth);
    if (errorVal) return errorVal;

    // Get stats for last month

    dateTimeStart = QDateTime(QDate(lastMonth.year(), lastMonth.month(), 1), QTime(0, 0, 0));

    errorVal = getStats(tagId, dateTimeStart, QDateTime::currentDateTime(), CLapsDbase::reportAny, &rider->lastMonth);
    if (errorVal) return errorVal;


    // Get stats for all time

    dateTimeStart = QDateTime::currentDateTime().addYears(-100);

    errorVal = getStats(tagId, dateTimeStart, QDateTime::currentDateTime(), CLapsDbase::reportAny, &rider->allTime);
    if (errorVal)
        return errorVal;

    // Add priors

    int lapCountPrior = 0;
    float lapSecTotalPrior = 0.;
    float lapMTotalPrior = 0.;
    errorVal = getPriors(dBase, tagId, &lapCountPrior, &lapSecTotalPrior, &lapMTotalPrior);
    if (errorVal)
        return errorVal;
    rider->allTime.lapCount += lapCountPrior;
    rider->allTime.totalSec += lapSecTotalPrior;
    rider->allTime.totalM += lapMTotalPrior;

    return 0;
}



// Get stats from all database files for specified tag and period
//
int CLapsDbase::getStats(const QString &tagId, const QDateTime &start, const QDateTime &end, reportStatus_t reportStatus, CStats *stats) {
    stats->clear();
    int rc = getStats(dBase, tagId, start, end, reportStatus, stats);
    if (rc > 0) return rc;
    for (int i=0; i<dBasePriorList.size(); i++) {
        CStats newStats;
        rc = getStats(dBasePriorList[i], tagId, start, end, reportStatus, &newStats);
        if (rc > 0)
            return rc;
        stats->lapCount += newStats.lapCount;
        stats->totalM += newStats.totalM;
        stats->totalSec += newStats.totalSec;
        float statsSpeed = 0.;
        if (stats->bestLapSec > 0.) statsSpeed = stats->bestLapM / 1000. / stats->bestLapSec * 3600.;
        float newSpeed = 0.;
        if (newStats.bestLapSec > 0.) newSpeed = newStats.bestLapM / 1000. / newStats.bestLapSec * 3600.;
        if (newSpeed > statsSpeed) {
            stats->bestLapM = newStats.bestLapM;
            stats->bestLapSec = newStats.bestLapSec;
        }
    }
    return 0;
}




// Get stats from specified dbase for specified tagId and time period
//
int CLapsDbase::getStats(const QSqlDatabase &dBase, const QString &tagId, const QDateTime &dateTimeStart, const QDateTime &dateTimeEnd, reportStatus_t reportStatus, CStats *stats) {
    errorTextVal.clear();
    errorVal = 0;

    if (!dBase.isOpen()) {
        errorTextVal = "CLapsDbase closed";
        errorVal = 2;
        return errorVal;
    }

    if (dateTimeStart > dateTimeEnd) {
        errorTextVal = "dateTimeStart > dateTimeEnd in getStatsForPeriod";
        errorVal = 3;
        return errorVal;
    }

    QSqlQuery query(dBase);
    if (reportStatus == reportPending) {
        query.prepare("SELECT * FROM lapsTable WHERE tagId = :tagId AND reportStatus = :reportStatus AND dateTime BETWEEN :dateTimeStart AND :dateTimeEnd");
    }
    else {
        query.prepare("SELECT * FROM lapsTable WHERE tagId = :tagId AND dateTime BETWEEN :dateTimeStart AND :dateTimeEnd");
    }
    query.bindValue(":tagId", tagId);
    query.bindValue(":reportStatus", CLapsDbase::reportPending);
    query.bindValue(":dateTimeStart", CDateTime(dateTimeStart).toUInt());
    query.bindValue(":dateTimeEnd", CDateTime(dateTimeEnd).toUInt());
    if (!query.exec()) {
        errorTextVal = query.lastError().text();
        errorVal = 3;
        return errorVal;
    }

    int lapsecIndex = query.record().indexOf("lapsec");
    if (lapsecIndex < 0) {
        errorTextVal = "Could not find lapsec index in getStatsForPeriod";
        errorVal = 4;
        return errorVal;
    }

    int lapmIndex = query.record().indexOf("lapm");
    if (lapmIndex < 0) {
        errorTextVal = "Could not find lapm index in getStatsforPeriod";
        errorVal = 5;
        return errorVal;
    }

    int dateTimeIndex = query.record().indexOf("dateTime");
    if (dateTimeIndex < 0) {
        errorTextVal = "Could not find dateTime index in getStatusForPeriod";
        errorVal = 6;
        return errorVal;
    }


    // Determine workout count, lap count, average lap time, best lap time, distance

    float localBestLapSec = -1.;
    float localBestLapM = 0.;
    QDateTime previousLapDateTime = QDateTime::currentDateTime().addYears(-100);
    int localWorkoutCount = 0;
    int localLapCount = 0;
    float localTotalSec = 0.;
    float localTotalM = 0.;
    while (query.next()) {
        float lapSec = query.value(lapsecIndex).toFloat();
        float lapM = query.value(lapmIndex).toFloat();
        QDateTime lapDateTime = CDateTime(query.value(dateTimeIndex).toUInt()).toQDateTime();
        if (localBestLapSec <= 0.) {
            localBestLapSec = lapSec;
            localBestLapM = lapM;
        }
        else if (lapSec < localBestLapSec) {
            localBestLapSec = lapSec;
            localBestLapM = lapM;
        }
        localTotalSec += lapSec;
        localLapCount++;
        localTotalM += lapM;
        if (previousLapDateTime.secsTo(lapDateTime) > (4 * 3600)) {
            localWorkoutCount++;
        }
        previousLapDateTime = lapDateTime;
    }

    stats->lapCount = localLapCount;
    stats->workoutCount = localWorkoutCount;
    stats->bestLapSec = localBestLapSec;
    stats->bestLapM = localBestLapM;
    stats->totalSec = localTotalSec;
    stats->totalM = localTotalM;

    return 0;
}



// Get stats from specified dbase for specified tagId and time period
//
int CLapsDbase::getPriors(const QSqlDatabase &dBase, const QString &tagId, int *lapCount, float *lapSecTotal, float *lapMTotal) {
    errorTextVal.clear();
    errorVal = 0;

    *lapCount = 0;
    *lapSecTotal = 0.;
    *lapMTotal = 0.;

    if (!dBase.isOpen()) {
        errorTextVal = "Database not open";
        errorVal = 1;
        return errorVal;
    }

    QSqlQuery query(dBase);
    query.prepare("SELECT lapCount, lapSecTotal, lapMTotal FROM priorsTable WHERE tagId = :tagId");
    query.bindValue(":tagId", tagId);
    if (!query.exec()) {
        errorTextVal = "Could not select priors";
        errorVal = 0;
        return errorVal;
    }

    int lapCountIndex = query.record().indexOf("lapCount");
    if (lapCountIndex < 0) {
        errorTextVal = "Could not find lapCount index in getPriors";
        errorVal = 4;
        return errorVal;
    }

    int lapSecTotalIndex = query.record().indexOf("lapSecTotal");
    if (lapSecTotalIndex < 0) {
        errorTextVal = "Could not find lapSecTotal index in getPriors";
        errorVal = 4;
        return errorVal;
    }

    int lapMTotalIndex = query.record().indexOf("lapMTotal");
    if (lapMTotalIndex < 0) {
        errorTextVal = "Could not find lapMTotal index in getPriors";
        errorVal = 5;
        return errorVal;
    }

    while (query.next()) {
        *lapCount = query.value(lapCountIndex).toInt();
        *lapSecTotal = query.value(lapSecTotalIndex).toFloat();
        *lapMTotal = query.value(lapMTotalIndex).toFloat();
    }
    return 0;
}



// Set reportStatus for specified tagId and time period
//
int CLapsDbase::setReportStatus(reportStatus_t reportStatus, const QString &tagId, const QDateTime &start, const QDateTime &end) {
    errorTextVal.clear();
    errorVal = 0;

    if (!dBase.isOpen()) {
        errorTextVal = "CLapsDbase closed";
        errorVal = 2;
        return errorVal;
    }

    if (start > end) {
        errorTextVal = "dateTimeStart > dateTimeEnd in getStatsForPeriod";
        errorVal = 3;
        return errorVal;
    }

    QSqlQuery query(dBase);
    query.prepare("UPDATE lapsTable SET reportStatus = :reportStatus WHERE tagId = :tagId AND dateTime BETWEEN :dateTimeStart AND :dateTimeEnd");
    query.bindValue(":tagId", tagId);
    query.bindValue(":reportStatus", reportStatus);
    query.bindValue(":dateTimeStart", CDateTime(start).toUInt());
    query.bindValue(":dateTimeEnd", CDateTime(end).toUInt());
    if (!query.exec()) {
        errorTextVal = "Could not update database.";
        return 4;
    }

    return 0;
}



// Get stats for specified tagId and time period from dbase
//
int CLapsDbase::getLaps(const QString &tagId, const QDateTime &start, const QDateTime &end, CLapsDbase::reportStatus_t reportStatus, QList<int> *lapsList) {
    errorTextVal.clear();
    errorVal = 0;

    if (!dBase.isOpen()) {
        errorTextVal = "CLapsDbase closed";
        errorVal = 2;
        return errorVal;
    }

    if (start > end) {
        errorTextVal = "dateTimeStart > dateTimeEnd in getLapsInPeriod";
        errorVal = 3;
        return errorVal;
    }

    QSqlQuery query(dBase);
    query.prepare("SELECT id FROM lapsTable WHERE tagId = :tagId AND reportStatus = :reportStatus AND dateTime BETWEEN :dateTimeStart AND :dateTimeEnd");
    query.bindValue(":tagId", tagId);
    query.bindValue(":dateTimeStart", CDateTime(start).toUInt());
    query.bindValue(":dateTimeEnd", CDateTime(end).toUInt());
    query.bindValue(":reportStatus", reportStatus);
    if (!query.exec()) {
        errorTextVal = query.lastError().text();
        errorVal = 4;
        return errorVal;
    }

    int idIndex = query.record().indexOf("id");
    if (idIndex < 0) {
        errorTextVal = "Could not find id index in getLapsInPeriod";
        errorVal = 5;
        return errorVal;
    }

    while (query.next()) {
        lapsList->append(query.value(idIndex).toInt());
    }
    return 0;
}



QString CLapsDbase::errorText(void) {
    return errorTextVal;
}



bool CLapsDbase::isOpen(void) {
    return dBase.isOpen();
}


