// mainwindow.h
//


#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QMutex>
#include <QDateTime>
#include <QTableView>
#include <QLabel>
#include <QPushButton>
#include <QTableWidgetItem>
#include <QMessageBox>


#include "creader.h"
#include "cdbase.h"




namespace Ui {
class MainWindow;
}


#include <QAbstractTableModel>


class MainWindow;


class CMembershipTableModel : public QAbstractTableModel {
    Q_OBJECT
public:
    CMembershipTableModel(QObject *parent);
    QStringList tagIdList;
    QStringList firstNameList;
    QStringList lastNameList;
    QStringList membershipList;
    QStringList caRegistrationList;
    QStringList eMailList;
    bool add(const QString &tagId, const QString &firstName, const QString &lastName, const QString &membership, const QString &caRegistration, const QString &email);
    bool remove(const QString &tagId);
    int rowCount(const QModelIndex &parent=QModelIndex()) const Q_DECL_OVERRIDE;
    int columnCount(const QModelIndex &parent=QModelIndex()) const Q_DECL_OVERRIDE;
    QVariant data(const QModelIndex &index, int role=Qt::DisplayRole) const Q_DECL_OVERRIDE;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role) Q_DECL_OVERRIDE;
    Qt::ItemFlags flags(const QModelIndex &index) const Q_DECL_OVERRIDE;
    bool insertRows(int position, int rows, const QModelIndex &index=QModelIndex());
    bool removeRows(int position, int rows, const QModelIndex &index=QModelIndex());
private:
    MainWindow *mainWindow;
private slots:
signals:
};


class CActiveRidersTableModel : public QAbstractTableModel {
    Q_OBJECT
public:
    CActiveRidersTableModel(QObject *parent);
    QList<CRider> activeRidersList;
    int rowCount(const QModelIndex &parent=QModelIndex()) const Q_DECL_OVERRIDE;
    int columnCount(const QModelIndex &parent=QModelIndex()) const Q_DECL_OVERRIDE;
    QVariant data(const QModelIndex &index, int role=Qt::DisplayRole) const Q_DECL_OVERRIDE;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role) Q_DECL_OVERRIDE;
    Qt::ItemFlags flags(const QModelIndex &index) const Q_DECL_OVERRIDE;
    bool insertRows(int position, int rows, const QModelIndex &index=QModelIndex());
    bool removeRows(int position, int rows, const QModelIndex &index=QModelIndex());
    void newTrackTag(CTagInfo tagInfo);
    void purgeTable(void);
private:
    MainWindow *mainWindow;
private slots:
signals:
};


class CLapsTableModel : public QAbstractTableModel {
    Q_OBJECT
public:
    CLapsTableModel(QObject *parent);
    bool addEntry(CRider rider);
    QStringList nameList;
    QList<int> lapList;
    QStringList timeList;
    QList<long long> timeStampList;
    QList<float> lapSecList;
    QList<float> lapSpeedList;
    QStringList commentList;
    int rowCount(const QModelIndex &parent=QModelIndex()) const Q_DECL_OVERRIDE;
    int columnCount(const QModelIndex &parent=QModelIndex()) const Q_DECL_OVERRIDE;
    QVariant data(const QModelIndex &index, int role=Qt::DisplayRole) const Q_DECL_OVERRIDE;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    Qt::ItemFlags flags(const QModelIndex &index) const Q_DECL_OVERRIDE;
    bool insertRows(int position, int rows, const QModelIndex &index=QModelIndex());
    bool removeRows(int position, int rows, const QModelIndex &index=QModelIndex());
    void purgeTable(void);
private:
    MainWindow *mainWindow;
private slots:
signals:
};


class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    Ui::MainWindow *ui;
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    CMembershipDbase membershipDbase;
    CLapsDbase lapsDbase;
    float maxAcceptableLapSec;        // max time allowable for lap.  If greater, rider must have left and return to track
    QList<float> trackLengthM;      // length of track (1 lap) at height of each antenna
    float blackLineDistancem;
    float blueLineDistancem;
    void guiCritical(QString);
    void guiInformation(QString);
    QMessageBox::StandardButtons guiQuestion(QString s, QMessageBox::StandardButtons b=QMessageBox::Ok);
    long long tablePurgeIntervalSec;
    CLapsTableModel *lapsTableModel;
private:
    CMembershipTableModel *membershipTableModel;
    CActiveRidersTableModel *activeRidersTableModel;
    QTimer clockTimer;
    QTimer purgeActiveRidersListTimer;
    CReader *trackReader;
    CReader *deskReader;
    QList<QThread *> readerThreadList;
    QMutex lapsTableMutex;
    QMutex activeRidersTableMutex;
    bool lapsTableSortingEnabled;
    bool activeRidersTableSortingEnabled;
    int lapsTableMaxSizeWithSort;
    float lapSpeed(float lapSec, float lapM);
    QSettings settings;
    void initializeSettingsPanel(void);
public slots:
    void updateDbaseButtons(void);
    void onDbaseSearchPushButtonClicked(void);
    void onDbaseAddPushButtonClicked(void);
    void onDbaseClearPushButtonClicked(void);
    void onDbaseRemovePushButtonClicked(void);
    void onDbaseUpdatePushButtonClicked(void);
    void onDbaseReadPushButtonClicked(bool);
    void onDbaseTagIdTextChanged(QString);
    void onDbaseFirstNameTextChanged(QString);
    void onDbaseLastNameTextChanged(QString);
    void onDbaseMembershipNumberTextChanged(QString);
    void onDbaseCaRegistrationTextChanged(QString);
    void onDbaseEMailTextChanged(QString);
private slots:
    void onReaderConnected(void);
    void onClockTimerTimeout(void);
    void onPurgeActiveRidersList(void);
    void onNewTrackTag(CTagInfo);
    void onNewDeskTag(CTagInfo);
    void onNewLogMessage(QString);
    void onLapsTableSortedCheckBoxClicked(bool);
    void onActiveRidersTableSortedCheckBoxClicked(bool);
//    void onAntenna1ComboBoxActivated(int);
    void onApplySettingsPushButtonClicked(void);
    void onSaveSettingsPushButtonClicked(void);
    void onTrackAntenna1PowerComboBoxActivated(int);
};

#endif // MAINWINDOW_H
