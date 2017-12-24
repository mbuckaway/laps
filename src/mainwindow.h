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


class CNamesModel : public QAbstractTableModel {
    Q_OBJECT
public:
    CNamesModel(QObject *parent);
    QStringList tagIdList;
    QStringList firstNameList;
    QStringList lastNameList;
    void loadNames(void);
    void addName(QByteArray tagId, QByteArray firstName, QByteArray lastName);
    int rowCount(const QModelIndex &parent=QModelIndex()) const Q_DECL_OVERRIDE;
    int columnCount(const QModelIndex &parent=QModelIndex()) const Q_DECL_OVERRIDE;
    QVariant data(const QModelIndex &index, int role=Qt::DisplayRole) const Q_DECL_OVERRIDE;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role) Q_DECL_OVERRIDE;
    Qt::ItemFlags flags(const QModelIndex &index) const Q_DECL_OVERRIDE;
    bool insertRows(int position, int rows, const QModelIndex &index=QModelIndex());
    bool removeRows(int position, int rows, const QModelIndex &index=QModelIndex());
private:
private slots:
signals:
//    void dataChanged(void);
};




class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    CDbase dbase;
private:
    Ui::MainWindow *ui;
    CNamesModel *namesModel;
    QTimer clockTimer;
    QTimer purgeActiveRidersListTimer;
    CReader *trackReader;
    CReader *deskReader;
    QList<QThread *> readerThreadList;
    QList<CRider> activeRidersList;
    QMutex lapsTableMutex;
    QMutex activeRidersTableMutex;
    long long tablePurgeIntervalSec;
    float maxAcceptableLapSec;        // max time allowable for lap.  If greater, rider must have left and return to track
    float blackLineDistancem;
    float blueLineDistancem;
    bool lapsTableSortingEnabled;
    bool activeRidersTableSortingEnabled;
    int lapsTableMaxSizeWithSort;
    void guiCritical(QString);
    void guiInformation(QString);
    QMessageBox::StandardButtons guiQuestion(QString s, QMessageBox::StandardButtons b=QMessageBox::Ok);
    float lapSpeed(float lapSec, float lapM);
    QList<float> trackLengthM;      // length of track (1 lap) at height of each antenna
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
private slots:
    void onReaderConnected(void);
    void onClockTimerTimeout(void);
    void onPurgeActiveRidersList(void);
    void onNewTrackTag(CTagInfo);
    void onNewDeskTag(CTagInfo);
    void onNewLogMessage(QString);
    void onLapsTableHorizontalHeaderSectionClicked(int);
    void onActiveRidersTableHorizontalHeaderSectionClicked(int);
    void onLapsTableSortedCheckBoxClicked(bool);
    void onActiveRidersTableSortedCheckBoxClicked(bool);
//    void onAntenna1ComboBoxActivated(int);
    void onApplySettingsPushButtonClicked(void);
    void onSaveSettingsPushButtonClicked(void);
    void onTrackAntenna1PowerComboBoxActivated(int);
};

#endif // MAINWINDOW_H
