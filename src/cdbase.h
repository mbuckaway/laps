#ifndef CDBASE_H
#define CDBASE_H

#include <QString>
#include <QtSql/QtSql>
#include <QSqlDatabase>
#include <QAbstractTableModel>

#include "crider.h"



class CMembershipDbase
{
public:
    CMembershipDbase();
    int open(const QString &filename, const QString &username, const QString &password);
    void close(void);
    int add(const QString &tagId, const QString &firstName, const QString &lastName, const QString &membershipNumber, const QString &caRegistration, const QString &email);
    int update(const QString &tagId, const QString &firstName, const QString &lastName, const QString &membershipNumber, const QString &caRegistration, const QString &email);
    int removeTagId(const QString &tagId);
    int findNameFromTagId(const QString &tagId, QString *firstName, QString *lastName);
    int findTagIdFromName(const QString &firstName, const QString &lastName, QString *tagId);
    int getIdFromTagId(const QString &tagId);
    int getIdFromName(const QString &firstName, const QString &lastName);
    int getIdFromMembershipNumber(const QString &membershipNumber);
    int getAllFromId(int id, QString *tagId, QString *firstName, QString *lastName, QString *membershipNumber, QString *caRegistration, QString *email);
    int namesRowCount(void);
    int error(void);
    bool isOpen(void);
    QString errorText(void);
    QSqlDatabase dBase;
private:
    QString errorTextVal;
    int errorVal;
};


class CLapsDbase
{
public:
    CLapsDbase();
    int open(const QString &filename, const QString &username, const QString &password);
    void close(void);
    int addLap(const QString &tagId, int year, int month, int day, int hour, int minute, int second, float sec, float lapm);
    QList<float> getLapSec(const QString &tagId, int yearStart=0, int monthStart=0, int dayStart=0, int yearEnd=0, int monthEnd=0, int dayEnd=0);
    int getStats(const QString &tagId, CRider *rider);
    int getStatsForPeriod(const QString &tagId, unsigned int dateTimeStart, unsigned int dateTimeEnd, CStats *stats);
    unsigned int dateTime2Int(int year, int month, int day, int hour, int minute, int second);
    void int2DateTime(unsigned int dateTime, int *year, int *month, int *day, int *hour, int *minute, int *second);
    int error(void);
    bool isOpen(void);
    QString errorText(void);
    QSqlDatabase dBase;
private:
    QString errorTextVal;
    int errorVal;
};

#endif // CDBASE_H
