#include "device.h"
#include <QtConcurrent/QtConcurrent>
#include <QDebug>
#include "settings.h"
#include <QtMath>


std::map <E_DEVICE_SPEED, QString> Device::kSpeedMap= {{E_SPEED_025_MMS, "0.25 мм/c"},
                                                {E_SPEED_050_MMS, "0.50 мм/c"},
                                                {E_SPEED_100_MMS, "1.00 мм/c"},
                                                {E_SPEED_200_MMS, "2.00 мм/c"}};

//======================================================================
Device::Device(QString port, E_DEVICE_TYPE type): thread_running_(false), trace_started_(false), require_stop_(false)
{
    port_ = new serial::Serial(port.toStdString(), 460800, serial::Timeout::simpleTimeout(6000));
//serial::Timeout t =;
  //  port_->setTimeout(t);

    device_type_ = type;
}

//======================================================================
Device::Device(): thread_running_(false), trace_started_(false), require_stop_(false) //do autodetect
{
    CalibrationData();
    port_ = nullptr;

    detection_timer_ = new QTimer();

    connect(detection_timer_, &QTimer::timeout, this, [=]() {

            std::string portname;
            if (detect(&portname))
            {
                if (port_)
                    delete port_;

                port_ = new serial::Serial(portname, 460800);
                serial::Timeout t =serial::Timeout::simpleTimeout(500);
                port_->setTimeout(t);
                address_ = portname;
                pts_readed_ = 0;
                emit deviceOpened();
                thread_running_ = false;
                trace_started_ = false;
                require_stop_ = false;
                start();
            }
            else
            {

                emit deviceLost();
                delete port_;
                port_ = nullptr;
                detection_timer_->start(1000);
            }
    }
    );


    detection_timer_->setSingleShot(true);
    detection_timer_->start(1000);
}


//======================================================================
Device::~Device()
{
    qDebug() << "~Device";
    stop();
    try {
        mainthread_ft_.waitForFinished();
    } catch (std::exception e) {
        qDebug() << "Device main thread wait exception" << e.what();
    }

    if (port_)
        delete port_;
}

//======================================================================
bool Device::connected()
{
    return port_->isOpen();
}

//======================================================================
bool Device::running()
{
    return mainthread_ft_.isRunning();
}


//======================================================================
void Device::start()
{
    mainthread_ft_ = QtConcurrent::run(this, &Device::mainThread);
}
//======================================================================
void Device::stop()
{
    require_stop_ = true;
}

//======================================================================
long Device::pointsReaded()
{
    return pts_readed_;
}


//======================================================================
void Device::moveX(int val)
{
    if (val == 0 )
        writeOrDeviceLost("DX0");
    else if (val > 0)
        writeOrDeviceLost("DX-");
    else if (val < 0)
        writeOrDeviceLost("DX+");
}

//======================================================================
void Device::moveY(int val)
{
    if (val == 0 )
        writeOrDeviceLost("DY0");
    else if (val < 0)
        writeOrDeviceLost("DY-");
    else if (val > 0)
        writeOrDeviceLost("DY+");
}

//======================================================================
void Device::moveZ(int val)
{
    if (val == 0 )
        writeOrDeviceLost("DZ0");
    else if (val < 0)
        writeOrDeviceLost("DZ-");
    else if (val > 0)
        writeOrDeviceLost("DZ+");
}

//======================================================================
void Device::moveXFixed(double val)
{
    int ival = val*10;

    std::string data(4,0);
    qToLittleEndian(val, &data);
    if (ival < 0)
        writeOrDeviceLost("Dx-"+data.substr(0,3));
    else if (ival > 0)
        writeOrDeviceLost("Dx+"+data.substr(0,3));

}

//======================================================================
void Device::moveYFixed(double val)
{
    int ival = val*10;

    std::string data(4,0);
    qToLittleEndian(val, &data);
    if (ival < 0)
        writeOrDeviceLost("Dy-"+data.substr(0,3));
    else if (ival > 0)
        writeOrDeviceLost("Dy+"+data.substr(0,3));
}

//======================================================================
void Device::moveZFixed(double val)
{
    int ival = val*10;

    std::string data(4,0);
    qToLittleEndian(val, &data);
    if (ival < 0)
        writeOrDeviceLost("Dz-"+data.substr(0,3));
    else if (ival > 0)
        writeOrDeviceLost("Dz+"+data.substr(0,3));
}

//======================================================================
void Device::moveTipUp()
{
    if (limit_tip_acsend_)
        writeOrDeviceLost("Cu" + tip_level_.toStdString());
    else
        writeOrDeviceLost("CU");
}

//======================================================================
void Device::moveTipDown()
{
    writeOrDeviceLost("CD");
}

//======================================================================
void Device::autoMaxCalib()
{
    writeOrDeviceLost("CF");
}

//======================================================================
void Device::startTrace()
{
    writeOrDeviceLost("CS");
    trace_started_ = true;
}

//======================================================================
void Device::stopTrace()
{
    trace_started_ = false;
    st_bytes_read_ =false;
    writeOrDeviceLost("CC");
}

//======================================================================
void Device::setSpeed(int spd)
{
    writeOrDeviceLost((QString("PS")+QString::number(spd)).toStdString());
}

//======================================================================
inline QByteArray Device::readOrDeviceLost(int bc) {


    if (require_stop_)
    {

        return QByteArray();
    }


    QByteArray bytes;
    try {
         bytes = QByteArray::fromStdString(port_->read(bc));
    } catch (serial::SerialException e) {
        emit deviceLost();
    }



    if (bytes.size() > 0)
        return bytes;
    else
    {
        qDebug() << "NO BYTES";

       // port_->close();
       // emit deviceLost();
       // require_stop_ = true;
    }
    return  QByteArray();
}

//======================================================================
void Device::writeOrDeviceLost(const std::string &data)
{

    try {
        if (port_)
            port_->write(data);
    }
    catch (...) {
        qDebug() << "Exception during device write";
        port_->close();
        emit deviceLost();
    }

}

//======================================================================
void Device::find()
{
    detection_timer_->start();
}

void Device::setType(int type)
{
    device_type_ = (E_DEVICE_TYPE) type;
}

E_DEVICE_TYPE Device::type()
{
    return device_type_;
}

//======================================================================
void Device::mainThread()
{

    qDebug() << "Main thread tick";

    pts_readed_ = 0;
    thread_running_ = true;

    while (!require_stop_)
    {
        int bytes_skipped = 0;

        if (device_type_ == CNTR220)
        {
            while (!require_stop_)
            {
                QByteArray bytes = readOrDeviceLost(1);

                if (bytes[0] == 'B')
                {
                    break;
                }
                else
                    bytes_skipped++;
            }

            QByteArray actual_data = readOrDeviceLost(4+3+1+1);

            int angle  = qFromLittleEndian<qint32>(actual_data.mid(0,4)) - 0x40000000;  //0x4000000 is zero-value
            uint xval = (qFromLittleEndian<quint32>(actual_data.mid(4, 3)) - 0x400000) * 2; //0x40000 is zero-value, 2 is nm*perstep


            if (!limit_tip_acsend_)
                tip_level_ = actual_data.mid(0,4);

            uint alarm_byte = actual_data[8];

            pts_readed_ ++;


            double impulse_len_mm = angle * Settings::instance().device.length_per_pulse_angle_nm * 1e-6;
            double len_per_r = Settings::instance().device.mount_point_radius_mm * impulse_len_mm;

            double angle_rad = qDegreesToRadians( (len_per_r * M_2_PI / 360.0));

            emit dataRecieved(angle_rad, xval, alarm_byte);

            if (alarm_byte != 0 )
                trace_started_ = false;
        }

        if (device_type_ == PROF130)
        {

            while (!require_stop_)
            {
                QByteArray bytes = readOrDeviceLost(1);

                if (bytes[0] == 'B' || bytes[0] == 'Z' )
                {
                    break;
                }
                else
                    bytes_skipped++;
            }

            //for profi
            QByteArray actual_data = readOrDeviceLost(2);


            uint xval = pts_readed_;

            uint val = qFromBigEndian<quint16>(actual_data);



            pts_readed_ ++;
            emit dataRecieved(val, xval, 0);

        }

        if (device_type_ == KRUG)
        {
            qDebug() << "DEVICE KRUG Not imeplemented yet, working as PRF130";
            while (!require_stop_)
            {
                QByteArray bytes = readOrDeviceLost(1);

                if (bytes[0] == 'B' || bytes[0] == 'Z' )
                {
                    break;
                }
                else
                    bytes_skipped++;
            }

            //for profi
            QByteArray actual_data = readOrDeviceLost(2);


            uint xval = pts_readed_;

            uint val = qFromBigEndian<quint16>(actual_data);



            pts_readed_ ++;
            emit dataRecieved(val, xval, 0);

        }


    }

    thread_running_ = false;
}

//======================================================================
bool Device::traceStarted()
{
    return trace_started_;
}

//======================================================================
bool Device::detect(std::string *detection)
{

    auto ports = serial::list_ports();

    for(auto port_info : ports)
    {

        if (port_info.port == "/dev/ttyS4")
            continue;

        try {
            serial::Serial port(port_info.port, 460800);

            if (!port.isOpen())
            {
                port.close();
                continue;
            }
            else
            {
                port.close();
               *detection = port.getPort();
                return true;
            }
        } catch (serial::IOException &e) {

            continue;
        }

    }

    return false;
}

//======================================================================
void Device::limitTipAscendToCurrentLevel(bool v)
{
    limit_tip_acsend_  = v;
}

//======================================================================
QString Device::address()
{
    return QString::fromStdString(address_);
}
