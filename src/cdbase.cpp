// cdbase.cpp


#include "cdbase.h"
#include "crider.h"



CDbase::CDbase() {
    dBase = QSqlDatabase::addDatabase("QSQLITE");
}


int CDbase::openDbase(const QString &filename, const QString &username, const QString &password) {
    errorTextVal.clear();
    dBase.setUserName(username);
    dBase.setPassword(password);
    dBase.setDatabaseName(filename);
    if (!dBase.open()) {
        errorTextVal = dBase.lastError().text();
        return 1;
    }
    return 0;
}



void CDbase::close(void) {
    dBase.close();
}



QString CDbase::errorText(void) {
    return errorTextVal;
}



bool CDbase::isOpen(void) {
    return dBase.isOpen();
}



// ************************************************************************************************

CMembershipDbase::CMembershipDbase() {
    dBase = QSqlDatabase::addDatabase("QSQLITE");
}


int CMembershipDbase::open(const QString &filename, const QString &username, const QString &password) {
    QSqlQuery query;
    if (!openDbase(filename, username, password)) {
        errorTextVal = dBase.lastError().text();
        return 1;
    }

    bool showNames = true;//false;

    // Make sure tables exist and create as necessary
    // The first table is a list of tagId values and corresponding names

    QStringList tableList = dBase.tables();
    QString tableNames = "names";
    if (!tableList.contains(tableNames)) {
        query.prepare("create table " + tableNames + " (id INTEGER PRIMARY KEY AUTOINCREMENT, tagId VARCHAR(20) UNIQUE, firstName VARCHAR(20), lastName VARCHAR(20), membershipNumber INTEGER UNIQUE)");
        if (!query.exec()) {
            errorTextVal = query.lastError().text();
            throw("Error creating table " + tableNames + ": " + errorTextVal);
        }
        qDebug() << "Created new names table";
    }
    tableList = dBase.tables();

    if (showNames) {
        qDebug() << "List of tagId and names in names table " + filename + "...";
        query.prepare("select * from names");
        if (!query.exec()) {
            errorTextVal = query.lastError().text();
            qDebug() << errorTextVal;
            return 2;
        }
        int id = query.record().indexOf("id");
        int idTagId = query.record().indexOf("tagId");
        int idFirst = query.record().indexOf("firstName");
        int idLast = query.record().indexOf("lastName");
        int idMembership = query.record().indexOf("membershipNumber");
        while (query.next()) {
            qDebug("id=%d tagId=%s name=%s %s membershipNumber=%d", query.value(id).toInt(), query.value(idTagId).toString().toLatin1().data(), query.value(idFirst).toString().toLatin1().data(), query.value(idLast).toString().toLatin1().data(), query.value(idMembership).toInt());
        }
    }



    // The second table is a list of completed laps: tagId values, timeDateInt stamp values and laptime usec values and lapdistance values

    QString tableLaps = "laps";

//    bool deleteLapsTable = false;
//    if (deleteLapsTable) {
//        query.prepare("drop table laps");
//        if (!query.exec()) {
//            errorTextVal = query.lastError().text();
//            qDebug() << "Error deleting table " << tableLaps << ": " << errorTextVal;
//            return 3;
//        }
//        qDebug() << "Laps table deleted";
//    }

    if (!tableList.contains(tableLaps)) {
        query.prepare("create table " + tableLaps + " (id INTEGER PRIMARY KEY AUTOINCREMENT, tagId VARCHAR(20), dateTime INTEGER(10), lapmsec INTEGER(10), lapm FLOAT(10))");
        if (!query.exec()) {
            errorTextVal = query.lastError().text();
            qDebug() << "Error creating table " << tableLaps << ": " << errorTextVal;
            return 4;
        }
        qDebug() << "Created new laps table";
    }

    bool showLaps = false;
    if (showLaps) {
        qDebug() << "List of lap info in laps table...";
        QSqlQuery queryLaps;
        queryLaps.prepare("select * from laps");
        if (!queryLaps.exec()) {
            errorTextVal = queryLaps.lastError().text();
            qDebug() << errorTextVal;
            return 5;
        }
        int id = queryLaps.record().indexOf("id");
        int idTagId = queryLaps.record().indexOf("tagId");
        int idDateTime = queryLaps.record().indexOf("dateTime");
        int idLapmsec = queryLaps.record().indexOf("lapmsec");
        int idLapm = queryLaps.record().indexOf("lapm");
        while (queryLaps.next()) {
            int lapmsec = queryLaps.value(idLapmsec).toInt();
            float lapm = queryLaps.value(idLapm).toFloat();
            qDebug("id=%d tagId=%s dateTime=%d lapmsec=%d lapm=%f", queryLaps.value(id).toInt(), queryLaps.value(idTagId).toString().toLatin1().data(), queryLaps.value(idDateTime).toInt(), lapmsec, lapm);
        }
    }

    return 0;
}



// add
// Add entry to membership database.  TagId and first-last name must be unique.
//
int CMembershipDbase::add(const QByteArray &tagId, const QString &firstName, const QString &lastName, const int membershipNumber, const int ccaRegistration, const QByteArray email) {
    if (!dBase.isOpen()) return 1;

    QSqlQuery query;
    query.prepare("INSERT INTO names (tagId, firstName, lastName, membershipNumber) VALUES (:tagId, :firstName, :lastName, :membershipNumber)");
    query.bindValue(":tagId", tagId);
    query.bindValue(":firstName", firstName);
    query.bindValue(":lastName", lastName);
    query.bindValue(":membershipNumber", membershipNumber);
    if (!query.exec()) {
        errorTextVal = "Could not add to database";
        return 2;
    }
    return 0;
}



// updateTagId
//
int CMembershipDbase::update(const QByteArray &tagId, const QString &firstName, const QString &lastName, const int membershipNumber, const int ccaRegistration, const QByteArray &email) {
    if (!dBase.isOpen()) return 1;

    QSqlQuery query;
    query.prepare("UPDATE names SET firstName = :firstName WHERE tagId = :tagId");
    query.bindValue(":tagId", tagId);
    query.bindValue(":firstName", firstName);
    if (!query.exec()) {
        errorTextVal = "Could not update first name in database";
        return 2;
    }
    query.prepare("UPDATE names SET lastName = :lastName WHERE tagId = :tagId");
    query.bindValue(":tagId", tagId);
    query.bindValue(":lastName", lastName);
    if (!query.exec()) {
        errorTextVal = "Could not update last name database";
        return 3;
    }
    query.prepare("UPDATE names SET membershipNumber = :membershipNumber WHERE tagId = :tagId");
    query.bindValue(":tagId", tagId);
    query.bindValue(":membershipNumber", membershipNumber);
    if (!query.exec()) {
        errorTextVal = "Could not update membership number database";
        return 4;
    }

    return 0;
}


// removeTagId
// Remove entry with specified tagId
//
int CMembershipDbase::removeTagId(const QByteArray &tagId) {
    if (!dBase.isOpen()) return 1;

    QSqlQuery query;
    query.prepare("DELETE FROM names WHERE tagId = (:tagId)");
    query.bindValue(":tagId", tagId);
    if (!query.exec()) {
        errorTextVal = query.lastError().text();
        qDebug() << errorTextVal;
        return 2;
    }
    return 0;
}



int CMembershipDbase::findTagIdFromName(const QString &firstName, const QString &lastName, QByteArray *tagId) {
    if (!dBase.isOpen()) return 1;

    QSqlQuery query;
    if (!lastName.isEmpty() && !firstName.isEmpty()) {
        query.prepare("SELECT tagId FROM names WHERE lastName = (:lastName) AND firstName = (:firstName)");
        query.bindValue(":lastName", lastName);
        query.bindValue(":firstName", firstName);
    }
    else if (!lastName.isEmpty()) {
        query.prepare("SELECT tagId FROM names WHERE lastName = (:lastName)");
        query.bindValue(":lastName", lastName);
    }
    else if (!firstName.isEmpty()) {
        query.prepare("SELECT tagId FROM names WHERE firstName = (:firstName)");
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
int CMembershipDbase::getIdFromTagId(const QByteArray &tagId) {
    if (!dBase.isOpen()) return 0;
    if (tagId.isEmpty()) return 0;

    QSqlQuery query;
    query.prepare("SELECT id FROM names WHERE tagId = (:tagId)");
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
    if (!dBase.isOpen()) return 0;

    QSqlQuery query;
    if (!lastName.isEmpty() && !firstName.isEmpty()) {
        query.prepare("SELECT id FROM names WHERE lastName = (:lastName) AND firstName = (:firstName) COLLATE NOCASE");
        query.bindValue(":lastName", lastName);
        query.bindValue(":firstName", firstName);
    }
    else if (!lastName.isEmpty()) {
        query.prepare("SELECT id FROM names WHERE lastName = (:lastName) COLLATE NOCASE");
        query.bindValue(":lastName", lastName);
    }
    else if (!firstName.isEmpty()) {
        query.prepare("SELECT id FROM names WHERE firstName = (:firstName) COLLATE NOCASE");
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
int CMembershipDbase::getIdFromMembershipNumber(const int membershipNumber) {
    if (!dBase.isOpen()) return 0;

    QSqlQuery query;
    query.prepare("SELECT id FROM names WHERE membershipNumber = (:membershipNumber)");
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



int CMembershipDbase::getAllFromId(int id, QByteArray *tagId, QString *firstName, QString *lastName, int *membershipNumber) {
    if (!dBase.isOpen())
        return 1;

    QSqlQuery query;
    query.prepare("SELECT * FROM names WHERE id = (:id)");
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
    if ((tagIdIndex < 0) || (firstNameIndex < 0) || (lastNameIndex < 0) || (membershipNumberIndex < 0)) {
        errorTextVal = "Could not find index";
        qDebug() << errorTextVal;
        return 3;
    }

    if (query.next()) {
        *tagId = query.value(tagIdIndex).toString().toLatin1();
        *firstName = query.value(firstNameIndex).toString();
        *lastName = query.value(lastNameIndex).toString();
        *membershipNumber = query.value(membershipNumberIndex).toInt();
        errorTextVal.clear();
        return 0;
    }
    errorTextVal = "Id not found";
    return 4;
}



int CMembershipDbase::findNameFromTagId(const QByteArray &tagId, QString *firstName, QString *lastName) {
    if (!dBase.isOpen())
        return 1;

    QSqlQuery query;
    query.prepare("SELECT * FROM names WHERE tagId = (:tagId)");
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
        return -1;

    QSqlQuery query;
    query.prepare("SELECT Count(*) FROM names");
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



// getTagIdAndName()
// Return 0 on success
//
//int CDbase::getAllFromId(int id, QByteArray *tagId, QString *firstName, QString *lastName, int *membershipNumber) {
//    if (!dBase.isOpen())
//        return 1;

//    QSqlQuery query;
//    query.prepare("SELECT * FROM names WHERE id = (:id)");
//    query.bindValue(":id", id);
//    if (!query.exec()) {
//        errorTextVal = query.lastError().text();
//        qDebug() << errorTextVal;
//        return 1;
//    }

//    int tagIdIndex = query.record().indexOf("tagId");
//    int firstNameIndex = query.record().indexOf("firstName");
//    int lastNameIndex = query.record().indexOf("lastName");
//    if ((tagIdIndex < 0) || (firstNameIndex < 0) || (lastNameIndex < 0)) {
//        errorTextVal = "Could not find index";
//        qDebug() << errorTextVal;
//        return 2;
//    }

//    if (query.next()) {
//        *tagId = query.value(tagIdIndex).toString().toLatin1();
//        *firstName = query.value(firstNameIndex).toString();
//        *lastName = query.value(lastNameIndex).toString();
//        errorTextVal.clear();
//        return 0;
//    }
//    errorTextVal = "getTagIdAndName error";
//    qDebug() << errorTextVal;
//    return 3;
//}



// addLap
// Add entry to laps database
//
int CMembershipDbase::addLap(const QByteArray &tagId, int year, int month, int day, int hour, int minute, int second, int lapmsec, float lapm) {
    if (!dBase.isOpen())
        return 1;

    unsigned int dateTime = dateTime2Int(year, month, day, hour, minute, second);
    QSqlQuery query;
    query.prepare("INSERT INTO laps (tagId, dateTime, lapmsec, lapm) VALUES (:tagId, :dateTime, :lapmsec, :lapm)");
    query.bindValue(":tagId", tagId);
    query.bindValue(":dateTime", dateTime);
    query.bindValue(":lapmsec", lapmsec);
    query.bindValue(":lapm", lapm);
    if (!query.exec()) {
        errorTextVal = "Could not add to laps table";
        qDebug() << errorTextVal;
        return 1;
    }
    return 0;
}





// Calculate stats for specified rider (tagId) from dbase entries and populate CRider
//
int CMembershipDbase::getStats(const QByteArray &tagId, CRider *rider) {
    if (!dBase.isOpen())
        return 1;

    int thisMonthYear = QDateTime::currentDateTime().date().year();
    int thisMonthMonth = QDateTime::currentDateTime().date().month();
    QDateTime lastMonthDateTime(QDateTime::currentDateTime().addMonths(-1));
    int lastMonthYear = lastMonthDateTime.date().year();
    int lastMonthMonth = lastMonthDateTime.date().month();

    rider->thisMonth.clear();
    rider->lastMonth.clear();
    rider->allTime.clear();

    // Get stats for this month

    unsigned int dateTimeStart = dateTime2Int(thisMonthYear, thisMonthMonth, 0);
    unsigned int dateTimeEnd = dateTime2Int(thisMonthYear, thisMonthMonth, 31);

    getStatsForPeriod(tagId, dateTimeStart, dateTimeEnd, &rider->thisMonth);


    // Get stats for last month

    dateTimeStart = dateTime2Int(lastMonthYear, lastMonthMonth, 0);
    dateTimeEnd = dateTime2Int(lastMonthYear, lastMonthMonth, 31);

    getStatsForPeriod(tagId, dateTimeStart, dateTimeEnd, &rider->lastMonth);


    // Get stats for all time

    int allTimeYear = 2000;     // min value is 2000
    int allTimeMonth = 0;
    dateTimeStart = dateTime2Int(allTimeYear, allTimeMonth, 0);
    dateTimeEnd = dateTime2Int(thisMonthYear, thisMonthMonth, 31);

    getStatsForPeriod(tagId, dateTimeStart, dateTimeEnd, &rider->allTime);

    return 0;
}



// Get stats for specified tagId and time period from dbase
//
int CMembershipDbase::getStatsForPeriod(const QByteArray &tagId, unsigned int dateTimeStart, unsigned int dateTimeEnd, CStats *stats) {
    if (!dBase.isOpen())
        return 1;

    if (dateTimeStart > dateTimeEnd)
        throw("dateTimeStart > dateTimeEnd in getStatsForPeriod");


    // Determine minimum time difference in dateTimeInt that separates workouts

    int workoutDateTimeSeparation = dateTime2Int(2000,0,0,12,0,0) - dateTime2Int(2000,0,0,0,0,0);

    QSqlQuery query;
    query.prepare("SELECT * FROM laps WHERE tagId = :tagId AND dateTime BETWEEN :dateTimeStart AND :dateTimeEnd");
    query.bindValue(":tagId", tagId);
    query.bindValue(":dateTimeStart", dateTimeStart);
    query.bindValue(":dateTimeEnd", dateTimeEnd);
    if (!query.exec()) {
        errorTextVal = query.lastError().text();
        qDebug() << errorTextVal;
        return 1;
    }
    int lapmsecIndex = query.record().indexOf("lapmsec");
    if (lapmsecIndex < 0) {
        errorTextVal = "Could not find lapmsec index in getStats";
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

    int localBestLapmsec = -1;
    float localBestLapM = 0.;
    int previousDateTime = 0;
    int localWorkoutCount = 0;
    int localLapCount = 0;
    float localTotalSec = 0.;
    float localTotalM = 0.;
    while (query.next()) {
        int lapmsec = query.value(lapmsecIndex).toInt();
        float lapm = query.value(lapmIndex).toFloat();
        float lapspeed = 0.;
        if (lapmsec > 0) lapspeed = lapm / lapspeed;
        int dateTime = query.value(dateTimeIndex).toInt();
        if (localBestLapmsec < 0) {
            localBestLapmsec = lapmsec;
            localBestLapM = lapm;
        }
        else if (lapmsec < localBestLapmsec) {
            localBestLapmsec = lapmsec;
            localBestLapM = lapm;
        }
        localTotalSec += (float)lapmsec / 1000.;
        localLapCount++;
        localTotalM += lapm;
        int dateTimeDif = dateTime - previousDateTime;
        if (dateTimeDif > workoutDateTimeSeparation) {
            localWorkoutCount++;
        }

        previousDateTime = dateTime;
    }

    stats->lapCount = localLapCount;
    stats->workoutCount = localWorkoutCount;
    stats->bestLapSec = localBestLapmsec;
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
unsigned int CMembershipDbase::dateTime2Int(int year, int month, int day, int hour, int minute, int second) {
    if (year < 2000)
        throw("Year < 2000 in dateTime2Int");

    unsigned int dateTime = (year-2000) << 26 | month << 22 | day << 17 | hour << 12 | minute << 6 | (second & 0x3f);
    return dateTime;
}


// int2DateTime()
//
void CMembershipDbase::int2DateTime(unsigned int dateTime, int *year, int *month, int *day, int *hour, int *minute, int *second) {
    *year = ((dateTime >> 26) & 0x00ff) + 2000;
    *month = (dateTime >> 22) & 0x0f;
    *day = (dateTime >> 17) & 0x1f;
    *hour = (dateTime >> 12) & 0x0f;
    *minute = (dateTime >> 6) & 0x1f;
    *second = (dateTime >> 0) & 0x1f;
}





















CLapsDbase::CLapsDbase() {
    dBase = QSqlDatabase::addDatabase("QSQLITE");
}


int CLapsDbase::open(QString filename, QString username, QString password) {
    QSqlQuery query;
    errorTextVal.clear();
    dBase.setUserName(username);
    dBase.setPassword(password);
    dBase.setDatabaseName(filename);
    if (!dBase.open()) {
        errorTextVal = dBase.lastError().text();
        return 1;
    }

    bool showNames = true;//false;

    // Make sure tables exist and create as necessary
    // The first table is a list of tagId values and corresponding names

    QStringList tableList = dBase.tables();
    QString tableNames = "names";
    if (!tableList.contains(tableNames)) {
        query.prepare("create table " + tableNames + " (id INTEGER PRIMARY KEY AUTOINCREMENT, tagId VARCHAR(20) UNIQUE, firstName VARCHAR(20), lastName VARCHAR(20), membershipNumber INTEGER UNIQUE)");
        if (!query.exec()) {
            errorTextVal = query.lastError().text();
            throw("Error creating table " + tableNames + ": " + errorTextVal);
        }
        qDebug() << "Created new names table";
    }
    tableList = dBase.tables();

    if (showNames) {
        qDebug() << "List of tagId and names in names table " + filename + "...";
        query.prepare("select * from names");
        if (!query.exec()) {
            errorTextVal = query.lastError().text();
            qDebug() << errorTextVal;
            return 2;
        }
        int id = query.record().indexOf("id");
        int idTagId = query.record().indexOf("tagId");
        int idFirst = query.record().indexOf("firstName");
        int idLast = query.record().indexOf("lastName");
        int idMembership = query.record().indexOf("membershipNumber");
        while (query.next()) {
            qDebug("id=%d tagId=%s name=%s %s membershipNumber=%d", query.value(id).toInt(), query.value(idTagId).toString().toLatin1().data(), query.value(idFirst).toString().toLatin1().data(), query.value(idLast).toString().toLatin1().data(), query.value(idMembership).toInt());
        }
    }



    // The second table is a list of completed laps: tagId values, timeDateInt stamp values and laptime usec values and lapdistance values

    QString tableLaps = "laps";

//    bool deleteLapsTable = false;
//    if (deleteLapsTable) {
//        query.prepare("drop table laps");
//        if (!query.exec()) {
//            errorTextVal = query.lastError().text();
//            qDebug() << "Error deleting table " << tableLaps << ": " << errorTextVal;
//            return 3;
//        }
//        qDebug() << "Laps table deleted";
//    }

    if (!tableList.contains(tableLaps)) {
        query.prepare("create table " + tableLaps + " (id INTEGER PRIMARY KEY AUTOINCREMENT, tagId VARCHAR(20), dateTime INTEGER(10), lapmsec INTEGER(10), lapm FLOAT(10))");
        if (!query.exec()) {
            errorTextVal = query.lastError().text();
            qDebug() << "Error creating table " << tableLaps << ": " << errorTextVal;
            return 4;
        }
        qDebug() << "Created new laps table";
    }

    bool showLaps = false;
    if (showLaps) {
        qDebug() << "List of lap info in laps table...";
        QSqlQuery queryLaps;
        queryLaps.prepare("select * from laps");
        if (!queryLaps.exec()) {
            errorTextVal = queryLaps.lastError().text();
            qDebug() << errorTextVal;
            return 5;
        }
        int id = queryLaps.record().indexOf("id");
        int idTagId = queryLaps.record().indexOf("tagId");
        int idDateTime = queryLaps.record().indexOf("dateTime");
        int idLapmsec = queryLaps.record().indexOf("lapmsec");
        int idLapm = queryLaps.record().indexOf("lapm");
        while (queryLaps.next()) {
            int lapmsec = queryLaps.value(idLapmsec).toInt();
            float lapm = queryLaps.value(idLapm).toFloat();
            qDebug("id=%d tagId=%s dateTime=%d lapmsec=%d lapm=%f", queryLaps.value(id).toInt(), queryLaps.value(idTagId).toString().toLatin1().data(), queryLaps.value(idDateTime).toInt(), lapmsec, lapm);
        }
    }

    return 0;
}



// add
// Add entry to membership database.  TagId and first-last name must be unique.
//
int CLapsDbase::add(const QByteArray &tagId, const QString &firstName, const QString &lastName, const int membershipNumber) {
    if (!dBase.isOpen()) return 1;

    QSqlQuery query;
    query.prepare("INSERT INTO names (tagId, firstName, lastName, membershipNumber) VALUES (:tagId, :firstName, :lastName, :membershipNumber)");
    query.bindValue(":tagId", tagId);
    query.bindValue(":firstName", firstName);
    query.bindValue(":lastName", lastName);
    query.bindValue(":membershipNumber", membershipNumber);
    if (!query.exec()) {
        errorTextVal = "Could not add to database";
        return 2;
    }
    return 0;
}



// updateTagId
//
int CLapsDbase::update(const QByteArray &tagId, const QString &firstName, const QString &lastName, const int membershipNumber) {
    if (!dBase.isOpen()) return 1;

    QSqlQuery query;
    query.prepare("UPDATE names SET firstName = :firstName WHERE tagId = :tagId");
    query.bindValue(":tagId", tagId);
    query.bindValue(":firstName", firstName);
    if (!query.exec()) {
        errorTextVal = "Could not update first name in database";
        return 2;
    }
    query.prepare("UPDATE names SET lastName = :lastName WHERE tagId = :tagId");
    query.bindValue(":tagId", tagId);
    query.bindValue(":lastName", lastName);
    if (!query.exec()) {
        errorTextVal = "Could not update last name database";
        return 3;
    }
    query.prepare("UPDATE names SET membershipNumber = :membershipNumber WHERE tagId = :tagId");
    query.bindValue(":tagId", tagId);
    query.bindValue(":membershipNumber", membershipNumber);
    if (!query.exec()) {
        errorTextVal = "Could not update membership number database";
        return 4;
    }

    return 0;
}


// removeTagId
// Remove entry with specified tagId
//
int CLapsDbase::removeTagId(const QByteArray &tagId) {
    if (!dBase.isOpen()) return 1;

    QSqlQuery query;
    query.prepare("DELETE FROM names WHERE tagId = (:tagId)");
    query.bindValue(":tagId", tagId);
    if (!query.exec()) {
        errorTextVal = query.lastError().text();
        qDebug() << errorTextVal;
        return 2;
    }
    return 0;
}



int CLapsDbase::findTagIdFromName(const QString &firstName, const QString &lastName, QByteArray *tagId) {
    if (!dBase.isOpen()) return 1;

    QSqlQuery query;
    if (!lastName.isEmpty() && !firstName.isEmpty()) {
        query.prepare("SELECT tagId FROM names WHERE lastName = (:lastName) AND firstName = (:firstName)");
        query.bindValue(":lastName", lastName);
        query.bindValue(":firstName", firstName);
    }
    else if (!lastName.isEmpty()) {
        query.prepare("SELECT tagId FROM names WHERE lastName = (:lastName)");
        query.bindValue(":lastName", lastName);
    }
    else if (!firstName.isEmpty()) {
        query.prepare("SELECT tagId FROM names WHERE firstName = (:firstName)");
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
int CLapsDbase::getIdFromTagId(const QByteArray &tagId) {
    if (!dBase.isOpen()) return 0;
    if (tagId.isEmpty()) return 0;

    QSqlQuery query;
    query.prepare("SELECT id FROM names WHERE tagId = (:tagId)");
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
int CLapsDbase::getIdFromName(const QString &firstName, const QString &lastName) {
    if (!dBase.isOpen()) return 0;

    QSqlQuery query;
    if (!lastName.isEmpty() && !firstName.isEmpty()) {
        query.prepare("SELECT id FROM names WHERE lastName = (:lastName) AND firstName = (:firstName) COLLATE NOCASE");
        query.bindValue(":lastName", lastName);
        query.bindValue(":firstName", firstName);
    }
    else if (!lastName.isEmpty()) {
        query.prepare("SELECT id FROM names WHERE lastName = (:lastName) COLLATE NOCASE");
        query.bindValue(":lastName", lastName);
    }
    else if (!firstName.isEmpty()) {
        query.prepare("SELECT id FROM names WHERE firstName = (:firstName) COLLATE NOCASE");
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
int CLapsDbase::getIdFromMembershipNumber(const int membershipNumber) {
    if (!dBase.isOpen()) return 0;

    QSqlQuery query;
    query.prepare("SELECT id FROM names WHERE membershipNumber = (:membershipNumber)");
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



int CLapsDbase::getAllFromId(int id, QByteArray *tagId, QString *firstName, QString *lastName, int *membershipNumber) {
    if (!dBase.isOpen())
        return 1;

    QSqlQuery query;
    query.prepare("SELECT * FROM names WHERE id = (:id)");
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
    if ((tagIdIndex < 0) || (firstNameIndex < 0) || (lastNameIndex < 0) || (membershipNumberIndex < 0)) {
        errorTextVal = "Could not find index";
        qDebug() << errorTextVal;
        return 3;
    }

    if (query.next()) {
        *tagId = query.value(tagIdIndex).toString().toLatin1();
        *firstName = query.value(firstNameIndex).toString();
        *lastName = query.value(lastNameIndex).toString();
        *membershipNumber = query.value(membershipNumberIndex).toInt();
        errorTextVal.clear();
        return 0;
    }
    errorTextVal = "Id not found";
    return 4;
}



int CLapsDbase::findNameFromTagId(const QByteArray &tagId, QString *firstName, QString *lastName) {
    if (!dBase.isOpen())
        return 1;

    QSqlQuery query;
    query.prepare("SELECT * FROM names WHERE tagId = (:tagId)");
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




int CLapsDbase::namesRowCount(void) {
    if (!dBase.isOpen())
        return -1;

    QSqlQuery query;
    query.prepare("SELECT Count(*) FROM names");
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



// getTagIdAndName()
// Return 0 on success
//
//int CDbase::getAllFromId(int id, QByteArray *tagId, QString *firstName, QString *lastName, int *membershipNumber) {
//    if (!dBase.isOpen())
//        return 1;

//    QSqlQuery query;
//    query.prepare("SELECT * FROM names WHERE id = (:id)");
//    query.bindValue(":id", id);
//    if (!query.exec()) {
//        errorTextVal = query.lastError().text();
//        qDebug() << errorTextVal;
//        return 1;
//    }

//    int tagIdIndex = query.record().indexOf("tagId");
//    int firstNameIndex = query.record().indexOf("firstName");
//    int lastNameIndex = query.record().indexOf("lastName");
//    if ((tagIdIndex < 0) || (firstNameIndex < 0) || (lastNameIndex < 0)) {
//        errorTextVal = "Could not find index";
//        qDebug() << errorTextVal;
//        return 2;
//    }

//    if (query.next()) {
//        *tagId = query.value(tagIdIndex).toString().toLatin1();
//        *firstName = query.value(firstNameIndex).toString();
//        *lastName = query.value(lastNameIndex).toString();
//        errorTextVal.clear();
//        return 0;
//    }
//    errorTextVal = "getTagIdAndName error";
//    qDebug() << errorTextVal;
//    return 3;
//}



// addLap
// Add entry to laps database
//
int CLapsDbase::addLap(const QByteArray &tagId, int year, int month, int day, int hour, int minute, int second, int lapmsec, float lapm) {
    if (!dBase.isOpen())
        return 1;

    unsigned int dateTime = dateTime2Int(year, month, day, hour, minute, second);
    QSqlQuery query;
    query.prepare("INSERT INTO laps (tagId, dateTime, lapmsec, lapm) VALUES (:tagId, :dateTime, :lapmsec, :lapm)");
    query.bindValue(":tagId", tagId);
    query.bindValue(":dateTime", dateTime);
    query.bindValue(":lapmsec", lapmsec);
    query.bindValue(":lapm", lapm);
    if (!query.exec()) {
        errorTextVal = "Could not add to laps table";
        qDebug() << errorTextVal;
        return 1;
    }
    return 0;
}





// Calculate stats for specified rider (tagId) from dbase entries and populate CRider
//
int CLapsDbase::getStats(const QByteArray &tagId, CRider *rider) {
    if (!dBase.isOpen())
        return 1;

    int thisMonthYear = QDateTime::currentDateTime().date().year();
    int thisMonthMonth = QDateTime::currentDateTime().date().month();
    QDateTime lastMonthDateTime(QDateTime::currentDateTime().addMonths(-1));
    int lastMonthYear = lastMonthDateTime.date().year();
    int lastMonthMonth = lastMonthDateTime.date().month();

    rider->thisMonth.clear();
    rider->lastMonth.clear();
    rider->allTime.clear();

    // Get stats for this month

    unsigned int dateTimeStart = dateTime2Int(thisMonthYear, thisMonthMonth, 0);
    unsigned int dateTimeEnd = dateTime2Int(thisMonthYear, thisMonthMonth, 31);

    getStatsForPeriod(tagId, dateTimeStart, dateTimeEnd, &rider->thisMonth);


    // Get stats for last month

    dateTimeStart = dateTime2Int(lastMonthYear, lastMonthMonth, 0);
    dateTimeEnd = dateTime2Int(lastMonthYear, lastMonthMonth, 31);

    getStatsForPeriod(tagId, dateTimeStart, dateTimeEnd, &rider->lastMonth);


    // Get stats for all time

    int allTimeYear = 2000;     // min value is 2000
    int allTimeMonth = 0;
    dateTimeStart = dateTime2Int(allTimeYear, allTimeMonth, 0);
    dateTimeEnd = dateTime2Int(thisMonthYear, thisMonthMonth, 31);

    getStatsForPeriod(tagId, dateTimeStart, dateTimeEnd, &rider->allTime);

    return 0;
}



// Get stats for specified tagId and time period from dbase
//
int CLapsDbase::getStatsForPeriod(const QByteArray &tagId, unsigned int dateTimeStart, unsigned int dateTimeEnd, CStats *stats) {
    if (!dBase.isOpen())
        return 1;

    if (dateTimeStart > dateTimeEnd)
        throw("dateTimeStart > dateTimeEnd in getStatsForPeriod");


    // Determine minimum time difference in dateTimeInt that separates workouts

    int workoutDateTimeSeparation = dateTime2Int(2000,0,0,12,0,0) - dateTime2Int(2000,0,0,0,0,0);

    QSqlQuery query;
    query.prepare("SELECT * FROM laps WHERE tagId = :tagId AND dateTime BETWEEN :dateTimeStart AND :dateTimeEnd");
    query.bindValue(":tagId", tagId);
    query.bindValue(":dateTimeStart", dateTimeStart);
    query.bindValue(":dateTimeEnd", dateTimeEnd);
    if (!query.exec()) {
        errorTextVal = query.lastError().text();
        qDebug() << errorTextVal;
        return 1;
    }
    int lapmsecIndex = query.record().indexOf("lapmsec");
    if (lapmsecIndex < 0) {
        errorTextVal = "Could not find lapmsec index in getStats";
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

    int localBestLapmsec = -1;
    float localBestLapM = 0.;
    int previousDateTime = 0;
    int localWorkoutCount = 0;
    int localLapCount = 0;
    float localTotalSec = 0.;
    float localTotalM = 0.;
    while (query.next()) {
        int lapmsec = query.value(lapmsecIndex).toInt();
        float lapm = query.value(lapmIndex).toFloat();
        float lapspeed = 0.;
        if (lapmsec > 0) lapspeed = lapm / lapspeed;
        int dateTime = query.value(dateTimeIndex).toInt();
        if (localBestLapmsec < 0) {
            localBestLapmsec = lapmsec;
            localBestLapM = lapm;
        }
        else if (lapmsec < localBestLapmsec) {
            localBestLapmsec = lapmsec;
            localBestLapM = lapm;
        }
        localTotalSec += (float)lapmsec / 1000.;
        localLapCount++;
        localTotalM += lapm;
        int dateTimeDif = dateTime - previousDateTime;
        if (dateTimeDif > workoutDateTimeSeparation) {
            localWorkoutCount++;
        }

        previousDateTime = dateTime;
    }

    stats->lapCount = localLapCount;
    stats->workoutCount = localWorkoutCount;
    stats->bestLapSec = localBestLapmsec;
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

    unsigned int dateTime = (year-2000) << 26 | month << 22 | day << 17 | hour << 12 | minute << 6 | (second & 0x3f);
    return dateTime;
}


// int2DateTime()
//
void CLapsDbase::int2DateTime(unsigned int dateTime, int *year, int *month, int *day, int *hour, int *minute, int *second) {
    *year = ((dateTime >> 26) & 0x00ff) + 2000;
    *month = (dateTime >> 22) & 0x0f;
    *day = (dateTime >> 17) & 0x1f;
    *hour = (dateTime >> 12) & 0x0f;
    *minute = (dateTime >> 6) & 0x1f;
    *second = (dateTime >> 0) & 0x1f;
}
