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

namespace Ui {
class MainWindow;
}

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

private:
    Ui::MainWindow *ui;
    QTcpSocket *tcpClient;
    bool Linked;
};

#endif // MAINWINDOW_H
