// cdbase.cpp


#include "cdbase.h"
#include "crider.h"




// ************************************************************************************************

CMembershipDbase::CMembershipDbase() {
    errorTextVal.clear();
    if (!QSqlDatabase::drivers().contains("QSQLITE"))
        qDebug() << "QSqlDatabase drivers:" << QSqlDatabase::drivers() << "does not contain QSQLITE";

    dBase = QSqlDatabase::addDatabase("QSQLITE", "membership");
}



int CMembershipDbase::open(const QString &filename, const QString &username, const QString &password) {
    errorTextVal.clear();
    dBase.setUserName(username);
    dBase.setPassword(password);
    dBase.setDatabaseName(filename);

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
        qDebug() << "Creating new membershipTable in" << filename;
        query.prepare("create table membershipTable (id INTEGER PRIMARY KEY AUTOINCREMENT, tagId VARCHAR(20) UNIQUE, firstName VARCHAR(20), lastName VARCHAR(20), membershipNumber INTEGER UNIQUE, caRegistration VARCHAR(20) UNIQUE, eMail VARCHAR(20))");
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
        while (query.next()) {
            qDebug("id=%d tagId=%s name=%s %s membershipNumber=%s caRegistration=%s eMail=%s", query.value(id).toString().toInt(), query.value(idTagId).toString().toLatin1().data(), query.value(idFirst).toString().toLatin1().data(), query.value(idLast).toString().toLatin1().data(), query.value(idMembership).toString().toLatin1().data(), query.value(idcaRegistration).toString().toLatin1().data(), query.value(idEmail).toString().toLatin1().data());
        }
    }

//    {
//        QSqlQuery query(dBase);
//        qDebug() << "Integrity check of membershipTable...";
//        query.prepare("PRAGMA integrity_check");
//        if (!query.exec()) {
//            errorTextVal = query.lastError().text();
//            qDebug() << errorTextVal;
//            return 4;
//        }
//        int id = query.record().indexOf("id");
//        int idTagId = query.record().indexOf("tagId");
//        int idFirst = query.record().indexOf("firstName");
//        int idLast = query.record().indexOf("lastName");
//        int idMembership = query.record().indexOf("membershipNumber");
//        int idcaRegistration = query.record().indexOf("caRegistration");
//        int idEmail = query.record().indexOf("eMail");
//        while (query.next()) {
//            qDebug("id=%d tagId=%s name=%s %s membershipNumber=%s caRegistration=%s eMail=%s", query.value(id).toString().toInt(), query.value(idTagId).toString().toLatin1().data(), query.value(idFirst).toString().toLatin1().data(), query.value(idLast).toString().toLatin1().data(), query.value(idMembership).toString().toLatin1().data(), query.value(idcaRegistration).toString().toLatin1().data(), query.value(idEmail).toString().toLatin1().data());
//        }
//    }
    return 0;
}



void CMembershipDbase::close(void) {
    if (dBase.isOpen())
        dBase.close();
}



// add
// Add entry to membership database.  TagId and first-last name must be unique.
//
int CMembershipDbase::add(const QString &tagId, const QString &firstName, const QString &lastName, const QString &membershipNumber, const QString &caRegistration, const QString &eMail) {
    if (!dBase.isOpen())
        return 1;

    QSqlQuery query(dBase);
    query.prepare("INSERT INTO membershipTable (tagId, firstName, lastName, membershipNumber, caRegistration, eMail) VALUES (:tagId, :firstName, :lastName, :membershipNumber, :caRegistration, :eMail)");
    query.bindValue(":tagId", tagId);
    query.bindValue(":firstName", firstName);
    query.bindValue(":lastName", lastName);
    query.bindValue(":membershipNumber", membershipNumber);
    query.bindValue(":caRegistration", caRegistration);
    query.bindValue(":eMail", eMail);
    if (!query.exec()) {
        errorTextVal = "Could not add to database";
        return 2;
    }
    return 0;
}



// updateTagId
//
int CMembershipDbase::update(const QString &tagId, const QString &firstName, const QString &lastName, const QString &membershipNumber, const QString &caRegistration, const QString &eMail) {
    if (!dBase.isOpen())
        return 1;

    QSqlQuery query(dBase);
    query.prepare("UPDATE membershipTable SET firstName = :firstName WHERE tagId = :tagId");
    query.bindValue(":tagId", tagId);
    query.bindValue(":firstName", firstName);
    if (!query.exec()) {
        errorTextVal = "Could not update first name in database";
        return 2;
    }
    query.prepare("UPDATE membershipTable SET lastName = :lastName WHERE tagId = :tagId");
    query.bindValue(":tagId", tagId);
    query.bindValue(":lastName", lastName);
    if (!query.exec()) {
        errorTextVal = "Could not update last name in database";
        return 3;
    }
    query.prepare("UPDATE membershipTable SET membershipNumber = :membershipNumber WHERE tagId = :tagId");
    query.bindValue(":tagId", tagId);
    query.bindValue(":membershipNumber", membershipNumber);
    if (!query.exec()) {
        errorTextVal = "Could not update membership number in database";
        return 4;
    }
    query.prepare("UPDATE membershipTable SET caRegistration = :caRegistration WHERE tagId = :tagId");
    query.bindValue(":tagId", tagId);
    query.bindValue(":caRegistration", caRegistration);
    if (!query.exec()) {
        errorTextVal = "Could not update cycling association registration in database";
        return 5;
    }
    query.prepare("UPDATE membershipTable SET eMail = :eMail WHERE tagId = :tagId");
    query.bindValue(":tagId", tagId);
    query.bindValue(":eMail", eMail);
    if (!query.exec()) {
        errorTextVal = "Could not update eMail in database";
        return 6;
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
        qDebug() << errorTextVal;
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
        *tagId = query.value(0).toString().toLatin1();
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



int CMembershipDbase::getAllFromId(int id, QString *tagId, QString *firstName, QString *lastName, QString *membershipNumber, QString *caRegistration, QString *eMail) {
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
    if ((tagIdIndex < 0) || (firstNameIndex < 0) || (lastNameIndex < 0) || (membershipNumberIndex < 0) || (caRegistrationIndex < 0) || (eMailIndex < 0)) {
        errorTextVal = "Could not find index";
        qDebug() << errorTextVal;
        return 3;
    }

    if (query.next()) {
        *tagId = query.value(tagIdIndex).toString().toLatin1();
        *firstName = query.value(firstNameIndex).toString();
        *lastName = query.value(lastNameIndex).toString();
        *membershipNumber = query.value(membershipNumberIndex).toString();
        *caRegistration = query.value(caRegistrationIndex).toString();
        *eMail = query.value(eMailIndex).toString();
        errorTextVal.clear();
        return 0;
    }
    errorTextVal = "Id not found";
    return 4;
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
















// ***************************************************************************************************

CLapsDbase::CLapsDbase() {
    errorTextVal.clear();
    if (!QSqlDatabase::drivers().contains("QSQLITE"))
        qDebug() << "QSqlDatabase drivers:" << QSqlDatabase::drivers() << "does not contain QSQLITE";

    dBase = QSqlDatabase::addDatabase("QSQLITE", "laps");
}


int CLapsDbase::open(const QString &filename, const QString &username, const QString &password) {
    errorTextVal.clear();
    dBase.setUserName(username);
    dBase.setPassword(password);
    dBase.setDatabaseName(filename);

    if (!dBase.open()) {
        errorTextVal = dBase.lastError().text();
        qDebug() << "Error opening " + filename + ":" << errorText();
        return 1;
    }

    bool showContents = false;

    // Make sure table exists and create as necessary

    QStringList tableList = dBase.tables();
    if (!tableList.contains("lapsTable")) {
        QSqlQuery query(dBase);
        qDebug() << "Creating new lapsTable in " + filename;
        query.prepare("create table lapsTable (id INTEGER PRIMARY KEY AUTOINCREMENT, tagId VARCHAR(20), dateTime UNSIGNED INTEGER(10), lapsec FLOAT(10), lapm FLOAT(10))");
        if (!query.exec()) {
            errorTextVal = query.lastError().text();
            qDebug() << "Error creating new lapsTable in " + filename << errorTextVal;
            return 2;
        }
        qDebug() << "  Created new lapsTable";
    }

    if (showContents) {
        QSqlQuery query(dBase);
        qDebug() << "List of lapsTable...";
        query.prepare("select * from lapsTable");
        if (!query.exec()) {
            errorTextVal = query.lastError().text();
            qDebug() << errorTextVal;
            return 3;
        }
        int id = query.record().indexOf("id");
        int idTagId = query.record().indexOf("tagId");
        int idDateTime = query.record().indexOf("dateTime");
        int idLapSec = query.record().indexOf("lapsec");
        int idLapM = query.record().indexOf("lapm");
        while (query.next()) {
            int year;
            int month;
            int day;
            int hour;
            int minute;
            int second;
            unsigned int dateTime = query.value(idDateTime).toUInt();
            int2DateTime(dateTime, &year, &month, &day, &hour, &minute, &second);
            float lapSec = query.value(idLapSec).toFloat();
            float lapM = query.value(idLapM).toFloat();
            qDebug("id=%d tagId=%s dateTime=%u (%d %d %d %d %d %d) lapSec=%f lapM=%f", query.value(id).toInt(), query.value(idTagId).toString().toLatin1().data(), dateTime, year, month, day, hour, minute, second, lapSec, lapM);
        }
    }

    return 0;
}



void CLapsDbase::close(void) {
    if (dBase.isOpen())
        dBase.close();
}




// addLap
// Add entry to laps database
//
int CLapsDbase::addLap(const QString &tagId, int year, int month, int day, int hour, int minute, int second, float lapsec, float lapm) {
    if (!dBase.isOpen())
        return 1;

    unsigned int dateTime = dateTime2Int(year, month, day, hour, minute, second);
    QSqlQuery query(dBase);
    query.prepare("INSERT INTO lapsTable (tagId, dateTime, lapsec, lapm) VALUES (:tagId, :dateTime, :lapsec, :lapm)");
    query.bindValue(":tagId", tagId);
    query.bindValue(":dateTime", dateTime);
    query.bindValue(":lapsec", lapsec);
    query.bindValue(":lapm", lapm);
    if (!query.exec()) {
        errorTextVal = "Could not add to laps table";
        qDebug() << errorTextVal;
        return 2;
    }
    return 0;
}



// Calculate stats for specified rider (tagId) from dbase entries and populate CRider
//
int CLapsDbase::getStats(const QString &tagId, CRider *rider) {
    if (!dBase.isOpen())
        return 1;

    QDateTime dateTime(QDateTime::currentDateTime());
    int thisMonthYear = dateTime.date().year();
    int thisMonthMonth = dateTime.date().month();
    QDateTime lastMonthDateTime(dateTime.addMonths(-1));
    int lastMonthYear = lastMonthDateTime.date().year();
    int lastMonthMonth = lastMonthDateTime.date().month();

    rider->thisMonth.clear();
    rider->lastMonth.clear();
    rider->allTime.clear();

    // Get stats for this month

    unsigned int dateTimeStart = dateTime2Int(thisMonthYear, thisMonthMonth, 0, 0, 0, 0);
    unsigned int dateTimeEnd = dateTime2Int(thisMonthYear, thisMonthMonth, 31, 24, 0, 0);

    getStatsForPeriod(tagId, dateTimeStart, dateTimeEnd, &rider->thisMonth);


    // Get stats for last month

    dateTimeStart = dateTime2Int(lastMonthYear, lastMonthMonth, 0, 0, 0, 0);
    dateTimeEnd = dateTime2Int(lastMonthYear, lastMonthMonth, 31, 24, 0, 0);

    getStatsForPeriod(tagId, dateTimeStart, dateTimeEnd, &rider->lastMonth);


    // Get stats for all time

    int allTimeYear = 2000;     // min value is 2000
    int allTimeMonth = 0;
    dateTimeStart = dateTime2Int(allTimeYear, allTimeMonth, 0, 0, 0, 0);
    dateTimeEnd = dateTime2Int(thisMonthYear, thisMonthMonth, 31, 24, 0, 0);

    getStatsForPeriod(tagId, dateTimeStart, dateTimeEnd, &rider->allTime);

    return 0;
}


// Get stats for specified tagId and time period from dbase
//
int CLapsDbase::getStatsForPeriod(const QString &tagId, unsigned int dateTimeStart, unsigned int dateTimeEnd, CStats *stats) {
    if (!dBase.isOpen())
        return 1;

    if (dateTimeStart > dateTimeEnd)
        throw("dateTimeStart > dateTimeEnd in getStatsForPeriod");


    // Determine minimum time difference in dateTimeInt that separates workouts

    unsigned int workoutDateTimeSeparation = dateTime2Int(2000, 0, 0, 12, 0, 0) - dateTime2Int(2000, 0, 0, 0, 0, 0);

    QSqlQuery query(dBase);
    query.prepare("SELECT * FROM lapsTable WHERE tagId = :tagId AND dateTime BETWEEN :dateTimeStart AND :dateTimeEnd");
    query.bindValue(":tagId", tagId);
    query.bindValue(":dateTimeStart", dateTimeStart);
    query.bindValue(":dateTimeEnd", dateTimeEnd);
    if (!query.exec()) {
        errorTextVal = query.lastError().text();
        qDebug() << errorTextVal;
        return 1;
    }
    int lapsecIndex = query.record().indexOf("lapsec");
    if (lapsecIndex < 0) {
        errorTextVal = "Could not find lapsec index in getStats";
        qDebug() << errorTextVal;
        return 2;
    }
    int lapmIndex = query.record().indexOf("lapm");
    if (lapmIndex < 0) {
        errorTextVal = "Could not find lapm index in getStats";
        qDebug() << errorTextVal;
        return 2;
    }
    int dateTimeIndex = query.record().indexOf("dateTime");
    if (dateTimeIndex < 0) {
        errorTextVal = "Could not find dateTime index in getStatus";
        qDebug() << errorTextVal;
        return 3;
    }
    errorTextVal.clear();

    // Determine workout count, lap count, average lap time, best lap time, distance

    float localBestLapSec = -1.;
    float localBestLapM = 0.;
    unsigned int previousDateTime = 0;
    int localWorkoutCount = 0;
    int localLapCount = 0;
    float localTotalSec = 0.;
    float localTotalM = 0.;
    while (query.next()) {
        float lapSec = query.value(lapsecIndex).toFloat();
        float lapM = query.value(lapmIndex).toFloat();
//        float lapSpeed = 0.;
//        if (lapSec > 0.)
//            lapSpeed = lapM / lapSec;
        unsigned int dateTime = query.value(dateTimeIndex).toUInt();
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
        unsigned int dateTimeDif = dateTime - previousDateTime;
        if (dateTimeDif > workoutDateTimeSeparation) {
            localWorkoutCount++;
        }

        previousDateTime = dateTime;
    }

    stats->lapCount = localLapCount;
    stats->workoutCount = localWorkoutCount;
    stats->bestLapSec = localBestLapSec;
    stats->bestLapM = localBestLapM;
    stats->totalSec = localTotalSec;
    stats->totalM = localTotalM;

    return 0;
}






// dateTime2Int()
// seconds (0 - 63) - 6 bits (0 - 5)
// minutes (0 - 63) - 6 bits (6 - 11)
// hours   (0 -31)  - 5 bits (12 - 16)
// day     (0 - 31) - 5 bits (17 - 21)
// month   (0 - 15) - 4 bits (22 - 25)
// year    (0 - 63) - 6 bits (26 - 31)
//
unsigned int CLapsDbase::dateTime2Int(int year, int month, int day, int hour, int minute, int second) {
    if (year < 2000)
        throw("Year < 2000 in dateTime2Int");

    unsigned int dateTime = ((year-2000) & 0x3f) << 26 | (month & 0x0f) << 22 | (day & 0x1f) << 17 | (hour & 0x1f) << 12 | (minute & 0x3f) << 6 | (second & 0x3f);
    return dateTime;
}


// int2DateTime()
//
void CLapsDbase::int2DateTime(unsigned int dateTime, int *year, int *month, int *day, int *hour, int *minute, int *second) {
    *year = ((dateTime >> 26) & 0x3f) + 2000;
    *month = (dateTime >> 22) & 0x0f;
    *day = (dateTime >> 17) & 0x1f;
    *hour = (dateTime >> 12) & 0x1f;
    *minute = (dateTime >> 6) & 0x3f;
    *second = (dateTime >> 0) & 0x3f;
}





QString CLapsDbase::errorText(void) {
    return errorTextVal;
}



bool CLapsDbase::isOpen(void) {
    return dBase.isOpen();
}


