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
#include <QFile>
#include <QDir>
#include <QStandardPaths>
#include <QFileDialog>
#include <QDateTime>
#include <QTextStream>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonParseError>

namespace Ui {
class MainWindow;
}

union SendData
{
    char buf[12];
    struct
    {
        uint8_t header[4];
        int32_t value;
        uint8_t cmd;
        uint8_t type;
        uint8_t todo;
        uint8_t sum;
    }data;
};

union RecData
{
    uint8_t buf[40];
    struct
    {
        int32_t data[8];
        uint8_t cmd;
        uint8_t val;
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

    void on_Button_clear_clicked();

    void on_Button_export_clicked();

    void on_Button_import_clicked();

private:
    Ui::MainWindow *ui;

    QString Setting_IP;
    int Setting_Port;
    QString Setting_DataName[8];
    bool Setting_DataDisp[8];
    int Setting_DataSel[4];
    int Setting_MaxCount;
    int Setting_MinTime;
    QString Setting_ButtonName[4];

    QStringList WaveName;
    QTcpSocket *tcpClient;
    QSettings DefaultSettings;
    QByteArray Rec_FIFO;
    RecData Rec_Data;
    bool Linked;
    void Send_data(uint8_t cmd, uint8_t type, int32_t value);
    void Refresh_Wave();

    void ApplyDefaultSettings();
    void ApplyCurrentSettings();
    QJsonObject SaveCurrentSettingsToJson();
};

#endif // MAINWINDOW_H
