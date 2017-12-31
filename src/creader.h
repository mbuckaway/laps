//********************************************************************
//      created:        2017/07/15
//      filename:       CREADER.CPP
//
//  (C) Copyright 2017 Forestcity Velodrome
//
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//*********************************************************************
// creader.h
//
// IIPJ R1000 Reader defaults
// IP: 192.168.1.98
// username: root
// password: impinj
//

#ifndef CREADER_H
#define CREADER_H

#include <cstdint>
#include <memory>

#include <QObject>
#include <QString>
#include <QList>
#include <QThread>

#include <ltkcpp.h>


class CTagInfo {
public:
    CTagInfo(void);
    void clear(void);
    int readerId;
    int antennaId;
    QString tagId;
    unsigned long long timeStampUSec;
    unsigned long long firstSeenInApplicationUSec;
};

Q_DECLARE_METATYPE(CTagInfo)


class CReader : public QObject
{
    Q_OBJECT
public:
    enum antennaPositionType {track, desk};
    explicit CReader(QString hostName, int readerId, antennaPositionType antennaPosition);
    virtual ~CReader(void);
    int connectToReader(void);
    QList<int> *getTransmitPowerList(void);
    //int setTransmitPower(int index);
    int setTrackLength(int antenna);    // set length of track at height of specified antenna
    int setReaderConfiguration(void);
    int processReports(void);
    antennaPositionType antennaPosition;
    QThread *thread;
    int readerId;
private:
    QList<CTagInfo> currentTagsList;    // list of tags currently seen by reader
    QString hostName;
    bool waitingForFirstTag;
    long long timeStampCorrectionUSec;
    unsigned messageId;
    int checkConnectionStatus(void);
    int scrubConfiguration(void);
    int resetConfigurationToFactoryDefaults(void);
    int deleteAllROSpecs(void);
    int addROSpec(void);
    int enableROSpec(void);
    int startROSpec(void);
    void printXMLMessage(LLRP::CMessage *pMessage);
    void handleReaderEventNotification(LLRP::CReaderEventNotificationData *pNtfData);
    void handleAntennaEvent(LLRP::CAntennaEvent *pAntennaEvent);
    void handleReaderExceptionEvent(LLRP::CReaderExceptionEvent *pReaderExceptionEvent);
    int checkLLRPStatus(LLRP::CLLRPStatus *pLLRPStatus, char *pWhatStr);
    int sendMessage(LLRP::CMessage *pSendMsg);
    void processTagList(LLRP::CRO_ACCESS_REPORT *pRO_ACCESS_REPORT);
    int getTransmitPowerCapabilities(void);
    QList<int> transmitPowerList;
    bool simulateReaderMode;
    unsigned long long maxAllowableTimeInListUSec;
    LLRP::CConnection *connectionToReader;
    LLRP::CTypeRegistry *typeRegistry;
    LLRP::CMessage *recvMessage(int nMaxMS);
    LLRP::CMessage *transact (LLRP::CMessage *sendMsg);
signals:
    void connected(void);
    void newTag(CTagInfo);
    void newLogMessage(QString);
    void error(QString);
private slots:
    void onStarted(void);
};

#endif // CREADER_H
