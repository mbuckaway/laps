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
    float totalSec;
    float totalM;
    float averageKph;
    float bestLapSec;
    float bestLapM;
    float bestLapKph;
};


class CRider {
public:
    enum lap_t {unknown=0, firstCrossing=1, regularCrossing=2, onBreak=3, firstCrossingAfterBreak=4};
    CRider(void);
    ~CRider(void);
    void clear();
    QString tagId;          // from reader
    QString name;           // from dBase if available
    unsigned long long previousTimeStampUSec;   // timestamp from reader, updated with each lap
    float lapSec;
    float lapM;
    float lapKph;
    int lapCount;
    float totalSec;    // used to calculate average time
    float totalM;
    float averageKph;
    float bestLapSec;
    float bestLapM;
    float bestLapKph;
    CStats thisMonth;
    CStats lastMonth;
    CStats allTime;
    QString comment;
    lap_t lapType;
    lap_t nextLapType;
    int reportStatus;       // 0 if lap should not be included in email report, 1 if it should
    bool inDbase;
    int readerId;
    int antennaId;
};









#endif // CRIDER_H
