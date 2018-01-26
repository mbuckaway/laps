#include "crider.h"




CStats::CStats(void) {
    clear();
}



CStats::~CStats(void) {
}



void CStats::clear(void) {
    workoutCount = 0;
    lapCount = 0;
    totalSec = 0.;
    totalM = 0.;
    averageKph = 0.;
    bestLapSec = 0.;
    bestLapM = 0.;
    bestLapKph = 0.;
    bestKKph = 0.;
}



CRider::CRider(void) {
    clear();
}



CRider::~CRider(void) {
}



void CRider::clear(void) {
    previousTimeStampUSec = 0;
    lapCount = 0;
    lapSec = 0.;
    lapM = 0.;
    lapKph = 0.;
    readerId = 0;
    antennaId = 0;
    totalSec = 0.;
    totalM = 0.;
    averageKph = 0.;
    bestLapSec = 0.;
    bestLapM = 0.;
    bestLapKph = 0.;
    reportStatus = 0;
    inDbase = false;
    lapType = unknown;
    nextLapType = unknown;
    lapMList.clear();
    lapSecList.clear();
    lapMSum = 0.;
    lapSecSum = 0.;
    bestKKmph = 0.;
}



