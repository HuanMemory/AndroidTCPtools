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

namespace Ui {
class MainWindow;
}

enum SendType
{
    SENDTYPE_FUNC,
    SENDTYPE_SET
};

enum DrawType
{
    DRAW_NOTHING,
    DRAW_STRING,
    DRAW_WAVE
};

union SendData
{
    char buf[16];
    struct
    {
       char header[3];
       char type;
       int variable;
       int value;
       char cmd;
       char state;
       char len;
       char sum;
    }data;
};

union RecData
{
    char buf[44];
    struct
    {
        int32_t val[8];
        int32_t cmd;
        quint8 cmd_val[8];
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
    void on_Button_apply_clicked();
    void on_Button_restore_clicked();
    void on_Button_start_clicked();
    void on_Button_func1_clicked();
    void on_Button_func2_clicked();
    void on_Button_func3_clicked();
    void on_Button_func4_clicked();
    void on_Radio_none_toggled(bool checked);
    void on_Radio_decode_toggled(bool checked);
    void on_Radio_string_toggled(bool checked);

    void on_Button_clear_clicked();

private:
    Ui::MainWindow *ui;
    QTcpSocket *tcpClient;
    QSettings DefaultSettings;
    /*
    QVector<double> X_data;
    QVector<double> Y1_data;
    QVector<double> Y2_data;
    QVector<double> Y3_data;
    QVector<double> Y4_data;*/
    QByteArray Rec_FIFO;
    RecData Rec_Data;
    bool Linked;
    DrawType Rec_setting;
    void Send_data(SendType Type,int variable,int value);
    void Refresh_Wave();
};

#endif // MAINWINDOW_H
