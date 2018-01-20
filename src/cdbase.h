#ifndef CDBASE_H
#define CDBASE_H

#include <QString>
#include <QtSql/QtSql>
#include <QSqlDatabase>
#include <QAbstractTableModel>

#include "crider.h"



class CMembershipInfo {
public:
    QString tagId;
    QString firstName;
    QString lastName;
    QString membershipNumber;
    QString caRegistration;
    QString eMail;
    bool sendReports;
};



class CMembershipDbase : public QFileInfo
{
public:
    CMembershipDbase();
    int open(const QString &rootName, const QString &username, const QString &password);
    void close(void);
    int add(const CMembershipInfo &info);
    int update(const CMembershipInfo &info);
    int removeTagId(const QString &tagId);
    int findNameFromTagId(const QString &tagId, QString *firstName, QString *lastName);
    int findTagIdFromName(const QString &firstName, const QString &lastName, QString *tagId);
    int getIdFromTagId(const QString &tagId);
    int getIdFromName(const QString &firstName, const QString &lastName);
    int getIdFromMembershipNumber(const QString &membershipNumber);
    int getAllFromId(int id, CMembershipInfo *info);
    int getAllList(QList<CMembershipInfo> *infoList);
    int namesRowCount(void);
    int error(void);
    bool isOpen(void);
    QString errorText(void);
    QSqlDatabase dBase;
private:
    QString errorTextVal;
    int errorVal;
};



class CLapInfo {
public:
    CLapInfo(unsigned int dateTime=0, float sec=0., float m=0.);
    unsigned int dateTime;
    float lapSec;
    float lapM;
    int reportStatus;
};



class CLapsDbase : public QFileInfo
{
public:
    enum reportStatus_t {reportDisabled=0, reportPending=1, reportCompleted=2, reportAny=3};
    CLapsDbase(void);
    int open(const QString &connectionName, const QString &username, const QString &password);
    void close(void);
    int addLap(const CRider &rider, unsigned int dateTime);
    int getLap(int id, QString *tagId, CLapInfo *lapInfo=NULL);
    int getStats(const QString &tagId, CRider *rider);
    int getStatsForCurrentPeriod(const QString &tagId, unsigned int dateTimeStart, unsigned int dateTimeEnd, reportStatus_t reportStatus, CStats *stats);
    int getStatsForPriorPeriod(const QString &tagId, unsigned int dateTimeStart, unsigned int dateTimeEnd, reportStatus_t reportStatus, CStats *stats);
    int setReportStatus(reportStatus_t reportStatus, const QString &tagId, unsigned int dateTimeStart, unsigned int dateTimeEnd);
    int getLapsInPeriod(const QString &tagId, unsigned int dateTimeStart, unsigned int dateTimeEnd, reportStatus_t reportStatus, QList<int> *lapsList);
    static unsigned int dateTime2Int(int year, int month, int day, int hour, int minute, int second);
    static void int2DateTime(unsigned int dateTime, int *year, int *month, int *day, int *hour, int *minute, int *second);
    static QDateTime int2DateTime(unsigned int dateTIme);
    static unsigned int dateTime2Int(const QDateTime &dateTime);
    int error(void);
    bool isOpen(void);
    int getLapInfo(const QString &tagId, unsigned int dateTimeStart, unsigned int dateTimeEnd, QList<CLapInfo> *laps);
    QFileInfo prior;
    QString errorText(void);
    QSqlDatabase dBase;
    QList<QSqlDatabase> dBasePriorList;
private:
    QString errorTextVal;
    int getLapInfo(const QSqlDatabase &dBase, const QString &tagId, unsigned int dateTimeStart, unsigned int dateTimeEnd, QList<CLapInfo> *laps);
    int getStatsForPeriod(const QSqlDatabase &dBase, const QString &tagId, unsigned int dateTimeStart, unsigned int dateTimeEnd, reportStatus_t reportStatus, CStats *stats);
    int insertPriors(const QSqlDatabase &dBase, const QString &tagId, int lapCount, float lapSecTotal, float lapMTotal);
    int getPriors(const QSqlDatabase &dBase, const QString &tagId, int *lapCount, float *lapSecTotal, float *lapMTotal);
    int errorVal;
signals:
    void newLogMessage(QString);
};

#endif // CDBASE_H
