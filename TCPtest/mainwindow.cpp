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
    ui->TextBrowser->setText("DLUT Smartcar TCP Tools. by ZBT.");
    ui->tabWidget->setCurrentIndex(0);
    //恢复历史信息
    if(DefaultSettings.contains("IP"))
        ui->Edit_IP->setCurrentText(DefaultSettings.value("IP").toString());
    else
        ui->Edit_IP->setCurrentIndex(0);
    if(DefaultSettings.contains("PORT"))
        ui->Edit_Port->setValue(DefaultSettings.value("PORT").toInt());
    else
        ui->Edit_Port->setValue(3721);

    //初始化控件
    tcpClient = new QTcpSocket(this);
    tcpClient->abort();
    connect(tcpClient, SIGNAL(readyRead()), this, SLOT(TCPReadData()));
    connect(tcpClient, SIGNAL(error(QAbstractSocket::SocketError)),this, SLOT(TCPReadError(QAbstractSocket::SocketError)));
    Linked = false;
    QtimerId = startTimer(100);
    //初始化绘图
    QSharedPointer<QCPAxisTickerTime> timeTicker(new QCPAxisTickerTime);
    timeTicker->setTimeFormat("%m:%s");
    ui->Plot->addGraph();
    ui->Plot->addGraph();
    ui->Plot->addGraph();
    ui->Plot->xAxis->setTicker(timeTicker);
    ui->Plot->graph(0)->setPen(QPen(QColor("red")));
    ui->Plot->graph(1)->setPen(QPen(QColor("blue")));
    ui->Plot->graph(2)->setPen(QPen(QColor("black")));
    ui->Plot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::TCPReadData()
{
    QByteArray buffer = tcpClient->readAll();
    static int Decode_status = 0;   //0~3 Header 4 Rec
    if(!buffer.isEmpty())
    {
        if(Rec_FIFO.count()<48*8)
            Rec_FIFO.append(buffer);
        while(Rec_FIFO.count()>0)
        {
            while(Decode_status<4)
            {
                if(Rec_FIFO.at(0) == 0x7A)
                    Decode_status++;
                Rec_FIFO.remove(0,1);
                if(Rec_FIFO.count()==0)
                    break;
            }
            if(Decode_status == 4 && Rec_FIFO.count()>44)
            {
                for(int i = 0; i<44 ; i++)
                    Rec_Data.buf[i] = Rec_FIFO.at(i);
                Rec_FIFO.remove(0,44);
                Decode_status = 0;
                Refresh_Wave();
            }
            else
                break;
        }
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
            DefaultSettings.setValue("IP",ui->Edit_IP->currentText());
            DefaultSettings.setValue("PORT",ui->Edit_Port->value());
        }
        else
        {
            ui->TextBrowser->append("连接失败");
        }
    }
}

void MainWindow::Send_data(quint8 cmd, quint8 type, int32_t value)
{
    //帧格式：0x7B 0x7B 0x7B 0x7B [int32_t speed] [int32_t dir] [int32_t value] cmd type mode sum
    int8_t sum = 0;
    SendData Frame;
    Frame.data.header[0] = Frame.data.header[1] = Frame.data.header[2] = Frame.data.header[3] = 0x7B;
    Frame.data.speed = ui->Slider_speed->value();
    Frame.data.dir = ui->Slider_direction->value();
    Frame.data.cmd = cmd;
    Frame.data.type = type;
    Frame.data.value = value;
    if(ui->Check_control->isChecked())
        Frame.data.mode = 1;
    else
        Frame.data.mode = 0;
    for(int i = 0;i<19;i++)
        sum += Frame.buf[i];
    Frame.data.sum = sum;
    QByteArray data(Frame.buf,sizeof(Frame));
    tcpClient->write(data);
}

void MainWindow::Refresh_Wave()
{
    static QTime time(QTime::currentTime());
    double key = time.elapsed()/1000.0;
    if(ui->Radio_Speed->isChecked())
    {
        ui->Plot->graph(0)->addData(key,Rec_Data.data.SpeedTarget);
        ui->Plot->graph(1)->addData(key,Rec_Data.data.SpeedMeasure);

    }
    else if(ui->Radio_Directon->isChecked())
    {
        ui->Plot->graph(0)->addData(key,Rec_Data.data.Direction);
        ui->Plot->graph(1)->addData(key,0);
    }
    ui->Plot->graph(2)->addData(key,0);
    if(ui->Plot->graph(0)->dataCount()>2000)
    {
        double firstsortKey = ui->Plot->graph(0)->data()->at(0)->sortKey();
        ui->Plot->graph(0)->data()->remove(firstsortKey);
        ui->Plot->graph(1)->data()->remove(firstsortKey);
        ui->Plot->graph(2)->data()->remove(firstsortKey);
    }
    if(ui->Checkbox_autoscale->isChecked())
        ui->Plot->rescaleAxes();
    ui->Plot->replot();
}

void MainWindow::on_Button_clear_clicked()
{
    ui->Plot->graph(0)->data()->clear();
    ui->Plot->graph(1)->data()->clear();
    ui->Plot->graph(2)->data()->clear();
    ui->Plot->replot();
}

void MainWindow::on_Radio_Speed_toggled(bool checked)
{
    if(!checked)
        return;
    ui->Plot->graph(0)->data()->clear();
    ui->Plot->graph(1)->data()->clear();
    ui->Plot->graph(2)->data()->clear();
    ui->Plot->replot();
}

void MainWindow::on_Radio_Directon_toggled(bool checked)
{
    if(!checked)
        return;
    ui->Plot->graph(0)->data()->clear();
    ui->Plot->graph(1)->data()->clear();
    ui->Plot->graph(2)->data()->clear();
    ui->Plot->replot();
}

void MainWindow::on_Button_zero_clicked()
{
    ui->Slider_speed->setValue(0);
    ui->Slider_direction->setValue(0);
}

void MainWindow::timerEvent(QTimerEvent *event)
{
    if(event->timerId() != QtimerId)
        return;
    if(!Linked)
        return;
    if(CommandList.empty())
        Send_data(0,0,0);
    else
    {
        CommandData cmd = CommandList.dequeue();
        Send_data(cmd.cmd,cmd.type,cmd.value);
    }
}

void MainWindow::on_Button_sdstart_clicked()
{
    if(!Linked)
    {
        ui->TextBrowser->append("尚未连接!");
        return;
    }
}

void MainWindow::on_Button_sdstop_clicked()
{
    if(!Linked)
    {
        ui->TextBrowser->append("尚未连接!");
        return;
    }
}

void MainWindow::on_Button_lighton_clicked()
{
    if(!Linked)
    {
        ui->TextBrowser->append("尚未连接!");
        return;
    }
}

void MainWindow::on_Button_lightoff_clicked()
{
    if(!Linked)
    {
        ui->TextBrowser->append("尚未连接!");
        return;
    }
}

void MainWindow::on_Button_timesync_clicked()
{
    if(!Linked)
    {
        ui->TextBrowser->append("尚未连接!");
        return;
    }
}

void MainWindow::on_Button_buz_clicked()
{
    if(!Linked)
    {
        ui->TextBrowser->append("尚未连接!");
        return;
    }
    CommandData newcommand;
    newcommand.cmd = 1;
    newcommand.type = 5;
    newcommand.value = 0;
    CommandList.append(newcommand);
}

void MainWindow::on_Button_start_clicked()
{
    if(!Linked)
    {
        ui->TextBrowser->append("尚未连接!");
        return;
    }
    CommandData newcommand;
    newcommand.cmd = 1;
    newcommand.type = 2;
    newcommand.value = 0;
    CommandList.append(newcommand);
}

void MainWindow::on_Button_stop_clicked()
{
    if(!Linked)
    {
        ui->TextBrowser->append("尚未连接!");
        return;
    }
    CommandData newcommand;
    newcommand.cmd = 1;
    newcommand.type = 1;
    newcommand.value = 0;
    CommandList.append(newcommand);
}
