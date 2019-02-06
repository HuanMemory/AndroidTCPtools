#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->Edit_IP->addItem("192.168.4.1");
    ui->Edit_IP->addItem("192.168.8.1");
    ui->Edit_IP->addItem("192.168.31.1");
    ui->Edit_IP->addItem("127.0.0.1");
    ui->Edit_IP->setCurrentIndex(0);
    ui->Edit_Port->setValue(3721);
    ui->TextBrowser->setText("DLUT Smartcar TCP Tools. by ZBT.");
    ui->tabWidget->setCurrentIndex(0);
    tcpClient = new QTcpSocket(this);
    tcpClient->abort();
    connect(tcpClient, SIGNAL(readyRead()), this, SLOT(TCPReadData()));
    connect(tcpClient, SIGNAL(error(QAbstractSocket::SocketError)),this, SLOT(TCPReadError(QAbstractSocket::SocketError)));
    Linked = false;
}

MainWindow::~MainWindow()
{
    delete ui;
}
/*
void MainWindow::on_linkBTN_clicked()
{
    if(Linked)
    {
        tcpClient->disconnectFromHost();
        if (tcpClient->state() == QAbstractSocket::UnconnectedState || tcpClient->waitForDisconnected(1000))
        {
            ui->linkBTN->setText("Link");
            Linked = false;
        }
    }
    else
    {
        tcpClient->connectToHost(ui->IPEdit->text(), 3721);
        if (tcpClient->waitForConnected(1000))
        {
            Linked = true;
            ui->linkBTN->setText("DisConnect");
        }
    }
}

void MainWindow::on_testBTN_clicked()
{
    QString data = "HelloWorld";
    tcpClient->write(data.toLatin1());
}

void MainWindow::on_pushButton_clicked()
{
    QScreen *screen=QGuiApplication::primaryScreen ();
    QRect ScreenSize=screen->availableGeometry() ;
    int screen_width = ScreenSize.width();
    int screen_height = ScreenSize.height();
    this->setFixedSize(screen_width,screen_height);
    ui->tabWidget->setFixedSize(screen_width,screen_height);
}
*/
void MainWindow::TCPReadData()
{
    QByteArray buffer = tcpClient->readAll();
    if(!buffer.isEmpty())
    {
       ui->TextBrowser->append(buffer);
       if(ui->TextBrowser->document()->lineCount()>10)
           ui->TextBrowser->clear();
    }
}

void MainWindow::TCPReadError(QAbstractSocket::SocketError)
{
    tcpClient->disconnectFromHost();
    ui->TextBrowser->setText("通信出错,断开连接 异常信息:"+tcpClient->errorString());
    ui->Button_Link->setText("连接");
    Linked = false;
}

void MainWindow::on_Button_Link_clicked()
{
    if(Linked)
    {
        ui->TextBrowser->setText("断开连接");
        tcpClient->disconnectFromHost();
        if (tcpClient->state() == QAbstractSocket::UnconnectedState || tcpClient->waitForDisconnected(1000))
        {
            ui->Button_Link->setText("连接");
            Linked = false;
        }
    }
    else
    {
        ui->TextBrowser->setText("正在连接 IP:"+ui->Edit_IP->currentText()+"端口:"+QString::number(ui->Edit_Port->value()));
        tcpClient->connectToHost(ui->Edit_IP->currentText(), quint16(ui->Edit_Port->value()));
        if (tcpClient->waitForConnected(1000))
        {
            Linked = true;
            ui->Button_Link->setText("断开");
            ui->TextBrowser->append("连接成功");
        }
        else
        {
            ui->TextBrowser->append("连接失败");
        }
    }
}

void MainWindow::on_Button_stop_clicked()
{
    if(!Linked)
    {
        ui->TextBrowser->append("尚未连接!");
        return;
    }
    QString data = "TCP Link Test!";
    tcpClient->write(data.toLatin1());
}
