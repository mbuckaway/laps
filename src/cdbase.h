#ifndef CDBASE_H
#define CDBASE_H

#include <QString>
#include <QObject>
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



// CDateTime
// Format for dateTime used in database where dateTime is encoded into a single unsigned int (32 bits)
//
class CDateTime {
public:
    CDateTime(int year, int month, int day, int hour, int minute, int second);
    CDateTime(const QDateTime &);
    CDateTime(const QDate &, int hour, int minute, int second);
    CDateTime(unsigned int dateTime);
    int year() const;
    int month() const;
    int day() const;
    int hour() const;
    int minute() const;
    int second() const;
    unsigned int toUInt() const;
    QDateTime toQDateTime(void) const;
    QDate toQDate(void) const;
    QTime toQTime(void) const;
private:
    int yearVal;
    int monthVal;
    int dayVal;
    int hourVal;
    int minuteVal;
    int secondVal;
    unsigned int intVal;
    void calculateUIntVal(void);
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
//    Q_OBJECT
public:
    enum reportStatus_t {reportDisabled=0, reportPending=1, reportCompleted=2, reportAny=3};
    CLapsDbase(void);
    int open(const QString &connectionName, const QString &username, const QString &password);
    void close(void);
    int addLap(const CRider &rider, const QDateTime &dateTime);
    int getLap(int id, QString *tagId, CLapInfo *lapInfo=NULL);
    int getStats(const QString &tagId, CRider *rider);
    int getStats(const QString &tagId, const QDateTime &start, const QDateTime &end, reportStatus_t reportStatus, CStats *stats);
    int setReportStatus(reportStatus_t reportStatus, const QString &tagId, const QDateTime &start, const QDateTime &end);
    int getLaps(const QString &tagId, const QDateTime &start, const QDateTime &end, reportStatus_t reportStatus, QList<int> *lapsList);
    int error(void);
    bool isOpen(void);
    int getLapInfo(const QString &tagId, const QDateTime &start, const QDateTime &end, QList<CLapInfo> *laps);
    QFileInfo prior;
    QString errorText(void);
    QSqlDatabase dBase;
    QList<QSqlDatabase> dBasePriorList;
    float allTimeBestLapKph;
    QString allTimeBestLapKphName;
private:
    QString errorTextVal;
    int getLapInfo(const QSqlDatabase &dBase, const QString &tagId, const QDateTime &start, const QDateTime &end, QList<CLapInfo> *laps);
    int getStats(const QSqlDatabase &dBase, const QString &tagId, const QDateTime &start, const QDateTime &end, reportStatus_t reportStatus, CStats *stats);
    int insertPriors(const QSqlDatabase &dBase, const QString &tagId, int lapCount, float lapSecTotal, float lapMTotal);
    int getPriors(const QSqlDatabase &dBase, const QString &tagId, int *lapCount, float *lapSecTotal, float *lapMTotal);
    int errorVal;
signals:
//    void newLogMessage(QString);
};

#endif // CDBASE_H
