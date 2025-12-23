#ifndef MAINWINDOW_H
#define MAINWINDOW_H


#include <QMainWindow>
#include <QLineEdit>
#include <QLabel>
#include <QTcpSocket>
#include <QLabel>
#include <QSerialPort>
#include "QCheckBox"
#include "QComboBox"
#include"QVector"
#include "QStateMachine"
#include "QState"
#include "QPushButton"
#include "QGroupBox"
#include "QHBoxLayout"
#include "QSpinBox"
#include <QSettings>


struct dataStruct
{
    double h1;
    double h2;
    double h3;
    double h4;
    double h5;
    // QVector<double> dataVector;
    int progressBar;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

signals:
    void connectionLostSignal();
    void disConnectedSignal();
    void notDisConnectedSingal();
    void notConnectedSignal();
    void connectingSignal();

private:
    void OpenFile();
    void readingFile(const QString &filePath);
    void sendingData();
    void sendNextLine();
    void checkConnection();
    void stopSendig();
    void loadProts();
    void openPort();
    void sendViaSerialPort();
    void sendNextLineSerial();
    void stopSerialPort();
    void connectinonCheck();
    void disConnectCheck();
    void checkBoxes();
    void saveSetting();
    void loadStting();





private:
    QWidget *mainWidget;
    QLineEdit *portLineEdit;
    QLineEdit *ipLineEdit;
    QLineEdit *timeLineEdit;
    QLabel *statusLabel;
    QComboBox *serialComboBox;
    QComboBox *baudRateComboBox;
    QPushButton *stopPushButton;
    QCheckBox *tcpCheckBox;
    QCheckBox *serialCheckBox;
    QSpinBox *serialSpinBox;
    QSpinBox *tcpSpinBox;

    QWidget *serialWidget;
    QWidget *ipportWidget;

    QTimer *tcpTimer ;
    QTimer *serialTimer;
    QTimer *tryTimer;
    int currentLineIndex = 0;
    QTcpSocket *socket;
    QSerialPort *serialPortTest;

    QList<dataStruct> dataRows;

    QStateMachine *machine;
    QState *stateConnecting;
    QState *stateDisconnect;
    QState *stateStopped;
    QState *stateConnected;

    QSettings *setting;



};
#endif // MAINWINDOW_H
