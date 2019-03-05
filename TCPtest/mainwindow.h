#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpSocket>
#include <QHostAddress>
#include <QHostInfo>
#include <QNetworkInterface>
#include <QMessageBox>
#include <QScreen>
#include <QDebug>
#include <QSettings>
#include <QQueue>
#include <QTimerEvent>

namespace Ui {
class MainWindow;
}

typedef struct
{
    quint8 cmd;
    quint8 type;
    int32_t value;
}CommandData;

union SendData
{
    char buf[20];
    struct
    {
       char header[4];
       int32_t speed;
       int32_t dir;
       int32_t value;
       quint8 cmd;
       quint8 type;
       quint8 mode;
       char sum;
    }data;
};

union RecData
{
    char buf[44];
    struct
    {
        int32_t SpeedTarget;
        int32_t SpeedMeasure;
        int32_t Direction;
        int32_t Gyro_x;
        int32_t Gyro_y;
        int32_t Gyro_z;
        int32_t Acc_x;
        int32_t Acc_y;
        int32_t Acc_z;
        int32_t Time;
        quint8 Todo1;
        quint8 Todo2;
        quint8 mode;
        quint8 state;
    }data;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_Button_Link_clicked();
    void TCPReadData();
    void TCPReadError(QAbstractSocket::SocketError);
    void on_Button_stop_clicked();
    void on_Button_start_clicked();
    void on_Button_clear_clicked();
    void on_Button_sdstart_clicked();
    void on_Radio_Speed_toggled(bool checked);
    void on_Radio_Directon_toggled(bool checked);
    void on_Button_sdstop_clicked();
    void on_Button_lighton_clicked();
    void on_Button_lightoff_clicked();
    void on_Button_timesync_clicked();
    void on_Button_buz_clicked();
    void on_Button_zero_clicked();

private:
    Ui::MainWindow *ui;
    QTcpSocket *tcpClient;
    QSettings DefaultSettings;
    QByteArray Rec_FIFO;
    RecData Rec_Data;
    QQueue<CommandData> CommandList;
    int QtimerId;
    bool Linked;
    void Send_data(quint8 cmd, quint8 type, int32_t value);
    void Refresh_Wave();
    virtual void timerEvent( QTimerEvent *event);
};

#endif // MAINWINDOW_H
