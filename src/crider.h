#ifndef CRIDER_H
#define CRIDER_H


#include <QString>

// CRider is a structure used to keep all information available for each rider


class CStats {
public:
    CStats(void);
    ~CStats(void);
    void clear(void);
    int workoutCount;
    int lapCount;
    float bestLapSec;
    float bestLapM;
    float totalSec;
    float totalM;
};


class CRider {//: public CStats {
public:
    CRider(void);
    ~CRider(void);
    void clear();
    QString tagId;          // from reader
    QString name;           // from dBase if available
    unsigned long long previousTimeStampUSec;   // timestamp from reader, updated with each lap
    float lapSec;
    float lapM;
    float lapSpeed;
    int lapCount;
    float totalSec;    // used to calculate average time
    float totalM;
    float bestLapSec;
    float bestLapM;
    CStats thisMonth;
    CStats lastMonth;
    CStats allTime;
    QString comment;
    bool firstLap;
    bool firstLapAfterBreak;
    bool onBreak;
    int reportStatus;       // 0 if lap should not be included in email report, 1 if it should
    bool inDbase;
};









#endif // CRIDER_H
