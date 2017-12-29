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
    int add(const QByteArray &tagId, const QString &firstName, const QString &lastName, const int membershipNumber, const int ccaRegistration, QByteArray email);
    int update(const QByteArray &tagId, const QString &firstName, const QString &lastName, const int membershipNumber, const int ccaRegistration, const QByteArray &email);
    int removeTagId(const QByteArray &tagId);
    int findNameFromTagId(const QByteArray &tagId, QString *firstName, QString *lastName);
    int findTagIdFromName(const QString &firstName, const QString &lastName, QByteArray *tagId);
    int getIdFromTagId(const QByteArray &tagId);
    int getIdFromName(const QString &firstName, const QString &lastName);
    int getIdFromMembershipNumber(const int membershipNumber);
    int getAllFromId(int id, QByteArray *tagId, QString *firstName, QString *lastName, int *membershipNumber);
    int namesRowCount(void);
    int error(void);
    bool isOpen(void);
    QString errorText(void);
private:
    QSqlDatabase dBase;
    QString errorTextVal;
    int errorVal;
};


class CLapsDbase
{
public:
    CLapsDbase();
    int open(const QString &filename, const QString &username, const QString &password);
    void close(void);
    int addLap(const QByteArray &tagId, int year, int month, int day, int hour, int minute, int second, int msec, float lapm);
    QList<int> getLapmsec(const QByteArray &tagId, int yearStart=0, int monthStart=0, int dayStart=0, int yearEnd=0, int monthEnd=0, int dayEnd=0);
    int getStats(const QByteArray &tagId, CRider *rider);
    int getStatsForPeriod(const QByteArray &tagId, unsigned int dateTimeStart, unsigned int dateTimeEnd, CStats *stats);
    unsigned int dateTime2Int(int year=0, int month=0, int day=0, int hour=0, int minute=0, int second=0);
    void int2DateTime(unsigned int dateTime, int *year, int *month, int *day, int *hour, int *minute, int *second);
    int error(void);
    bool isOpen(void);
    QString errorText(void);
private:
    QSqlDatabase dBase;
    QString errorTextVal;
    int errorVal;
};

#endif // CDBASE_H
