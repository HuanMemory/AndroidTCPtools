#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setFixedSize(this->width(),this->height());
    ui->TextBrowser->setText("DLUT Smartcar TCP Tools. by ZBT.");
    ui->tabWidget->setCurrentIndex(0);

    ApplyDefaultSettings();

    Setting_DataSel[0] = 0;Setting_DataSel[1] = 1;Setting_DataSel[2] = 8;Setting_DataSel[3] = 8;
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

    QObject::connect(ui->Slider_maxpoint, SIGNAL(valueChanged(int)), ui->Lcd_maxpoint, SLOT(display(int)));
    QObject::connect(ui->Slider_mintime, SIGNAL(valueChanged(int)), ui->Lcd_mintime, SLOT(display(int)));
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
        if(Rec_FIFO.count()<(sizeof(Rec_Data)+4)*8)
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
            if(Decode_status == 4 && Rec_FIFO.count()>sizeof(Rec_Data))
            {
                for(int i = 0; i<sizeof(Rec_Data); i++)
                    Rec_Data.buf[i] = Rec_FIFO.at(i);
                Rec_FIFO.remove(0,sizeof(Rec_Data));
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
        ui->TextBrowser->setText("正在连接 IP:"+Setting_IP+"端口:"+QString::number(Setting_Port));
        tcpClient->connectToHost(Setting_IP, quint16(Setting_Port));
        if (tcpClient->waitForConnected(1000))
        {
            Linked = true;
            ui->Button_Link->setText("断开");
            ui->TextBrowser->append("连接成功");
            DefaultSettings.setValue("IP",Setting_IP);
            DefaultSettings.setValue("PORT",Setting_Port);
        }
        else
        {
            ui->TextBrowser->append("连接失败");
        }
    }
}

void MainWindow::on_Button_restore_clicked()
{
    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Question);
    msgBox.setText("恢复初始设置?");
    msgBox.setInformativeText("将清除未保存的所有设置，该操作不可恢复!");
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);
    msgBox.setDefaultButton(QMessageBox::Cancel);
    int ret = msgBox.exec();
    if(ret == QMessageBox::Cancel)
        return;
    ApplyDefaultSettings();
    ApplyCurrentSettings();
}

QJsonObject MainWindow::SaveCurrentSettingsToJson()
{
    QJsonObject json;
    QDateTime current_date_time = QDateTime::currentDateTime();
    QString DataTime = current_date_time.toString(Qt::ISODate);
    QJsonArray buttonnameArray;
    QJsonArray wavenameArray;
    QJsonArray waveselArray;
    QJsonArray wavedispArray;
    json.insert("time",DataTime);
    json.insert("IP",Setting_IP);
    json.insert("Port",Setting_Port);
    json.insert("mintime",Setting_MinTime);
    json.insert("maxcount",Setting_MaxCount);
    for(int i = 0;i<4;i++)
        buttonnameArray.append(Setting_ButtonName[i]);
    for(int i = 0;i<4;i++)
        waveselArray.append(Setting_DataSel[i]);
    for(int i = 0;i<8;i++)
        wavenameArray.append(Setting_DataName[i]);
    for(int i = 0;i<8;i++)
        wavedispArray.append(Setting_DataDisp[i]);
    json.insert("ButtonName",buttonnameArray);
    json.insert("WaveSelect",waveselArray);
    json.insert("WaveName",wavenameArray);
    json.insert("WaveDisplay",wavedispArray);
    return json;
}

void MainWindow::on_Button_apply_clicked()
{
    ApplyCurrentSettings();

    QString filepath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/DLUT_ZBT_Tools";
    QDir dir;
    if(!dir.mkpath(filepath))
        return;
    filepath += "/WaveAutoSave.json";
    QFile file(filepath);
    if (!file.open(QIODevice::ReadWrite | QIODevice::Text))
    {
        QMessageBox::critical(NULL, "ERROR", "Can not open/creat autosave file!", QMessageBox::Yes, QMessageBox::Yes);
        return;
    }
    QTextStream save(&file);
    QJsonObject json = SaveCurrentSettingsToJson();
    json.insert("Description","AutoSave");
    QJsonDocument document;
    document.setObject(json);
    QByteArray byte_array = document.toJson(QJsonDocument::Indented);
    QString JsonStr = QString(byte_array);
    save << JsonStr;
    file.close();
}

void MainWindow::on_Button_import_clicked()
{
    QString path = QFileDialog::getOpenFileName(this, "Open", ".", tr("Json File(*.json)"));
    if (path.isEmpty())
    return;
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QMessageBox::critical(NULL, "ERROR", "Can not open file!", QMessageBox::Yes, QMessageBox::Yes);
        return;
    }
    QByteArray byteArray = file.readAll();
    file.close();
    QJsonParseError json_error;
    QJsonDocument jsonDocument(QJsonDocument::fromJson(byteArray, &json_error));
    if (json_error.error != QJsonParseError::NoError)
    {
        QMessageBox::critical(NULL, "ERROR", "Wrong File Content!", QMessageBox::Yes, QMessageBox::Yes);
        return;
    }
    QJsonObject json = jsonDocument.object();
    ui->Edit_IP->setCurrentText(json.value("IP").toString());
    ui->Edit_Port->setValue(json.value("Port").toInt());
    ///////////////////ToDo
    ApplyCurrentSettings();
}

void MainWindow::on_Button_export_clicked()
{
    QString path = QFileDialog::getSaveFileName(this, "Save", "./PlotSettings.json", tr("Json File(*.json)"));
    if (path.isEmpty())
        return;
    QFile file(path);
    if (!file.open(QIODevice::ReadWrite | QIODevice::Text))
    {
        QMessageBox::critical(NULL, "ERROR", "Can not open/creat file!", QMessageBox::Yes, QMessageBox::Yes);
        return;
    }
    QTextStream save(&file);
    QJsonObject json = SaveCurrentSettingsToJson();
    QString description = QInputDialog::getText(this, tr("Log"),
            tr("Description:"), QLineEdit::Normal);
    json.insert("Description",description);
    QJsonDocument document;
    document.setObject(json);
    QByteArray byte_array = document.toJson(QJsonDocument::Indented);
    QString JsonStr = QString(byte_array);
    save << JsonStr;
    file.close();
}

void MainWindow::ApplyDefaultSettings()
{
    ui->Edit_IP->clear();
    ui->Edit_IP->addItem("192.168.4.1");
    ui->Edit_IP->addItem("192.168.8.1");
    ui->Edit_IP->addItem("192.168.31.1");
    ui->Edit_IP->addItem("127.0.0.1");
    ui->Edit_IP->setCurrentText("192.168.4.1");
    ui->Edit_Port->setValue(3721);
    WaveName.clear();
    WaveName.append("数据1");WaveName.append("数据2");WaveName.append("数据3");WaveName.append("数据4");
    WaveName.append("数据5");WaveName.append("数据6");WaveName.append("数据7");WaveName.append("数据8");
    WaveName.append("0");
    ui->Combo_waveSel1->addItems(WaveName);
    ui->Combo_waveSel2->addItems(WaveName);
    ui->Combo_waveSel3->addItems(WaveName);
    ui->Combo_waveSel4->addItems(WaveName);
    ui->Combo_waveSel1->setCurrentIndex(0);
    ui->Combo_waveSel2->setCurrentIndex(1);
    ui->Combo_waveSel3->setCurrentIndex(8);
    ui->Combo_waveSel4->setCurrentIndex(8);
    Setting_DataSel[0] = 0;Setting_DataSel[1] = 1;Setting_DataSel[2] = 8;Setting_DataSel[3] = 8;
    ui->Edit_data1->setText("数据1");
    ui->Edit_data2->setText("数据2");
    ui->Edit_data3->setText("数据3");
    ui->Edit_data4->setText("数据4");
    ui->Edit_data5->setText("数据5");
    ui->Edit_data6->setText("数据6");
    ui->Edit_data7->setText("数据7");
    ui->Edit_data8->setText("数据8");
    ui->Edit_button1->setText("功能1");
    ui->Edit_button2->setText("功能2");
    ui->Edit_button3->setText("功能3");
    ui->Edit_button4->setText("功能4");
    ui->Button_func1->setText("功能1");
    ui->Button_func2->setText("功能2");
    ui->Button_func3->setText("功能3");
    ui->Button_func4->setText("功能4");
    ui->Checkbox_data1disp->setChecked(false);
    ui->Checkbox_data2disp->setChecked(false);
    ui->Checkbox_data3disp->setChecked(false);
    ui->Checkbox_data4disp->setChecked(false);
    ui->Checkbox_data5disp->setChecked(false);
    ui->Checkbox_data6disp->setChecked(false);
    ui->Checkbox_data7disp->setChecked(false);
    ui->Checkbox_data8disp->setChecked(false);
    ui->Slider_mintime->setValue(10);
    ui->Slider_maxpoint->setValue(2000);
}

void MainWindow::ApplyCurrentSettings()
{
    Setting_IP = ui->Edit_IP->currentText();
    Setting_Port = ui->Edit_Port->value();
    Setting_DataSel[0] = ui->Combo_waveSel1->currentIndex();
    Setting_DataSel[1] = ui->Combo_waveSel2->currentIndex();
    Setting_DataSel[2] = ui->Combo_waveSel3->currentIndex();
    Setting_DataSel[3] = ui->Combo_waveSel4->currentIndex();
    Setting_MinTime = ui->Slider_mintime->value();
    Setting_DataDisp[0] = ui->Checkbox_data1disp->isChecked();
    Setting_DataDisp[1] = ui->Checkbox_data2disp->isChecked();
    Setting_DataDisp[2] = ui->Checkbox_data3disp->isChecked();
    Setting_DataDisp[3] = ui->Checkbox_data4disp->isChecked();
    Setting_DataDisp[4] = ui->Checkbox_data5disp->isChecked();
    Setting_DataDisp[5] = ui->Checkbox_data6disp->isChecked();
    Setting_DataDisp[6] = ui->Checkbox_data7disp->isChecked();
    Setting_DataDisp[7] = ui->Checkbox_data8disp->isChecked();
    Setting_DataName[0] = ui->Edit_data1->text();
    Setting_DataName[1] = ui->Edit_data2->text();
    Setting_DataName[2] = ui->Edit_data3->text();
    Setting_DataName[3] = ui->Edit_data4->text();
    Setting_DataName[4] = ui->Edit_data5->text();
    Setting_DataName[5] = ui->Edit_data6->text();
    Setting_DataName[6] = ui->Edit_data7->text();
    Setting_DataName[7] = ui->Edit_data8->text();
    Setting_MaxCount = ui->Slider_maxpoint->value();
    Setting_ButtonName[0] = ui->Edit_button1->text();
    Setting_ButtonName[1] = ui->Edit_button2->text();
    Setting_ButtonName[2] = ui->Edit_button3->text();
    Setting_ButtonName[3] = ui->Edit_button4->text();
    for(int i = 0;i<8;i++)
    {
        ui->Combo_waveSel1->setItemText(i,Setting_DataName[i]);
        ui->Combo_waveSel2->setItemText(i,Setting_DataName[i]);
        ui->Combo_waveSel3->setItemText(i,Setting_DataName[i]);
        ui->Combo_waveSel4->setItemText(i,Setting_DataName[i]);
    }
    ui->Button_func1->setText(Setting_ButtonName[0]);
    ui->Button_func2->setText(Setting_ButtonName[1]);
    ui->Button_func3->setText(Setting_ButtonName[2]);
    ui->Button_func4->setText(Setting_ButtonName[3]);
}

void MainWindow::Send_data(uint8_t cmd, uint8_t type, int32_t value)
{
    //帧格式：0x7A 0x7A 0x7A [int32_t val] cmd type todo sum
    if(!Linked)
    {
        ui->TextBrowser->append("尚未连接!");
        return;
    }
    uint8_t sum = 0;
    SendData Frame;
    Frame.data.header[0] = Frame.data.header[1] = Frame.data.header[2] = Frame.data.header[3] = 0x7A;
    Frame.data.cmd = cmd;
    Frame.data.type = type;
    Frame.data.value = value;
    for(int i = 0;i<sizeof(Frame)-1;i++)
        sum += Frame.buf[i];
    Frame.data.sum = (char)sum;
    QByteArray data(Frame.buf,16);
    tcpClient->write(data);
}

void MainWindow::on_Button_start_clicked()
{
    Send_data(1,5,0);
}

void MainWindow::on_Button_stop_clicked()
{
    Send_data(1,6,0);
}

void MainWindow::on_Button_func1_clicked()
{
    Send_data(1,1,0);
}

void MainWindow::on_Button_func2_clicked()
{
    Send_data(1,2,0);
}

void MainWindow::on_Button_func3_clicked()
{
    Send_data(1,3,0);
}

void MainWindow::on_Button_func4_clicked()
{
    Send_data(1,4,0);
}

void MainWindow::Refresh_Wave()
{
    static QTime time(QTime::currentTime());
    static double Lasttime = 0;
    double key = time.elapsed()/1000.0;
    if(key - Lasttime<Setting_MinTime/1000.0)
        return;
    Lasttime = key;
    ui->Plot->graph(0)->addData(key, (Setting_DataSel[0]<8)?(Rec_Data.data.data[0]):0);
    ui->Plot->graph(1)->addData(key, (Setting_DataSel[1]<8)?(Rec_Data.data.data[1]):0);
    ui->Plot->graph(2)->addData(key, (Setting_DataSel[2]<8)?(Rec_Data.data.data[2]):0);
    ui->Plot->graph(3)->addData(key, (Setting_DataSel[3]<8)?(Rec_Data.data.data[3]):0);
    ui->Plot->graph(4)->addData(key, 0);
    if(ui->Plot->graph(0)->dataCount()>Setting_MaxCount)
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
