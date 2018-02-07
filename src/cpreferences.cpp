// cpreferences.cpp


#include <QWidget>
#include <QSettings>

#include "cpreferences.h"
#include "ui_cpreferences.h"
#include "mainwindow.h"


extern MainWindow *mainWindow;
extern CMessages *messagesWindow;
extern CPreferences *preferencesWindow;




CPreferences::CPreferences(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CPreferences)
{
    QString s;
    ui->setupUi(this);
    changesMade = false;

    setWindowTitle(QCoreApplication::applicationName() + " Preferences");


    // Configure widgets and load with initial default values

    // tablePurgeInterval is the interval (hours) on which tables are purged of inactive riders.

    tablePurgeIntervalHours = 3.;
    ui->tablePurgeIntervalDoubleSpinBox->setMinimum(0.0);

    // emailReportLatency is the interval (hours) after a rider being purged before an email report is sent.

    emailReportLatencyHours = 3.;
    //ui->emailReportLatencySpinBox->setMinimum(2);


    ui->trackReaderStatusLabel->clear();
    ui->deskReaderStatusLabel->clear();


    trackReaderAntennaEnable = QList<bool>{false, false, false, false};

    // Populate antenna power comboBoxes (enabled when readers connect)

//    ui->trackAntenna1PowerComboBox->setEnabled(false);
//    ui->trackAntenna2PowerComboBox->setEnabled(false);
//    ui->trackAntenna3PowerComboBox->setEnabled(false);
//    ui->trackAntenna4PowerComboBox->setEnabled(false);

//    ui->deskAntenna1PowerComboBox->setEnabled(false);
//    ui->deskAntenna2PowerComboBox->setEnabled(false);
//    ui->deskAntenna3PowerComboBox->setEnabled(false);
//    ui->deskAntenna4PowerComboBox->setEnabled(false);

//    connect(ui->trackAntenna1PowerComboBox, SIGNAL(activated(int)), this, SLOT(onTrackAntenna1TransmitPowerComboBoxActivated(int)));
//    connect(ui->trackAntenna2PowerComboBox, SIGNAL(activated(int)), this, SLOT(onTrackAntenna2TransmitPowerComboBoxActivated(int)));
//    connect(ui->trackAntenna3PowerComboBox, SIGNAL(activated(int)), this, SLOT(onTrackAntenna3TransmitPowerComboBoxActivated(int)));
//    connect(ui->trackAntenna4PowerComboBox, SIGNAL(activated(int)), this, SLOT(onTrackAntenna4TransmitPowerComboBoxActivated(int)));
//    connect(ui->deskAntenna1PowerComboBox, SIGNAL(activated(int)), this, SLOT(onDeskAntenna1TransmitPowerComboBoxActivated(int)));
//    connect(ui->deskAntenna2PowerComboBox, SIGNAL(activated(int)), this, SLOT(onDeskAntenna2TransmitPowerComboBoxActivated(int)));
//    connect(ui->deskAntenna3PowerComboBox, SIGNAL(activated(int)), this, SLOT(onDeskAntenna3TransmitPowerComboBoxActivated(int)));
//    connect(ui->deskAntenna4PowerComboBox, SIGNAL(activated(int)), this, SLOT(onDeskAntenna4TransmitPowerComboBoxActivated(int)));



    // Set track length at position of each track antenna.  Lap speed is estimated from
    // these values assuming rider maintains same position on track for entire lap.
    // If this value is not set correctly, speed and distance cycled will be in error.
    // Set track length to -1 if antenna is not used (zero length will be tested on each new tag event and
    // used to check for missing track length settings).

    trackLengthM = QList<float>{0., 0., 0., 0.};


    // Initialize widgets with settings values

    loadPreferences();


    // Check that all enabled antennas have reasonable track length values and warn user application will not start functioning
    // until configured

    float maxTrackLength = 0.;
    bool trackLengthsInitialized = true;

    for (int i=0; i<trackLengthM.size(); i++) {
        if (trackLengthM[i] == 0.)
            trackLengthsInitialized = false;

        if (trackLengthM[i] > maxTrackLength)
            maxTrackLength = trackLengthM[i];
    }
    if (maxTrackLength <= 0.)
        trackLengthsInitialized = false;

    connect(ui->trackNameLineEdit, SIGNAL(textEdited(QString)), this, SLOT(onTrackNameLineEditTextEdited(QString)));
    connect(ui->tablePurgeIntervalDoubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(onTablePurgeIntervalDoubleSpinBoxValueChanged(double)));


    connect(ui->trackReaderIpLineEdit, SIGNAL(textEdited(QString)), this, SLOT(onTrackReaderIpLineEditTextEdited(QString)));
    connect(ui->trackReaderAntenna1EnableCheckBox, SIGNAL(clicked(bool)), this, SLOT(onTrackReaderAntenna1EnableCheckBoxClicked(bool)));
    connect(ui->trackReaderAntenna2EnableCheckBox, SIGNAL(clicked(bool)), this, SLOT(onTrackReaderAntenna2EnableCheckBoxClicked(bool)));
    connect(ui->trackReaderAntenna3EnableCheckBox, SIGNAL(clicked(bool)), this, SLOT(onTrackReaderAntenna3EnableCheckBoxClicked(bool)));
    connect(ui->trackReaderAntenna4EnableCheckBox, SIGNAL(clicked(bool)), this, SLOT(onTrackReaderAntenna4EnableCheckBoxClicked(bool)));
    connect(ui->trackLength1LineEdit, SIGNAL(textEdited(QString)), this, SLOT(onTrackLength1Edited(QString)));
    connect(ui->trackLength2LineEdit, SIGNAL(textEdited(QString)), this, SLOT(onTrackLength2Edited(QString)));
    connect(ui->trackLength3LineEdit, SIGNAL(textEdited(QString)), this, SLOT(onTrackLength3Edited(QString)));
    connect(ui->trackLength4LineEdit, SIGNAL(textEdited(QString)), this, SLOT(onTrackLength4Edited(QString)));

    connect(ui->deskReaderIpLineEdit, SIGNAL(textEdited(QString)), this, SLOT(onTrackReaderIpLineEditTextEdited(QString)));
    connect(ui->deskReaderAntenna1EnableCheckBox, SIGNAL(clicked(bool)), this, SLOT(onDeskReaderAntenna1EnableCheckBoxClicked(bool)));
    connect(ui->deskReaderAntenna2EnableCheckBox, SIGNAL(clicked(bool)), this, SLOT(onDeskReaderAntenna2EnableCheckBoxClicked(bool)));
    connect(ui->deskReaderAntenna3EnableCheckBox, SIGNAL(clicked(bool)), this, SLOT(onDeskReaderAntenna3EnableCheckBoxClicked(bool)));
    connect(ui->deskReaderAntenna4EnableCheckBox, SIGNAL(clicked(bool)), this, SLOT(onDeskReaderAntenna4EnableCheckBoxClicked(bool)));
    //connect(ui->deskLength1LineEdit, SIGNAL(textEdited(QString)), this, SLOT(onTrackLength1Edited(QString)));
    //connect(ui->trackLength2LineEdit, SIGNAL(textEdited(QString)), this, SLOT(onTrackLength2Edited(QString)));
    //connect(ui->trackLength3LineEdit, SIGNAL(textEdited(QString)), this, SLOT(onTrackLength3Edited(QString)));
    //connect(ui->trackLength4LineEdit, SIGNAL(textEdited(QString)), this, SLOT(onTrackLength4Edited(QString)));

    connect(ui->savePreferencesPushButton, SIGNAL(clicked()), this, SLOT(onSavePreferencesPushButtonClicked()));
    connect(ui->closePreferencesPushButton, SIGNAL(clicked()), this, SLOT(onClosePreferencesPushButtonClicked()));

//    connect(ui->eMailTestSendPushButton, SIGNAL(clicked()), this, SLOT(onEMailTestSendPushButtonClicked()));
//    connect(ui->saveSessionsPushButton, SIGNAL(clicked()), this, SLOT(onSaveSessionsPushButtonClicked()));

    updateSavePushButton();
}



CPreferences::~CPreferences() {
    delete ui;
}



// savePreferences()
//
// Save settings using QSettings and apply changes as required
//
void CPreferences::onSavePreferencesPushButtonClicked(void) {
    QString s;
    QString s1, s2, s3, s4;

    // Config panel

    settings.setValue("trackName", trackName);

    settings.setValue("tablePurgeIntervalHours", tablePurgeIntervalHours);

    //settings.setValue("emailReportLatency", ui->emailReportLatencySpinBox->value());

    settings.setValue("trackReaderIp", ui->trackReaderIpLineEdit->text());

    settings.setValue("trackReaderAntennaEnable", s1.setNum(trackReaderAntennaEnable[0]) + " " + s2.setNum(trackReaderAntennaEnable[1]) + " " + s3.setNum(trackReaderAntennaEnable[2]) + " " + s4.setNum(trackReaderAntennaEnable[3]));

//    settings.setValue("trackTransmitPower1", ui->trackAntenna1PowerComboBox->currentText());
//    settings.setValue("trackTransmitPower2", ui->trackAntenna2PowerComboBox->currentText());
//    settings.setValue("trackTransmitPower3", ui->trackAntenna3PowerComboBox->currentText());
//    settings.setValue("trackTransmitPower4", ui->trackAntenna4PowerComboBox->currentText());

    settings.setValue("trackLengthM", s1.setNum(trackLengthM[0]) + " " + s2.setNum(trackLengthM[1]) + " " + s3.setNum(trackLengthM[2]) + " " + s4.setNum(trackLengthM[3]));


    settings.setValue("deskReaderIp", ui->deskReaderIpLineEdit->text());

    settings.setValue("deskReaderAntennaEnable", s1.setNum(deskReaderAntennaEnable[0]) + " " + s2.setNum(deskReaderAntennaEnable[1]) + " " + s3.setNum(deskReaderAntennaEnable[2]) + " " + s4.setNum(deskReaderAntennaEnable[3]));

//    settings.setValue("trackTransmitPower1", ui->deskAntenna1PowerComboBox->currentText());
//    settings.setValue("trackTransmitPower2", ui->deskAntenna2PowerComboBox->currentText());
//    settings.setValue("trackTransmitPower3", ui->deskAntenna3PowerComboBox->currentText());
//    settings.setValue("trackTransmitPower4", ui->deskAntenna4PowerComboBox->currentText());

    settings.setValue("smtpUsername", ui->smtpUsernameLineEdit->text());
    settings.setValue("smtpPassword", ui->smtpPasswordLineEdit->text());
    settings.setValue("smtpServer", ui->smtpServerLineEdit->text());
    settings.setValue("smtpPort", ui->smtpPortLineEdit->text().toInt());
    settings.setValue("emailSendReports", ui->emailSendReportsCheckBox->isChecked());
    settings.setValue("emailFrom", ui->emailFromLineEdit->text());
    settings.setValue("emailSubject", ui->emailSubjectLineEdit->text());


    changesMade = false;
    updateSavePushButton();
}



// loadPreferences()
// Load widgets and local member variables with values from saved settings
//
void CPreferences::loadPreferences(void) {
    QString s;

    trackName = settings.value("trackName").toString();
    ui->trackNameLineEdit->setText(trackName);

    tablePurgeIntervalHours = settings.value("tablePurgeIntervalHours").toFloat();
    ui->tablePurgeIntervalDoubleSpinBox->setValue(tablePurgeIntervalHours);
    int interval = (int)(tablePurgeIntervalHours * 3600. * 1000. / 4.);
    if (interval < 1000)
        interval = 1000;
    if (mainWindow)
        mainWindow->purgeActiveRidersListTimer.setInterval(interval);

    trackReaderIp = settings.value("trackReaderIp").toString();
    ui->trackReaderIpLineEdit->setText(trackReaderIp);

    deskReaderIp = settings.value("deskReaderIp").toString();
    ui->deskReaderIpLineEdit->setText(deskReaderIp);


    trackReaderAntennaEnable = QList<bool>{parseBoolList(settings.value("trackReaderAntennaEnable").toString())};
    while (trackReaderAntennaEnable.size() < 4)
        trackReaderAntennaEnable.append(false);

    ui->trackReaderAntenna1EnableCheckBox->setChecked(trackReaderAntennaEnable[0]);
    ui->trackReaderAntenna2EnableCheckBox->setChecked(trackReaderAntennaEnable[1]);
    ui->trackReaderAntenna3EnableCheckBox->setChecked(trackReaderAntennaEnable[2]);
    ui->trackReaderAntenna4EnableCheckBox->setChecked(trackReaderAntennaEnable[3]);

    trackLengthM = QList<float>{parseFloatList(settings.value("trackLengthM").toString())};
    while (trackLengthM.size() < 4)
        trackLengthM.append(0.);
    ui->trackLength1LineEdit->setText(s.setNum(trackLengthM[0]));
    ui->trackLength2LineEdit->setText(s.setNum(trackLengthM[1]));
    ui->trackLength3LineEdit->setText(s.setNum(trackLengthM[2]));
    ui->trackLength4LineEdit->setText(s.setNum(trackLengthM[3]));


    deskReaderAntennaEnable = QList<bool>{parseBoolList(settings.value("deskReaderAntenna1Enable").toString())};
    while (deskReaderAntennaEnable.size() < 4)
        deskReaderAntennaEnable.append(false);

    ui->deskReaderAntenna1EnableCheckBox->setChecked(deskReaderAntennaEnable[0]);
    ui->deskReaderAntenna2EnableCheckBox->setChecked(deskReaderAntennaEnable[1]);
    ui->deskReaderAntenna3EnableCheckBox->setChecked(deskReaderAntennaEnable[2]);
    ui->deskReaderAntenna4EnableCheckBox->setChecked(deskReaderAntennaEnable[3]);

    deskReaderLocation = parseQuotedStringList(settings.value("deskReaderLocation").toString());
    if (deskReaderLocation.size() > 0)
        ui->deskLocation1LineEdit->setText(deskReaderLocation[0]);
    if (deskReaderLocation.size() > 1)
        ui->deskLocation2LineEdit->setText(deskReaderLocation[1]);
    if (deskReaderLocation.size() > 2)
        ui->deskLocation3LineEdit->setText(deskReaderLocation[2]);
    if (deskReaderLocation.size() > 3)
        ui->deskLocation4LineEdit->setText(deskReaderLocation[3]);



    ui->smtpUsernameLineEdit->setText(settings.value("smtpUsername").toString());
    ui->smtpPasswordLineEdit->setText(settings.value("smtpPassword").toString());
    ui->smtpServerLineEdit->setText(settings.value("smtpServer").toString());
    ui->smtpPortLineEdit->setText(settings.value("smtpPort").toString());
    ui->emailSendReportsCheckBox->setChecked(settings.value("emailSendReports").toBool());
    ui->emailFromLineEdit->setText(settings.value("emailFrom").toString());
    ui->emailSubjectLineEdit->setText(settings.value("emailSubject").toString());
//    ui->emailReportLatencySpinBox->setValue(settings.value("emailReportLatency").toInt());

//    for (int row=0; row<ui->sessionsTableWidget->rowCount(); row++) {
//        for (int col=0; col<ui->sessionsTableWidget->columnCount(); col++) {
//            ui->sessionsTableWidget->setItem(row, col, new QTableWidgetItem());
//        }
//        QString s;
//        ui->sessionsTableWidget->item(row, 0)->setText(settings.value(s.sprintf("scheduleItem%dDay", row)).toString());
//        ui->sessionsTableWidget->item(row, 1)->setText(settings.value(s.sprintf("scheduleItem%dSession", row)).toString());
//        ui->sessionsTableWidget->item(row, 2)->setText(settings.value(s.sprintf("scheduleItem%dStartTime", row)).toString());
//        ui->sessionsTableWidget->item(row, 3)->setText(settings.value(s.sprintf("scheduleItem%dEndTime", row)).toString());
//    }

    changesMade = false;
    updateSavePushButton();
}



void CPreferences::onTrackReaderStatus(QString s) {
    ui->trackReaderStatusLabel->setText(s);
}



void CPreferences::onDeskReaderStatus(QString s) {
    ui->trackReaderStatusLabel->setText(s);
}



void CPreferences::onTrackReaderIpLineEditTextEdited(QString text) {
    trackReaderIp = text;
    changesMade = true;
    updateSavePushButton();
}



void CPreferences::onTablePurgeIntervalDoubleSpinBoxValueChanged(double value) {
    tablePurgeIntervalHours = value;
    int interval = (int)(tablePurgeIntervalHours * 3600. * 1000. / 4.);
    if (interval < 1000)
        interval = 1000;
    if (mainWindow)
        mainWindow->purgeActiveRidersListTimer.setInterval(interval);
    changesMade = true;
    updateSavePushButton();
}




void CPreferences::setTrackReaderAntennaPowerComboBox(const QList<int> &list) {
    //        ui->trackAntenna1PowerComboBox->clear();
    //        for (int i=0; i<transmitPowerList->size(); i++) {
    //            ui->trackAntenna1PowerComboBox->addItem(s.setNum(transmitPowerList->at(i)));
    //        }
    //        ui->trackAntenna1PowerComboBox->setEnabled(true);
    //        ui->trackAntenna2PowerComboBox->clear();
    //        for (int i=0; i<transmitPowerList->size(); i++) {
    //            ui->trackAntenna2PowerComboBox->addItem(s.setNum(transmitPowerList->at(i)));
    //        }
    //        ui->trackAntenna2PowerComboBox->setEnabled(true);
    //        ui->trackAntenna3PowerComboBox->clear();
    //        for (int i=0; i<transmitPowerList->size(); i++) {
    //            ui->trackAntenna3PowerComboBox->addItem(s.setNum(transmitPowerList->at(i)));
    //        }
    //        ui->trackAntenna3PowerComboBox->setEnabled(true);
    //        ui->trackAntenna4PowerComboBox->clear();
    //        for (int i=0; i<transmitPowerList->size(); i++) {
    //            ui->trackAntenna4PowerComboBox->addItem(s.setNum(transmitPowerList->at(i)));
    //        }
    //        ui->trackAntenna4PowerComboBox->setEnabled(true);
}



void CPreferences::setDeskReaderAntennaPowerComboBox(const QList<int> &list) {
    //        ui->deskAntenna1PowerComboBox->clear();
    //        for (int i=0; i<transmitPowerList->size(); i++) {
    //            ui->deskAntenna1PowerComboBox->addItem(s.setNum(transmitPowerList->at(i)));
    //        }
    //        ui->deskAntenna1PowerComboBox->setEnabled(true);
    //        ui->deskAntenna2PowerComboBox->clear();
    //        for (int i=0; i<transmitPowerList->size(); i++) {
    //            ui->deskAntenna2PowerComboBox->addItem(s.setNum(transmitPowerList->at(i)));
    //        }
    //        ui->deskAntenna2PowerComboBox->setEnabled(true);
    //        ui->deskAntenna3PowerComboBox->clear();
    //        for (int i=0; i<transmitPowerList->size(); i++) {
    //            ui->deskAntenna3PowerComboBox->addItem(s.setNum(transmitPowerList->at(i)));
    //        }
    //        ui->deskAntenna3PowerComboBox->setEnabled(true);
    //        ui->deskAntenna4PowerComboBox->clear();
    //        for (int i=0; i<transmitPowerList->size(); i++) {
    //            ui->deskAntenna4PowerComboBox->addItem(s.setNum(transmitPowerList->at(i)));
    //        }
    //        ui->deskAntenna4PowerComboBox->setEnabled(true);
}



void CPreferences::onTrackReaderConnected(void) {
    CReader *sendingReader = (CReader *)sender();
    QString s;

    // Populate comboBox with available power settings for each reader

    QList<int> *transmitPowerList = sendingReader->getTransmitPowerList();

    ui->trackAntenna1PowerComboBox->clear();
    for (int i=0; i<transmitPowerList->size(); i++) {
        ui->trackAntenna1PowerComboBox->addItem(s.setNum(transmitPowerList->at(i)));
    }
    ui->trackAntenna1PowerComboBox->setEnabled(true);

    ui->trackAntenna2PowerComboBox->clear();
    for (int i=0; i<transmitPowerList->size(); i++) {
        ui->trackAntenna2PowerComboBox->addItem(s.setNum(transmitPowerList->at(i)));
    }
    ui->trackAntenna2PowerComboBox->setEnabled(true);

    ui->trackAntenna3PowerComboBox->clear();
    for (int i=0; i<transmitPowerList->size(); i++) {
        ui->trackAntenna3PowerComboBox->addItem(s.setNum(transmitPowerList->at(i)));
    }
    ui->trackAntenna3PowerComboBox->setEnabled(true);

    ui->trackAntenna4PowerComboBox->clear();
    for (int i=0; i<transmitPowerList->size(); i++) {
        ui->trackAntenna4PowerComboBox->addItem(s.setNum(transmitPowerList->at(i)));
    }
    ui->trackAntenna4PowerComboBox->setEnabled(true);
}




void CPreferences::onDeskReaderConnected(void) {
    CReader *sendingReader = (CReader *)sender();
    QString s;

    // Populate comboBox with available power settings for each reader

    QList<int> *transmitPowerList = sendingReader->getTransmitPowerList();

    ui->deskAntenna1PowerComboBox->clear();
    for (int i=0; i<transmitPowerList->size(); i++) {
        ui->deskAntenna1PowerComboBox->addItem(s.setNum(transmitPowerList->at(i)));
    }
    ui->deskAntenna1PowerComboBox->setEnabled(ui->deskReaderAntenna1EnableCheckBox->isChecked());

    ui->deskAntenna2PowerComboBox->clear();
    for (int i=0; i<transmitPowerList->size(); i++) {
        ui->deskAntenna2PowerComboBox->addItem(s.setNum(transmitPowerList->at(i)));
    }
    ui->deskAntenna2PowerComboBox->setEnabled(ui->deskReaderAntenna2EnableCheckBox->isChecked());

    ui->deskAntenna3PowerComboBox->clear();
    for (int i=0; i<transmitPowerList->size(); i++) {
        ui->deskAntenna3PowerComboBox->addItem(s.setNum(transmitPowerList->at(i)));
    }
    ui->deskAntenna3PowerComboBox->setEnabled(ui->deskReaderAntenna3EnableCheckBox->isChecked());

    ui->deskAntenna4PowerComboBox->clear();
    for (int i=0; i<transmitPowerList->size(); i++) {
        ui->deskAntenna4PowerComboBox->addItem(s.setNum(transmitPowerList->at(i)));
    }
    ui->deskAntenna4PowerComboBox->setEnabled(ui->deskReaderAntenna4EnableCheckBox->isChecked());
}



void CPreferences::onTrackNameLineEditTextEdited(QString s) {
    trackName = ui->trackNameLineEdit->text();
    changesMade = true;
    updateSavePushButton();
}



void CPreferences::onTrackLength1Edited(QString text) {
    while (trackLengthM.size() < 1)
        trackLengthM.append(0.);
    trackLengthM[0] = text.toFloat();
    changesMade = true;
    updateSavePushButton();
}



void CPreferences::onTrackLength2Edited(QString text) {
    while (trackLengthM.size() < 2)
        trackLengthM.append(0.);
    trackLengthM[1] = text.toFloat();
    changesMade = true;
    updateSavePushButton();
}



void CPreferences::onTrackLength3Edited(QString text) {
    while (trackLengthM.size() < 3)
        trackLengthM.append(0.);
    trackLengthM[2] = text.toFloat();
    changesMade = true;
    updateSavePushButton();
}



void CPreferences::onTrackLength4Edited(QString text) {
    while (trackLengthM.size() < 4)
        trackLengthM.append(0.);
    trackLengthM[3] = text.toFloat();
    changesMade = true;
    updateSavePushButton();
}



void CPreferences::onTrackReaderAntenna1EnableCheckBoxClicked(bool state) {
    ui->trackAntenna1PowerComboBox->setEnabled(state);
    trackReaderAntennaEnable = QList<bool> {ui->trackReaderAntenna1EnableCheckBox->isChecked(), ui->trackReaderAntenna2EnableCheckBox->isChecked(), ui->trackReaderAntenna3EnableCheckBox->isChecked(), ui->trackReaderAntenna4EnableCheckBox->isChecked()};
    changesMade = true;
    updateSavePushButton();
}



void CPreferences::onTrackReaderAntenna2EnableCheckBoxClicked(bool state) {
    ui->trackAntenna2PowerComboBox->setEnabled(state);
    trackReaderAntennaEnable = QList<bool> {ui->trackReaderAntenna1EnableCheckBox->isChecked(), ui->trackReaderAntenna2EnableCheckBox->isChecked(), ui->trackReaderAntenna3EnableCheckBox->isChecked(), ui->trackReaderAntenna4EnableCheckBox->isChecked()};
    changesMade = true;
    updateSavePushButton();
}



void CPreferences::onTrackReaderAntenna3EnableCheckBoxClicked(bool state) {
    ui->trackAntenna3PowerComboBox->setEnabled(state);
    trackReaderAntennaEnable = QList<bool> {ui->trackReaderAntenna1EnableCheckBox->isChecked(), ui->trackReaderAntenna2EnableCheckBox->isChecked(), ui->trackReaderAntenna3EnableCheckBox->isChecked(), ui->trackReaderAntenna4EnableCheckBox->isChecked()};
    changesMade = true;
    updateSavePushButton();
}



void CPreferences::onTrackReaderAntenna4EnableCheckBoxClicked(bool state) {
    ui->trackAntenna4PowerComboBox->setEnabled(state);
    trackReaderAntennaEnable = QList<bool> {ui->trackReaderAntenna1EnableCheckBox->isChecked(), ui->trackReaderAntenna2EnableCheckBox->isChecked(), ui->trackReaderAntenna3EnableCheckBox->isChecked(), ui->trackReaderAntenna4EnableCheckBox->isChecked()};
    changesMade = true;
    updateSavePushButton();
}



void CPreferences::onDeskReaderAntenna1EnableCheckBoxClicked(bool state) {
    ui->deskAntenna1PowerComboBox->setEnabled(state);
    deskReaderAntennaEnable = QList<bool> {ui->deskReaderAntenna1EnableCheckBox->isChecked(), ui->deskReaderAntenna2EnableCheckBox->isChecked(), ui->deskReaderAntenna3EnableCheckBox->isChecked(), ui->deskReaderAntenna4EnableCheckBox->isChecked()};
    changesMade = true;
    updateSavePushButton();
}



void CPreferences::onDeskReaderAntenna2EnableCheckBoxClicked(bool state) {
    ui->deskAntenna2PowerComboBox->setEnabled(state);
    deskReaderAntennaEnable = QList<bool> {ui->deskReaderAntenna1EnableCheckBox->isChecked(), ui->deskReaderAntenna2EnableCheckBox->isChecked(), ui->deskReaderAntenna3EnableCheckBox->isChecked(), ui->deskReaderAntenna4EnableCheckBox->isChecked()};
    changesMade = true;
    updateSavePushButton();
}



void CPreferences::onDeskReaderAntenna3EnableCheckBoxClicked(bool state) {
    ui->deskAntenna3PowerComboBox->setEnabled(state);
    deskReaderAntennaEnable = QList<bool> {ui->deskReaderAntenna1EnableCheckBox->isChecked(), ui->deskReaderAntenna2EnableCheckBox->isChecked(), ui->deskReaderAntenna3EnableCheckBox->isChecked(), ui->deskReaderAntenna4EnableCheckBox->isChecked()};
    changesMade = true;
    updateSavePushButton();
}



void CPreferences::onDeskReaderAntenna4EnableCheckBoxClicked(bool state) {
    ui->deskAntenna4PowerComboBox->setEnabled(state);
    deskReaderAntennaEnable = QList<bool> {ui->deskReaderAntenna1EnableCheckBox->isChecked(), ui->deskReaderAntenna2EnableCheckBox->isChecked(), ui->deskReaderAntenna3EnableCheckBox->isChecked(), ui->deskReaderAntenna4EnableCheckBox->isChecked()};
    changesMade = true;
    updateSavePushButton();
}



void CPreferences::updateSavePushButton(void) {
    ui->savePreferencesPushButton->setEnabled(changesMade);
}



void CPreferences::onClosePreferencesPushButtonClicked(void) {
    if (changesMade) {
        int rc = guiQuestion("Changes have been made to preference settings.  Do you wish to save these changes?", QMessageBox::Yes | QMessageBox::No);
        if (rc == QMessageBox::Yes)
            onSavePreferencesPushButtonClicked();
    }
    hide();
}



void CPreferences::onTrackAntenna1TransmitPowerComboBoxActivated(int /*antennaIndex*/) {
    guiCritical("This feature not yet implemented");
}



void CPreferences::onTrackAntenna2TransmitPowerComboBoxActivated(int /*antennaIndex*/) {
    guiCritical("This feature not yet implemented");
}



void CPreferences::onTrackAntenna3TransmitPowerComboBoxActivated(int /*antennaIndex*/) {
    guiCritical("This feature not yet implemented");
}



void CPreferences::onTrackAntenna4TransmitPowerComboBoxActivated(int /*antennaIndex*/) {
    guiCritical("This feature not yet implemented");
}



void CPreferences::onDeskAntenna1TransmitPowerComboBoxActivated(int /*antennaIndex*/) {
    guiCritical("This feature not yet implemented");
}



void CPreferences::onDeskAntenna2TransmitPowerComboBoxActivated(int /*antennaIndex*/) {
    guiCritical("This feature not yet implemented");
}



void CPreferences::onDeskAntenna3TransmitPowerComboBoxActivated(int /*antennaIndex*/) {
    guiCritical("This feature not yet implemented");
}



void CPreferences::onDeskAntenna4TransmitPowerComboBoxActivated(int /*antennaIndex*/) {
    guiCritical("This feature not yet implemented");
}


void CPreferences::onEMailTestSendPushButtonClicked(void) {
//    CSmtp *smtp = new CSmtp(ui->smtpUsernameLineEdit->text(), ui->smtpPasswordLineEdit->text(), ui->smtpServerLineEdit->text(), ui->smtpPortLineEdit->text().toInt());
//    connect(smtp, SIGNAL(completed(int)), this, SLOT(onTestMailSent(int)));
//    connect(smtp, SIGNAL(newLogMessage(QString)), this, SLOT(onNewLogMessage(QString)));

//    QString body("This is test email message.");

//    body.append("\n\nReport generated by llrpLaps " + QCoreApplication::applicationVersion());

//    smtp->sendMail(ui->emailFromLineEdit->text(), ui->emailTestToLineEdit->text(), ui->emailTestSubjectLineEdit->text(), body.toLatin1().data());
}



//void CPreferences::onTestMailSent(void) {
//    QMessageBox::information(this, "EMail Test", "Email message sent");
//}



QList<bool> CPreferences::parseBoolList(const QString &s) {
    QStringList sList = s.split(' ', QString::SkipEmptyParts);
    QList<bool> list;
    for (int i=0; i<sList.size(); i++) {
        list.append((bool)sList[i].toInt());
    }
    return list;
}



QList<int> CPreferences::parseIntList(const QString &s) {
    QStringList sList = s.split(QRegExp("\\s"), QString::SkipEmptyParts);
    QList<int> list;
    for (int i=0; i<sList.size(); i++) {
        list.append(sList[i].toInt());
    }
    return list;
}



QList<float> CPreferences::parseFloatList(const QString &s) {
    QStringList sList = s.split(QRegExp("\\s"), QString::SkipEmptyParts);
    QList<float> list;
    for (int i=0; i<sList.size(); i++) {
        list.append(sList[i].toFloat());
    }
    return list;
}



QList<double> CPreferences::parseDoubleList(const QString &s) {
    QStringList sList = s.split(QRegExp("\\s"), QString::SkipEmptyParts);
    QList<double> list;
    for (int i=0; i<sList.size(); i++) {
        list.append(sList[i].toDouble());
    }
    return list;
}



QStringList CPreferences::parseQuotedStringList(const QString &s) {
    QStringList list;

    bool inside = (s.startsWith("\"")); //true if the first character is "
    QStringList tmpList = s.split(QRegExp("\""), QString::SkipEmptyParts); // Split by " and make sure you don't have an empty string at the beginning
    foreach (QString s2, tmpList) {
        if (inside) { // If 's2' is inside quotes ...
            list.append(s2); // ... get the whole string
        } else { // If 's2' is outside quotes ...
            list.append(s2.split(" ", QString::SkipEmptyParts)); // ... get the splitted string
        }
        inside = !inside;
    }
    return list;
}




void CPreferences::guiCritical(const QString &s) {
    if (messagesWindow)
        messagesWindow->addMessage("Critical: " + s);
    QMessageBox::critical(this, QCoreApplication::applicationName() + " Critical Error", s, QMessageBox::Ok);
}



int CPreferences::guiQuestion(const QString &s, QMessageBox::StandardButtons b) {
    return ((MainWindow *)parent())->guiQuestion(s, b);
}

