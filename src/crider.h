#ifndef CRIDER_H
#define CRIDER_H


#include <QString>
#include <QList>

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
    float averageKmph;
    float bestLapKmph;
    float bestKKmph;
};


class CRider {//: public CStats {
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
    float lapKmph;
    int lapCount;
    float totalSec;    // used to calculate average time
    float totalM;
    float averageKmph;
    float bestLapKmph;
    float bestKKmph;
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

    QList<float> lapMList;
    QList<float> lapSecList;
    float lapMSum;
    float lapSecSum;
};









#endif // CRIDER_H
