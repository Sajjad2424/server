#include "mainwindow.h"
#include "QHBoxLayout"
#include "QVBoxLayout"
#include "QPushButton"
#include "QFileDialog"
#include "QComboBox"
#include "QDebug"
#include "QGridLayout"
#include "QStackedWidget"
#include "QTcpSocket"
#include "QGroupBox"
#include "QFormLayout"
#include "QMessageBox"
#include "QStringList"
#include "QLabel"
#include "QIntValidator"
#include "QTimer"
#include "QStringList"
#include "QtSerialPort/QSerialPortInfo"
#include "QVector"
#include "QSpinBox"
#include "QButtonGroup"
#include "QIntValidator"
#include <QRegularExpressionValidator>


QDataStream &operator<<(QDataStream &out, const dataStruct &data)
{
    out << data.h1 << data.h2 << data.h3 << data.h4 << data.h5<<data.progressBar ;
    return out;
}

QDataStream &operator>>(QDataStream &in, dataStruct &data)
{
    in >> data.h1 >> data.h2 >> data.h3 >> data.h4 >> data.h5 ;
    return in;
}


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    QWidget *mainWidget = new QWidget(this);


    QGroupBox *checkBoxGroupBox = new QGroupBox("Connection");
    QVBoxLayout *checkboxLayout = new QVBoxLayout();

    tcpCheckBox = new QCheckBox("Tcp");
    serialCheckBox = new QCheckBox("Serial");
    QButtonGroup *buttonGroup = new QButtonGroup();
    buttonGroup->addButton(serialCheckBox);
    buttonGroup->addButton(tcpCheckBox);
    buttonGroup->setExclusive(true);
    checkboxLayout->addWidget(tcpCheckBox);
    checkboxLayout->addWidget(serialCheckBox);
    checkBoxGroupBox->setLayout(checkboxLayout);

    QIntValidator *portValidator = new QIntValidator();
    portValidator->setRange(1,65535);

    // QString ipRange = "(?:[0-1]?[0-9]?[0-9]|2[0-4][0-9]|25[0-5])";
    // QRegularExpression ipRegex("^" + ipRange+ "(\\." + ipRange + "){3}$");
    // QRegularExpressionValidator *ipValidator = new QRegularExpressionValidator(ipRegex, this);

    ipportWidget = new QWidget();
    QGridLayout *ipPortGridLayout = new QGridLayout();

    QLabel *ipLabel = new QLabel("IP", this);
    QLabel *portLabel = new QLabel("Port", this);
    QLabel *timelabel1 = new QLabel("Time(s)");
    ipLineEdit = new QLineEdit("0.0.0.0");
    ipLineEdit->setInputMask("000.000.000.000");
    //ipLineEdit->setValidator(ipValidator);
    portLineEdit = new QLineEdit("8080");
    portLineEdit->setValidator(portValidator);
    tcpSpinBox = new QSpinBox();
    tcpSpinBox->setValue(5);
    tcpSpinBox->setRange(3,15);

    ipPortGridLayout->addWidget(ipLabel,0,0);
    ipPortGridLayout->addWidget(portLabel,0,1);
    ipPortGridLayout->addWidget(timelabel1,0,2);
    ipPortGridLayout->addWidget(ipLineEdit,1,0);
    ipPortGridLayout->addWidget(portLineEdit,1,1);
    ipPortGridLayout->addWidget(tcpSpinBox,1,2);
    ipportWidget->setLayout(ipPortGridLayout);

    serialWidget = new QWidget();
    QGridLayout *serialLayout = new QGridLayout();

    serialComboBox = new QComboBox();
    serialSpinBox = new QSpinBox();
    serialSpinBox->setSuffix("s");
    serialSpinBox->setRange(9,9);
    QLabel *comBoxLabel = new QLabel("Serial Name");
    QLabel *comBoxLabe2 = new QLabel("BaudRate");
    QLabel *timelabel = new QLabel("Timer");
    baudRateComboBox = new QComboBox();
    baudRateComboBox->addItems({"9600", "19200", "38400", "57600", "115200"});

    serialLayout->addWidget(comBoxLabel,0,0);
    serialLayout->addWidget(comBoxLabe2,0,1);
    serialLayout->addWidget(timelabel,0,2);
    serialLayout->addWidget(serialComboBox,1,0);
    serialLayout->addWidget(baudRateComboBox,1,1);
    serialLayout->addWidget(serialSpinBox,1,2);
    serialWidget->setLayout(serialLayout);

    QGroupBox *tcpSerialGroupBox = new QGroupBox("Config");
    QVBoxLayout *tcpSerialLayout = new QVBoxLayout();
    tcpSerialLayout->addWidget(ipportWidget);
    tcpSerialLayout->addWidget(serialWidget);
    tcpSerialGroupBox->setLayout(tcpSerialLayout);

    QGroupBox *connectionGrupBox = new QGroupBox();
    QGridLayout *connectionLayout = new QGridLayout();

    connectionLayout->addWidget(checkBoxGroupBox,0,0);
    connectionLayout->addWidget(tcpSerialGroupBox,1,0);
    connectionGrupBox->setLayout(connectionLayout);

    QGroupBox *buttonGroupBox = new QGroupBox();
    QGridLayout *buttonGridLayout = new QGridLayout();

    statusLabel = new QLabel(this);
    QPushButton *SelectPushbutton = new QPushButton("Select");
    QPushButton *sendPushbutton = new QPushButton("Send");
    stopPushButton = new QPushButton("Stop");

    buttonGridLayout->addWidget(SelectPushbutton,0,0);
    buttonGridLayout->addWidget(sendPushbutton,0,1);
    buttonGridLayout->addWidget(stopPushButton,0,2);
    buttonGridLayout->addWidget(statusLabel,1,0);
    buttonGroupBox->setLayout(buttonGridLayout);

    QGridLayout *mainLayout = new QGridLayout();
    mainLayout->addWidget(connectionGrupBox,0,0);
    mainLayout->addWidget(buttonGroupBox,1,0);

    mainWidget->setLayout(mainLayout);
    setCentralWidget(mainWidget);
    setFixedSize(500, 480);

    setting = new QSettings("/home/myuser/Repos/server/MyApp.ini",QSettings::IniFormat);
    connect(tcpSpinBox,&QSpinBox::textChanged,this,&MainWindow::saveSetting);
    QString spinBoxSetting = setting->value("spinbox","").toString();

    tcpTimer = new QTimer(this);
    serialTimer = new QTimer(this);
    tryTimer = new QTimer(this);
    tryTimer->setInterval(spinBoxSetting.toInt()*1000);
    socket = new QTcpSocket(this);
    serialPortTest = new QSerialPort(this);

    connect(tcpTimer, &QTimer::timeout, this, &MainWindow::sendNextLine);
    connect(serialTimer, &QTimer::timeout, this, &MainWindow::sendNextLineSerial);
    connect(tryTimer,&QTimer::timeout,this,&MainWindow::connectionLostSignal);

    connect(sendPushbutton, &QPushButton::clicked, this, &MainWindow::sendingData);
    connect(SelectPushbutton, &QPushButton::clicked, this, &MainWindow::OpenFile);
    connect(stopPushButton, &QPushButton::clicked, this, &MainWindow::stopSerialPort);

    connect(tcpCheckBox, &QCheckBox::clicked, this, &MainWindow::checkBoxes);
    connect(serialCheckBox, &QCheckBox::clicked, this, &MainWindow::checkBoxes);

    QString path = "/home/myuser/Desktop/Numbers.txt";
    readingFile(path);
    loadProts();

    machine = new QStateMachine(this);

    stateDisconnect = new QState();
    stateConnecting = new QState();
    stateStopped = new QState();
    stateConnected = new QState();

    //بعد دکمه سند اگر چک باکس  تی سی پی  تیک خورده بود
    stateDisconnect->addTransition(this, &MainWindow::connectingSignal, stateConnecting);

    stateConnecting->addTransition(socket, &QTcpSocket::connected, stateConnected);
    stateConnecting->addTransition(this, &MainWindow::notConnectedSignal, stateDisconnect);

    stateConnected->addTransition(stopPushButton, &QPushButton::clicked, stateStopped);
    stateConnected->addTransition(sendPushbutton,&QPushButton::clicked,stateConnecting);
    stateConnected->addTransition(this, &MainWindow::connectionLostSignal, stateConnecting);

    stateStopped->addTransition(this, &MainWindow::disConnectedSignal, stateDisconnect);
    stateStopped->addTransition(this, &MainWindow::notDisConnectedSingal, stateConnected);

    connect(stateDisconnect, &QState::entered, [this]() {
        tcpTimer->stop();
        statusLabel->setText("State Disconnect");
        qDebug() << "state Disconnect ";
    });

    connect(stateConnecting, &QState::entered, [this]() {   
        qDebug() << "state Connecting ...";
        socket->abort();
        socket->connectToHost(ipLineEdit->text(), portLineEdit->text().toUShort());
        emit  connectinonCheck();

    });

    connect(stateConnected, &QState::entered, [this]() {
        tcpTimer->start(2000);
        qDebug() << "state Connected: timer Start";

    });

    connect(stateStopped, &QState::entered, [this]() {
        socket->abort();
        //  tcpTimer->stop();
        disConnectCheck();
        qDebug() << "state stopped";

    });

    machine->addState(stateConnected);
    machine->addState(stateConnecting);
    machine->addState(stateDisconnect);
    machine->addState(stateStopped);
    machine->setInitialState(stateDisconnect);
    machine->start();
}


void MainWindow::connectinonCheck()
{
    if (!socket->waitForConnected(2000))
    {
        emit notConnectedSignal();   //تغیر به state DisConnecet
        return;
    }
}

void MainWindow::disConnectCheck()
{
    // اطمینان از قطع شدن اتصال در stateStopped
    if (socket->state() == QAbstractSocket::UnconnectedState)
    {
        emit disConnectedSignal();
    }
    else
    {
        notDisConnectedSingal();
    }
}


void MainWindow::OpenFile()
{
    QString filetxt = QFileDialog::getOpenFileName(this,"select file", "/home/myuser/Desktop/","Text Files (*.txt);;All Files (*)");
    if(!filetxt.isEmpty())
    {
        readingFile(filetxt);
        currentLineIndex=0;
    }
    else
    {
        QMessageBox::warning(this,"Error", "No file selected");
        return;
    }
}

void MainWindow::readingFile(const QString &filePath)
{
    dataRows.clear();
    QFile mainFile(filePath);
    if(!mainFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QMessageBox::warning(this,"warning","Unable to Open The File");
        return;
    }

    QTextStream in(&mainFile);

    while (!in.atEnd())
    {
        QString line = in.readLine();
        QStringList parts = line.split(',');

        if (parts.size() == 5)
        {
            dataStruct newRow;
            newRow.h1 = parts[0].toDouble();
            newRow.h2 = parts[1].toDouble();
            newRow.h3 = parts[2].toDouble();
            newRow.h4 = parts[3].toDouble();
            newRow.h5 = parts[4].toDouble();
            dataRows.append(newRow);
        }
    }
    mainFile.close();
}


void MainWindow::checkBoxes()
{
    if(tcpCheckBox->isChecked())
    {
        serialWidget->setEnabled(false);
    }
    else
    {
        serialWidget->setEnabled(true);
    }

    if(serialCheckBox->isChecked())
    {
        ipportWidget->setEnabled(false);
    }
    else
    {
        ipportWidget->setEnabled(true);
    }
}


void MainWindow::sendingData()
{
    if (!tcpCheckBox->isChecked() && !serialCheckBox->isChecked())
    {
        statusLabel->setText("Select a check Box First");
        return;
    }

    if (dataRows.isEmpty())
    {
        QMessageBox::warning(this, "Warning", "Select a File First");
        // qDebug() << "No data loaded!";
        return;
    }

    if(serialCheckBox->isChecked())
    {
        // emit connectionLostSignal();
        // qDebug() << "Send via SerialPort";
        openPort();
        sendViaSerialPort();
        tcpTimer->stop();

    }
    if(tcpCheckBox->isChecked())
    {
        qDebug() << "Send via Tcp";
        emit connectingSignal();
        serialTimer->stop();
    }
}

void MainWindow::sendNextLine()
{
    int progress = ((currentLineIndex + 1) * 100 ) / dataRows.size();

    if (currentLineIndex < dataRows.size())
    {
        dataStruct &currentData = dataRows[currentLineIndex];
        currentData.progressBar=progress;

        QDataStream out(socket);
        out.setVersion(QDataStream::Qt_5_13);
        out << currentData;

        qDebug() << "Data:"  << currentData.h1 << currentData.h2 << currentData.h3 << currentData.h4 << currentData.h5 <<"Progres"<<currentData.progressBar;

        statusLabel->setText(QString("Line %1: %2 %3 %4 %5 %6")
                                 .arg(currentLineIndex +1)
                                 .arg(currentData.h1)
                                 .arg(currentData.h2)
                                 .arg(currentData.h3)
                                 .arg(currentData.h4)
                                 .arg(currentData.h5));

        if (socket->bytesAvailable())
        {
            QByteArray ack =socket->readAll();
            QString message(ack);
        }
        else
        {
            tcpTimer->stop();
            tryTimer->start();
            statusLabel->setText("Try again in "+ QString::number(tcpSpinBox->value())+ " Second");
            return;
        }
        currentLineIndex++;
    }

    else
    {
        tcpTimer->stop();
        statusLabel->setText("All Line Sent");
        currentLineIndex = 0;
    }
}


MainWindow::~MainWindow()
{
}


void MainWindow::loadProts()
{
    foreach (auto port, QSerialPortInfo::availablePorts())
    {
        qDebug()<< port.portName();
        serialComboBox->addItem(port.portName());
    }
}


void MainWindow::openPort()
{

    if (serialPortTest !=nullptr )
    {
        serialPortTest->close();
        delete serialPortTest;
    }

    serialPortTest = new QSerialPort();
    serialPortTest->setPortName(serialComboBox->currentText());
    serialPortTest->setBaudRate(baudRateComboBox->currentText().toUInt());
    serialPortTest->setParity(QSerialPort::EvenParity);
    serialPortTest->setStopBits(QSerialPort::OneStop);
    serialPortTest->setDataBits(QSerialPort::Data8);

    if(serialPortTest->open(QIODevice::ReadWrite))
    {
        statusLabel->setText("Port is open");
    }
    else
    {
        QMessageBox::critical(this,"Warining","Port Can't be open");
    }

}

void MainWindow::sendViaSerialPort()
{
    if(tcpTimer->isActive())
    {
        tcpTimer->stop();
        qDebug() << "TCP Timer Stopped";
    }

    if(!serialPortTest->isOpen())
    {
        statusLabel->setText("Port is not Open");
        qDebug() << "Port is not Open";
        return;
    }

    if (dataRows.isEmpty())
    {
        //QMessageBox::warning(this, "Warning", "Select a File First");
        statusLabel->setText("No data loaded");
        qDebug() << "No data loaded!";
        return;
    }

    serialTimer->start(2000);
}

void MainWindow::sendNextLineSerial()
{
    if (currentLineIndex >= dataRows.size())
    {
        statusLabel->setText("All Line Sent");
        currentLineIndex = 0;
        serialTimer->stop();
        return;
    }

    dataStruct currentData = dataRows[currentLineIndex];

    QString lineText = QString("%1 %2 %3 %4 %5\n")
                           .arg(currentData.h1)
                           .arg(currentData.h2)
                           .arg(currentData.h3)
                           .arg(currentData.h4)
                           .arg(currentData.h5);

    serialPortTest->write(lineText.toUtf8());

    if(!serialPortTest->waitForReadyRead(serialSpinBox->value()*1000))
    {
        stopSerialPort();
        return;
    }

    qDebug() << "Sending line:" << currentLineIndex +1 << "Data:"
             << currentData.h1 << currentData.h2 << currentData.h3 << currentData.h4 << currentData.h5;
    statusLabel->setText(QString("Serial Line %1: %2").arg(currentLineIndex + 1).arg(lineText));
    currentLineIndex++;
    return;
}


void MainWindow::stopSerialPort()
{
    serialPortTest->close();
    serialTimer->stop();
    statusLabel->setText("Sending Stopped");
    qDebug() << "Sending Stopped";
}


void MainWindow::saveSetting()
{
    setting->setValue("spinbox",tcpSpinBox->text());
}

