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
    if(DefaultSettings.contains("BTN1_NAME"))
    {
        ui->Edit_button1->setText(DefaultSettings.value("BTN1_NAME").toString());
        ui->Button_func1->setText(DefaultSettings.value("BTN1_NAME").toString());
    }
    if(DefaultSettings.contains("BTN2_NAME"))
    {
        ui->Edit_button2->setText(DefaultSettings.value("BTN2_NAME").toString());
        ui->Button_func2->setText(DefaultSettings.value("BTN2_NAME").toString());
    }
    if(DefaultSettings.contains("BTN3_NAME"))
    {
        ui->Edit_button3->setText(DefaultSettings.value("BTN3_NAME").toString());
        ui->Button_func3->setText(DefaultSettings.value("BTN3_NAME").toString());
    }
    if(DefaultSettings.contains("BTN4_NAME"))
    {
        ui->Edit_button4->setText(DefaultSettings.value("BTN4_NAME").toString());
        ui->Button_func4->setText(DefaultSettings.value("BTN4_NAME").toString());
    }
    if(DefaultSettings.contains("REC_SETTING"))
    {
        Rec_setting = (DrawType)DefaultSettings.value("REC_SETTING").toInt();
        if(Rec_setting == DRAW_WAVE)
            ui->Radio_decode->setChecked(true);
        else if(Rec_setting == DRAW_STRING)
            ui->Radio_string->setChecked(true);
    }
    else
        Rec_setting = DRAW_NOTHING;

    //初始化控件
    tcpClient = new QTcpSocket(this);
    tcpClient->abort();
    connect(tcpClient, SIGNAL(readyRead()), this, SLOT(TCPReadData()));
    connect(tcpClient, SIGNAL(error(QAbstractSocket::SocketError)),this, SLOT(TCPReadError(QAbstractSocket::SocketError)));
    Linked = false;

    //初始化绘图
    QSharedPointer<QCPAxisTickerTime> timeTicker(new QCPAxisTickerTime);
    timeTicker->setTimeFormat("%m:%s");
    ui->Plot->addGraph();
    ui->Plot->addGraph();
    ui->Plot->addGraph();
    ui->Plot->addGraph();
    ui->Plot->addGraph();
    ui->Plot->xAxis->setTicker(timeTicker);
    ui->Plot->graph(0)->setPen(QPen(QColor("red")));
    ui->Plot->graph(1)->setPen(QPen(QColor("blue")));
    ui->Plot->graph(2)->setPen(QPen(QColor("aqua")));
    ui->Plot->graph(3)->setPen(QPen(QColor("lime")));
    ui->Plot->graph(4)->setPen(QPen(QColor("black")));
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
        if(Rec_setting == DRAW_STRING)
        {
            ui->TextBrowser->append(buffer);
            if(ui->TextBrowser->document()->lineCount()>10)
                ui->TextBrowser->clear();
        }
        else if(Rec_setting == DRAW_WAVE)
        {
            if(Rec_FIFO.count()<48*8)
                Rec_FIFO.append(buffer);
            while(Rec_FIFO.count()>0)
            {
                while(Decode_status<4)
                {
                    if(Rec_FIFO.at(0) == 0x5A)
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

void MainWindow::on_Button_apply_clicked()
{
    ui->Button_func1->setText(ui->Edit_button1->text());
    ui->Button_func2->setText(ui->Edit_button2->text());
    ui->Button_func3->setText(ui->Edit_button3->text());
    ui->Button_func4->setText(ui->Edit_button4->text());
    DefaultSettings.setValue("BTN1_NAME",ui->Edit_button1->text());
    DefaultSettings.setValue("BTN2_NAME",ui->Edit_button2->text());
    DefaultSettings.setValue("BTN3_NAME",ui->Edit_button3->text());
    DefaultSettings.setValue("BTN4_NAME",ui->Edit_button4->text());
}

void MainWindow::on_Button_restore_clicked()
{
    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Question);
    msgBox.setText("清除命令表？");
    msgBox.setInformativeText("将清楚保存的所有变量表，该操作不可恢复");
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);
    msgBox.setDefaultButton(QMessageBox::Cancel);
    int ret = msgBox.exec();
    if(ret == QMessageBox::Cancel)
        return;
    ui->Button_func1->setText("功能1");
    ui->Edit_button1->setText("功能1");
    ui->Button_func2->setText("功能2");
    ui->Edit_button2->setText("功能2");
    ui->Button_func3->setText("功能3");
    ui->Edit_button3->setText("功能3");
    ui->Button_func4->setText("功能4");
    ui->Edit_button4->setText("功能4");
    DefaultSettings.remove("BTN1_NAME");
    DefaultSettings.remove("BTN2_NAME");
    DefaultSettings.remove("BTN3_NAME");
    DefaultSettings.remove("BTN4_NAME");
}

void MainWindow::Send_data(SendType Type, int32_t variable, int32_t value)
{
    //帧格式：0x7A 0x7A 0x7A Type [int32_t variable] [int32_t value] cmd state len sum
    if(!Linked)
    {
        ui->TextBrowser->append("尚未连接!");
        return;
    }
    int8_t sum = 0;
    SendData Frame;
    Frame.data.header[0] = Frame.data.header[1] = Frame.data.header[2] = 0x7A;
    switch(Type)
    {
    case SENDTYPE_FUNC:
        Frame.data.type = 0x01;
        Frame.data.variable = variable;
        Frame.data.value = 0;
        break;
    case SENDTYPE_SET:
        Frame.data.type = 0x02;
        Frame.data.variable = variable;
        Frame.data.value = value;
        break;
    }
    Frame.data.cmd = 0;
    Frame.data.state = 0;
    Frame.data.len = 0;
    for(int i = 0;i<15;i++)
        sum += Frame.buf[i];
    Frame.data.sum = sum;
    QByteArray data(Frame.buf,16);
    tcpClient->write(data);
}

void MainWindow::on_Button_start_clicked()
{   //0-Stop 1-Run 2~5 Func1~4
    Send_data(SENDTYPE_FUNC,0,0);
}

void MainWindow::on_Button_stop_clicked()
{
    Send_data(SENDTYPE_FUNC,1,0);
}

void MainWindow::on_Button_func1_clicked()
{
    Send_data(SENDTYPE_FUNC,2,0);
}

void MainWindow::on_Button_func2_clicked()
{
    Send_data(SENDTYPE_FUNC,3,0);
}

void MainWindow::on_Button_func3_clicked()
{
    Send_data(SENDTYPE_FUNC,4,0);
}

void MainWindow::on_Button_func4_clicked()
{
    Send_data(SENDTYPE_FUNC,5,0);
}

void MainWindow::on_Radio_none_toggled(bool checked)
{
    if(checked)
    {
        Rec_setting = DRAW_NOTHING;
        ui->TextBrowser->clear();
        DefaultSettings.setValue("REC_SETTING",DRAW_NOTHING);
    }
}

void MainWindow::on_Radio_decode_toggled(bool checked)
{
    if(checked)
    {
        Rec_setting = DRAW_WAVE;
        ui->TextBrowser->clear();
        DefaultSettings.setValue("REC_SETTING",DRAW_WAVE);
    }
}

void MainWindow::on_Radio_string_toggled(bool checked)
{
    if(checked)
    {
        Rec_setting = DRAW_STRING;
        ui->TextBrowser->clear();
        DefaultSettings.setValue("REC_SETTING",DRAW_STRING);
    }
}

void MainWindow::Refresh_Wave()
{
    static QTime time(QTime::currentTime());
    double key = time.elapsed()/1000.0;
    ui->Plot->graph(0)->addData(key, Rec_Data.data.val[0]);
    ui->Plot->graph(1)->addData(key, Rec_Data.data.val[1]);
    ui->Plot->graph(2)->addData(key, Rec_Data.data.val[2]);
    ui->Plot->graph(3)->addData(key, Rec_Data.data.val[3]);
    ui->Plot->graph(4)->addData(key, 0);
    if(ui->Plot->graph(0)->dataCount()>2000)
    {
        double firstsortKey = ui->Plot->graph(0)->data()->at(0)->sortKey();
        ui->Plot->graph(0)->data()->remove(firstsortKey);
        ui->Plot->graph(1)->data()->remove(firstsortKey);
        ui->Plot->graph(2)->data()->remove(firstsortKey);
        ui->Plot->graph(3)->data()->remove(firstsortKey);
        ui->Plot->graph(4)->data()->remove(firstsortKey);
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
    ui->Plot->graph(3)->data()->clear();
    ui->Plot->graph(4)->data()->clear();
    ui->Plot->replot();
}
