#ifndef CUsers_H
#define CUsers_H

#include <QWidget>
#include <QSettings>
#include <QMessageBox>
#include <QAbstractTableModel>
#include <cdbase.h>
#include <creader.h>
#include <ui_cusers.h>
#include <ui_cusereditor.h>



namespace Ui {
class CUsers;
class CUserEditor;
}


class CUsersTableModel : public QAbstractTableModel {
    Q_OBJECT
public:
    CUsersTableModel(QObject *parent);
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
 //   MainWindow *mainWindow;
private slots:
signals:
};


class CUsers : public QWidget
{
    Q_OBJECT
public:
    explicit CUsers(QWidget *parent = 0);
    ~CUsers();
private:
    Ui::CUsers *ui;
    Ui::CUserEditor *uiUserEditor;
    bool changesMade;
    CUsersTableModel *usersTableModel;
    QSortFilterProxyModel *usersProxyModel;
    QList<bool> parseBoolList(const QString &s);
    QList<int> parseIntList(const QString &s);
    QList<float> parseFloatList(const QString &s);
    QList<double> parseDoubleList(const QString &s);
    QStringList parseQuotedStringList(const QString &s);
    void guiCritical(const QString &);
    int guiQuestion(const QString &, QMessageBox::StandardButtons);
    void updateSavePushButton(void);
signals:
public slots:
private slots:
    void onNewDeskTag(CTagInfo);
    void onClosePushButtonClicked(void);
//    void updateButtons(void);
//    void onSearchPushButtonClicked(void);
    void onAddPushButtonClicked(void);
    void onEditPushButtonClicked(void);
//    void onRemovePushButtonClicked(void);
//    void onUpdatePushButtonClicked(void);
//    void onReadPushButtonClicked(bool);
//    void onTagIdTextEdited(QString);
//    void onFirstNameTextEdited(QString);
//    void onLastNameTextEdited(QString);
//    void onMembershipNumberTextEdited(QString);
//    void onCaRegistrationTextEdited(QString);
//    void onEMailTextEdited(QString);
};

#endif // CUsers_H
