#ifndef CDBASE_H
#define CDBASE_H

#include <QString>
#include <QtSql/QtSql>
#include <QSqlDatabase>
#include <QAbstractTableModel>

#include "crider.h"







class CDbase
{
public:
    CDbase();
    int open(QString filename, QString username, QString password);
    void close(void);
    int error(void);
    bool isOpen(void);
    QString errorText(void);
    int addTagId(const QByteArray &tagId, const QString &firstName, const QString &lastName);
    int updateTagId(const QByteArray &tagId, const QString &firstName, const QString &lastName);
    int removeTagId(const QByteArray &tagId);
    int findNameFromTagId(const QByteArray &tagId, QString *firstName, QString *lastName);
    int findTagIdFromName(const QString &firstName, const QString &lastName, QByteArray *tagId);
    int getIdFromName(const QString &firstName, const QString &lastName);
    int getTagIdAndName(int id, QByteArray *tagId, QString *firstName, QString *lastName);
    int getAllFromId(int id, QByteArray *tagId, QString *firstName, QString *lastName);
    int namesRowCount(void);
    int addLap(const QByteArray &tagId, int year, int month, int day, int hour, int minute, int second, int msec, float lapm);
    QList<int> getLapmsec(const QByteArray &tagId, int yearStart=0, int monthStart=0, int dayStart=0, int yearEnd=0, int monthEnd=0, int dayEnd=0);
    int getStats(const QByteArray &tagId, CRider *rider);
//    int getStatsForPeriod(const QByteArray &tagId, unsigned int dateTimeStart, unsigned int dateTimeEnd, int *lapCount, int *workoutCount, float *totalSec, float *totalM, float *bestLapSec, float *bestLapM);
    int getStatsForPeriod(const QByteArray &tagId, unsigned int dateTimeStart, unsigned int dateTimeEnd, CStats *stats);
    unsigned int dateTime2Int(int year=0, int month=0, int day=0, int hour=0, int minute=0, int second=0);
    void int2DateTime(unsigned int dateTime, int *year, int *month, int *day, int *hour, int *minute, int *second);
private:
    QSqlDatabase dBase;
    QString errorTextVal;
    int errorVal;
private slots:
};

#endif // CDBASE_H
