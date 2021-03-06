/*
* Copyright (C) 2017 Bastian Gschrey & Markus Ippy
*
* Digital Gauges for Apexi Power FC for RX7 on Raspberry Pi
*
*
* This software comes under the GPL (GNU Public License)
* You may freely copy,distribute etc. this as long as the source code
* is made available for FREE.
*
* No warranty is made or implied. You use this program at your own risk.
*/

/*!
  \file serial.cpp
  \brief Raspexi Viewer Power FC related functions
  \author Bastian Gschrey & Markus Ippy
*/


#include "serial.h"
#include "serialobd.h"
#include "nissanconsultcom.h"
#include "decoder.h"
#include "dashboard.h"
#include "serialport.h"
#include "appsettings.h"
#include "gopro.h"
#include "gps.h"
#include <QDebug>
#include <QTime>
#include <QTimer>
#include <QThread>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QQmlContext>
#include <QQmlApplicationEngine>
#include <QModbusRtuSerialMaster>
#include <QFile>
#include <QTextStream>
#include <QByteArrayMatcher>
#include <QProcess>



int requestIndex = 0; //ID for requested data type Power FC
int ecu =3; //0=apex, 1=adaptronic;2= OBD; 3= Dicktator ECU
int interface; // 0=fcHako, 1=fc-datalogIt
int logging; // 0 Logging off , 1 Logging to file
int loggingstatus;
int Bytesexpected;
int Bytes;
QString Logfilename;
QByteArray checksumhex;
QByteArray recvchecksumhex;

//reply = new QModbusReply;




Serial::~Serial()
{

    if (modbusDevice)
        modbusDevice->disconnectDevice();
    delete modbusDevice;

}

Serial::Serial(QObject *parent) :
    QObject(parent),
    m_serialport(Q_NULLPTR),
    m_decoder(Q_NULLPTR),
    m_dashBoard(Q_NULLPTR),
    m_gopro(Q_NULLPTR),
    m_gps(Q_NULLPTR),
    m_obd(Q_NULLPTR),
    m_nissanconsultcom(Q_NULLPTR),
    m_bytesWritten(0),
    lastRequest(nullptr),
    modbusDevice(nullptr)


{
    modbusDevice = new QModbusRtuSerialMaster(this);
    getPorts();
    m_dashBoard = new DashBoard(this);
    m_decoder = new Decoder(m_dashBoard, this);
    m_appSettings = new AppSettings(this);
    m_gopro = new GoPro(this);
    m_gps = new GPS(m_dashBoard, this);
    m_nissanconsultcom = new NissanconsultCom(m_dashBoard, this);
    m_obd = new SerialOBD(m_dashBoard, this);
    connect(m_decoder,SIGNAL(sig_adaptronicReadFinished()),this,SLOT(AdaptronicStartStream()));
    QQmlApplicationEngine *engine = dynamic_cast<QQmlApplicationEngine*>( parent );
    if (engine == Q_NULLPTR)
        return;
    engine->rootContext()->setContextProperty("Dashboard", m_dashBoard);
    engine->rootContext()->setContextProperty("Decoder", m_decoder);
    engine->rootContext()->setContextProperty("AppSettings", m_appSettings);
    engine->rootContext()->setContextProperty("GoPro", m_gopro);
    engine->rootContext()->setContextProperty("GPS", m_gps);
    engine->rootContext()->setContextProperty("NissanconsultCom", m_nissanconsultcom);
    engine->rootContext()->setContextProperty("OBD", m_obd);
}

void Serial::initSerialPort()
{
    if (m_serialport)
        delete m_serialport;
    m_serialport = new SerialPort(this);
    connect(this->m_serialport,SIGNAL(readyRead()),this,SLOT(readyToRead()));
    connect(m_serialport, static_cast<void (QSerialPort::*)(QSerialPort::SerialPortError)>(&QSerialPort::error),
            this, &Serial::handleError);
    connect(m_serialport, &QSerialPort::bytesWritten, this, &Serial::handleBytesWritten);
    connect(&m_timer, &QTimer::timeout, this, &Serial::handleTimeout);
    m_readData.clear();
    //m_timer.start(5000);


}
void Serial::getEcus()
{
    QStringList EcuList;
    EcuList.append("PowerFC");
    EcuList.append("Adaptronic");
}


/*void Serial::setEcus(QStringList ECUList)
{

}*/

void Serial::getPorts()
{
    QStringList PortList;
    foreach(const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
    {
        PortList.append(info.portName());
    }
    setPortsNames(PortList);
    // Check available ports evry 1000 ms
    QTimer::singleShot(1000, this, SLOT(getPorts()));
}
//function for flushing all serial buffers
void Serial::clear() const
{
    m_serialport->clear();
}


//function to open serial port
void Serial::openConnection(const QString &portName, const int &ecuSelect, const int &interfaceSelect, const int &loggingSelect)
{
    ecu = ecuSelect;
    interface = interfaceSelect;
    logging = loggingSelect;


    //Apexi
    if (ecuSelect == 0)
    {

        initSerialPort();
        m_serialport->setPortName(portName);
        m_serialport->setBaudRate(QSerialPort::Baud57600);
        m_serialport->setParity(QSerialPort::NoParity);
        m_serialport->setDataBits(QSerialPort::Data8);
        m_serialport->setStopBits(QSerialPort::OneStop);
        m_serialport->setFlowControl(QSerialPort::NoFlowControl);;

        if(m_serialport->open(QIODevice::ReadWrite) == false)
        {
            m_dashBoard->setSerialStat(m_serialport->errorString());
        }
        else
        {
            m_dashBoard->setSerialStat(QString("Connected to Serialport"));
        }

        requestIndex = 0;

        Serial::sendRequest(requestIndex);





    }


    //Adaptronic
    if (ecuSelect == 1)
    {

        if (!modbusDevice)
            return;

        if (modbusDevice->state() != QModbusDevice::ConnectedState)
        {
            modbusDevice->setConnectionParameter(QModbusDevice::SerialPortNameParameter,portName);
            modbusDevice->setConnectionParameter(QModbusDevice::SerialBaudRateParameter,57600);
            modbusDevice->setConnectionParameter(QModbusDevice::SerialDataBitsParameter,8);
            modbusDevice->setConnectionParameter(QModbusDevice::SerialParityParameter,0);
            modbusDevice->setConnectionParameter(QModbusDevice::SerialStopBitsParameter,1);
            modbusDevice->setTimeout(200);
            modbusDevice->setNumberOfRetries(10);
            modbusDevice->connectDevice();

            Serial::AdaptronicStartStream();

        }

    }
    //OBD
    if (ecuSelect == 2)
    {
    m_obd->SelectPort(portName);
    }
    //Nissan Consult
    if (ecuSelect == 3)
    {
        m_nissanconsultcom->LiveReqMsg(1,0,0,0,1,0,0,1,1,1,1,1,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0);
        m_nissanconsultcom->openConnection(portName);

    }
    //Dicktator
    if (ecuSelect == 5)
    {

        initSerialPort();
        m_serialport->setPortName(portName);
        m_serialport->setBaudRate(QSerialPort::Baud19200);
        m_serialport->setParity(QSerialPort::NoParity);
        m_serialport->setDataBits(QSerialPort::Data8);
        m_serialport->setStopBits(QSerialPort::OneStop);
        m_serialport->setFlowControl(QSerialPort::NoFlowControl);

        if(m_serialport->open(QIODevice::ReadWrite) == false)
        {
            m_dashBoard->setSerialStat(m_serialport->errorString());
        }
        else
        {
            m_dashBoard->setSerialStat(QString("Connected to Serialport"));
        }
    }

}
void Serial::closeConnection()
{

    if(ecu == 0){
        m_serialport->close();
    }
    if(ecu == 1){
        modbusDevice->disconnectDevice();
    }
    if(ecu == 3){
        m_serialport->close();
    }
}

void Serial::update()
{


    bool bStatus = false;

    QStringList args;
    qint64      pid = 0;

    args << "&";
    bStatus = QProcess::startDetached("/home/pi/update.sh", args, ".", &pid);

}
void Serial::handleTimeout()
{   
    m_dashBoard->setTimeoutStat(QString("Is Timeout : Y"));

/*
    QString fileName = "Errors.txt";
    QFile mFile(fileName);
    if(!mFile.open(QFile::Append | QFile::Text)){
    }
    QTextStream out(&mFile);
    out << "Timeout Request Index " << m_writeData.toHex() << " lenght received "<< int(m_buffer.length())<< " Bytes "<< " Expected Bytes "<< int(Bytesexpected)<< " bytes " <<" Message "<< QByteArray(m_buffer.toHex()) <<endl;
    mFile.close();
    Serial::clear();
    m_serialport->flush();
    m_readData.clear();
    */
    m_timer.stop();
    m_serialport->close();
    if(m_serialport->open(QIODevice::ReadWrite) == false)
        {
            m_dashBoard->setSerialStat(m_serialport->errorString());
        }
        else
        {
            m_dashBoard->setSerialStat(QString("Connected to Serialport"));
        }

        requestIndex = 0;
        m_readData.clear();

        Serial::sendRequest(requestIndex);
}

void Serial::handleError(QSerialPort::SerialPortError serialPortError)
{
    if (serialPortError == QSerialPort::ReadError) {
        QString fileName = "Errors.txt";
        QFile mFile(fileName);
        if(!mFile.open(QFile::Append | QFile::Text)){
        }
        QTextStream out(&mFile);
        out << "Serial Error " << (m_serialport->errorString()) <<endl;
        mFile.close();
        m_dashBoard->setSerialStat(m_serialport->errorString());

    }
}



void Serial::readyToRead()
{
    if(ecu == 0)
    {
        m_readData = m_serialport->readAll();

        m_dashBoard->setRecvData(QString("Receive Data : " + m_readData.toHex()));
        //m_dashBoard->setRunStat(QString("Process : START"));

        Serial::apexiECU(m_readData);
    }

    if(ecu == 1)
    {

        auto reply = qobject_cast<QModbusReply *>(sender());
        if(!reply)
            return;
        if(reply->error() == QModbusDevice::NoError){
            const QModbusDataUnit unit = reply->result();
            m_decoder->decodeAdaptronic(unit);

        }



    }
    if(ecu == 3) //Dicktator ECU
    {
        m_readData = m_serialport->readAll();
        Serial::dicktatorECU(m_readData);
        m_readData.clear();
    }

}

void Serial::dicktatorECU(const QByteArray &buffer)
{
    //Appending the message until the patterns Start and End Are found , then removing all bytes before and after the message
    m_buffer.append(buffer);
    QByteArray startpattern("START");
    QByteArrayMatcher startmatcher(startpattern);
    QByteArray endpattern("END");
    QByteArrayMatcher endmatcher(endpattern);
    int pos = 0;
    while((pos = startmatcher.indexIn(m_buffer, pos)) != -1)
    {

        if (pos !=0)
        {
            m_buffer.remove(0, pos);
        }
        if (pos == 0 ) break;
    }
    int pos2 = 0;
    while((pos2 = endmatcher.indexIn(m_buffer, pos2)) != -1)
    {


    if (pos2 > 30)
        {
            m_buffer.remove(0,pos2-30);
        }

        if (pos2 == 30 )
        {
            m_dicktatorMsg = m_buffer;
            m_buffer.clear();
            m_decoder->decodeDicktator(m_dicktatorMsg);
            break;
        }
    }

}

void Serial::apexiECU(const QByteArray &buffer)
{
    /*(if (Bytesexpected != m_buffer.length())
    {
        m_timer.start(5000);
    }
    */
    m_buffer.append(buffer);

	
    QByteArray startpattern = m_writeData.left(1);
    QByteArrayMatcher startmatcher(startpattern);
    
    int pos = 0;
    while((pos = startmatcher.indexIn(m_buffer, pos)) != -1)
    {
    	m_dashBoard->setRunStat(m_buffer.toHex());
        if (pos !=0)
        {
            m_buffer.remove(0, pos);
            if (m_buffer.length() > Bytesexpected)
            {
                m_buffer.remove(Bytesexpected,m_buffer.length() );
            }
        }

        if (pos == 0 )
        {
            break;
        }


    }
    
    /*
    while(m_buffer.length() >= Bytesexpected)
    {
    	if (m_buffer[0] == m_writeData[0] && m_buffer[1] == m_writeData[1])
    	{
    		if (m_buffer.length() > Bytesexpected)
    		{
    			m_buffer = m_buffer.left(Bytesexpected);
    		}
    		
    		break;
    	}
    	else
    		m_buffer.remove(0, 2);
    }
    */
    

    if (m_buffer.length() == Bytesexpected)
    {
        m_dashBoard->setTimeoutStat(QString("Is Timeout : N"));

        m_apexiMsg =  m_buffer;
        m_buffer.clear();
        m_timer.stop();
        if(requestIndex <= 5){requestIndex++;}
        else{requestIndex = 2;}
        readData(m_apexiMsg);
        m_apexiMsg.clear();
        Serial::sendRequest(requestIndex);
        //        }
    }
}




void Serial::readData(QByteArray serialdata)
{

    if( serialdata.length() )
    {
        //Power FC Decode
        quint8 requesttype = serialdata[0];
        //qDebug() << "Processing Message"<< serialdata.toHex();

        //Write all OK Serial Messages to a file
        if(serialdata[1] + 1 == serialdata.length())
        {
            if (logging ==1 ){
                qDebug() << "logging";
                QString fileName = "OK Messages.txt";
                QFile mFile(fileName);
                if(!mFile.open(QFile::Append | QFile::Text)){
                    //qDebug() << "Could not open file for writing";
                }
                QTextStream out(&mFile);
                out << QByteArray(serialdata.toHex())<< endl;
                mFile.close();
            }



            if(requesttype == 0xF0){m_decoder->decodeAdv(serialdata);}
            if(requesttype == 0xDD){m_decoder->decodeSensorStrings(serialdata);}
            if(requesttype == 0xDE){m_decoder->decodeSensor(serialdata);}
            if(requesttype == 0x00){m_decoder->decodeAux(serialdata);}
            if(requesttype == 0x00){m_decoder->decodeAux2(serialdata);}
            if(requesttype == 0xDB){m_decoder->decodeMap(serialdata);}
            if(requesttype == 0xDA){m_decoder->decodeBasic(serialdata);}
            if(requesttype == 0xB8){m_decoder->decodeRevIdle(serialdata);}
            if(requesttype == 0x7D){m_decoder->decodeTurboTrans(serialdata);}
            if(requesttype == 0x76){m_decoder->decodeLeadIgn(serialdata, 0);}
            if(requesttype == 0x77){m_decoder->decodeLeadIgn(serialdata, 5);}
            if(requesttype == 0x78){m_decoder->decodeLeadIgn(serialdata, 10);}
            if(requesttype == 0x79){m_decoder->decodeLeadIgn(serialdata, 15);}
            if(requesttype == 0x81){m_decoder->decodeTrailIgn(serialdata, 0);}
            if(requesttype == 0x82){m_decoder->decodeTrailIgn(serialdata, 5);}
            if(requesttype == 0x83){m_decoder->decodeTrailIgn(serialdata, 10);}
            if(requesttype == 0x84){m_decoder->decodeTrailIgn(serialdata, 15);}
            if(requesttype == 0x86){m_decoder->decodeInjcorr(serialdata, 0);}
            if(requesttype == 0x87){m_decoder->decodeInjcorr(serialdata, 5);}
            if(requesttype == 0x88){m_decoder->decodeInjcorr(serialdata, 10);}
            if(requesttype == 0x89){m_decoder->decodeInjcorr(serialdata, 15);}

            if(requesttype == 0xB0){m_decoder->decodeFuelBase(serialdata, 0);}
            if(requesttype == 0xB1){m_decoder->decodeFuelBase(serialdata, 1);}
            if(requesttype == 0xB2){m_decoder->decodeFuelBase(serialdata, 2);}
            if(requesttype == 0xB3){m_decoder->decodeFuelBase(serialdata, 3);}
            if(requesttype == 0xB4){m_decoder->decodeFuelBase(serialdata, 4);}
            if(requesttype == 0xB5){m_decoder->decodeFuelBase(serialdata, 5);}
            if(requesttype == 0xB6){m_decoder->decodeFuelBase(serialdata, 6);}
            if(requesttype == 0xB7){m_decoder->decodeFuelBase(serialdata, 7);}

            if(requesttype == 0xF5){m_decoder->decodeVersion(serialdata);}
            if(requesttype == 0xF3){m_decoder->decodeInit(serialdata);}
            if(requesttype == 0xAB){m_decoder->decodeBoostCont(serialdata);}
            if(requesttype == 0x7B){m_decoder->decodeInjOverlap(serialdata);}
            if(requesttype == 0x92){m_decoder->decodeInjPriLagvsBattV(serialdata);}
            if(requesttype == 0x9F){m_decoder->decodeInjScLagvsBattV(serialdata);}
            if(requesttype == 0x8D){m_decoder->decodeFuelInjectors(serialdata);}
        }
        serialdata.clear();
        

    }


}
void Serial::handleBytesWritten(qint64 bytes)
{
    m_bytesWritten += bytes;
    if (m_bytesWritten == m_writeData.size()) {
        m_bytesWritten = 0;
        //qDebug() <<("Data successfully sent to port") << (m_serialport->portName());

    }
}
// Serial requests are send via Serial
void Serial::writeRequestPFC(QByteArray p_request)
{
    //qDebug() << "write request" << p_request.toHex();
    m_writeData = p_request;
    qint64 bytesWritten = m_serialport->write(p_request);
    m_dashBoard->setSerialStat(QString("Sending Request " + p_request.toHex()));

    //Action to be implemented
    if (bytesWritten == -1) {
        m_dashBoard->setSerialStat(m_serialport->errorString());
        //qDebug() << "Write request to port failed" << (m_serialport->errorString());
    } else if (bytesWritten != m_writeData.size()) {
        m_dashBoard->setSerialStat(m_serialport->errorString());
        //qDebug() << "could not write complete request to port" << (m_serialport->errorString());
    }

    //m_timer.start(5000);
}

//Power FC requests

void Serial::sendRequest(int requestIndex)
{
    switch (requestIndex){
    /*
    case 0:
        //First request from (this is what FC Edit does seems to get a 4 or 8 Byte response dependant on Aux inputs ??)
        Serial::writeRequestPFC(QByteArray::fromHex("0102FC"));
        if (interface ==0)
        {Bytesexpected = 4;}
        if (interface ==1)
        {Bytesexpected = 8;}
        break;
*/
    //case 1:
    case 0:
        //Init Platform (This returns the Platform String )
        Serial::writeRequestPFC(QByteArray::fromHex("F3020A"));
        Bytesexpected = 11;
        break;
        /*
    case 2:
        //Serial::getWarConStrFlags();
        Serial::writeRequestPFC(QByteArray::fromHex("D60227"));
        Bytesexpected = 88;
        break;
    case 3:
        //Serial::getVersion();
        Serial::writeRequestPFC(QByteArray::fromHex("F50208"));
        Bytesexpected = 8;
        break;
    case 4:
        //Serial::getMapRef();
        Serial::writeRequestPFC(QByteArray::fromHex("8A0273"));
        Bytesexpected = 83;
        break;
    case 5:
        //Serial::getRevIdle();
        Serial::writeRequestPFC(QByteArray::fromHex("B80245"));
        Bytesexpected = 17;
        break;
    case 6:
        //Serial::getLeadign1();
        Serial::writeRequestPFC(QByteArray::fromHex("760287"));
        Bytesexpected = 103;
        break;
    case 7:
        //Serial::getLeadign2();
        Serial::writeRequestPFC(QByteArray::fromHex("770286"));
        Bytesexpected = 103;
        break;
    case 8:
        //Serial::getLeadign3();
        Serial::writeRequestPFC(QByteArray::fromHex("780285"));
        Bytesexpected = 103;
        break;
    case 9:
        //Serial::getLeadign4();
        Serial::writeRequestPFC(QByteArray::fromHex("790284"));
        Bytesexpected= 103;
        break;
    case 10:
        //Serial::getTrailIgn1();
        Serial::writeRequestPFC(QByteArray::fromHex("81027C"));
        Bytesexpected= 103;
        break;
    case 11:
        //Serial::getTrailIgn2();
        Serial::writeRequestPFC(QByteArray::fromHex("82027B"));
        Bytesexpected= 103;
        break;
    case 12:
        //Serial::getTrailIgn3();
        Serial::writeRequestPFC(QByteArray::fromHex("83027A"));
        Bytesexpected = 103;
        break;
    case 13:
        //Serial::getTrailIgn4();
        Serial::writeRequestPFC(QByteArray::fromHex("840279"));
        Bytesexpected = 103;
        break;
    case 14:
        //Serial::getPimStrInjA();
        Serial::writeRequestPFC(QByteArray::fromHex("CB0232"));
        Bytesexpected = 110;
        break;
    case 15:
        //Serial::getInjOverlap();
        Serial::writeRequestPFC(QByteArray::fromHex("7B0282"));
        Bytesexpected = 9;
        break;
    case 16:
        //Serial::getInjvsFuelT();
        Serial::writeRequestPFC(QByteArray::fromHex("7C0281"));
        Bytesexpected = 12;
        break;
    case 17:
        //Serial::getTurboTrans();
        Serial::writeRequestPFC(QByteArray::fromHex("7D0280"));
        Bytesexpected = 12;
        break;
    case 18:
        //Serial::getOilervsWaterT();
        Serial::writeRequestPFC(QByteArray::fromHex("7E027F"));
        Bytesexpected = 9;
        break;
    case 19:
        //Serial::getFanvsWater();
        Serial::writeRequestPFC(QByteArray::fromHex("7F027E"));
        Bytesexpected = 6;
        break;
    case 20:
        //Serial::getInjcorr1();
        Serial::writeRequestPFC(QByteArray::fromHex("860277"));
        Bytesexpected = 103;
        break;
    case 21:
        //Serial::getInjcorr2();
        Serial::writeRequestPFC(QByteArray::fromHex("870276"));
        Bytesexpected = 103;
        break;
    case 22:
        //Serial::getInjcorr3();
        Serial::writeRequestPFC(QByteArray::fromHex("880275"));
        Bytesexpected = 103;
        break;
    case 23:
        //Serial::getInjcorr4();
        Serial::writeRequestPFC(QByteArray::fromHex("890274"));
        Bytesexpected = 103;
        break;
    case 24:
        //Serial::getFuelInj();
        Serial::writeRequestPFC(QByteArray::fromHex("8D0270"));
        Bytesexpected = 27;
        break;
    case 25:
        //Serial::getCranking();
        Serial::writeRequestPFC(QByteArray::fromHex("8E026F"));
        Bytesexpected = 15;
        break;
    case 26:
        //Serial::getWaterTcorr();
        Serial::writeRequestPFC(QByteArray::fromHex("8F026E"));
        Bytesexpected = 17;
        break;
    case 27:
        //Serial::getInjvsWaterBoost();
        Serial::writeRequestPFC(QByteArray::fromHex("90026D"));
        Bytesexpected = 9;
        break;
    case 28:
        //Serial::getInjvsAirTBoost();
        Serial::writeRequestPFC(QByteArray::fromHex("91026C"));
        Bytesexpected = 11;
        break;
    case 29:
        //Serial::getInjPrimaryLag();
        Serial::writeRequestPFC(QByteArray::fromHex("92026B"));
        Bytesexpected = 15;
        break;
    case 30:
        //Serial::getAccInj();
        Serial::writeRequestPFC(QByteArray::fromHex("93026A"));
        Bytesexpected = 28;
        break;
    case 31:
        //Serial::getInjvsAccel();
        Serial::writeRequestPFC(QByteArray::fromHex("940269"));
        Bytesexpected = 12;
        break;
    case 32:
        //Serial::getIgnvsAircold();
        Serial::writeRequestPFC(QByteArray::fromHex("960267"));
        Bytesexpected = 7;
        break;
    case 33:
        //Serial::getIgnvsWater();
        Serial::writeRequestPFC(QByteArray::fromHex("980265"));
        Bytesexpected = 7;
        break;
    case 34:
        //Serial::getIgnvsAirwarm();
        Serial::writeRequestPFC(QByteArray::fromHex("9A0263"));
        Bytesexpected = 9;
        break;
    case 35:
        //Serial::getLIgnvsRPM();
        Serial::writeRequestPFC(QByteArray::fromHex("9B0262"));
        Bytesexpected = 9;
        break;
    case 36:
        //Serial::getIgnvsBatt();
        Serial::writeRequestPFC(QByteArray::fromHex("9C0261"));
        Bytesexpected = 9;
        break;
    case 37:
        //Serial::getBoostvsIgn();
        Serial::writeRequestPFC(QByteArray::fromHex("9D0260"));
        Bytesexpected = 7;
        break;
    case 38:
        //Serial::getTrailIgnvsRPM();
        Serial::writeRequestPFC(QByteArray::fromHex("9E025F"));
        Bytesexpected = 9;
        break;
    case 39:
        //Serial::getInjSecLagvsBattV();
        Serial::writeRequestPFC(QByteArray::fromHex("9F025E"));
        Bytesexpected = 15;
        break;
    case 40:
        //Serial::getKnockWarn();
        Serial::writeRequestPFC(QByteArray::fromHex("A90254"));
        Bytesexpected = 7;
        break;
    case 41:
        //Injejtor warning
        Serial::writeRequestPFC(QByteArray::fromHex("A80255"));
        Bytesexpected = 7;
        break;
    case 42:
        //Serial::getO2Feedback();
        Serial::writeRequestPFC(QByteArray::fromHex("AA0253"));
        Bytesexpected = 6;
        break;
    case 43:
        //Serial::getBoostcontrol();
        Serial::writeRequestPFC(QByteArray::fromHex("AB0252"));
        Bytesexpected = 14;
        break;
    case 44:
        //Serial::getSettingProtections();
        Serial::writeRequestPFC(QByteArray::fromHex("AC0251"));
        Bytesexpected = 13;
        break;
    case 45:
        //Serial::getTunerString();
        Serial::writeRequestPFC(QByteArray::fromHex("AD0250"));
        Bytesexpected = 11;
        break;
    case 46:
        //Serial::getFuelBase0();
        Serial::writeRequestPFC(QByteArray::fromHex("B0024D"));
        Bytesexpected = 103;
        break;
    case 47:
        //Serial::getFuelBase1();
        Serial::writeRequestPFC(QByteArray::fromHex("B1024C"));
        Bytesexpected = 103;
        break;
    case 48:
        //Serial::getFuelBase2();
        Serial::writeRequestPFC(QByteArray::fromHex("B2024B"));
        Bytesexpected = 103;
        break;
    case 49:
        //Serial::getFuelBase3();
        Serial::writeRequestPFC(QByteArray::fromHex("B3024A"));
        Bytesexpected = 103;
        break;
    case 50:
        //Serial::getFuelBase4();
        Serial::writeRequestPFC(QByteArray::fromHex("B40249"));
        Bytesexpected = 103;
        break;
    case 51:
        //Serial::getFuelBase5();
        Serial::writeRequestPFC(QByteArray::fromHex("B50248"));
        Bytesexpected = 103;
        break;
    case 52:
        //Serial::getFuelBase6();
        Serial::writeRequestPFC(QByteArray::fromHex("B60247"));
        Bytesexpected = 103;
        break;
    case 53:
        //Serial::getFuelBase7();
        Serial::writeRequestPFC(QByteArray::fromHex("B70246"));
        Bytesexpected = 103;
        break;
    case 54:
        //Serial::getInjvsAirTemp();
        Serial::writeRequestPFC(QByteArray::fromHex("B90244"));
        Bytesexpected = 15;
        break;
    case 55:
        //Serial::getInjvsTPS();
        Serial::writeRequestPFC(QByteArray::fromHex("BB0242"));
        Bytesexpected = 11;
        break;
    case 56:
        //Serial::getPIMScaleOffset();
        Serial::writeRequestPFC(QByteArray::fromHex("BC0241"));
        Bytesexpected = 23;
        break;
    case 57:
        //Init Platform
        Serial::writeRequestPFC(QByteArray::fromHex("F3020A"));
        Bytesexpected = 11;
        break;
*/
        //case 58:
    case 1:
        //Serial::getSensorStrings();
        Serial::writeRequestPFC(QByteArray::fromHex("DD0220"));
        Bytesexpected = 83;
        break;



        // Live Data
    case 2:
        //Serial::getAdvData();
        Serial::writeRequestPFC(QByteArray::fromHex("F0020D"));
        Bytesexpected = 33;
        break;

    case 3:
        //Serial::getMapIndices();
        Serial::writeRequestPFC(QByteArray::fromHex("DB0222"));
        Bytesexpected = 5;
        break;
    case 4:
        //Serial::getSensorData();
        Serial::writeRequestPFC(QByteArray::fromHex("DE021F"));
        Bytesexpected = 21;
        break;
    case 5:
        //Serial::getBasic();
        Serial::writeRequestPFC(QByteArray::fromHex("DA0223"));
        Bytesexpected = 23;
        break;
    case 6:
        //Serial::getAux();
        Serial::writeRequestPFC(QByteArray::fromHex("0002FD"));
        Bytesexpected = 7;

        break;
    }
    
    m_timer.start(5000);
}


// Adaptronic streaming comms

void Serial::AdaptronicStartStream()
{
    auto *reply = modbusDevice->sendReadRequest(QModbusDataUnit(QModbusDataUnit::HoldingRegisters, 4096, 21),1); // read first twenty-one realtime values
    if (!reply->isFinished())
        connect(reply, &QModbusReply::finished, this,&Serial::readyToRead);
    else
        delete reply;

}
void Serial::AdaptronicStopStream()
{

}


//function for Start Logging
void Serial::startLogging(const QString &logfilenameSelect, const int &loggeron)
{
    loggingstatus = loggeron;
    Logfilename = logfilenameSelect;
    if (ecu == 0)    //Apexi
    {
        {
            m_decoder->loggerApexi(Logfilename);
        }
    }
    if (ecu == 1)    //Adaptronic
    {
        QString filename = Logfilename + ".csv";
        QFile file( filename );
        //qDebug() << "Adaptronic start Log";
        if ( file.open(QIODevice::ReadWrite) )
        {
            QTextStream stream( &file );
            stream << "Time (s),RPM,MAP (kPa),MAT (°C),WT (°C),AuxT (°C),AFR,Knock,TPS %,Idle,MVSS (km/h),SVSS (km/h),Batt (V),Inj 1 (ms), Inj 2 (ms),Inj 3 (ms),Inj 4 (ms),Ign 1 (°),Ign 2 (°),Ign 3 (°),Ign 4 (°),Trim" << endl;
        }
        file.close();
        m_decoder->loggerAdaptronic(Logfilename);
        m_decoder->loggerActivationstatus(loggingstatus);
    }


    return;
}

//function for Stop Logging
void Serial::stopLogging(const int &loggeron)
{
    loggingstatus = loggeron;
    m_decoder->loggerActivationstatus(loggingstatus);
    return;
}


void Serial::Auxcalc (const QString &unitaux1,const int &an1V0,const int &an2V5,const QString &unitaux2,const int &an3V0,const int &an4V5)
{
    int aux1min = an1V0;
    int aux2max = an2V5;
    int aux3min = an3V0;
    int aux4max = an4V5;
    QString Auxunit1 = unitaux1;
    QString Auxunit2 = unitaux2;

    m_decoder->calculatorAux(aux1min,aux2max,aux3min,aux4max,Auxunit1,Auxunit2);
}


