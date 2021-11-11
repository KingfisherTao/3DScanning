#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "settingsdialog.h"
#include "qextserialport/qextserialport.h"

#include "myhelper.h"
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_status(new QLabel)
{
    ui->setupUi(this);
    m_settings = new SettingsDialog(ui->Lab_COMName, this);

    initUI();
    initConnect();

    myHelper::formInCenter(this);
}

MainWindow::~MainWindow()
{
    delete m_settings;
    delete ui;
}

void MainWindow::initUI()
{
    this->setWindowTitle(AppName);
    ui->statusbar->addWidget(m_status);
    setLinkState(false);

    const SettingsDialog::Settings p = m_settings->settings();
    ui->Lab_COMName->setText(p.name);

    for (int i = 0; i < 3; ++i)
    {
        ui->CB_ControllerNum->addItem(QString::number(i), i);
    }
    QStringList _SW;
    _SW << "常闭" << "常开";
    ui->CB_LimitSW->addItems(_SW);
    ui->CB_ScramSW->addItems(_SW);
    ui->CB_LimitSW->setCurrentIndex(1);
    ui->CB_ScramSW->setCurrentIndex(1);

    ui->CB_MotorPattern->addItem("模式1：点动模式");
    ui->CB_MotorPattern->addItem("模式2：单步模式(上位机实时控制首选)");
    ui->CB_MotorPattern->addItem("模式3：自动往返模式");
    ui->CB_MotorPattern->addItem("模式4：单次往返模式");
    ui->CB_MotorPattern->addItem("模式5：按次往返模式");
    ui->CB_MotorPattern->addItem("模式6：按键回零单方向模式");
    ui->CB_MotorPattern->addItem("模式7：前进回零往返模式");
    ui->CB_MotorPattern->addItem("模式8：单方向运行模式");
    ui->CB_MotorPattern->setCurrentIndex(2);
    setModel4NumVisible(false);

    ui->CB_State->addItem("0:当前运行状态");
    ui->CB_State->addItem("1:剩余脉冲高位");
    ui->CB_State->addItem("2:剩余脉冲中位");
    ui->CB_State->addItem("3:剩余脉冲低位");
    ui->CB_State->addItem("4:剩余往返次数");
    ui->CB_State->addItem("5:输入开关状态");
    ui->CB_State->addItem("6:电位器输入电压");
    ui->CB_State->addItem("8:查控制器硬件号");
    ui->CB_State->addItem("251:查控制器固件版本");

    for (int i = 0; i < 640; ++i){
        ui->CB_TravelDistanceNum->addItem(QString::number(i * 100 + 100));
        ui->CB_OperatingSpeedBase->addItem(QString::number(i * 100 + 100));
    }
    ui->CB_TravelDistanceNum->setCurrentText("25500");
    ui->CB_OperatingSpeedBase->setCurrentText("25500");

    for (int i = 0; i < 255; ++i)
    {
        ui->CB_SpeedUpNum->addItem(QString::number(i * 10 + 10));
        ui->CB_SlowDownNum->addItem(QString::number(i * 10 + 10));
    }
    ui->CB_SpeedUpNum->setCurrentText("550");
    ui->CB_SlowDownNum->setCurrentText("550");

    for (int i = 1; i<255; ++i)
        ui->CB_Model4Num->addItem(QString::number(i));
    ui->CB_Model4Num->setCurrentText("3");
    ui->CB_OutputFrequency->addItem("1倍输出频率（最大输出频率60KHz）");
    ui->CB_OutputFrequency->addItem("0.2倍输出频率（最大输出频率13KHz）");
}

void MainWindow::initConnect()
{
    connect(ui->QA_ComSettings, &QAction::triggered, m_settings, &SettingsDialog::show);
    connect(ui->QA_About, &QAction::triggered, this, &MainWindow::About);
    connect(ui->Btn_COM, &QPushButton::clicked, this, &MainWindow::btnCOM_clicked);

    // 用一个计时器读取数据
    m_timerRead = new QTimer(this);
    m_timerRead->setInterval(500);
    connect(m_timerRead, &QTimer::timeout, this, &MainWindow::readData);

    connect(ui->Btn_Forward, &QPushButton::clicked, this, &MainWindow::btnForward_clicked);
    connect(ui->Btn_Reverse, &QPushButton::clicked, this, &MainWindow::btnReverse_clicked);
    connect(ui->Btn_Stop, &QPushButton::clicked, this, &MainWindow::btnStop_clicked);
    connect(ui->Btn_Query, &QPushButton::clicked, this, &MainWindow::btnQuery_clicked);
    connect(ui->CB_MotorPattern, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::cbMotorPatternChanged);
}

void MainWindow::setLinkState(bool on)
{
    if (on)
        ui->Lab_LinkState->setText("<font color=forestgreen>已连接</font>");
    else
        ui->Lab_LinkState->setText("<font color=red>未连接</font>");
}

void MainWindow::btnCOM_clicked()
{
    if (ui->Btn_COM->text() == "打开串口") {
        m_serial = new QextSerialPort(ui->Lab_COMName->text(), QextSerialPort::Polling);
        COM_IsOk = m_serial->open(QIODevice::ReadWrite);

        if (COM_IsOk) {
            setLinkState(true);
            const SettingsDialog::Settings p = m_settings->settings();
            m_serial->flush();
            m_serial->setBaudRate((BaudRateType)p.baudRate);
            m_serial->setDataBits((DataBitsType)p.dataBits);
            m_serial->setParity((ParityType)p.parity);
            m_serial->setStopBits((StopBitsType)p.stopBits);
            m_serial->setFlowControl(FLOW_OFF);
            m_serial->setTimeout(10);

            ui->Btn_COM->setText("关闭串口");
            m_timerRead->start();
            showStatusMessage(QString("<font color=forestgreen>Connected to %1 : %2, %3, %4, %5, %6</font>")
                              .arg(p.name).arg(p.stringBaudRate).arg(p.stringDataBits)
                              .arg(p.stringParity).arg(p.stringStopBits).arg(p.stringFlowControl));
        }
        else
        {
            QMessageBox::critical(this, "错误", m_serial->errorString());
            showStatusMessage("<font color=red>打开串口失败</font>");
        }

    } else {
        setLinkState(false);
        m_timerRead->stop();
        m_serial->close();
        ui->Btn_COM->setText("打开串口");
        COM_IsOk = false;
        showStatusMessage("<font color=red>关闭串口</font>");
    }
}

void MainWindow::btnForward_clicked()
{
    QString _data = "BA";                   // 数据头
    _data.append(getMotorPattern());        // 运动模式
    _data.append(getOperatingSpeedBase());  // 分频基数
    _data.append(getControllerNum());       // 控制器地址
    _data.append(getModle4Num());           // 执行次数 提示：只在04:按次往返模式时有效
    _data.append("01");                     // 正方向
    _data.append(getCheckEnable());         // 获取分拆控制位
    _data.append(getTravelDistanceNum());   // 获取行进距离总脉冲数
    _data.append(getSpeedUpNum());          // 获取加速脉冲数
    _data.append(getSlowDownNum());         // 获取降速脉冲数
    _data.append(getCRCBySend(_data));      // 计算校验位
    _data.append("FE");                     // 结束位
    sendData(_data);
}

void MainWindow::btnReverse_clicked()
{
    QString _data = "BA";
    _data.append(getMotorPattern());
    _data.append(getOperatingSpeedBase());
    _data.append(getControllerNum());
    _data.append(getModle4Num());
    _data.append("02");
    _data.append(getCheckEnable());
    _data.append(getTravelDistanceNum());
    _data.append(getSpeedUpNum());
    _data.append(getSlowDownNum());
    _data.append(getCRCBySend(_data));
    _data.append("FE");
    sendData(_data);
}

void MainWindow::btnStop_clicked()
{
    QString _data = "BA";
    _data.append(getMotorPattern());
    _data.append(getOperatingSpeedBase());
    _data.append(getControllerNum());
    _data.append(getModle4Num());
    _data.append("03");
    _data.append(getCheckEnable());
    _data.append(getTravelDistanceNum());
    _data.append(getSpeedUpNum());
    _data.append(getSlowDownNum());
    _data.append(getCRCBySend(_data));
    _data.append("FE");
    sendData(_data);
}

void MainWindow::btnQuery_clicked()
{
    QString _data = "B6";
    _data.append(getControllerNum());
    _data.append(getQueryState());
    _data.append(getCRCByQuery());
    _data.append("FE");
    sendData(_data);
}

void MainWindow::cbMotorPatternChanged(int index)
{
    if (index == 4)
        setModel4NumVisible(true);
    else
        setModel4NumVisible(false);
}

void MainWindow::setModel4NumVisible(bool on)
{
    ui->CB_Model4Num->setEnabled(on);
    ui->CB_Model4Num->setVisible(on);
    ui->Lab_Model4Num->setVisible(on);
}

QString MainWindow::getControllerNum()
{
    return QString("%1").arg(ui->CB_ControllerNum->currentIndex(),2,16,QLatin1Char('0'));
}

QString MainWindow::getQueryState()
{
    switch(ui->CB_State->currentIndex())
    {
    case 7:
        return QString("%1").arg(8,2,16,QLatin1Char('0'));
    case 8:
        return "FB";
    default:
        return QString("%1").arg(ui->CB_State->currentIndex(),2,16,QLatin1Char('0'));
    }
}

QString MainWindow::getCRCByQuery()
{
    byte _CRC = 0x00;
    switch(ui->CB_State->currentIndex())
    {
    case 7:
        _CRC = 0x08;
        break;
    case 8:
        _CRC = 0xFB;
        break;
    default:
        _CRC = ui->CB_State->currentIndex();
        break;
    }
    _CRC = 0xB6 ^ ui->CB_ControllerNum->currentIndex() ^ _CRC;
    return QString("%1").arg(_CRC,2,16,QLatin1Char('0'));
}

QString MainWindow::getMotorPattern()
{
    return QString("%1").arg(ui->CB_MotorPattern->currentIndex(),2,16,QLatin1Char('0'));
}

QString MainWindow::getOperatingSpeedBase()
{
    unsigned int _operatingSpeedBase = ui->CB_OperatingSpeedBase->currentText().toUInt();
    if (_operatingSpeedBase < 256)
        _operatingSpeedBase = 256;
    else if(_operatingSpeedBase > 65535)
        _operatingSpeedBase = 65535;

    return QString("%1").arg(_operatingSpeedBase,4,16,QLatin1Char('0'));
}

QString MainWindow::getModle4Num()
{
    return QString("%1").arg(ui->CB_Model4Num->currentText().toUInt(),2,16,QLatin1Char('0'));
}

QString MainWindow::getCheckEnable()
{
    QString _data;
    if (ui->C_StartPowerOn->isChecked())    // 启动上电运行使能
        _data.append("1");
    else
        _data.append("0");
    if (ui->C_PowerOnZero->isChecked())     // 启动上电回零使能
        _data.append("1");
    else
        _data.append("0");
    _data.append(QString::number(ui->CB_ScramSW->currentIndex()));  // 急停常开
    _data.append(QString::number(ui->CB_LimitSW->currentIndex()));  // 限位常开
    _data.append(QString::number(ui->CB_OutputFrequency->currentIndex()));  // 启动0.2 倍频率输出
    if (ui->C_SingleSW->isChecked())    // 启动单开关触发
        _data.append("1");
    else
        _data.append("0");
    if (ui->C_ReadOnly->isChecked())    // 启动输入开关失效
        _data.append("1");
    else
        _data.append("0");
    if (ui->C_PosControl->isChecked())  // 启动位置控制使能
        _data.append("1");
    else
        _data.append("0");

    return myHelper::BinaryToStrHex(_data);
}

QString MainWindow::getTravelDistanceNum()
{
    unsigned int _travelDistanceNum = ui->CB_TravelDistanceNum->currentText().toUInt();
    if (_travelDistanceNum > 16777214)
        _travelDistanceNum = 16777214;

    return QString("%1").arg(_travelDistanceNum,6,16,QLatin1Char('0'));
}

QString MainWindow::getSpeedUpNum()
{
    unsigned int _speedUpNum = ui->CB_SpeedUpNum->currentText().toUInt();
    if (_speedUpNum > 65535)
        _speedUpNum = 65535;
    return QString("%1").arg(_speedUpNum,4,16,QLatin1Char('0'));
}

QString MainWindow::getSlowDownNum()
{
    unsigned int _slowDownNum = ui->CB_SlowDownNum->currentText().toUInt();
    if (_slowDownNum > 65535)
        _slowDownNum = 65535;
    return QString("%1").arg(_slowDownNum,4,16,QLatin1Char('0'));
}


QString MainWindow::getCRCBySend(QString data)
{
    QByteArray buffer = myHelper::hexStrToByteArray(data);
    byte _CRC = 0x00;   // 这步初始化很关键...

    for (int i = 0; i < buffer.length(); ++i){
        _CRC = _CRC ^ buffer[i];
    }

    return QString("%1").arg(_CRC,2,16,QLatin1Char('0'));
}

void MainWindow::readData()
{
    if (m_serial->bytesAvailable() <= 0)
        return;

    myHelper::sleep(500);
    QByteArray data = m_serial->readAll();
    int dataLen = data.length();

    if (dataLen <= 0)
        return;

    ui->LE_Receive->setText(myHelper::byteArrayToHexStr(data));
}

void MainWindow::sendData(QString sendText)
{
    if (m_serial == nullptr || !m_serial->isOpen())
    {
        QMessageBox::warning(this, "警告", "请先打开串口", QMessageBox::Yes, QMessageBox::Yes);
        return;
    }
    ui->LE_Receive->setText("");

    QByteArray buffer = myHelper::hexStrToByteArray(sendText);
    m_serial->write(buffer);
    ui->LE_Send->setText(myHelper::formatString(sendText));
}

void MainWindow::showStatusMessage(const QString &message)
{
    m_status->setText(message);
}

void MainWindow::About()
{
    QMessageBox::about(this, "关于",
                       "本软件是BE-1105运动控制器的控制程序，\n"
                       "本软件需要有485-USB驱动为基础驱动，否则无法访问硬件\n"
                       "                                       本软件的最终解释权属于陶晶");
}
