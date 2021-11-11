#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSerialPort>

QT_BEGIN_NAMESPACE

namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class SettingsDialog;
class QextSerialPort;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void About();
    void btnCOM_clicked();
    void btnForward_clicked();
    void btnReverse_clicked();
    void btnStop_clicked();
    void btnQuery_clicked();
    void cbMotorPatternChanged(int index);

private:
    void initUI();
    void initConnect();

    // COM Func
    bool COM_IsOk;                      // 串口是否打开
    void readData();                    // 读取串口数据
    void sendData(QString sendText);    // 发送串口数据

    // BE-1105 专用函数
    QString getControllerNum();         // 获取控制器编号
    QString getQueryState();            // 获得查询状态
    QString getCRCByQuery();            // 获得查询时的校验位
    QString getMotorPattern();          // 获得运动模式
    QString getOperatingSpeedBase();    // 获得分频基数
    QString getModle4Num();             // 获得执行次数
    QString getCheckEnable();           // 分拆的二进制数据位
    QString getTravelDistanceNum();     // 获取行进距离总脉冲数
    QString getSpeedUpNum();            // 获取加速脉冲数
    QString getSlowDownNum();           // 获取降速脉冲数
    QString getCRCBySend(QString data); // 获取发送命令时的校验位

    // ui 用方法
    void setLinkState(bool on);
    void showStatusMessage(const QString &message);
    void setModel4NumVisible(bool on);

    Ui::MainWindow *ui;
    QLabel *m_status = nullptr;         // 窗口状态栏
    QTimer *m_timerRead;                // 定时读取串口数据
    QextSerialPort *m_serial = nullptr;
    SettingsDialog *m_settings = nullptr;
};
#endif // MAINWINDOW_H
