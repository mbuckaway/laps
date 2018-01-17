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
#include "csmtp.h"




namespace Ui {
class MainWindow;
}


#include <QAbstractTableModel>


class MainWindow;



class CMembershipTableModel : public QAbstractTableModel {
    Q_OBJECT
public:
    CMembershipTableModel(QObject *parent);
    QList<CMembershipInfo> membershipInfoList;
    bool add(const CMembershipInfo &info);
    bool update(const CMembershipInfo &info);
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
    void newTrackTag(const CTagInfo &tagInfo);
    QList<CRider> purgeTable(void);
private:
    MainWindow *mainWindow;
private slots:
signals:
};


class CLapsTableModel : public QAbstractTableModel {
    Q_OBJECT
public:
    CLapsTableModel(QObject *parent);
    bool add(CRider rider);
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




class CScheduleItem {
public:
    CScheduleItem(void);
    QString day;
    QString activity;
    QString startTime;
    QString endTime;
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
//    QList<CLapsDbase *> oldLapsDbaseList;
    float maxAcceptableLapSec;        // max time allowable for lap.  If greater, rider must have left and return to track
    QList<float> trackLengthM;      // length of track (1 lap) at height of each antenna
    float blackLineDistancem;
    float blueLineDistancem;
    void guiCritical(QString);
    void guiInformation(QString);
    QMessageBox::StandardButtons guiQuestion(QString s, QMessageBox::StandardButtons b=QMessageBox::Ok);
    CLapsTableModel *lapsTableModel;
    QSortFilterProxyModel *lapsProxyModel;
    QFile *logFile;
    QTextStream *logTextStream;
    int bestLapCountInSession;
    float bestLapSpeedInSession;
private:
    CSmtp *smtp;
    QList<CRider> purgedRiders;
    CMembershipTableModel *membershipTableModel;
    QSortFilterProxyModel *membershipProxyModel;
    CActiveRidersTableModel *activeRidersTableModel;
    QSortFilterProxyModel *activeRidersProxyModel;
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
    bool tagInDbase;
    bool entryEdited;
    void sendInactiveRiderReports(void);
    void prepareNextReport(void);
    void sendReport(const CMembershipInfo &info, const QString &body);
    QList<CMembershipInfo> membershipInfoNotReported;
    unsigned int dateTimeOfReportStart;
    unsigned int dateTimeOfReportEnd;
    QString getSession(const QDateTime &);
signals:
    void newTrackTag(CTagInfo tagInfo);
public slots:
    void updateDbaseButtons(void);
    void onDbaseSearchPushButtonClicked(void);
    void onDbaseAddPushButtonClicked(void);
    void onDbaseClearPushButtonClicked(void);
    void onDbaseRemovePushButtonClicked(void);
    void onDbaseUpdatePushButtonClicked(void);
    void onDbaseReadPushButtonClicked(bool);
    void onDbaseTagIdTextEdited(QString);
    void onDbaseFirstNameTextEdited(QString);
    void onDbaseLastNameTextEdited(QString);
    void onDbaseMembershipNumberTextEdited(QString);
    void onDbaseCaRegistrationTextEdited(QString);
    void onDbaseEMailTextEdited(QString);
private slots:
    void onReaderConnected(void);
    void onClockTimerTimeout(void);
    void onPurgeActiveRidersList(void);
    void onNewTrackTag(CTagInfo);
    void onNewDeskTag(CTagInfo);
    void onNewLogMessage(QString);
    void onLapsTableSortEnableCheckBoxClicked(bool);
    void onActiveRidersTableSortEnableCheckBoxClicked(bool);
//    void onAntenna1ComboBoxActivated(int);
    void onApplySettingsPushButtonClicked(void);
    void onSaveSettingsPushButtonClicked(void);
    void onSaveSessionsPushButtonClicked(void);
    void onTrackAntenna1PowerComboBoxActivated(int);
    void onMailSent(int);
    void onEMailTestPushButtonClicked(void);
    void onTestMailSent(void);
};

#endif // MAINWINDOW_H
