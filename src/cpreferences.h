#ifndef CPreferences_H
#define CPreferences_H

#include <QWidget>
#include <QSettings>
#include <QMessageBox>



namespace Ui {
class CPreferences;
}


class CPreferences : public QWidget
{
    Q_OBJECT
public:
    explicit CPreferences(QWidget *parent = 0);
    ~CPreferences();
    void loadSettings(void);
    void saveSettings(void);
    QString trackName;
    float tablePurgeIntervalHours;
    float emailReportLatencyHours;
    QString trackReaderIp;
    QString deskReaderIp;
    QList<bool> trackReaderAntennaEnable;
    QList<bool> deskReaderAntennaEnable;
    QList<float> trackReaderAntennaPower;
    QList<int> deskReaderAntennaPower;
    QList<float> trackLengthM;
    QStringList deskReaderLocation;
    void setTrackReaderAntennaPowerComboBox(const QList<int> &);
    void setDeskReaderAntennaPowerComboBox(const QList<int> &);
private:
    Ui::CPreferences *ui;
    QSettings settings;
    bool changesMade;
    void loadPreferences(void);
    QList<bool> parseBoolList(const QString &s);
    QList<int> parseIntList(const QString &s);
    QList<float> parseFloatList(const QString &s);
    QList<double> parseDoubleList(const QString &s);
    QStringList parseQuotedStringList(const QString &s);
    void guiCritical(const QString &);
    int guiQuestion(const QString &, QMessageBox::StandardButtons);
    void updateSavePushButton(void);
signals:
    void newLogMessage(const QString &);
public slots:
    void onTrackReaderConnected(void);
    void onDeskReaderConnected(void);
    void onTrackReaderStatus(QString);
    void onDeskReaderStatus(QString);
private slots:
    void onTrackNameLineEditTextEdited(QString);
    void onTablePurgeIntervalDoubleSpinBoxValueChanged(double);
    void onTrackReaderIpLineEditTextEdited(QString);
    void onTrackAntenna1TransmitPowerComboBoxActivated(int);
    void onTrackAntenna2TransmitPowerComboBoxActivated(int);
    void onTrackAntenna3TransmitPowerComboBoxActivated(int);
    void onTrackAntenna4TransmitPowerComboBoxActivated(int);
    void onDeskAntenna1TransmitPowerComboBoxActivated(int);
    void onDeskAntenna2TransmitPowerComboBoxActivated(int);
    void onDeskAntenna3TransmitPowerComboBoxActivated(int);
    void onDeskAntenna4TransmitPowerComboBoxActivated(int);
    void onTrackReaderAntenna1EnableCheckBoxClicked(bool);
    void onTrackReaderAntenna2EnableCheckBoxClicked(bool);
    void onTrackReaderAntenna3EnableCheckBoxClicked(bool);
    void onTrackReaderAntenna4EnableCheckBoxClicked(bool);
    void onTrackLength1Edited(QString text=QString());
    void onTrackLength2Edited(QString text=QString());
    void onTrackLength3Edited(QString text=QString());
    void onTrackLength4Edited(QString text=QString());
    void onDeskReaderAntenna1EnableCheckBoxClicked(bool);
    void onDeskReaderAntenna2EnableCheckBoxClicked(bool);
    void onDeskReaderAntenna3EnableCheckBoxClicked(bool);
    void onDeskReaderAntenna4EnableCheckBoxClicked(bool);
    void onSavePreferencesPushButtonClicked(void);
    void onClosePreferencesPushButtonClicked(void);
    void onEMailTestSendPushButtonClicked(void);
};

#endif // CPreferences_H
