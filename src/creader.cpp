// creader.cpp
// Definition of class CReader used to interface with reader hardware using the LTKCPP and LLRP libraries.
// Signals generated:
//   newLogMessage(QString message) - sends text message to gui
//   connected(int readerId)        - tells gui we have successfully connected to reader with readerId
//   newTag(CTagInfo tagInfo)       - tells gui a new tag has been detected


#include <QList>
#include <QDateTime>
#include <QDebug>

#include <unistd.h>

#include "creader.h"


// **********************************************************************************************

CTagInfo::CTagInfo(void) {
    clear();

}


void CTagInfo::clear(void) {
    tagId.clear();
    timeStampUSec = 0;
    antennaId = 0;
    readerId = 0;
}




// **********************************************************************************************

CReader::CReader(QString hostName, int readerId, CReader::antennaPositionType antennaPosition) {
    this->hostName = hostName;
    this->readerId = readerId;
    this->antennaPosition = antennaPosition;
    messageId = 0;
    simulateReaderMode = hostName.isEmpty();
    waitingForFirstTag = true;
    timeStampCorrectionUSec = 0;
    connectionToReader = NULL;
    typeRegistry = NULL;
    qRegisterMetaType<CTagInfo>();      // required to emit signal with CTagInfo
    thread = NULL;
}



CReader::~CReader(void) {
    QString s;

    /*
     * After we're done, try to leave the reader
     * in a clean state for next use. This is best
     * effort and no checking of the result is done.
     */

    emit newLogMessage(s.sprintf("INFO: Clean up reader configuration..."));

    try {
        scrubConfiguration();

        // Close the connection and release its resources

        if (connectionToReader) {
            connectionToReader->closeConnectionToReader();
            delete connectionToReader;
            connectionToReader = NULL;
        }

        // Done with the registry

        if (typeRegistry) {
            delete typeRegistry;
            typeRegistry = NULL;
        }
    }
    catch (const std::exception &e) {
        // Log exception when logging is implemented
    }

    emit newLogMessage(s.sprintf("INFO: Finished"));
}



void CReader::onStarted(void) {
    QString s;
    CTagInfo tag;

    // If simulateReaderMode flag is set, enter endless loop to emit newTag signals at random intervals

    if (simulateReaderMode) {
        emit newLogMessage("Simulation mode to simulate reader signals without being connected to reader");
        emit connected();
        tag.readerId = readerId;
        int averageIntervalMSec = 0; // average interval between tags seen
        int count = 0;

        switch (antennaPosition) {
        case track:
            averageIntervalMSec = 10000;
            break;
        case desk:
            averageIntervalMSec = 10000;
            break;
        }

        forever {
            for (int i=0; i<50; i++) {
                count++;
                tag.antennaId = (rand() % 4) + 1;   // random antennaId between 1 and 4
                tag.timeStampUSec = QDateTime::currentMSecsSinceEpoch() * 1000;
                int id = (rand() % 16) + 1;      // random number between 1 and 16
                tag.tagId = s.sprintf("2016000000%02x", id).toLatin1();
                int intervalMSec = rand() % averageIntervalMSec + 1;     // next interval between 1 and 2000 msec
                usleep(intervalMSec * 100);
                emit newTag(tag);

                if (thread && thread->isInterruptionRequested()) {
                    thread->quit();
                    return;
                }
            }
            sleep(3);

            // emit null tag to update tables

            CTagInfo tagInfo;
            tagInfo.readerId = readerId;
            tagInfo.antennaId = 0;
            tagInfo.timeStampUSec = QDateTime::currentMSecsSinceEpoch() * 1000;
            emit newTag(tagInfo);
        }
    }

    // Otherwise open connection to reader and enter endless loop in which we process tag reports as they arrive

    forever {
        int rc = connectToReader();
        if (rc == 0) {
            emit connected();
            forever {
                processReports();

                // Check for errors and break out of loop if connection is lost

                if (thread && thread->isInterruptionRequested()) {
                    thread->quit();
                    return;
                }
            }
        }
        if (connectionToReader) {
            connectionToReader->closeConnectionToReader();
            delete connectionToReader;
            connectionToReader = NULL;
        }
        if (typeRegistry) {
            delete typeRegistry;
            typeRegistry = NULL;
        }
    }
}



int CReader::connectToReader(void) {
    QString s;
    int rc;

    /*
     * Allocate the type registry. This is needed
     * by the connection to decode.
     */

    typeRegistry = LLRP::getTheTypeRegistry();
    if (!typeRegistry) {
        emit newLogMessage(s.sprintf("ERROR: getTheTypeRegistry failed"));
        return 1;
    }

    /*
     * Construct a connection (LLRP::CConnection).
     * Using a 32kb max frame size for send/recv.
     * The connection object is ready for business
     * but not actually connected to the reader yet.
     */

    connectionToReader = new LLRP::CConnection(typeRegistry, 32u*1024u);
    if (!connectionToReader) {
        delete typeRegistry;
        typeRegistry = NULL;
        emit newLogMessage(s.sprintf("ERROR: new CConnection failed"));
        return 2;
    }

    /*
     * Open connection to the reader
     */

    emit newLogMessage(s.sprintf("INFO: Connecting to reader %d at %s....", readerId, hostName.toLatin1().data()));

    rc = connectionToReader->openConnectionToReader(hostName.toLatin1().data());
    if (rc) {
        emit newLogMessage(s.sprintf("ERROR: openConnectionToReader(id=%d):: %s, error code %d", readerId, connectionToReader->getConnectError(), rc));
        delete typeRegistry;
        typeRegistry = NULL;
        delete connectionToReader;
        connectionToReader = NULL;
        return 3;
    }

    /*
     * Commence the sequence and check for errors as we go.
     * See comments for each routine for details.
     * Each routine prints messages.
     */

    emit newLogMessage(s.sprintf("INFO: Connected, checking status...."));

    rc = checkConnectionStatus();
    if (rc != 0) {
        emit newLogMessage(s.sprintf("checkConnectionStatus(): error %d - Cannot connect to reader.  This may mean another instance of this program is already running.", rc));
        return 4;
    }

    if (scrubConfiguration() != 0) {
        emit newLogMessage(s.sprintf("scrubConfiguration failed"));
        return 5;
    }

    // Get transmit power values for reader.  Gui will use these to populate comboBox.

    rc = getTransmitPowerCapabilities();
    if (rc) {
        emit newLogMessage(s.sprintf("getTransmitPowerCapabilities() failed, rc=%d", rc));
        return 6;
    }

    rc = setReaderConfiguration();
    if (rc) {
        emit newLogMessage(s.sprintf("setReaderConfiguration() failed, rc=%d", rc));
        return 7;
    }

    if (addROSpec() != 0) {
        emit newLogMessage(s.sprintf("addROSpec() failed"));
        return 8;
    }

    if(enableROSpec() != 0) {
        emit newLogMessage(s.sprintf("enableROSpec() failed"));
        return 9;
    }

    if (startROSpec() != 0) {
        return 10;
    }

    return 0;
}



/**
 *****************************************************************************
 **
 ** @brief  Await and check the connection status message from the reader
 **
 ** We are expecting a READER_EVENT_NOTIFICATION message that
 ** tells us the connection is OK. The reader is suppose to
 ** send the message promptly upon connection.
 **
 ** If there is already another LLRP connection to the
 ** reader we'll get a bad Status.
 **
 ** The message should be something like:
 **
 **     <READER_EVENT_NOTIFICATION MessageID='0'>
 **       <ReaderEventNotificationData>
 **         <UTCTimestamp>
 **           <Microseconds>1184491439614224</Microseconds>
 **         </UTCTimestamp>
 **         <ConnectionAttemptEvent>
 **           <Status>Success</Status>
 **         </ConnectionAttemptEvent>
 **       </ReaderEventNotificationData>
 **     </READER_EVENT_NOTIFICATION>
 **
 ** @return     ==0             Everything OK
 **             !=0             Something went wrong
 **
 *****************************************************************************/

int CReader::checkConnectionStatus(void) {
    QString s;
    LLRP::CMessage *pMessage;
    LLRP::CREADER_EVENT_NOTIFICATION *pNtf;
    LLRP::CReaderEventNotificationData *pNtfData;
    LLRP::CConnectionAttemptEvent *pEvent;

    /*
     * Expect the notification within 10 seconds.
     * It is suppose to be the very first message sent.
     */

    pMessage = recvMessage(10000);

    /*
     * recvMessage() returns NULL if something went wrong.
     */

    if (NULL == pMessage) {
        /* recvMessage already tattled */
        goto fail;
    }

    /*
     * Check to make sure the message is of the right type.
     * The type label (pointer) in the message should be
     * the type descriptor for READER_EVENT_NOTIFICATION.
     */

    if (&LLRP::CREADER_EVENT_NOTIFICATION::s_typeDescriptor != pMessage->m_pType) {
        goto fail;
    }

    /*
     * Now that we are sure it is a READER_EVENT_NOTIFICATION,
     * traverse to the ReaderEventNotificationData parameter.
     */

    pNtf = (LLRP::CREADER_EVENT_NOTIFICATION *) pMessage;
    pNtfData = pNtf->getReaderEventNotificationData();
    if (NULL == pNtfData) {
        goto fail;
    }

    /*
     * The ConnectionAttemptEvent parameter must be present.
     */

    pEvent = pNtfData->getConnectionAttemptEvent();
    if (NULL == pEvent) {
        goto fail;
    }

    /*
     * The status in the ConnectionAttemptEvent parameter
     * must indicate connection success.
     */

    if (LLRP::ConnectionAttemptStatusType_Success != pEvent->getStatus()) {
        goto fail;
    }

    /*
     * Done with the message
     */

    if (pMessage) delete pMessage;

    emit newLogMessage(s.sprintf("INFO: Connection status OK"));

    /*
     * Victory.
     */

    return 0;

  fail:

    /*
     * Something went wrong. Tattle. Clean up. Return error.
     */

    emit newLogMessage(s.sprintf("ERROR: checkConnectionStatus failed"));

    if (pMessage) delete pMessage;

    return -1;
}


/**
 *****************************************************************************
 **
 ** @brief  Scrub the reader configuration
 **
 ** The steps:
 **     - Try to reset configuration to factory defaults,
 **       this feature is optional and may not be supported
 **       by the reader.
 **     - Delete all ROSpecs
 **
 ** @return     ==0             Everything OK
 **             !=0             Something went wrong
 **
 *****************************************************************************/

int CReader::scrubConfiguration(void) {

    if ( 0 != resetConfigurationToFactoryDefaults()) {
        return -1;
    }

    if (0 != deleteAllROSpecs()) {
        return -2;
    }

    return 0;
}


/**
 *****************************************************************************
 **
 ** @brief  Send a SET_READER_CONFIG message that resets the
 **         reader to factory defaults.
 **
 ** NB: The ResetToFactoryDefault semantics vary between readers.
 **     It might have no effect because it is optional.
 **
 ** The message is:
 **
 **     <SET_READER_CONFIG MessageID='101'>
 **       <ResetToFactoryDefault>1</ResetToFactoryDefault>
 **     </SET_READER_CONFIG>
 **
 ** @return     ==0             Everything OK
 **             !=0             Something went wrong
 **
 *****************************************************************************/

int CReader::resetConfigurationToFactoryDefaults(void) {
    LLRP::CSET_READER_CONFIG *pCmd;
    LLRP::CMessage *pRspMsg;
    LLRP::CSET_READER_CONFIG_RESPONSE *pRsp;
    QString s;

    /*
     * Compose the command message
     */

    pCmd = new LLRP::CSET_READER_CONFIG();
    pCmd->setMessageID(messageId++);
    pCmd->setResetToFactoryDefault(1);

    /*
     * Send the message, expect the response of certain type
     */

    pRspMsg = transact(pCmd);

    /*
     * Done with the command message
     */

    delete pCmd;

    /*
     * transact() returns NULL if something went wrong.
     */

    if (NULL == pRspMsg) {
        /* transact already tattled */
        return -1;
    }

    /*
     * Cast to a SET_READER_CONFIG_RESPONSE message.
     */

    pRsp = (LLRP::CSET_READER_CONFIG_RESPONSE *) pRspMsg;

    /*
     * Check the LLRPStatus parameter.
     */

    if (0 != checkLLRPStatus(pRsp->getLLRPStatus(), "resetConfigurationToFactoryDefaults")) {
        /* checkLLRPStatus already tattled */
        delete pRspMsg;
        return -1;
    }

    /*
     * Done with the response message.
     */

    delete pRspMsg;

    /*
     * Tattle progress, maybe
     */

    emit newLogMessage(s.sprintf("INFO: Configuration reset to factory defaults"));

    /*
     * Victory.
     */

    return 0;
}



/**
 *****************************************************************************
 **
 ** @brief  Delete all ROSpecs using DELETE_ROSPEC message
 **
 ** Per the spec, the DELETE_ROSPEC message contains an ROSpecID
 ** of 0 to indicate we want all ROSpecs deleted.
 **
 ** The message is
 **
 **     <DELETE_ROSPEC MessageID='102'>
 **       <ROSpecID>0</ROSpecID>
 **     </DELETE_ROSPEC>
 **
 ** @return     ==0             Everything OK
 **             !=0             Something went wrong
 **
 *****************************************************************************/

int CReader::deleteAllROSpecs(void) {
    LLRP::CDELETE_ROSPEC *            pCmd;
    LLRP::CMessage *                  pRspMsg;
    LLRP::CDELETE_ROSPEC_RESPONSE *   pRsp;
    QString s;

    /*
     * Compose the command message
     */

    pCmd = new LLRP::CDELETE_ROSPEC();
    pCmd->setMessageID(102);
    pCmd->setROSpecID(0);               /* All */

    /*
     * Send the message, expect the response of certain type
     */

    pRspMsg = transact(pCmd);

    /*
     * Done with the command message
     */

    delete pCmd;

    /*
     * transact() returns NULL if something went wrong.
     */

    if (NULL == pRspMsg) {
        /* transact already tattled */
        return -1;
    }

    /*
     * Cast to a DELETE_ROSPEC_RESPONSE message.
     */

    pRsp = (LLRP::CDELETE_ROSPEC_RESPONSE *) pRspMsg;

    /*
     * Check the LLRPStatus parameter.
     */

    if (0 != checkLLRPStatus(pRsp->getLLRPStatus(), "deleteAllROSpecs")) {
        /* checkLLRPStatus already tattled */
        delete pRspMsg;
        return -1;
    }

    /*
     * Done with the response message.
     */

    delete pRspMsg;

    /*
     * Tattle progress, maybe
     */

    emit newLogMessage(s.sprintf("INFO: All ROSpecs are deleted"));

    /*
     * Victory.
     */

    return 0;
}



/**
 *****************************************************************************
 **
 ** @brief  Add our ROSpec using ADD_ROSPEC message
 **
 ** This ROSpec waits for a START_ROSPEC message,
 ** then takes inventory on all antennas for 5 seconds.
 **
 ** The tag report is generated after the ROSpec is done.
 **
 ** This example is deliberately streamlined.
 ** Nothing here configures the antennas, RF, or Gen2.
 ** The current defaults are used. Remember we just reset
 ** the reader to factory defaults (above). Normally an
 ** application would be more precise in configuring the
 ** reader and in its ROSpecs.
 **
 ** Experience suggests that typical ROSpecs are about
 ** double this in size.
 **
 ** The message is
 **
 **     <ADD_ROSPEC MessageID='201'>
 **       <ROSpec>
 **         <ROSpecID>123</ROSpecID>
 **         <Priority>0</Priority>
 **         <CurrentState>Disabled</CurrentState>
 **         <ROBoundarySpec>
 **           <ROSpecStartTrigger>
 **             <ROSpecStartTriggerType>Null</ROSpecStartTriggerType>
 **           </ROSpecStartTrigger>
 **           <ROSpecStopTrigger>
 **             <ROSpecStopTriggerType>Null</ROSpecStopTriggerType>
 **             <DurationTriggerValue>0</DurationTriggerValue>
 **           </ROSpecStopTrigger>
 **         </ROBoundarySpec>
 **         <AISpec>
 **           <AntennaIDs>0</AntennaIDs>
 **           <AISpecStopTrigger>
 **             <AISpecStopTriggerType>Duration</AISpecStopTriggerType>
 **             <DurationTrigger>5000</DurationTrigger>
 **           </AISpecStopTrigger>
 **           <InventoryParameterSpec>
 **             <InventoryParameterSpecID>1234</InventoryParameterSpecID>
 **             <ProtocolID>EPCGlobalClass1Gen2</ProtocolID>
 **           </InventoryParameterSpec>
 **         </AISpec>
 **         <ROReportSpec>
 **           <ROReportTrigger>Upon_N_Tags_Or_End_Of_ROSpec</ROReportTrigger>
 **           <N>0</N>
 **           <TagReportContentSelector>
 **             <EnableROSpecID>0</EnableROSpecID>
 **             <EnableSpecIndex>0</EnableSpecIndex>
 **             <EnableInventoryParameterSpecID>0</EnableInventoryParameterSpecID>
 **             <EnableAntennaID>0</EnableAntennaID>
 **             <EnableChannelIndex>0</EnableChannelIndex>
 **             <EnablePeakRSSI>0</EnablePeakRSSI>
 **             <EnableFirstSeenTimestamp>0</EnableFirstSeenTimestamp>
 **             <EnableLastSeenTimestamp>0</EnableLastSeenTimestamp>
 **             <EnableTagSeenCount>0</EnableTagSeenCount>
 **             <EnableAccessSpecID>0</EnableAccessSpecID>
 **           </TagReportContentSelector>
 **         </ROReportSpec>
 **       </ROSpec>
 **     </ADD_ROSPEC>
 **
 ** @return     ==0             Everything OK
 **             !=0             Something went wrong
 **
 *****************************************************************************/

int CReader::addROSpec(void) {
    QString s;

    LLRP::CROSpecStartTrigger *pROSpecStartTrigger = new LLRP::CROSpecStartTrigger();
    pROSpecStartTrigger->setROSpecStartTriggerType(LLRP::ROSpecStartTriggerType_Null);
//    pROSpecStartTrigger->setROSpecStartTriggerType(LLRP::ROSpecStartTriggerType_Immediate);

    LLRP::CROSpecStopTrigger *pROSpecStopTrigger = new LLRP::CROSpecStopTrigger();
    pROSpecStopTrigger->setROSpecStopTriggerType(LLRP::ROSpecStopTriggerType_Null);
//    pROSpecStopTrigger->setROSpecStopTriggerType(LLRP::ROSpecStopTriggerType_Duration);
    pROSpecStopTrigger->setDurationTriggerValue(0);     /* n/a */

    LLRP::CROBoundarySpec *pROBoundarySpec = new LLRP::CROBoundarySpec();
    pROBoundarySpec->setROSpecStartTrigger(pROSpecStartTrigger);
    pROBoundarySpec->setROSpecStopTrigger(pROSpecStopTrigger);


    LLRP::CAISpecStopTrigger *pAISpecStopTrigger = new LLRP::CAISpecStopTrigger();
//    pAISpecStopTrigger->setAISpecStopTriggerType(LLRP::AISpecStopTriggerType_Duration);
    pAISpecStopTrigger->setAISpecStopTriggerType(LLRP::AISpecStopTriggerType_Null);
//    pAISpecStopTrigger->setDurationTrigger(1000);//1000

    LLRP::CInventoryParameterSpec *pInventoryParameterSpec = new LLRP::CInventoryParameterSpec();
    pInventoryParameterSpec->setInventoryParameterSpecID(1234);
    pInventoryParameterSpec->setProtocolID(LLRP::AirProtocols_EPCGlobalClass1Gen2);

    LLRP::llrp_u16v_t AntennaIDs = LLRP::llrp_u16v_t(1);
    AntennaIDs.m_pValue[0] = 0;         /* All */

    LLRP::CAISpec *pAISpec = new LLRP::CAISpec();
    pAISpec->setAntennaIDs(AntennaIDs);
    pAISpec->setAISpecStopTrigger(pAISpecStopTrigger);
    pAISpec->addInventoryParameterSpec(pInventoryParameterSpec);

    LLRP::CTagReportContentSelector *pTagReportContentSelector = new LLRP::CTagReportContentSelector();
    pTagReportContentSelector->setEnableROSpecID(FALSE);
    pTagReportContentSelector->setEnableSpecIndex(FALSE);
    pTagReportContentSelector->setEnableInventoryParameterSpecID(FALSE);
    pTagReportContentSelector->setEnableAntennaID(TRUE);
    pTagReportContentSelector->setEnableChannelIndex(FALSE);
    pTagReportContentSelector->setEnablePeakRSSI(FALSE);
    pTagReportContentSelector->setEnableFirstSeenTimestamp(TRUE);
    pTagReportContentSelector->setEnableLastSeenTimestamp(TRUE);
    pTagReportContentSelector->setEnableTagSeenCount(TRUE);
    pTagReportContentSelector->setEnableAccessSpecID(TRUE);

    LLRP::CROReportSpec *pROReportSpec = new LLRP::CROReportSpec();
    //pROReportSpec->setROReportTrigger(LLRP::ROReportTriggerType_None);
    pROReportSpec->setROReportTrigger(LLRP::ROReportTriggerType_Upon_N_Tags_Or_End_Of_ROSpec);
    pROReportSpec->setN(1);         /* Unlimited */
    pROReportSpec->setTagReportContentSelector(pTagReportContentSelector);

    LLRP::CROSpec *pROSpec = new LLRP::CROSpec();
    pROSpec->setROSpecID(123);
    pROSpec->setPriority(0);
    pROSpec->setCurrentState(LLRP::ROSpecState_Disabled);
    pROSpec->setROBoundarySpec(pROBoundarySpec);
    pROSpec->addSpecParameter(pAISpec);
    pROSpec->setROReportSpec(pROReportSpec);

    LLRP::CADD_ROSPEC *pCmd;
    LLRP::CMessage *pRspMsg;
    LLRP::CADD_ROSPEC_RESPONSE *pRsp;

    /*
     * Compose the command message.
     * N.B.: After the message is composed, all the parameters
     *       constructed, immediately above, are considered "owned"
     *       by the command message. When it is destructed so
     *       too will the parameters be.
     */

    pCmd = new LLRP::CADD_ROSPEC();
    pCmd->setMessageID(201);
    pCmd->setROSpec(pROSpec);

    /*
     * Send the message, expect the response of certain type
     */

    pRspMsg = transact(pCmd);

    /*
     * Done with the command message.
     * N.B.: And the parameters
     */

    delete pCmd;

    /*
     * transact() returns NULL if something went wrong.
     */

    if (NULL == pRspMsg) {
        /* transact already tattled */
        return -1;
    }

    /*
     * Cast to a ADD_ROSPEC_RESPONSE message.
     */

    pRsp = (LLRP::CADD_ROSPEC_RESPONSE *) pRspMsg;

    /*
     * Check the LLRPStatus parameter.
     */

    if (0 != checkLLRPStatus(pRsp->getLLRPStatus(), "addROSpec")) {
        /* checkLLRPStatus already tattled */
        delete pRspMsg;
        return -1;
    }

    /*
     * Done with the response message.
     */

    delete pRspMsg;

    /*
     * Tattle progress, maybe
     */

    emit newLogMessage(s.sprintf("INFO: ROSpec added"));

    /*
     * Victory.
     */

    return 0;
}


/**
 *****************************************************************************
 **
 ** @brief  Enable our ROSpec using ENABLE_ROSPEC message
 **
 ** Enable the ROSpec that was added above.
 **
 ** The message we send is:
 **     <ENABLE_ROSPEC MessageID='202'>
 **       <ROSpecID>123</ROSpecID>
 **     </ENABLE_ROSPEC>
 **
 ** @return     ==0             Everything OK
 **             !=0             Something went wrong
 **
 *****************************************************************************/

int CReader::enableROSpec (void) {
    LLRP::CENABLE_ROSPEC *            pCmd;
    LLRP::CMessage *                  pRspMsg;
    LLRP::CENABLE_ROSPEC_RESPONSE *   pRsp;
    QString s;

    /*
     * Compose the command message
     */

    pCmd = new LLRP::CENABLE_ROSPEC();
    pCmd->setMessageID(202);
    pCmd->setROSpecID(123);

    /*
     * Send the message, expect the response of certain type
     */

    pRspMsg = transact(pCmd);

    /*
     * Done with the command message
     */

    delete pCmd;

    /*
     * transact() returns NULL if something went wrong.
     */

    if(NULL == pRspMsg) {
        /* transact already tattled */
        return -1;
    }

    /*
     * Cast to a ENABLE_ROSPEC_RESPONSE message.
     */

    pRsp = (LLRP::CENABLE_ROSPEC_RESPONSE *) pRspMsg;

    /*
     * Check the LLRPStatus parameter.
     */

    if(0 != checkLLRPStatus(pRsp->getLLRPStatus(), "enableROSpec")) {
        /* checkLLRPStatus already tattled */
        delete pRspMsg;
        return -1;
    }

    /*
     * Done with the response message.
     */

    delete pRspMsg;

    /*
     * Tattle progress, maybe
     */

    emit newLogMessage(s.sprintf("INFO: ROSpec enabled"));

    /*
     * Victory.
     */

    return 0;
}


/**
 *****************************************************************************
 **
 ** @brief  Start our ROSpec using START_ROSPEC message
 **
 ** Start the ROSpec that was added above.
 **
 ** The message we send is:
 **     <START_ROSPEC MessageID='203'>
 **       <ROSpecID>123</ROSpecID>
 **     </START_ROSPEC>
 **
 ** @return     ==0             Everything OK
 **             !=0             Something went wrong
 **
 *****************************************************************************/

int CReader::startROSpec (void) {
    LLRP::CSTART_ROSPEC *pCmd;
    LLRP::CMessage *pRspMsg;
    LLRP::CSTART_ROSPEC_RESPONSE *pRsp;
    QString s;

    /*
     * Compose the command message
     */

    pCmd = new LLRP::CSTART_ROSPEC();
    pCmd->setMessageID(202);
    pCmd->setROSpecID(123);

    /*
     * Send the message, expect the response of certain type
     */

    pRspMsg = transact(pCmd);

    /*
     * Done with the command message
     */

    delete pCmd;

    /*
     * transact() returns NULL if something went wrong.
     */

    if (NULL == pRspMsg) {
        /* transact already tattled */
        return -1;
    }

    /*
     * Cast to a START_ROSPEC_RESPONSE message.
     */

    pRsp = (LLRP::CSTART_ROSPEC_RESPONSE *)pRspMsg;

    /*
     * Check the LLRPStatus parameter.
     */

    if (0 != checkLLRPStatus(pRsp->getLLRPStatus(), "startROSpec")) {
        /* checkLLRPStatus already tattled */
        delete pRspMsg;
        return -1;
    }

    /*
     * Done with the response message.
     */

    delete pRspMsg;

    /*
     * Tattle progress
     */

    emit newLogMessage(s.sprintf("INFO: ROSpec started"));

    /*
     * Victory.
     */

    return 0;
}




/**
 *****************************************************************************
 **
 ** @brief  Receive the RO_ACCESS_REPORT
 **
 ** Receive messages until an RO_ACCESS_REPORT is received.
 ** Time limit is 7 seconds. We expect a report within 5 seconds.
 **
 ** This shows how to determine the type of a received message.
 **
 ** @return     ==0             Everything OK
 **             !=0             Something went wrong
 **
 *****************************************************************************/

int CReader::processReports(void) {
    QString s;
    LLRP::CMessage *pMessage = NULL;
    const LLRP::CTypeDescriptor *pType = NULL;

    // Wait for a message. The report should occur within 1 seconds when tags are in antenna zone.
    // If not, timeout and clear currentTagsList.  Don't make the timeout too short, because
    // if the reader report is delayed for any reason (

//    qDebug() << "processReports";

    pMessage = recvMessage(5000);
    if (!pMessage) {
        processTagList(NULL);
        return 0;
    }

    // What happens depends on what kind of message
    // received. Use the type label (m_pType) to
    // discriminate message types.

    pType = pMessage->m_pType;

    // Is it a tag report? If so, process.

    if (&LLRP::CRO_ACCESS_REPORT::s_typeDescriptor == pType) {
        LLRP::CRO_ACCESS_REPORT *pNtf;
        pNtf = (LLRP::CRO_ACCESS_REPORT *)pMessage;
        processTagList(pNtf);
    }

    // Is it a reader event? This example only recognizes
    // AntennaEvents.

    else if (&LLRP::CREADER_EVENT_NOTIFICATION::s_typeDescriptor == pType) {
        LLRP::CREADER_EVENT_NOTIFICATION *pNtf;
        LLRP::CReaderEventNotificationData *pNtfData;

        pNtf = (LLRP::CREADER_EVENT_NOTIFICATION *)pMessage;

        pNtfData = pNtf->getReaderEventNotificationData();
        if (NULL != pNtfData) {
            handleReaderEventNotification(pNtfData);
        }
        else {
            emit newLogMessage(s.sprintf("WARNING: READER_EVENT_NOTIFICATION without data"));
        }
    }

    // Hmmm. Something unexpected. Just tattle and keep going.

    else {
        emit newLogMessage(s.sprintf("WARNING: Ignored unexpected message during monitor: %s", pType->m_pName));
    }

    delete pMessage;
    return 0;
}






/**
 *****************************************************************************
 **
 ** @brief  Helper routine to
 **
 ** The report is printed in list order, which is arbitrary.
 **
 ** TODO: It would be cool to sort the list by EPC and antenna,
 **       then print it.
 **
 ** @return     void
 **
 *****************************************************************************/

void CReader::processTagList (LLRP::CRO_ACCESS_REPORT *pRO_ACCESS_REPORT) {
    std::list<LLRP::CTagReportData *>::iterator Cur;
    static int count = 0;

    // Get current local time in application msec

    unsigned long long currentUSecSinceEpoch = QDateTime::currentMSecsSinceEpoch() * 1000;
    QList<CTagInfo> newTagsList;

    if (pRO_ACCESS_REPORT) for (Cur = pRO_ACCESS_REPORT->beginTagReportData(); Cur != pRO_ACCESS_REPORT->endTagReportData(); Cur++) {
        LLRP::CTagReportData *pTagReportData = *Cur;
        const LLRP::CTypeDescriptor *pType;
        LLRP::CParameter *pEPCParameter = pTagReportData->getEPCParameter();
        CTagInfo tagInfo;

        /*
         * Process the EPC. It could be a 96-bit EPC_96 parameter
         * or a variable length EPCData parameter.
         */

        if (NULL != pEPCParameter) {
            LLRP::llrp_u96_t my_u96;
            LLRP::llrp_u1v_t my_u1v;
            LLRP::llrp_u8_t *pValue = NULL;
            int n;

            pType = pEPCParameter->m_pType;
            if (&LLRP::CEPC_96::s_typeDescriptor == pType) {
                LLRP::CEPC_96             *pEPC_96;

                pEPC_96 = (LLRP::CEPC_96 *) pEPCParameter;
                my_u96 = pEPC_96->getEPC();
                pValue = my_u96.m_aValue;
                n = 12u;
            }
            else if (&LLRP::CEPCData::s_typeDescriptor == pType) {
                LLRP::CEPCData *pEPCData;

                pEPCData = (LLRP::CEPCData *)pEPCParameter;
                my_u1v = pEPCData->getEPC();
                pValue = my_u1v.m_pValue;
                n = (my_u1v.m_nBit + 7u) / 8u;
            }

            if (pValue) {
                tagInfo.readerId = readerId;
                tagInfo.antennaId = pTagReportData->getAntennaID()->getAntennaID();
                tagInfo.timeStampUSec = (unsigned long long)pTagReportData->getFirstSeenTimestampUTC()->getMicroseconds();
                for (int i=0; i<n; i++) {
                    QString s;
                    tagInfo.tagId.append(s.sprintf("%02x", pValue[i]));
                }

                // If this is the first tag from the reader, compare timeStampUSec with msecFromEmpoch and determine an offset
                // correction so that timeStamps from different readers will be synchronized (to 1 msec)

                if (waitingForFirstTag) {
                    timeStampCorrectionUSec = currentUSecSinceEpoch - tagInfo.timeStampUSec;
                    waitingForFirstTag = false;
                }
                tagInfo.timeStampUSec += timeStampCorrectionUSec;

                tagInfo.firstSeenInApplicationUSec = currentUSecSinceEpoch;


                // Add to newTagsList

                newTagsList.append(tagInfo);
            }
            else {
                emit newLogMessage(QString("Unknown-epc-data-type in tag"));
            }
        }
        else {
            emit newLogMessage(QString("Missing-epc-data in tag"));
        }
    }

    // newTagsList is a list of tags currently in antenna zone

    count++;
    printf("%d: Processing %d tags\n", count, newTagsList.size());
    fflush(stdout);

    // Loop through current tag list and remove any current tag that is not in new list

    if (currentTagsList.size() > 0) for (int i=currentTagsList.size()-1; i>=0; i--) {
        bool inNewList = false;
        for (int j=0; j<newTagsList.size(); j++) {
            if (currentTagsList[i].tagId == newTagsList[j].tagId) {
                inNewList = true;
                break;
            }
        }
        if (!inNewList) {
            currentTagsList.removeAt(i);
        }
    }


    // Loop through new tag list.
    // If tag is in current list, rider is sitting in antenna zone, so emit newTag signal only if desk reader.
    // If tag is not in list, rider has just arrived in antenna zone, so add to list and emit newTag signal
    // for both desk and track readers.

    for (int i=0; i<newTagsList.size(); i++) {
        bool inList = false;
        for (int j=0; j<currentTagsList.size(); j++) {
            if (newTagsList[i].tagId == currentTagsList[j].tagId) {
                inList = true;
                break;
            }
        }
        if (inList) {
            if (antennaPosition == desk)
                emit newTag(newTagsList[i]);
        }
        else {
            currentTagsList.append(newTagsList[i]);
            emit newTag(newTagsList[i]);
        }
        //qDebug("  %s", newTagsList[i].tagId.data());
    }
}







/**
 *****************************************************************************
 **
 ** @brief  Handle a ReaderEventNotification
 **
 ** Handle the payload of a READER_EVENT_NOTIFICATION message.
 ** This routine simply dispatches to handlers of specific
 ** event types.
 **
 ** @return     void
 **
 *****************************************************************************/

void CReader::handleReaderEventNotification (LLRP::CReaderEventNotificationData *pNtfData) {
    LLRP::CAntennaEvent *pAntennaEvent;
    LLRP::CReaderExceptionEvent *pReaderExceptionEvent;
    int nReported = 0;
    QString s;

    pAntennaEvent = pNtfData->getAntennaEvent();
    if (NULL != pAntennaEvent) {
        handleAntennaEvent(pAntennaEvent);
        nReported++;
    }

    pReaderExceptionEvent = pNtfData->getReaderExceptionEvent();
    if (NULL != pReaderExceptionEvent) {
        handleReaderExceptionEvent(pReaderExceptionEvent);
        nReported++;
    }

    /*
     * Similarly handle other events here:
     *      HoppingEvent
     *      GPIEvent
     *      ROSpecEvent
     *      ReportBufferLevelWarningEvent
     *      ReportBufferOverflowErrorEvent
     *      RFSurveyEvent
     *      AISpecEvent
     *      ConnectionAttemptEvent
     *      ConnectionCloseEvent
     *      Custom
     */

    if (0 == nReported) {
        emit newLogMessage(s.sprintf("NOTICE: Unexpected (unhandled) ReaderEvent"));
    }
}



/**
 *****************************************************************************
 **
 ** @brief  Handle an AntennaEvent
 **
 ** An antenna was disconnected or (re)connected. Tattle.
 **
 ** @return     void
 **
 *****************************************************************************/

void CReader::handleAntennaEvent (LLRP::CAntennaEvent *pAntennaEvent) {
    LLRP::EAntennaEventType eEventType;
    LLRP::llrp_u16_t AntennaID;
    char *pStateStr;
    QString s;

    eEventType = pAntennaEvent->getEventType();
    AntennaID = pAntennaEvent->getAntennaID();

    switch(eEventType)
    {
    case LLRP::AntennaEventType_Antenna_Disconnected:
        pStateStr = "disconnected";
        break;

    case LLRP::AntennaEventType_Antenna_Connected:
        pStateStr = "connected";
        break;

    default:
        pStateStr = "?unknown-event?";
        break;
    }

    emit newLogMessage(s.sprintf("NOTICE: Reader %d antenna %d is %s", readerId, AntennaID, pStateStr));
}



/**
 *****************************************************************************
 **
 ** @brief  Handle a ReaderExceptionEvent
 **
 ** Something has gone wrong. There are lots of details but
 ** all this does is print the message, if one.
 **
 ** @return     void
 **
 *****************************************************************************/

void CReader::handleReaderExceptionEvent (LLRP::CReaderExceptionEvent *pReaderExceptionEvent) {
    LLRP::llrp_utf8v_t                Message;
    QString s;

    Message = pReaderExceptionEvent->getMessage();

    if (0 < Message.m_nValue && NULL != Message.m_pValue) {
        emit newLogMessage(s.sprintf("NOTICE: ReaderException '%.*s'", Message.m_nValue, Message.m_pValue));
    }
    else {
        emit newLogMessage(s.sprintf("NOTICE: ReaderException but no message"));
    }
}



/**
 *****************************************************************************
 **
 ** @brief  Helper routine to check an LLRPStatus parameter
 **         and tattle on errors
 **
 ** Helper routine to interpret the LLRPStatus subparameter
 ** that is in all responses. It tattles on an error, if one,
 ** and tries to safely provide details.
 **
 ** This simplifies the code, above, for common check/tattle
 ** sequences.
 **
 ** @return     ==0             Everything OK
 **             !=0             Something went wrong, already tattled
 **
 *****************************************************************************/

int CReader::checkLLRPStatus (LLRP::CLLRPStatus *pLLRPStatus, char *pWhatStr) {
    QString s;

    /*
     * The LLRPStatus parameter is mandatory in all responses.
     * If it is missing there should have been a decode error.
     * This just makes sure (remember, this program is a
     * diagnostic and suppose to catch LTKC mistakes).
     */

    if (NULL == pLLRPStatus) {
        emit newLogMessage(s.sprintf("ERROR: %s missing LLRP status", pWhatStr));
        return -1;
    }

    /*
     * Make sure the status is M_Success.
     * If it isn't, print the error string if one.
     * This does not try to pretty-print the status
     * code. To get that, run this program with -vv
     * and examine the XML output.
     */

    if (LLRP::StatusCode_M_Success != pLLRPStatus->getStatusCode()) {
        LLRP::llrp_utf8v_t ErrorDesc;

        ErrorDesc = pLLRPStatus->getErrorDescription();

        if (0 == ErrorDesc.m_nValue) {
            emit newLogMessage(s.sprintf("ERROR: %s failed, no error description given", pWhatStr));
        }
        else {
            emit newLogMessage(s.sprintf("ERROR: %s failed, %.*s", pWhatStr, ErrorDesc.m_nValue, ErrorDesc.m_pValue));
        }
        return -2;
    }

    /*
     * Victory. Everything is fine.
     */

    return 0;
}



/**
 *****************************************************************************
 **
 ** @brief  Wrapper routine to do an LLRP transaction
 **
 ** Wrapper to transact a request/resposne.
 **     - Print the outbound message in XML if verbose level is at least 2
 **     - Send it using the LLRP_Conn_transact()
 **     - LLRP_Conn_transact() receives the response or recognizes an error
 **     - Tattle on errors, if any
 **     - Print the received message in XML if verbose level is at least 2
 **     - If the response is ERROR_MESSAGE, the request was sufficiently
 **       misunderstood that the reader could not send a proper reply.
 **       Deem this an error, free the message.
 **
 ** The message returned resides in allocated memory. It is the
 ** caller's obligtation to free it.
 **
 ** @return     ==NULL          Something went wrong, already tattled
 **             !=NULL          Pointer to a message
 **
 *****************************************************************************/

LLRP::CMessage *CReader::transact(LLRP::CMessage *pSendMsg) {
    LLRP::CMessage *pRspMsg;
    QString s;

    /*
     * Print the XML text for the outbound message if
     * verbosity is 2 or higher.
     */

//    if (1 < verbose) {
//        emit newLogMessage(s.sprintf("\n==================================="));
//        emit newLogMessage(s.sprintf("INFO: Transact sending"));
//        printXMLMessage(pSendMsg);
//    }

    /*
     * Send the message, expect the response of certain type.
     * If LLRP::CConnection::transact() returns NULL then there was
     * an error. In that case we try to print the error details.
     */

    pRspMsg = connectionToReader->transact(pSendMsg, 3000);

    if (NULL == pRspMsg) {
        const LLRP::CErrorDetails *   pError = connectionToReader->getTransactError();

        emit newLogMessage(s.sprintf("ERROR: %s transact failed, %s", pSendMsg->m_pType->m_pName, pError->m_pWhatStr ? pError->m_pWhatStr : "no reason given"));

        if (NULL != pError->m_pRefType) {
            emit newLogMessage(s.sprintf("ERROR: ... reference type %s", pError->m_pRefType->m_pName));
        }

        if (NULL != pError->m_pRefField) {
            emit newLogMessage(s.sprintf("ERROR: ... reference field %s", pError->m_pRefField->m_pName));
        }

        return NULL;
    }

    /*
     * Print the XML text for the inbound message if
     * verbosity is 2 or higher.
     */

//    if (1 < verbose) {
//        emit newLogMessage(s.sprintf("\n- - - - - - - - - - - - - - - - - -"));
//        emit newLogMessage(s.sprintf("INFO: Transact received response"));
//        printXMLMessage(pRspMsg);
//    }

    /*
     * If it is an ERROR_MESSAGE (response from reader
     * when it can't understand the request), tattle
     * and declare defeat.
     */

    if (&LLRP::CERROR_MESSAGE::s_typeDescriptor == pRspMsg->m_pType) {
        const LLRP::CTypeDescriptor * pResponseType;

        pResponseType = pSendMsg->m_pType->m_pResponseType;

        emit newLogMessage(s.sprintf("ERROR: Received ERROR_MESSAGE instead of %s", pResponseType->m_pName));
        delete pRspMsg;
        pRspMsg = NULL;
    }

    return pRspMsg;
}



/**
 *****************************************************************************
 **
 ** @brief  Wrapper routine to receive a message
 **
 ** This can receive notifications as well as responses.
 **     - Recv a message using the LLRP_Conn_recvMessage()
 **     - Tattle on errors, if any
 **     - Print the message in XML if verbose level is at least 2
 **
 ** The message returned resides in allocated memory. It is the
 ** caller's obligtation to free it.
 **
 ** @param[in]  nMaxMS          -1 => block indefinitely
 **                              0 => just peek at input queue and
 **                                   socket queue, return immediately
 **                                   no matter what
 **                             >0 => ms to await complete frame
 **
 ** @return     ==NULL          Something went wrong, already tattled
 **             !=NULL          Pointer to a message
 **
 *****************************************************************************/

LLRP::CMessage *CReader::recvMessage(int nMaxMS) {
    LLRP::CMessage *pMessage;
    QString s;

    /*
     * Receive the message subject to a time limit
     */

    pMessage = connectionToReader->recvMessage(nMaxMS);

    /*
     * If LLRP::CConnection::recvMessage() returns NULL then there was
     * an error. In that case we try to print the error details.
     */

    if (NULL == pMessage) {
        const LLRP::CErrorDetails *   pError = connectionToReader->getRecvError();

        //emit newLogMessage(s.sprintf("ERROR: recvMessage failed, %s", pError->m_pWhatStr ? pError->m_pWhatStr : "no reason given"));

        if (NULL != pError->m_pRefType) {
            emit newLogMessage(s.sprintf("ERROR: ... reference type %s", pError->m_pRefType->m_pName));
        }

        if (NULL != pError->m_pRefField) {
            emit newLogMessage(s.sprintf("ERROR: ... reference field %s", pError->m_pRefField->m_pName));
        }

        return NULL;
    }

    /*
     * Print the XML text for the inbound message if
     * verbosity is 2 or higher.
     */

//    if (1 < verbose) {
//        emit newLogMessage(s.sprintf("\n==================================="));
//        emit newLogMessage(s.sprintf("INFO: Message received"));
//        printXMLMessage(pMessage);
//    }

    return pMessage;
}



/**
 *****************************************************************************
 **
 ** @brief  Wrapper routine to send a message
 **
 ** Wrapper to send a message.
 **     - Print the message in XML if verbose level is at least 2
 **     - Send it using the LLRP_Conn_sendMessage()
 **     - Tattle on errors, if any
 **
 ** @param[in]  pSendMsg        Pointer to message to send
 **
 ** @return     ==0             Everything OK
 **             !=0             Something went wrong, already tattled
 **
 *****************************************************************************/

int CReader::sendMessage (LLRP::CMessage *pSendMsg) {
    QString s;

    /*
     * Print the XML text for the outbound message if
     * verbosity is 2 or higher.
     */

//    if (1 < verbose) {
//        emit newLogMessage(s.sprintf("\n==================================="));
//        emit newLogMessage(s.sprintf("INFO: Sending"));
//        printXMLMessage(pSendMsg);
//    }

    /*
     * If LLRP::CConnection::sendMessage() returns other than RC_OK
     * then there was an error. In that case we try to print
     * the error details.
     */

    if (LLRP::RC_OK != connectionToReader->sendMessage(pSendMsg)) {
        const LLRP::CErrorDetails *   pError = connectionToReader->getSendError();

        emit newLogMessage(s.sprintf("ERROR: %s sendMessage failed, %s", pSendMsg->m_pType->m_pName, pError->m_pWhatStr ? pError->m_pWhatStr : "no reason given"));

        if (NULL != pError->m_pRefType) {
            emit newLogMessage(s.sprintf("ERROR: ... reference type %s", pError->m_pRefType->m_pName));
        }

        if (NULL != pError->m_pRefField) {
            emit newLogMessage(s.sprintf("ERROR: ... reference field %s", pError->m_pRefField->m_pName));
        }

        return -1;
    }

    /*
     * Victory
     */

    return 0;
}



/**
 *****************************************************************************
 **
 ** @brief  Helper to print a message as XML text
 **
 ** Print a LLRP message as XML text
 **
 ** @param[in]  pMessage        Pointer to message to print
 **
 ** @return     void
 **
 *****************************************************************************/

void CReader::printXMLMessage (LLRP::CMessage *pMessage) {
    char aBuf[100*1024];
    QString s;

    /*
     * Convert the message to an XML string.
     * This fills the buffer with either the XML string
     * or an error message. The return value could
     * be checked.
     */

    pMessage->toXMLString(aBuf, sizeof aBuf);

    /*
     * Print the XML Text to the standard output.
     */

    emit newLogMessage(s.sprintf("%s", aBuf));
}





// getTransmitPowerCapabilities()
// Populate transmitPowerList member
//
int CReader::getTransmitPowerCapabilities(void) {
    LLRP::CGET_READER_CAPABILITIES *pCmd;
    LLRP::CMessage *pRspMsg;
    LLRP::CGET_READER_CAPABILITIES_RESPONSE *pRsp;
    LLRP::CRegulatoryCapabilities *pReg;
    LLRP::CUHFBandCapabilities *pUhf;
    LLRP::CTransmitPowerLevelTableEntry *pPwrLvl;
    LLRP::CGeneralDeviceCapabilities *pDeviceCap;
    std::list<LLRP::CTransmitPowerLevelTableEntry *>::iterator PwrLvl;

    transmitPowerList.clear();

    /*
    * Compose the command message
    */
    pCmd = new LLRP::CGET_READER_CAPABILITIES();
    pCmd->setMessageID(messageId++);//m_messageID++);
    pCmd->setRequestedData(LLRP::GetReaderCapabilitiesRequestedData_All);
    /*
    * Send the message, expect a certain type of response
    */
    pRspMsg = transact(pCmd);
    /*
    * Done with the command message
    */
    delete pCmd;
    /*
    * transact() returns NULL if something went wrong.
    */
    if(NULL == pRspMsg) {
        /* transact already tattled */
        return 1;
    }
    /*
    * Cast to a CGET_READER_CAPABILITIES_RESPONSE message.
    */
    pRsp = (LLRP::CGET_READER_CAPABILITIES_RESPONSE *)pRspMsg;
    /*
    * Check the LLRPStatus parameter.
    */
    if(0 != checkLLRPStatus(pRsp->getLLRPStatus(),"getReaderCapabilities")) {
        /* checkLLRPStatus already tattled */
        delete pRspMsg;
        return 2;
    }
    /*
    ** Get out the Regulatory Capabilities element
    */
    if(NULL == (pReg = pRsp->getRegulatoryCapabilities())) {
        delete pRspMsg;
        return 3;
    }
    /*
    ** Get out the UHF Band Capabilities element
    */
    if(NULL == (pUhf = pReg->getUHFBandCapabilities())) {
        delete pRspMsg;
        return 4;
    }

    // Populate member QList<int> transmitPowerList with available power values

    for (PwrLvl = pUhf->beginTransmitPowerLevelTableEntry(); PwrLvl != pUhf->endTransmitPowerLevelTableEntry(); PwrLvl++) {
        pPwrLvl = *PwrLvl;
        transmitPowerList.append(pPwrLvl->getTransmitPowerValue());
    }

    /* get the last power level in the table */
    PwrLvl = pUhf->endTransmitPowerLevelTableEntry();
    PwrLvl--;
    // store the index for use int the ROSpec.
    pPwrLvl = *PwrLvl;

//    int powerLevelIndex = pPwrLvl->getIndex();
//    if(1 < verbose) {
//        printf("INFO: Reader Max Power index %u, power %d\n", pPwrLvl->getIndex(), pPwrLvl->getTransmitPowerValue());
//    }

    /* if this parameter is missing, or if this is not an Impinj
    ** reader, we can’t determine its capabilities so we exit
    ** Impinj Private Enterprise NUmber is 25882 */
    if( (NULL == (pDeviceCap = pRsp->getGeneralDeviceCapabilities())) || (25882 != pDeviceCap->getDeviceManufacturerName())) {
        delete pRspMsg;
        return 5;
    }
//    int modelNumber = pDeviceCap->getModelName();
//    if(1 < verbose) {
//        printf("INFO: Reader Model Name %u\n", modelNumber);
//    }
    /*
    * Done with the response message.
    */
    delete pRspMsg;
    /*
    * Tattle progress, maybe
    */
//    if(verbose) {
//        printf("INFO:Found LLRP Capabilities\n");
//    }
    /*
    * Victory.
    */
    return 0;
}





// Use xml file to set default reader configuration and then make some mods
//
int CReader::setReaderConfiguration(void) {
//    LLRP::CGET_READER_CONFIG *pGetReaderCmd;
    LLRP::CMessage *pRspMsg;
//    LLRP::CGET_READER_CONFIG_RESPONSE *pGetReaderRsp;
//    std::list<LLRP::CAntennaConfiguration*>::iterator pAntCfg;

/*
    // Compose the command message

    pGetReaderCmd = new LLRP::CGET_READER_CONFIG();
    pGetReaderCmd->setMessageID(messageId++);
    pGetReaderCmd->setRequestedData(LLRP::GetReaderConfigRequestedData_All);

    // Send the message, expect a certain type of response

    pRspMsg = transact(pGetReaderCmd);

    // Done with the command message

    delete pGetReaderCmd;
    pGetReaderCmd = NULL;

    // transact() returns NULL if something went wrong.

    if(NULL == pRspMsg) {
        // transact already tattled
        return -1;
    }

    // Cast to a CGET_READER_CONFIG_RESPONSE message.

    pGetReaderRsp = (LLRP::CGET_READER_CONFIG_RESPONSE *) pRspMsg;

    // Check the LLRPStatus parameter.

    if (0 != checkLLRPStatus(pGetReaderRsp->getLLRPStatus(), "getReaderConfig")) {
        // checkLLRPStatus already tattled
        delete pRspMsg;
        return -1;
    }

    // just get the hop table and channel index out of
    // the first antenna configuration since they must all
    // be the same

    pAntCfg = pGetReaderRsp->beginAntennaConfiguration();
    unsigned hopTableID = 0;
    unsigned channelIndex = 0;
    if(pAntCfg != pGetReaderRsp->endAntennaConfiguration()) {
        LLRP::CRFTransmitter *prfTx;
        prfTx = (*pAntCfg)->getRFTransmitter();
        hopTableID = prfTx->getHopTableID();
        channelIndex = prfTx->getChannelIndex();
    }
    else
    {
        delete pRspMsg;
        return -1;
    }

    // Done with the response message.

    delete pRspMsg;

//    if(1 < verbose)
//    {
//                printf("INFO: Reader hopTableID %u, ChannelIndex %u\n", hopTableID, channelIndex);
//    }

*/


    LLRP::CMessage *pCmdMsg;
    LLRP::CSET_READER_CONFIG *pSetReaderCmd;
//    LLRP::CMessage *pRspMsg;
    LLRP::CSET_READER_CONFIG_RESPONSE *pSetReaderRsp;
    LLRP::CXMLTextDecoder *pDecoder;
    std::list<LLRP::CAntennaConfiguration *>::iterator Cur;

    // Build a decoder to extract the message from XML

    pDecoder = new LLRP::CXMLTextDecoder(typeRegistry, "../fcvtc/readerConfig.xml");
    if (NULL == pDecoder) {
        return -1;
    }
    pCmdMsg = pDecoder->decodeMessage();
    delete pDecoder;
    if (NULL == pCmdMsg) {
        return -2;
    }

    if (&LLRP::CSET_READER_CONFIG::s_typeDescriptor != pCmdMsg->m_pType) {
        return -3;
    }

    // get the message as a SET_READER_CONFIG

    pSetReaderCmd = (LLRP::CSET_READER_CONFIG *) pCmdMsg;

    // It’s always a good idea to give it a unique message ID

    pSetReaderCmd->setMessageID(messageId++);

    // at this point,we would be ready to send the message, but we need
    // to make a change to the transmit power for each enabled antenna.

//    Cur = pSetReaderCmd->beginAntennaConfiguration();
//    Cur++;

//    LLRP::CRFTransmitter *pRfTx = (*Cur)->getRFTransmitter();

    // we already have this element in our sample XML file, but
    // we check here to create one if it doesn’t exist to show
    // a more general usage

//    if(NULL == pRfTx) {
//        pRfTx = new LLRP::CRFTransmitter();
//        (*Cur)->setRFTransmitter(pRfTx);
//    }

    // Set the max power that we retreived from the capabilities
    // and the hopTableID and Channel index we got from the config

//    printf("Number of power settings: %d\n", transmitPowerList.size());
//    fflush(stdout);
//    unsigned powerLevelIndex = transmitPowerIndex;         // 1 = lowest power value
////    if (powerLevelIndex < 1) powerLevelIndex = 1;
////    if (powerLevelIndex > transmitPowerList.size())
//        powerLevelIndex = transmitPowerList.size();
//    pRfTx->setChannelIndex(channelIndex);
//    pRfTx->setHopTableID(hopTableID);
//    pRfTx->setTransmitPower(powerLevelIndex);

    // Send the message, expect a certain

    pRspMsg = transact(pSetReaderCmd);

    delete pSetReaderCmd;
    pSetReaderCmd = NULL;

    // transact() returns NULL if something went wrong.

    if (NULL == pRspMsg) {
        /* transact already tattled */
        return -1;
    }

    // Cast to a CSET_READER_CONFIG_RESPONSE message.

    pSetReaderRsp = (LLRP::CSET_READER_CONFIG_RESPONSE *)pRspMsg;

    // Check the LLRPStatus parameter.

    if (0 != checkLLRPStatus(pSetReaderRsp->getLLRPStatus(), "setImpinjReaderConfig")) {
        /* checkLLRPStatus already tattled */
        delete pRspMsg;
        return -1;
    }

    // Done with the response message.

    delete pRspMsg;

//    if (verbose)
//    {
//        printf("INFO: Set Impinj Reader Configuration \n");
//        fflush(stdout);
//    }


    return 0;
}



QList<int> *CReader::getTransmitPowerList(void) {
    return &transmitPowerList;
}
