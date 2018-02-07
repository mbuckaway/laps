#ifndef CMembers_H
#define CMembers_H

#include <QWidget>
#include <QMessageBox>

#include "creader.h"
#include "mainwindow.h"


namespace Ui {
class CMembers;
}





//class CMembershipTableModel : public QAbstractTableModel {
//    Q_OBJECT
//public:
//    CMembershipTableModel(QObject *parent);
//    QList<CMembershipInfo> membershipInfoList;
//    bool add(const CMembershipInfo &info);
//    bool update(const CMembershipInfo &info);
//    bool remove(const QString &tagId);
//    int rowCount(const QModelIndex &parent=QModelIndex()) const Q_DECL_OVERRIDE;
//    int columnCount(const QModelIndex &parent=QModelIndex()) const Q_DECL_OVERRIDE;
//    QVariant data(const QModelIndex &index, int role=Qt::DisplayRole) const Q_DECL_OVERRIDE;
//    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
//    bool setData(const QModelIndex &index, const QVariant &value, int role) Q_DECL_OVERRIDE;
//    Qt::ItemFlags flags(const QModelIndex &index) const Q_DECL_OVERRIDE;
//    bool insertRows(int position, int rows, const QModelIndex &index=QModelIndex());
//    bool removeRows(int position, int rows, const QModelIndex &index=QModelIndex());
//private:
////    MainWindow *mainWindow;
//private slots:
//signals:
//};




class CMembers : public QWidget
{
    Q_OBJECT
public:
    explicit CMembers(QWidget *parent = 0);
    ~CMembers();
private:
    Ui::CMembers *ui;
    bool tagInDbase;
    bool entryEdited;
    CMembershipTableModel *membershipTableModel;
    void guiCritical(const QString &, QMessageBox::StandardButtons b=QMessageBox::Ok);
    void guiInformation(const QString &, QMessageBox::StandardButtons b=QMessageBox::Ok);
    QMessageBox::StandardButtons guiQuestion(const QString &, QMessageBox::StandardButtons b=QMessageBox::Ok);
signals:
    void blockReaderSignals(bool);
public slots:
    void onNewDeskTag(CTagInfo);
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
    void onNamesTableClicked(const QModelIndex &);
};

#endif // CMembers_H
