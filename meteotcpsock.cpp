/*
 * Copyright © 2018-2019 Yaroslav Shkliar <mail@ilit.ru>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Research Laboratory of IT
 * www.ilit.ru on e-mail: mail@ilit.ru
 */

#include "meteotcpsock.h"

#include <QDebug>

#ifdef DUSTTCP_H
MeteoTcpSock::MeteoTcpSock(QObject *parent , QString *ip, quint16 *port) : QObject (parent)

{


    m_sock = new QTcpSocket(this);

    connect(m_sock, SIGNAL(readyRead()), this, SLOT(readData()));
    connect(m_sock, SIGNAL(bytesWritten(qint64)), this, SLOT(writes()));

    connect(m_sock, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(displayError(QAbstractSocket::SocketError)));
    //connect(this, SIGNAL(dataReady(QByteArray&)), this, SLOT(setData(QByteArray&)) );

    changeInterface(*ip, *port);
    m_sock->setSocketOption(QAbstractSocket::LowDelayOption, 0);
    //qDebug() << "Socket " << m_sock->socketOption(QAbstractSocket::SendBufferSizeSocketOption);
    // m_sock->setSocketOption(QAbstractSocket::SendBufferSizeSocketOption, 1024);
    // qDebug() << "Socket next " << m_sock->socketOption(QAbstractSocket::SendBufferSizeSocketOption);

    m_sock->setSocketOption(QAbstractSocket::TypeOfServiceOption, 64);

    measure = new  QMap<QString, float>;
    measure_prev = new  QMap<QString, float>;

    measure_prev->insert("bar", 0.0f);
    measure_prev->insert("temp_in", 0.0f);
    measure_prev->insert("hum_in", 0.0f);
    measure_prev->insert("temp_out", 0.0f);
    measure_prev->insert("speed_wind", 0.0f);
    measure_prev->insert("dir_wind", 0.0f);
    measure_prev->insert("dew_pt", 0.0f);
    measure_prev->insert("hum_out", 0.0f);
    measure_prev->insert("heat_indx", 0.0f);
    measure_prev->insert("chill_wind", 0.0f);
    measure_prev->insert("thsw_indx", 0.0f);
    measure_prev->insert("rain_rate", 0.0f);
    measure_prev->insert("uv_indx", 0.0f);
    measure_prev->insert("rad_solar", 0.0f);
    //measure->insert("rain_daily", 0);
    measure_prev->insert("rain", 0.0f); //mm per hour
    measure_prev->insert("et", 0.0f); //evaporotransportation in mm per day
    //measure->insert("batt_remote", 0);

    is_read = false;
    status = "";
    sample_t = 0;

    connected = m_sock->state();


    qDebug() << "Meteostation handling has been initialized.\n\r";

}
#endif



MeteoTcpSock::~MeteoTcpSock()
{
    m_sock->close();
    m_sock->disconnectFromHost();
}



void MeteoTcpSock::changeInterface(const QString &address, quint16 portNbr)
{
    m_sock->connectToHost(address, portNbr);
}





void MeteoTcpSock::on_cbEnabled_clicked(bool checked)
{
    if (checked)
    {
    }
    else {
        m_sock->disconnectFromHost();
    }
    //emit tcpPortActive(checked);
}


void MeteoTcpSock::readData()
{

    QStringList list;
    int ind;
    int running;
    QRegExp xRegExp("(-?\\d+(?:[\\.,]\\d+(?:e\\d+)?)?)");
    float _result;

    QByteArray data = m_sock->readAll();

    qDebug() << "Meteostation sent data: " << data << " lenght - " << data.length() << " \n\r";

    this->is_read = false;

    //emit (dataReady(data));


    blockSize = 0;

    if (sample_t == 0)
    {
        measure->insert("bar", 0.0f);
        measure->insert("temp_in", 0.0f);
        measure->insert("hum_in", 0.0f);
        measure->insert("temp_out", 0.0f);
        measure->insert("speed_wind", 0.0f);
        measure->insert("dir_wind", 0.0f);
        measure->insert("dew_pt", 0.0f);
        measure->insert("hum_out", 0.0f);
        measure->insert("heat_indx", 0.0f);
        measure->insert("chill_wind", 0.0f);
        measure->insert("thsw_indx", 0.0f);
        measure->insert("rain_rate", 0.0f);
        measure->insert("uv_indx", 0.0f);
        measure->insert("rad_solar", 0.0f);
        //measure->insert("rain_daily", 0);
        measure->insert("rain", 0.0f); //mm per hour
        measure->insert("et", 0.0f); //evaporotransportation in mm per day
    }

    _result = ((float)(((uchar(data[9])<<8)+(uchar(data[8]))))/1000)*25.4f;


    if ( (uchar(data[8]) == 0xff) || (_result > 800))
    {
        measure->insert("bar",  measure->value("bar") );//result is 0 - inchs Hg TO mm Hg Conversion Formula

    }
    else
    {

        _result = compare (_result, measure_prev->value("bar"));
        measure_prev->insert("bar",_result);
        measure->insert("bar",  measure->value("bar") + _result);//inchs Hg TO mm Hg Conversion Formula
    }

    _result =  ((float)((uchar(data[11])<<8) + (uchar(data[10])))/10-32)*5/9;

    if (( uchar(data[10]) == 0xff)|| (_result > 70))
    {
        measure->insert("temp_in", measure->value("temp_in") ); //Fahrenheit TO Celsius Conversion Formula

    } else {


        _result = compare (_result, measure_prev->value("temp_in"));
        measure_prev->insert("temp_in",_result);

        measure->insert("temp_in", measure->value("temp_in") + _result); //Fahrenheit TO Celsius Conversion Formula
    }

    _result =  (float)(uchar(data[12]));


    if (( uchar(data[12]) == 0xff) ||(_result > 100))
    {
        measure->insert("hum_in", measure->value("hum_in"));

    } else {
        _result = compare (_result, measure_prev->value("hum_in"));
        measure_prev->insert("hum_in",_result);

        measure->insert("hum_in", measure->value("hum_in") + _result);
    }

    _result = ((float)((uchar(data[14])<<8) + (uchar(data[13])))/10-32)*5/9;

    if (( uchar(data[13]) == 0xff)||(_result > 70))
    {
        measure->insert("temp_out", measure->value("temp_out"));
    } else {


        _result = compare (_result, measure_prev->value("temp_out"));
        measure_prev->insert("temp_out",_result);

        measure->insert("temp_out", measure->value("temp_out") + _result); //Fahrenheit TO Celsius Conversion Formula
    }

    _result = (float)(uchar(data[15]))*1609.344f/3600.0f;


    if (( uchar(data[15]) == 0xff)|| (_result > 100))
    {
        measure->insert("speed_wind", measure->value("speed_wind") );
    }

    else {

        _result = compare (_result, measure_prev->value("speed_wind"));
        measure_prev->insert("speed_wind",_result);

        measure->insert("speed_wind", measure->value("speed_wind") + _result);//mile to meter convertion formula
    }

    _result = (float)((uchar(data[18])<<8) + uchar(data[17]));


    if (( uchar(data[17]) == 0xff) || (_result > 360))
    {
        measure->insert("dir_wind",  measure->value("dir_wind") );

    } else {


        _result = compare (_result, measure_prev->value("dir_wind"));
        measure_prev->insert("dir_wind",_result);

        measure->insert("dir_wind",  measure->value("dir_wind") + _result);
    }

    _result = ((float)((uchar(data[32])<<8) + (uchar(data[31])))-32)*5/9;


    if (( uchar(data[31]) == 0xff) || (_result > 70))
    {
        measure->insert("dew_pt",  measure->value("dew_pt") ); //Fahrenheit TO Celsius Conversion Formula

    }
    else {
        _result = compare (_result, measure_prev->value("dew_pt"));
        measure_prev->insert("dew_pt",_result);

        measure->insert("dew_pt",  measure->value("dew_pt") + _result); //Fahrenheit TO Celsius Conversion Formula
    }

        _result = (float)(uchar(data[34]));

    if (( uchar(data[34]) == 0xff) || (_result > 100))
    {
        measure->insert("hum_out", measure->value("hum_out") );

    }
    else {


        _result = compare (_result, measure_prev->value("hum_out"));
        measure_prev->insert("hum_out",_result);

        measure->insert("hum_out", measure->value("hum_out") + _result);
    }


    _result = ((float)((uchar(data[37])<<8) + (uchar(data[36])))-32)*5/9;

    if (( uchar(data[36]) == 0xff)||(_result > 70)) //if dashed data - not signal
        measure->insert("heat_indx", measure->value("heat_indx") );
    else{
        measure->insert("heat_indx", measure->value("heat_indx") + _result); //Fahrenheit TO Celsius Conversion Formula
    }

    _result = ((float)((uchar(data[39])<<8) + (uchar(data[38])))-32)*5/9;

    if (( uchar(data[38]) == 0xff) || (_result > 70))

        measure->insert("chill_wind", measure->value("chill_wind") );
    else {

        measure->insert("chill_wind", measure->value("chill_wind") + _result); //Fahrenheit TO Celsius Conversion Formula
    }

    _result = ((float)((uchar(data[41])<<8) + (uchar(data[40])))-32)*5/9;

    if (( uchar(data[40]) == 0xff) || (_result > 70))
        measure->insert("thsw_indx", measure->value("thsw_indx") );
    else {
        measure->insert("thsw_indx", measure->value("thsw_indx") + _result ); //Fahrenheit TO Celsius Conversion Formula
    }

    _result =  ((float)(uchar(data[43])<<8) + (uchar(data[42])))*0.2f;

    if (( uchar(data[42]) == 0xff) || (_result > 100))
    {
        measure->insert("rain_rate", measure->value("rain_rate"));//mm per hour

    }
    else {


        _result = compare (_result, measure_prev->value("rain_rate"));
        measure_prev->insert("rain_rate",_result);

        measure->insert("rain_rate", measure->value("rain_rate") + _result);//mm per hour
    }

    _result =  (float)(uchar(data[44]));


    if ((uchar(data[44]) == 0xff) ||(_result > 100))
        measure->insert("uv_indx", measure->value("uv_indx") );
    else {
        measure->insert("uv_indx", measure->value("uv_indx") +  _result);
    }

    _result = ((float)(uchar(data[46])<<8) + (uchar(data[45])));

    if ((uchar(data[45]) == 0xff) || (_result > 70))
        measure->insert("rad_solar", measure->value("rad_solar") +0.0f);
    else {
        measure->insert("rad_solar",  measure->value("rad_solar") + _result);//unit in watt on m2
    }
    //measure->insert("rain_daily",  measure->value("rain_daily") + ((float)(uchar(data[52])<<8) + (uchar(data[51])))*0.2f);//last day quantity
    _result = ((float)(uchar(data[56])<<8) + (uchar(data[55])))*0.2f;


    if (( uchar(data[55]) == 0xff) ||(_result > 100))
    {
        measure->insert("rain", measure->value("rain") ); //last hour quantity in mm

    }
    else {


        _result = compare (_result, measure_prev->value("rain"));
        measure_prev->insert("rain",_result);
        measure->insert("rain", measure->value("rain") + _result); //last hour quantity in mm
    }


    _result =  ((float)(((uchar(data[58])<<8)+(uchar(data[57]))))/1000)*25.4f;


    if (( uchar(data[57]) == 0xff) || (_result > 10))
    {
        measure->insert("et",  measure->value("et") );//inchs  TO mm Conversion Evapotranspiration Formula

    }
    else {

        _result = compare (_result, measure_prev->value("et"));
        measure_prev->insert("et",_result);

        measure->insert("et",  measure->value("et") +_result);//inchs  TO mm Conversion Evapotranspiration Formula
    }
    //   measure->insert("batt_remote",  measure->value("batt_remote") + (float)(uchar(data[87])));//%

    sample_t++;
    first_run = false;
}

void MeteoTcpSock::displayError(QAbstractSocket::SocketError socketError)
{
    switch (socketError) {
    case QAbstractSocket::RemoteHostClosedError:
        break;
    case QAbstractSocket::HostNotFoundError:
        qDebug()<<   ("Meteostation handling error: The host was not found. Please check the "
                      "host name and port settings.\n\r");
        break;
    case QAbstractSocket::ConnectionRefusedError:
        qDebug()<< ("Meteostation handling error: The connection was refused by the peer. "
                    "Make sure the fortune server is running, "
                    "and check that the host name and port "
                    "settings are correct.\n\r");
        break;
    default:
        qDebug()<< ("Meteostation handling error: ") << (m_sock->errorString())<<"\n\r";
    }
    if (m_sock->isOpen())
        m_sock->close();
    connected = m_sock->state();


}

void MeteoTcpSock::sendData( char *data)
{

    char *str = (char*)(malloc(strlen(data) * sizeof(char) + 1));
    *str = '\0';
    strcat(str, data);
    strcat(str,  "\n");
    qint64 lnt = qint64(strlen(str));

    lnt = m_sock->write(str, lnt);
    // lnt = m_sock->flush();

    qDebug()<< "Meteostation command: " << data <<"\n\r";
}

void MeteoTcpSock::writes()
{
    qDebug()<< "written " ;
}

float MeteoTcpSock::compare(float _in, float _prev)
{
    if (!first_run ){
        if (std::abs(_prev - _in) < (_prev*0.15f)) //new value don't exceed of 15% per sample
        {
            return _in;
        } else {
            return  _prev;
        }
    } else {

        return  _in;
    }

}
