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

private:
    Ui::MainWindow *ui;
    QTcpSocket *tcpClient;
    QSettings DefaultSettings;
    bool Linked;
    void Send_data(SendType Type,int variable,int value);
};

#endif // MAINWINDOW_H
