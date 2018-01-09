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
    bestLapSec = 0.;
    bestLapM = 0.;
}



CRider::CRider(void) {
    clear();
}



CRider::~CRider(void) {
}



void CRider::clear(void) {
    tagCount = 0;
    previousTimeStampUSec = 0;
    lapCount = 0;
    lapSec = 0.;
    lapM = 0.;
    totalSec = 0.;
    totalM = 0.;
    bestLapSec = 0.;
    bestLapM = 0.;
    reportStatus = 0;
    inDbase = false;
    lapType = unknown;
    nextLapType = unknown;
}



