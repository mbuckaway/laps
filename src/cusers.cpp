// cusers.cpp


#include <QWidget>
#include <QSettings>

#include "cusers.h"
#include "ui_cusers.h"
#include "mainwindow.h"


extern MainWindow *mainWindow;
extern CMessages *messagesWindow;
extern CPreferences *preferencesWindow;




// *****************************************************************************
// CUsersTableModel
//
CUsersTableModel::CUsersTableModel(QObject *parent) : QAbstractTableModel(parent) {
    membershipInfoList.clear();

    QSqlQuery query(mainWindow->membershipDbase.dBase);
    query.prepare("select * from membershipTable");
    if (!query.exec())
        return;

    int idTagId = query.record().indexOf("tagId");
    int idFirst = query.record().indexOf("firstName");
    int idLast = query.record().indexOf("lastName");
    int idMembership = query.record().indexOf("membershipNumber");
    int idCaRegistration = query.record().indexOf("caRegistration");
    int idEmail = query.record().indexOf("eMail");
    int idSendReports = query.record().indexOf("sendReports");
    while (query.next()) {
        CMembershipInfo info;
        info.tagId = query.value(idTagId).toString();
        info.firstName = query.value(idFirst).toString();
        info.lastName = query.value(idLast).toString();
        info.membershipNumber = query.value(idMembership).toString();
        info.caRegistration = query.value(idCaRegistration).toString();
        info.eMail = query.value(idEmail).toString();
        info.sendReports = query.value(idSendReports).toInt();
        membershipInfoList.append(info);
    }
    QModelIndex topLeft = index(0, 0);
    QModelIndex bottomRight = index(membershipInfoList.size() - 1, 0);
    emit dataChanged(topLeft, bottomRight);
}



bool CUsersTableModel::add(const CMembershipInfo &info) {
    int row = rowCount();
    insertRows(row, 1);
    setData(index(row, 0), info.tagId, Qt::EditRole);
    setData(index(row, 1), info.firstName, Qt::EditRole);
    setData(index(row, 2), info.lastName, Qt::EditRole);
    setData(index(row, 3), info.membershipNumber, Qt::EditRole);
    setData(index(row, 4), info.caRegistration.toUpper(), Qt::EditRole);
    setData(index(row, 5), info.eMail, Qt::EditRole);
    setData(index(row, 6), info.sendReports, Qt::EditRole);
    QModelIndex topLeft = index(row, 0);
    QModelIndex bottomRight = index(row, 5);
    emit dataChanged(topLeft, bottomRight);
    return true;
}



bool CUsersTableModel::update(const CMembershipInfo &info) {
    int row = -1;
    for (int i=0; i<membershipInfoList.size(); i++) {
        if (membershipInfoList[i].tagId == info.tagId) {
            row = i;
            break;
        }
    }
    if (row < 0) {
        mainWindow->guiCritical("Could not find tagId in CMembershipInfoList in CMembershipTableModel::update");
        return false;
    }
    setData(index(row, 0), info.tagId, Qt::EditRole);
    setData(index(row, 1), info.firstName, Qt::EditRole);
    setData(index(row, 2), info.lastName, Qt::EditRole);
    setData(index(row, 3), info.membershipNumber, Qt::EditRole);
    setData(index(row, 4), info.caRegistration.toUpper(), Qt::EditRole);
    setData(index(row, 5), info.eMail, Qt::EditRole);
    setData(index(row, 6), info.sendReports, Qt::EditRole);
    QModelIndex topLeft = index(row, 0);
    QModelIndex bottomRight = index(row, 5);
    emit dataChanged(topLeft, bottomRight);
    return true;
}



bool CUsersTableModel::remove(const QString &tagId) {
    int row = -1;
    for (int i=0; i<membershipInfoList.size(); i++) {
        if (membershipInfoList[i].tagId == tagId) {
            row = i;
            break;
        }
    }
    if (row < 0) {
        mainWindow->guiCritical("Could not find tagId in CMembershipInfoList in CMembershipTableModel::remove");
        return false;
    }
    if (!removeRows(row, 1))
        return false;
    QModelIndex topLeft = index(row, 0);
    QModelIndex bottomRight = index(rowCount() - 1, 2);
    emit dataChanged(topLeft, bottomRight);
    return true;
}



bool CUsersTableModel::insertRows(int position, int count, const QModelIndex &/*parent*/) {
    beginInsertRows(QModelIndex(), position, position + count - 1);
    for (int row=0; row<count; row++)
        membershipInfoList.insert(position, CMembershipInfo());
    endInsertRows();
    QModelIndex topLeft = index(position, 0);
    QModelIndex bottomRight = index(position, 0);
    emit dataChanged(topLeft, bottomRight);
    return true;
}



bool CUsersTableModel::removeRows(int position, int count, const QModelIndex &/*parent*/) {
    beginRemoveRows(QModelIndex(), position, position + count - 1);
    for (int row = 0; row < count; row++) {
        membershipInfoList.removeAt(position);
    }
    endRemoveRows();
    return true;
}



int CUsersTableModel::rowCount(const QModelIndex &/*parent*/) const {
    return membershipInfoList.size();
}



int CUsersTableModel::columnCount(const QModelIndex &/*parent*/) const {
    return 7;
}



QVariant CUsersTableModel::data(const QModelIndex &index, int role) const {
    int row = index.row();
    int col = index.column();

    switch (role) {
    case Qt::DisplayRole:
        switch (col) {
        case 0:
            return membershipInfoList[row].tagId;
        case 1:
            return membershipInfoList[row].firstName;
        case 2:
            return membershipInfoList[row].lastName;
        case 3:
            return membershipInfoList[row].membershipNumber;
        case 4:
            return membershipInfoList[row].caRegistration;
        case 5:
            return membershipInfoList[row].eMail;
        case 6:
            return membershipInfoList[row].sendReports;
        }
        break;
    case Qt::FontRole:
        break;
    case Qt::BackgroundRole:
        break;
    case Qt::TextAlignmentRole:
        return Qt::AlignLeft + Qt::AlignVCenter;
    }

    return QVariant();
}



QVariant CUsersTableModel::headerData(int section, Qt::Orientation orientation, int role) const {
    switch (role) {
    case Qt::DisplayRole:
        if (orientation == Qt::Horizontal) {
            switch (section) {
            case 0:
                return QString("Tag Id");
            case 1:
                return QString("First Name");
            case 2:
                return QString("Last Name");
            case 3:
                return QString("Track Membership N");
            case 4:
                return QString("CA Registration");
            case 5:
                return QString("eMail");
            case 6:
                return QString("EmailReports");
            }
        }
    }
    return QVariant();
}



bool CUsersTableModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    int row = index.row();
    int col = index.column();
    if (role == Qt::EditRole) {
        switch (col) {
        case 0:
            membershipInfoList[row].tagId = value.toString();
            return true;
        case 1:
            membershipInfoList[row].firstName = value.toString();
            return true;
        case 2:
            membershipInfoList[row].lastName = value.toString();
            return true;
        case 3:
            membershipInfoList[row].membershipNumber = value.toString();
            return true;
        case 4:
            membershipInfoList[row].caRegistration = value.toString();
            return true;
        case 5:
            membershipInfoList[row].eMail = value.toString();
            return true;
        case 6:
            membershipInfoList[row].sendReports = value.toBool();
            return true;
        }
    }
    return false;
}



Qt::ItemFlags CUsersTableModel::flags(const QModelIndex &index) const {
    return QAbstractTableModel::flags(index);
}





CUsers::CUsers(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CUsers)
{
    qDebug() << "CUsers::";
    QString s;
    ui->setupUi(this);
    changesMade = false;

    // Initialize users table

    usersTableModel = new CUsersTableModel(this);
    usersProxyModel = new QSortFilterProxyModel;
    usersProxyModel->setSourceModel(usersTableModel);
    usersProxyModel->setSortCaseSensitivity(Qt::CaseInsensitive);


    ui->usersTableView->setModel(usersProxyModel);
    ui->usersTableView->setColumnWidth(0, 150);
    ui->usersTableView->setColumnWidth(1, 100);
    ui->usersTableView->setColumnWidth(2, 100);
    ui->usersTableView->setColumnWidth(3, 100);
    ui->usersTableView->setColumnWidth(4, 100);
    ui->usersTableView->setColumnWidth(5, 100);

    ui->usersTableView->setAlternatingRowColors(true);
    ui->usersTableView->horizontalHeader()->setStretchLastSection(true);
    ui->usersTableView->horizontalHeader()->setStyleSheet("QHeaderView{font: bold;}");
    ui->usersTableView->sortByColumn(2, Qt::AscendingOrder);     // must come before call to setSortingEnabled()
    ui->usersTableView->setSortingEnabled(true);


    // Disable if database not open

    if (!mainWindow->membershipDbase.isOpen()) {
        ui->addPushButton->setEnabled(false);
        ui->editPushButton->setEnabled(false);
    }



    connect(ui->closePushButton, SIGNAL(clicked()), this, SLOT(onClosePushButtonClicked()));
    connect(ui->addPushButton, SIGNAL(clicked()), this, SLOT(onAddPushButtonClicked()));
    connect(ui->editPushButton, SIGNAL(clicked()), this, SLOT(onEditPushButtonClicked()));
//    connect(ui->removePushButton, SIGNAL(clicked()), this, SLOT(onRemovePushButtonClicked()));
//    connect(ui->deletePushButton, SIGNAL(clicked()), this, SLOT(onDeletePushButtonClicked()));
//    connect(ui->deskUpdatePushButton, SIGNAL(clicked()), this, SLOT(onDbaseUpdatePushButtonClicked()));
//    connect(ui->deskReadPushButton, SIGNAL(clicked(bool)), this, SLOT(onDbaseReadPushButtonClicked(bool)));
//    connect(ui->deskTagIdLineEdit, SIGNAL(textEdited(QString)), this, SLOT(onDbaseTagIdTextEdited(QString)));
//    connect(ui->deskFirstNameLineEdit, SIGNAL(textEdited(QString)), this, SLOT(onDbaseFirstNameTextEdited(QString)));
//    connect(ui->deskLastNameLineEdit, SIGNAL(textEdited(QString)), this, SLOT(onDbaseLastNameTextEdited(QString)));
//    connect(ui->deskMembershipNumberLineEdit, SIGNAL(textEdited(QString)), this, SLOT(onDbaseMembershipNumberTextEdited(QString)));
//    connect(ui->deskCaRegistrationLineEdit, SIGNAL(textEdited(QString)), this, SLOT(onDbaseCaRegistrationTextEdited(QString)));
//    connect(ui->deskEMailLineEdit, SIGNAL(textEdited(QString)), this, SLOT(onDbaseEMailTextEdited(QString)));

//    updateDbaseButtons();



//    updateSavePushButton();
}



CUsers::~CUsers() {
    delete ui;
}



// **********************************************************************************************************

void CUsers::onClosePushButtonClicked(void) {
    qDebug() << "close";
    hide();
}



void CUsers::onEditPushButtonClicked(void) {
    qDebug() << "edit";
//    uiUserEditor->show();
    QString tagId = uiUserEditor->tagIdLineEdit->text();
    QString firstName = uiUserEditor->firstNameLineEdit->text();
    QString lastName = uiUserEditor->lastNameLineEdit->text();
//    QString trackNumber = uiUserEditor->trackRegistrationNumberLineEdit->text();


    // If tagId contains entry, search based on only that.
    // If found, update fields.  Otherwise clear fields.

    CMembershipInfo info;
/*
    if (!tagId.isEmpty() && firstName.isEmpty() && lastName.isEmpty() && membershipNumber.isEmpty()) {
        int id = membershipDbase.getIdFromTagId(ui->deskTagIdLineEdit->text().toLatin1());
        if (id > 0) {
            tagInDbase = true;
            membershipDbase.getAllFromId(id, &info);
            ui->deskFirstNameLineEdit->setText(info.firstName);
            ui->deskLastNameLineEdit->setText(info.lastName);
            ui->deskMembershipNumberLineEdit->setText(info.membershipNumber);
            ui->deskCaRegistrationLineEdit->setText(info.caRegistration);
            ui->deskEMailLineEdit->setText(info.eMail);
            ui->sendReportsCheckBox->setChecked(info.sendReports);
        }
        else {
            tagInDbase = false;
            ui->deskFirstNameLineEdit->clear();
            ui->deskLastNameLineEdit->clear();
            ui->deskMembershipNumberLineEdit->clear();
            ui->deskCaRegistrationLineEdit->clear();
            ui->deskEMailLineEdit->clear();
            ui->sendReportsCheckBox->setChecked(false);
        }
    }

    // Else if first or last name given, search on that.  Don't clear names on search fail.

    else if ((!lastName.isEmpty() || !firstName.isEmpty()) && membershipNumber.isEmpty()) {
        int id = membershipDbase.getIdFromName(firstName, lastName);
        if (id > 0) {
            tagInDbase = true;
            membershipDbase.getAllFromId(id, &info);
            ui->deskTagIdLineEdit->setText(info.tagId);
            ui->deskFirstNameLineEdit->setText(info.firstName);
            ui->deskLastNameLineEdit->setText(info.lastName);
            ui->deskMembershipNumberLineEdit->setText(info.membershipNumber);
            ui->deskCaRegistrationLineEdit->setText(info.caRegistration);
            ui->deskEMailLineEdit->setText(info.eMail);
            ui->sendReportsCheckBox->setChecked(info.sendReports);
        }
        else {
            tagInDbase = false;
        }
    }

    // Else if membership number is given, search on that.  Don't clear names on search fail.

    else if ((lastName.isEmpty() && firstName.isEmpty()) && !membershipNumber.isEmpty()) {
        int id = membershipDbase.getIdFromMembershipNumber(membershipNumber);
        if (id > 0) {
            tagInDbase = true;
            membershipDbase.getAllFromId(id, &info);
            ui->deskTagIdLineEdit->setText(info.tagId);
            ui->deskFirstNameLineEdit->setText(info.firstName);
            ui->deskLastNameLineEdit->setText(info.lastName);
            ui->deskMembershipNumberLineEdit->setText(info.membershipNumber);
            ui->deskCaRegistrationLineEdit->setText(info.caRegistration);
            ui->deskEMailLineEdit->setText(info.eMail);
            ui->sendReportsCheckBox->setChecked(info.sendReports);
        }
        else {
            tagInDbase = false;
        }
    }

    entryEdited = false;
    updateDbaseButtons();
*/}



void CUsers::onAddPushButtonClicked(void) {
    qDebug() << "add";

    // Check whether tagId is already in dbase

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

/*

void MainWindow::onDbaseClearPushButtonClicked(void) {
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



void MainWindow::onDbaseRemovePushButtonClicked(void) {
    QMessageBox::StandardButtons b = guiQuestion("You are about to remove this tag entry from the database.  Press Ok to continue.", QMessageBox::Ok | QMessageBox::Abort);
    if (b == QMessageBox::Ok) {
        QString tagId(ui->deskTagIdLineEdit->text());
        if (membershipDbase.removeTagId(tagId) != 0) {
            guiCritical("Error removing name from database");
            return;
        }
        if (!membershipTableModel->remove(tagId)) {
            guiCritical("Error removing name from namesTable");
            return;
        }

        // Update name in activeRidersList so it appears in activeRidersTable

        for (int i=0; i<activeRidersTableModel->activeRidersList.size(); i++) {
            if (activeRidersTableModel->activeRidersList[i].tagId == tagId) {
                activeRidersTableModel->activeRidersList[i].name.clear();
                break;
            }
        }
    }


    entryEdited = false;
    onDbaseClearPushButtonClicked();
}



void MainWindow::onDbaseUpdatePushButtonClicked(void) {

    // Confirm we should be changing database

    if (guiQuestion("You are about to modify an existing tag entry in the database.  Press Ok to continue.", QMessageBox::Ok | QMessageBox::Abort) != QMessageBox::Ok)
        return;

    CMembershipInfo info;
    info.tagId = ui->deskTagIdLineEdit->text();
    info.firstName = ui->deskFirstNameLineEdit->text();
    info.lastName = ui->deskLastNameLineEdit->text();
    info.membershipNumber = ui->deskMembershipNumberLineEdit->text();
    info.caRegistration = ui->deskCaRegistrationLineEdit->text();
    info.eMail = ui->deskEMailLineEdit->text();
    info.sendReports = ui->sendReportsCheckBox->isChecked();

    // Update entry in database

    int rc = membershipDbase.update(info);
    if (rc != 0) {
        guiCritical("Could not update database: " + membershipDbase.errorText());
        return;
    }

    // Add to table

    if (!membershipTableModel->update(info)) {
        guiCritical("Could not update membershipTable");
        return;
    }


    // Update name in activeRidersList so it appears in activeRidersTable

    for (int i=0; i<activeRidersTableModel->activeRidersList.size(); i++) {
        if (activeRidersTableModel->activeRidersList[i].tagId == info.tagId) {
            if (info.firstName.isEmpty())
                activeRidersTableModel->activeRidersList[i].name = info.lastName;
            else
                activeRidersTableModel->activeRidersList[i].name = info.firstName + " " + info.lastName;
            break;
        }
    }


    entryEdited = false;
    onDbaseClearPushButtonClicked();
}



void MainWindow::onDbaseReadPushButtonClicked(bool state) {
    if (state) {
        ui->deskReadPushButton->setChecked(true);
        onDbaseClearPushButtonClicked();
        if (deskReader)
            deskReader->blockSignals(false);
    }
    else {
        if (deskReader)
            deskReader->blockSignals(true);
        ui->deskReadPushButton->setChecked(false);
    }

    entryEdited = false;
    updateDbaseButtons();
}



void MainWindow::onDbaseTagIdTextEdited(QString) {
    tagInDbase = false;
    entryEdited = true;
    updateDbaseButtons();
}



void MainWindow::onDbaseFirstNameTextEdited(QString) {
    entryEdited = true;
    updateDbaseButtons();
}



void MainWindow::onDbaseLastNameTextEdited(QString) {
    entryEdited = true;
    updateDbaseButtons();
}


void MainWindow::onDbaseMembershipNumberTextEdited(QString) {
    entryEdited = true;
    updateDbaseButtons();
}


void MainWindow::onDbaseCaRegistrationTextEdited(QString) {
    entryEdited = true;
    updateDbaseButtons();
}


void MainWindow::onDbaseEMailTextEdited(QString) {
    entryEdited = true;
    updateDbaseButtons();
}


void MainWindow::updateDbaseButtons(void) {

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



void MainWindow::onNamesTableClicked(const QModelIndex &index) {
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

*/


void CUsers::onNewDeskTag(CTagInfo tagInfo) {
    QString s;

    // Add string to messages window

    if (messagesWindow && messagesWindow->detailedMessagesIsChecked())
        messagesWindow->addMessage(s.sprintf("readerId=%d antennaId=%d timeStampUSec=%llu tagData=%s", tagInfo.readerId, tagInfo.antennaId, tagInfo.timeStampUSec, tagInfo.tagId.toLatin1().data()));

    // Tags from antennas not enabled are ignored

    if (!preferencesWindow || !preferencesWindow->trackReaderAntennaEnable[tagInfo.antennaId - 1])
        return;

    // Tags from antenna 0 are ignored

    if (tagInfo.antennaId == 0)
        return;

//    deskReader->blockSignals(true);
//    onDbaseClearPushButtonClicked();
//    ui->deskTagIdLineEdit->setText(tagInfo.tagId);
//    onDbaseSearchPushButtonClicked();
//    ui->deskReadPushButton->setChecked(false);

//    entryEdited = false;
//    updateDbaseButtons();
}





QList<bool> CUsers::parseBoolList(const QString &s) {
    QStringList sList = s.split(' ', QString::SkipEmptyParts);
    QList<bool> list;
    for (int i=0; i<sList.size(); i++) {
        list.append((bool)sList[i].toInt());
    }
    return list;
}



QList<int> CUsers::parseIntList(const QString &s) {
    QStringList sList = s.split(QRegExp("\\s"), QString::SkipEmptyParts);
    QList<int> list;
    for (int i=0; i<sList.size(); i++) {
        list.append(sList[i].toInt());
    }
    return list;
}



QList<float> CUsers::parseFloatList(const QString &s) {
    QStringList sList = s.split(QRegExp("\\s"), QString::SkipEmptyParts);
    QList<float> list;
    for (int i=0; i<sList.size(); i++) {
        list.append(sList[i].toFloat());
    }
    return list;
}



QList<double> CUsers::parseDoubleList(const QString &s) {
    QStringList sList = s.split(QRegExp("\\s"), QString::SkipEmptyParts);
    QList<double> list;
    for (int i=0; i<sList.size(); i++) {
        list.append(sList[i].toDouble());
    }
    return list;
}



QStringList CUsers::parseQuotedStringList(const QString &s) {
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




void CUsers::guiCritical(const QString &s) {
    if (messagesWindow)
        messagesWindow->addMessage("Critical: " + s);
    QMessageBox::critical(this, QCoreApplication::applicationName() + " Critical Error", s, QMessageBox::Ok);
}



int CUsers::guiQuestion(const QString &s, QMessageBox::StandardButtons b) {
    return ((MainWindow *)parent())->guiQuestion(s, b);
}

