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
// Each file contains two tables.  The first, lapsTable, is a listing of raw lap data:
//   id: table index id
//   tagId: character string tagId
//   dateTime: unsigned integer timestamp
//   lapSec: float time duration (s) of completed lap
//   lapM: float distance of completed lap (m)
//   reportStatus: integer 0=not reported in email (yet), 1=reported
//
// The second table, priorsTable, is a summary of all laps in previous years (wip):
//   id:
//   tagId:
//   name:
//   lapSecTotal: sum of all lap times (s) completed before start of lapsTable
//   lapMTotal: sum of all lap distances (m) completed before start of lapsTable
//
CLapsDbase::CLapsDbase(void) {
    errorVal = 0;
    if (!QSqlDatabase::drivers().contains("QSQLITE"))
        qDebug() << "QSqlDatabase drivers:" << QSqlDatabase::drivers() << "does not contain QSQLITE";

    allTimeBestLapKph = 0.;
    yearOfCurrentDbase = 0;
}



int CLapsDbase::prepare(QSqlDatabase *dBase) {
    QSqlQuery query(*dBase);
    bool showContents = false;

    // Make sure lapsTable exists in dbase

    if (!dBase[0].tables().contains("lapsTable")) {
        query.prepare("create table lapsTable (id INTEGER PRIMARY KEY AUTOINCREMENT, tagId VARCHAR(20), dateTime UNSIGNED INTEGER, lapsec FLOAT, lapm FLOAT, reportStatus INTEGER)");
        if (!query.exec()) {
            errorTextVal = query.lastError().text();
            errorVal = 3;
            return errorVal;
        }
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

    if (!dBase[0].tables().contains("priorsTable")) {
        qDebug() << "Creating new priorsTable in " + prior.absoluteFilePath();
        query.prepare("create table priorsTable (id INTEGER PRIMARY KEY AUTOINCREMENT, tagId VARCHAR(20) UNIQUE, name VARCHAR(20), lapCount INTEGER, lapSecTotal FLOAT, lapMTotal FLOAT)");
        if (!query.exec()) {
            errorTextVal = query.lastError().text();
            errorVal = 5;
            return errorVal;
        }
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

    return 0;
}



int CLapsDbase::open(const QString &rootName, const QString &username, const QString &password) {
    errorTextVal.clear();
    errorVal = 0;
    QString s;
    QDate currentDate(QDate::currentDate());

    // Close dbase if already open

    close();


    // Determine database file name (rootName + year + ".db")

    QString connectionName = rootName + s.setNum(currentDate.year());
    setFile(connectionName + ".db");


    // Make a list of databases for each year

    QFileInfo fileInfo;
    yearOfCurrentDbase = currentDate.year();       // year of current database
    for (int year=yearOfCurrentDbase; year>=2000; year--) {
        connectionName = rootName + s.setNum(year);
        fileInfo.setFile(connectionName + ".db");
        if (QFile::exists(fileInfo.absoluteFilePath())) {
            dBase.append(QSqlDatabase::addDatabase("QSQLITE", connectionName));
            dBase.last().setUserName(username);
            dBase.last().setPassword(password);
            dBase.last().setDatabaseName(fileInfo.absoluteFilePath());
        }
        else {
            // Create dbase for current year if necessary

            if (year == yearOfCurrentDbase) {
                dBase.append(QSqlDatabase::addDatabase("QSQLITE", connectionName));
                dBase.last().setUserName(username);
                dBase.last().setPassword(password);
                dBase.last().setDatabaseName(fileInfo.absoluteFilePath());
            }
            break;
        }
    }

    // The dBase list must now have at least one entry (current year)

    if (dBase.isEmpty()) {
        errorTextVal = "Could not open dBase for current year or any other";
        errorVal = 1;
        return errorVal;
    }

    // Open dbase for current year

    if (!dBase[0].open()) {
        errorTextVal = dBase[0].lastError().text();
        errorVal = 1;
        for (int i=0; i<dBase.size(); i++) {
            dBase.removeAt(i);
        }
        dBase.clear();
        return errorVal;
    }

    // Ensure current year dBase has the required tables

    prepare(&dBase[0]);

    // If previous year dbase (second entry in dBase list) exists, open for processing

    bool calculatePriorsRequired = false;
    if (dBase.size() >= 2) {
        if (!dBase[1].isOpen()) {
            if (!dBase[1].open()) {
                errorTextVal = "Could not open previous year database";
                errorVal = 1;
                dBase.removeAt(1);
                return errorVal;
            }
        }

        // If required, calculate totals from dBasePriorList[0]:lapsTable and put into dBase:priorsTable

        if (calculatePriorsRequired) {
            qDebug() << "Calculating priors";

            QSqlQuery queryPrior(dBase[1]);
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
                int rc = getPriors(dBase[1], tagIdList[i], &lapCountPrior, &lapSecPrior, &lapMPrior);
                if (rc) {
                    return errorVal;
                }
                lapCount[i] += lapCountPrior;
                lapSecTotal[i] += lapSecPrior;
                lapMTotal[i] += lapMPrior;
            }

            // Write these priors to the current-year priorsTable

            QSqlQuery query(dBase[0]);
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

//    QList<CLapInfo> laps;
//    getLapInfo(QString(), QDateTime::currentDateTime().addYears(-100), QDateTime::currentDateTime(), &laps);

    return 0;
}



void CLapsDbase::close(void) {
    for (int i=0; i<dBase.size(); i++) {
        if (dBase[i].isOpen()) {
            dBase[i].close();
            dBase.removeAt(i);
        }
    }
    dBase.clear();
}




// addLap
// Add lap to database.
// If the year has changed since the previous lap, open new dbase and make the previous current dbase the new previous
//
int CLapsDbase::addLap(const CRider &rider, const QDateTime &dateTime) {
    errorTextVal.clear();
    errorVal = 0;

    if (!isOpen()) {
        errorTextVal = "CLapsDbase is closed";
        errorVal = 2;
        return errorVal;
    }

    // Check whether year has changed since last lap added to dBase and create new dBase if necessary

    if (QDate::currentDate().year() != yearOfCurrentDbase) {

    }

    QSqlQuery query(dBase[0]);
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
// Fill ClapsInfo list for all laps in all database files within specified period for specified tagId
//
int CLapsDbase::getLapInfo(const QString &tagId, const QDateTime &start, const QDateTime &end, QList<CLapInfo> *laps) {
    errorTextVal.clear();
    errorVal = 0;
    laps->clear();

    // Look through list of dBases and get lap info from any within time period

    int rc = 0;
    int year = QDate::currentDate().year();
    for (int i=0; i<dBase.size(); i++) {
        if ((year >= start.date().year()) && (year <= end.date().year())) {
            rc = getLapInfo(dBase[i], tagId, start, end, laps);
        }
        year--;
    }
    return rc;
}




int CLapsDbase::getLap(int id, QString *tagId, CLapInfo *lapInfo) {
    errorTextVal.clear();
    errorVal = 0;

    if (!isOpen()) {
        errorTextVal = "CLapsDbase closed";
        errorVal = 9;
        return errorVal;
    }

    QSqlQuery query(dBase[0]);
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



// Get stats for specified tagId and time period from current year dbase
//
int CLapsDbase::getLaps(const QString &tagId, const QDateTime &start, const QDateTime &end, CLapsDbase::reportStatus_t reportStatus, QList<int> *lapsList) {
    errorTextVal.clear();
    errorVal = 0;

    if (!isOpen()) {
        errorTextVal = "CLapsDbase closed";
        errorVal = 2;
        return errorVal;
    }

    if (start > end) {
        errorTextVal = "dateTimeStart > dateTimeEnd in getLapsInPeriod";
        errorVal = 3;
        return errorVal;
    }

    QSqlQuery query(dBase[0]);
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

    while (query.next())
        lapsList->append(query.value(idIndex).toInt());

    return 0;
}



// Calculate allTime stats for specified tagId
//
int CLapsDbase::getStats(const QString &tagId, CRider *rider) {
    errorTextVal.clear();
    errorVal = 0;

    if (!isOpen()) {
        errorTextVal = "CLapsDbase is closed";
        errorVal = 2;
        return errorVal;
    }

    rider->thisMonth.clear();
    rider->lastMonth.clear();
    rider->allTime.clear();

    QDateTime currentDateTime(QDateTime::currentDateTime());
    QDateTime startOfThisMonth(QDate(currentDateTime.date().year(), currentDateTime.date().month(), 1), QTime(0, 0, 0));
    QDateTime startOfLastMonth(startOfThisMonth.addMonths(-1));
    QDateTime startOfTime(QDate(2000, 1, 1), QTime(0, 0, 0));

    // Get stats for this month

    errorVal = getStats(tagId, startOfThisMonth, currentDateTime, CLapsDbase::reportAny, &rider->thisMonth);
    if (errorVal)
        return errorVal;

    // Get stats for last month

    errorVal = getStats(tagId, startOfLastMonth, startOfThisMonth.addMSecs(-1), CLapsDbase::reportAny, &rider->lastMonth);
    if (errorVal)
        return errorVal;

    // Get stats for all time

    errorVal = getStats(tagId, startOfTime, currentDateTime, CLapsDbase::reportAny, &rider->allTime);
    if (errorVal)
        return errorVal;

    return 0;
}



// Get stats for specified tag and period by combining stats from each year's database
//
int CLapsDbase::getStats(const QString &tagId, const QDateTime &start, const QDateTime &end, reportStatus_t reportStatus, CStats *stats) {
    stats->clear();
    int year = yearOfCurrentDbase;
    for (int i=0; i<dBase.size(); i++) {
        if ((start.date().year() <= year) && (end.date().year() >= year)) {
            CStats yearStats;
            int rc = getStats(dBase[i], tagId, start, end, reportStatus, &yearStats);
            if (rc > 0)
                return rc;
            stats->lapCount += yearStats.lapCount;
            stats->totalM += yearStats.totalM;
            stats->totalSec += yearStats.totalSec;

            if (yearStats.bestLapKmph > stats->bestLapKmph)
                stats->bestLapKmph = yearStats.bestLapKmph;

            if (yearStats.bestKKmph > stats->bestKKmph)
                stats->bestKKmph = yearStats.bestKKmph;
        }
        year--;
    }

    if (stats->totalSec > 0.)
        stats->averageKmph = stats->totalM / 1000. / stats->totalSec * 3600.;

    return 0;
}




// Get stats from specified dbase (year) for specified tagId and time period
//
int CLapsDbase::getStats(const QSqlDatabase &dBase, const QString &tagId, const QDateTime &start, const QDateTime &end, reportStatus_t reportStatus, CStats *stats) {
    errorTextVal.clear();
    errorVal = 0;
    stats->clear();

    if (!isOpen()) {
        errorTextVal = "CLapsDbase closed";
        errorVal = 2;
        return errorVal;
    }

    if (start > end) {
        errorTextVal = "start > end in getStatsForPeriod";
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
    query.bindValue(":dateTimeStart", CDateTime(start).toUInt());
    query.bindValue(":dateTimeEnd", CDateTime(end).toUInt());
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

    int workoutCount = 0;
    int lapCount = 0;
    float totalSec = 0.;
    float totalM = 0.;
    float bestLapKmph = 0.;
    float bestKKmph = 0.;
    QList<float> lapSecList;
    QList<float> lapMList;
    float lapMSum = 0.;
    float lapSecSum = 0.;
    QDateTime previousLapDateTime = QDateTime::currentDateTime().addYears(-100);
    while (query.next()) {
        QDateTime lapDateTime = CDateTime(query.value(dateTimeIndex).toUInt()).toQDateTime();
        float lapSec = query.value(lapsecIndex).toFloat();
        float lapM = query.value(lapmIndex).toFloat();
        float lapKmph = 0.;
        if (lapSec > 0.)
            lapKmph = lapM / 1000. / lapSec * 3600.;

        if (lapKmph > bestLapKmph)
            bestLapKmph = lapKmph;

        lapCount++;
        totalM += lapM;
        totalSec += lapSec;
        if (previousLapDateTime.secsTo(lapDateTime) > (3 * 3600))
            workoutCount++;

        // Find best k kph

        lapMList.append(lapM);
        lapSecList.append(lapSec);
        lapMSum += lapM;
        lapSecSum += lapSec;
        if (lapMSum >= 1000.) {
            float sec = lapSecSum;
            float m = lapMSum;
            float kph;
            do {
                lapSecSum -= lapSecList[0];
                lapMSum -= lapMList[0];
                lapSecList.removeFirst();
                lapMList.removeFirst();
            } while (lapMSum >= 1000.);
            kph = m / 1000. / sec * 3600.;
            if (kph > bestKKmph)
                bestKKmph = kph;
        }

        previousLapDateTime = lapDateTime;
    }

    stats->lapCount = lapCount;
    stats->workoutCount = workoutCount;
    stats->totalM = totalM;
    stats->totalSec = totalSec;
    stats->bestLapKmph = bestLapKmph;
    stats->bestKKmph = bestKKmph;

    return 0;
}



// Get stats from specified dbase for specified tagId and time period
//
int CLapsDbase::getPriors(const QSqlDatabase &dBase, const QString &tagId, int *lapCount, float *totalLapSec, float *totalLapM) {
    errorTextVal.clear();
    errorVal = 0;

    *lapCount = 0;
    *totalLapSec = 0.;
    *totalLapM = 0.;

    if (!isOpen()) {
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
        *totalLapSec = query.value(lapSecTotalIndex).toFloat();
        *totalLapM = query.value(lapMTotalIndex).toFloat();
    }
    return 0;
}



// Set reportStatus for specified tagId and time period
//
int CLapsDbase::setReportStatus(reportStatus_t reportStatus, const QString &tagId, const QDateTime &start, const QDateTime &end) {
    errorTextVal.clear();
    errorVal = 0;

    if (!isOpen()) {
        errorTextVal = "CLapsDbase closed";
        errorVal = 2;
        return errorVal;
    }

    if (start > end) {
        errorTextVal = "dateTimeStart > dateTimeEnd in getStatsForPeriod";
        errorVal = 3;
        return errorVal;
    }

    QSqlQuery query(dBase[0]);
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



QString CLapsDbase::errorText(void) {
    return errorTextVal;
}



// isOpen()
// Return true if at least one database is open
//
bool CLapsDbase::isOpen(void) {
    return !dBase.isEmpty() && dBase[0].isOpen();
}


