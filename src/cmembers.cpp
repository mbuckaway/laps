// cmembers.cpp



#include "cmembers.h"
#include "cmessages.h"
#include "mainwindow.h"
#include "cdbase.h"
#include "ui_cmembers.h"





CMembers::CMembers(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CMembers)
{
    ui->setupUi(this);
    tagInDbase = false;
    entryEdited = false;

//    ui->namesTableView->setModel(membershipProxyModel);
    ui->namesTableView->setColumnWidth(0, 150);
    ui->namesTableView->setColumnWidth(1, 150);
    ui->namesTableView->setColumnWidth(2, 200);
    ui->namesTableView->setColumnWidth(3, 200);
    ui->namesTableView->setColumnWidth(4, 200);
    ui->namesTableView->setColumnWidth(5, 200);

    ui->namesTableView->setAlternatingRowColors(true);
    ui->namesTableView->horizontalHeader()->setStretchLastSection(true);
    ui->namesTableView->horizontalHeader()->setStyleSheet("QHeaderView{font: bold;}");
    ui->namesTableView->sortByColumn(2, Qt::AscendingOrder);     // must come before call to setSortingEnabled()
    ui->namesTableView->setSortingEnabled(true);

//    connect(ui->namesTableView, SIGNAL(clicked(const QModelIndex &)), this, SLOT(onNamesTableClicked(const QModelIndex &)));
//    connect(ui->namesTableView, SIGNAL(doubleClicked(const QModelIndex &)), this, SLOT(onNamesTableDoubleClicked(const QModelIndex &)));



    // Configure Dbase page

//    connect(ui->deskClearPushButton, SIGNAL(clicked()), this, SLOT(onDbaseClearPushButtonClicked()));
//    connect(ui->deskSearchPushButton, SIGNAL(clicked()), this, SLOT(onDbaseSearchPushButtonClicked()));
//    connect(ui->deskAddPushButton, SIGNAL(clicked()), this, SLOT(onDbaseAddPushButtonClicked()));
//    connect(ui->deskRemovePushButton, SIGNAL(clicked()), this, SLOT(onDbaseRemovePushButtonClicked()));
//    connect(ui->deskUpdatePushButton, SIGNAL(clicked()), this, SLOT(onDbaseUpdatePushButtonClicked()));
//    connect(ui->deskReadPushButton, SIGNAL(clicked(bool)), this, SLOT(onDbaseReadPushButtonClicked(bool)));
//    connect(ui->deskTagIdLineEdit, SIGNAL(textEdited(QString)), this, SLOT(onDbaseTagIdTextEdited(QString)));
//    connect(ui->deskFirstNameLineEdit, SIGNAL(textEdited(QString)), this, SLOT(onDbaseFirstNameTextEdited(QString)));
//    connect(ui->deskLastNameLineEdit, SIGNAL(textEdited(QString)), this, SLOT(onDbaseLastNameTextEdited(QString)));
//    connect(ui->deskMembershipNumberLineEdit, SIGNAL(textEdited(QString)), this, SLOT(onDbaseMembershipNumberTextEdited(QString)));
//    connect(ui->deskCaRegistrationLineEdit, SIGNAL(textEdited(QString)), this, SLOT(onDbaseCaRegistrationTextEdited(QString)));
//    connect(ui->deskEMailLineEdit, SIGNAL(textEdited(QString)), this, SLOT(onDbaseEMailTextEdited(QString)));

    //updateDbaseButtons();
}

CMembers::~CMembers() {
    delete ui;
}



void CMembers::onNewDeskTag(CTagInfo tagInfo) {
    QString s;

    // Add string to messages window

//    if (messagesWindow.detailedMessagesIsChecked())
//        messagesWindow.addMessage(s.sprintf("readerId=%d antennaId=%d timeStampUSec=%llu tagData=%s", tagInfo.readerId, tagInfo.antennaId, tagInfo.timeStampUSec, tagInfo.tagId.toLatin1().data()));

    if (tagInfo.antennaId <= 0)
        return;

    //deskReader->blockSignals(true);
    emit blockReaderSignals(true);
    onDbaseClearPushButtonClicked();
    ui->deskTagIdLineEdit->setText(tagInfo.tagId);
    onDbaseSearchPushButtonClicked();
    ui->deskReadPushButton->setChecked(false);

    entryEdited = false;
    updateDbaseButtons();
}



void CMembers::onDbaseTagIdTextEdited(QString) {
    tagInDbase = false;
    entryEdited = true;
    updateDbaseButtons();
}



void CMembers::onDbaseFirstNameTextEdited(QString) {
    entryEdited = true;
    updateDbaseButtons();
}



void CMembers::onDbaseLastNameTextEdited(QString) {
    entryEdited = true;
    updateDbaseButtons();
}


void CMembers::onDbaseMembershipNumberTextEdited(QString) {
    entryEdited = true;
    updateDbaseButtons();
}


void CMembers::onDbaseCaRegistrationTextEdited(QString) {
    entryEdited = true;
    updateDbaseButtons();
}


void CMembers::onDbaseEMailTextEdited(QString) {
    entryEdited = true;
    updateDbaseButtons();
}


void CMembers::updateDbaseButtons(void) {

    // If ReadTag pushed, all others should be disabled

    if (ui->deskReadPushButton->isChecked()) {
        ui->deskSearchPushButton->setEnabled(false);
        ui->deskAddPushButton->setEnabled(false);
        ui->deskClearPushButton->setEnabled(false);
        ui->deskRemovePushButton->setEnabled(false);
        ui->deskUpdatePushButton->setEnabled(false);
        return;
    }

    // Search is enabled when one of TagId, FirstName and/or LastName, or membershipNumber is filled

    if (!ui->deskTagIdLineEdit->text().isEmpty() && ui->deskFirstNameLineEdit->text().isEmpty() && ui->deskLastNameLineEdit->text().isEmpty() && ui->deskMembershipNumberLineEdit->text().isEmpty())
        ui->deskSearchPushButton->setEnabled(true);

    else if (ui->deskTagIdLineEdit->text().isEmpty() && !ui->deskFirstNameLineEdit->text().isEmpty() && ui->deskLastNameLineEdit->text().isEmpty() && ui->deskMembershipNumberLineEdit->text().isEmpty())
        ui->deskSearchPushButton->setEnabled(true);

    else if (ui->deskTagIdLineEdit->text().isEmpty() && !ui->deskFirstNameLineEdit->text().isEmpty() && !ui->deskLastNameLineEdit->text().isEmpty() && ui->deskMembershipNumberLineEdit->text().isEmpty())
        ui->deskSearchPushButton->setEnabled(true);

    else if (ui->deskTagIdLineEdit->text().isEmpty() && ui->deskFirstNameLineEdit->text().isEmpty() && !ui->deskLastNameLineEdit->text().isEmpty() && ui->deskMembershipNumberLineEdit->text().isEmpty())
        ui->deskSearchPushButton->setEnabled(true);

    else if (ui->deskTagIdLineEdit->text().isEmpty() && ui->deskFirstNameLineEdit->text().isEmpty() && ui->deskLastNameLineEdit->text().isEmpty() && !ui->deskMembershipNumberLineEdit->text().isEmpty())
        ui->deskSearchPushButton->setEnabled(true);

    else
        ui->deskSearchPushButton->setEnabled(false);


    // Add is enabled when all fields (except firstname) are filled, tagInDbase is false, and with email optional

    if (!ui->deskTagIdLineEdit->text().isEmpty() && !ui->deskLastNameLineEdit->text().isEmpty() && !ui->deskMembershipNumberLineEdit->text().isEmpty() && !ui->deskCaRegistrationLineEdit->text().isEmpty() && !tagInDbase)
        ui->deskAddPushButton->setEnabled(true);

    else
        ui->deskAddPushButton->setEnabled(false);


    // Clear is enabled when any field is filled

    if (!ui->deskTagIdLineEdit->text().isEmpty() || !ui->deskFirstNameLineEdit->text().isEmpty() || !ui->deskLastNameLineEdit->text().isEmpty() || !ui->deskMembershipNumberLineEdit->text().isEmpty() || !ui->deskCaRegistrationLineEdit->text().isEmpty() || !ui->deskEMailLineEdit->text().isEmpty())
        ui->deskClearPushButton->setEnabled(true);

    else
        ui->deskClearPushButton->setEnabled(false);


    // Remove is enabled when tadId and last name are filled, and tagInDbase is true

    if (!ui->deskTagIdLineEdit->text().isEmpty() && !ui->deskLastNameLineEdit->text().isEmpty() && !ui->deskMembershipNumberLineEdit->text().isEmpty() && tagInDbase)
        ui->deskRemovePushButton->setEnabled(true);

    else
        ui->deskRemovePushButton->setEnabled(false);


    // Update is enabled when all fields (firstname and email optional) are filled, and tagInDbase is true and entryChanged is true

    if (!ui->deskTagIdLineEdit->text().isEmpty() && !ui->deskLastNameLineEdit->text().isEmpty() && !ui->deskMembershipNumberLineEdit->text().isEmpty() && !ui->deskCaRegistrationLineEdit->text().isEmpty() && tagInDbase && entryEdited)
        ui->deskUpdatePushButton->setEnabled(true);

    else
        ui->deskUpdatePushButton->setEnabled(false);

}



void CMembers::onNamesTableClicked(const QModelIndex &index) {
    onDbaseClearPushButtonClicked();
    switch (index.column()) {
    case 0:
        ui->deskTagIdLineEdit->setText(index.data().toString());
        break;
    case 1:
        ui->deskFirstNameLineEdit->setText(index.data().toString());
        break;
    case 2:
        ui->deskLastNameLineEdit->setText(index.data().toString());
        break;
    case 3:
        ui->deskMembershipNumberLineEdit->setText(index.data().toString());
        break;
    case 4:
        ui->deskCaRegistrationLineEdit->setText(index.data().toString());
        break;
    case 5:
        ui->deskEMailLineEdit->setText(index.data().toString());
        break;
    }
    updateDbaseButtons();
}




void CMembers::onDbaseSearchPushButtonClicked(void) {
    QString tagId = ui->deskTagIdLineEdit->text();
    QString firstName = ui->deskFirstNameLineEdit->text();
    QString lastName = ui->deskLastNameLineEdit->text();
    QString membershipNumber = ui->deskMembershipNumberLineEdit->text();


    // If tagId contains entry, search based on only that.
    // If found, update fields.  Otherwise clear fields.

    CMembershipInfo info;

//    if (!tagId.isEmpty() && firstName.isEmpty() && lastName.isEmpty() && membershipNumber.isEmpty()) {
//        int id = membershipDbase.getIdFromTagId(ui->deskTagIdLineEdit->text().toLatin1());
//        if (id > 0) {
//            tagInDbase = true;
//            membershipDbase.getAllFromId(id, &info);
//            ui->deskFirstNameLineEdit->setText(info.firstName);
//            ui->deskLastNameLineEdit->setText(info.lastName);
//            ui->deskMembershipNumberLineEdit->setText(info.membershipNumber);
//            ui->deskCaRegistrationLineEdit->setText(info.caRegistration);
//            ui->deskEMailLineEdit->setText(info.eMail);
//            ui->sendReportsCheckBox->setChecked(info.sendReports);
//        }
//        else {
//            tagInDbase = false;
//            ui->deskFirstNameLineEdit->clear();
//            ui->deskLastNameLineEdit->clear();
//            ui->deskMembershipNumberLineEdit->clear();
//            ui->deskCaRegistrationLineEdit->clear();
//            ui->deskEMailLineEdit->clear();
//            ui->sendReportsCheckBox->setChecked(false);
//        }
//    }

//    // Else if first or last name given, search on that.  Don't clear names on search fail.

//    else if ((!lastName.isEmpty() || !firstName.isEmpty()) && membershipNumber.isEmpty()) {
//        int id = membershipDbase.getIdFromName(firstName, lastName);
//        if (id > 0) {
//            tagInDbase = true;
//            membershipDbase.getAllFromId(id, &info);
//            ui->deskTagIdLineEdit->setText(info.tagId);
//            ui->deskFirstNameLineEdit->setText(info.firstName);
//            ui->deskLastNameLineEdit->setText(info.lastName);
//            ui->deskMembershipNumberLineEdit->setText(info.membershipNumber);
//            ui->deskCaRegistrationLineEdit->setText(info.caRegistration);
//            ui->deskEMailLineEdit->setText(info.eMail);
//            ui->sendReportsCheckBox->setChecked(info.sendReports);
//        }
//        else {
//            tagInDbase = false;
//        }
//    }

//    // Else if membership number is given, search on that.  Don't clear names on search fail.

//    else if ((lastName.isEmpty() && firstName.isEmpty()) && !membershipNumber.isEmpty()) {
//        int id = membershipDbase.getIdFromMembershipNumber(membershipNumber);
//        if (id > 0) {
//            tagInDbase = true;
//            membershipDbase.getAllFromId(id, &info);
//            ui->deskTagIdLineEdit->setText(info.tagId);
//            ui->deskFirstNameLineEdit->setText(info.firstName);
//            ui->deskLastNameLineEdit->setText(info.lastName);
//            ui->deskMembershipNumberLineEdit->setText(info.membershipNumber);
//            ui->deskCaRegistrationLineEdit->setText(info.caRegistration);
//            ui->deskEMailLineEdit->setText(info.eMail);
//            ui->sendReportsCheckBox->setChecked(info.sendReports);
//        }
//        else {
//            tagInDbase = false;
//        }
//    }

//    entryEdited = false;
//    updateDbaseButtons();
}



void CMembers::onDbaseAddPushButtonClicked(void) {

//    // Check whether tagId is already in dbase

//    int id = membershipDbase.getIdFromTagId(ui->deskTagIdLineEdit->text().toLatin1());
//    if (id != 0) {
//        guiCritical("Tag \"" + ui->deskTagIdLineEdit->text().toLatin1() + "\" already in database");
//        return;
//    }

//    // Add entry to database

//    CMembershipInfo info;
//    info.tagId = ui->deskTagIdLineEdit->text();
//    info.firstName = ui->deskFirstNameLineEdit->text();
//    info.lastName = ui->deskLastNameLineEdit->text();
//    info.membershipNumber = ui->deskMembershipNumberLineEdit->text();
//    info.caRegistration = ui->deskCaRegistrationLineEdit->text();
//    info.eMail = ui->deskEMailLineEdit->text();
//    info.sendReports = ui->sendReportsCheckBox->isChecked();
//    int rc = membershipDbase.add(info);
//    if (rc != 0) {
//        guiCritical(membershipDbase.errorText());
//        return;
//    }

//    // Add to table

//    if (!membershipTableModel->add(info)) {
//        guiCritical("Could not add entry to membershipTable");
//        return;
//    }

//    // Update name in activeRidersList so it appears in activeRidersTable

//    for (int i=0; i<activeRidersTableModel->activeRidersList.size(); i++) {
//        if (activeRidersTableModel->activeRidersList[i].tagId == info.tagId) {
//            if (info.firstName.isEmpty())
//                activeRidersTableModel->activeRidersList[i].name = info.lastName;
//            else
//                activeRidersTableModel->activeRidersList[i].name = info.firstName + " " + info.lastName;
//            break;
//        }
//    }

//    onDbaseClearPushButtonClicked();
}



void CMembers::onDbaseClearPushButtonClicked(void) {
    tagInDbase = false;
    entryEdited = false;

    ui->deskTagIdLineEdit->clear();
    ui->deskFirstNameLineEdit->clear();
    ui->deskLastNameLineEdit->clear();
    ui->deskMembershipNumberLineEdit->clear();
    ui->deskCaRegistrationLineEdit->clear();
    ui->deskEMailLineEdit->clear();
    ui->sendReportsCheckBox->setChecked(false);
    updateDbaseButtons();
}



void CMembers::onDbaseRemovePushButtonClicked(void) {
//    QMessageBox::StandardButtons b = guiQuestion("You are about to remove this tag entry from the database.  Press Ok to continue.", QMessageBox::Ok | QMessageBox::Abort);
//    if (b == QMessageBox::Ok) {
//        QString tagId(ui->deskTagIdLineEdit->text());
//        if (membershipDbase.removeTagId(tagId) != 0) {
//            guiCritical("Error removing name from database");
//            return;
//        }
//        if (!membershipTableModel->remove(tagId)) {
//            guiCritical("Error removing name from namesTable");
//            return;
//        }

//        // Update name in activeRidersList so it appears in activeRidersTable

//        for (int i=0; i<activeRidersTableModel->activeRidersList.size(); i++) {
//            if (activeRidersTableModel->activeRidersList[i].tagId == tagId) {
//                activeRidersTableModel->activeRidersList[i].name.clear();
//                break;
//            }
//        }
//    }


//    entryEdited = false;
//    onDbaseClearPushButtonClicked();
}



void CMembers::onDbaseUpdatePushButtonClicked(void) {

//    // Confirm we should be changing database

//    if (guiQuestion("You are about to modify an existing tag entry in the database.  Press Ok to continue.", QMessageBox::Ok | QMessageBox::Abort) != QMessageBox::Ok)
//        return;

//    CMembershipInfo info;
//    info.tagId = ui->deskTagIdLineEdit->text();
//    info.firstName = ui->deskFirstNameLineEdit->text();
//    info.lastName = ui->deskLastNameLineEdit->text();
//    info.membershipNumber = ui->deskMembershipNumberLineEdit->text();
//    info.caRegistration = ui->deskCaRegistrationLineEdit->text();
//    info.eMail = ui->deskEMailLineEdit->text();
//    info.sendReports = ui->sendReportsCheckBox->isChecked();

//    // Update entry in database

//    int rc = membershipDbase.update(info);
//    if (rc != 0) {
//        guiCritical("Could not update database: " + membershipDbase.errorText());
//        return;
//    }

//    // Add to table

//    if (!membershipTableModel->update(info)) {
//        guiCritical("Could not update membershipTable");
//        return;
//    }


//    // Update name in activeRidersList so it appears in activeRidersTable

//    for (int i=0; i<activeRidersTableModel->activeRidersList.size(); i++) {
//        if (activeRidersTableModel->activeRidersList[i].tagId == info.tagId) {
//            if (info.firstName.isEmpty())
//                activeRidersTableModel->activeRidersList[i].name = info.lastName;
//            else
//                activeRidersTableModel->activeRidersList[i].name = info.firstName + " " + info.lastName;
//            break;
//        }
//    }


//    entryEdited = false;
//    onDbaseClearPushButtonClicked();
}



void CMembers::onDbaseReadPushButtonClicked(bool state) {
//    if (state) {
//        ui->deskReadPushButton->setChecked(true);
//        onDbaseClearPushButtonClicked();
//        if (deskReader)
//            deskReader->blockSignals(false);
//    }
//    else {
//        if (deskReader)
//            deskReader->blockSignals(true);
//        ui->deskReadPushButton->setChecked(false);
//    }

//    entryEdited = false;
//    updateDbaseButtons();
}




void CMembers::guiCritical(const QString &s, QMessageBox::StandardButtons b) {
//    onNewLogMessage("Critical: " + s);
    QMessageBox::critical(this, "llrplaps Critical Error", s, b);
}


void CMembers::guiInformation(const QString &s, QMessageBox::StandardButtons b) {
//    onNewLogMessage("Information: " + s);
    QMessageBox::information(this, QCoreApplication::applicationName() + "\n\n", s, b);
}


QMessageBox::StandardButtons CMembers::guiQuestion(const QString &s, QMessageBox::StandardButtons b) {
    return QMessageBox::question(this, "llrplaps Question", s, b);
}


