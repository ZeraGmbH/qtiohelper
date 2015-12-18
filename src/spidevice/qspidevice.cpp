#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>
#include <QString>
#include "qspidevice.h"
#include "qspidevice_p.h"

// ************* QSPIDevicePrivate

QSPIDevicePrivate::QSPIDevicePrivate()
{
    bSWReverseRequired = false;
}

QSPIDevicePrivate::~QSPIDevicePrivate()
{
}

// ************* QSPIDevice

QSPIDevice::QSPIDevice(const QString &name) :
    QFile(name),
    d_ptr(new QSPIDevicePrivate())
{
}

QSPIDevice::QSPIDevice(int bus, int channel) :
    d_ptr(new QSPIDevicePrivate())
{
    QString strFileName = QString(QLatin1String("/dev/spidev%1.%2")).arg(bus).arg(channel);
    setFileName(strFileName);
}

bool QSPIDevice::open(OpenMode flags)
{
    Q_D(QSPIDevice);
    qInfo("SPI opening %s...", qPrintable(fileName()));
    flags |= QIODevice::Unbuffered;
    bool bOpen = exists() && QFile::open(flags);
    if(bOpen)
    {
        /* Check if we opened SPI by a simple ioctl check */
        __u8 dummy;
        __u32 dummy32;
        bOpen =
            ioctl(handle(), SPI_IOC_RD_MODE, &dummy) >= 0 &&
            ioctl(handle(), SPI_IOC_RD_LSB_FIRST, &dummy) >= 0 &&
            ioctl(handle(), SPI_IOC_RD_BITS_PER_WORD, &dummy) >= 0 &&
            ioctl(handle(), SPI_IOC_WR_MAX_SPEED_HZ, &dummy32) >= 0;

        if(!bOpen)
        {
            qWarning("%s does not support SPI ioctls!", qPrintable(fileName()));
            close();
        }
        d->bSWReverseRequired = false;
    }
    return bOpen;
}

void QSPIDevice::close()
{
    qInfo("SPI closing %s...", qPrintable(fileName()));
    return QFile::close();
}

bool QSPIDevice::isSequential() const
{
    return true;
}

bool QSPIDevice::setMode(quint8 Mode)
{
    qInfo("SPI set mode %u...", Mode);
    __u8 mode = 0;
    switch(Mode)
    {
    case 0:
        mode = SPI_MODE_0;
        break;
    case 1:
        mode = SPI_MODE_1;
        break;
    case 2:
        mode = SPI_MODE_2;
        break;
    case 3:
        mode = SPI_MODE_3;
        break;
    default:
        qWarning("SetMode: invalid mode %u!", Mode);
        return false;
    }

    if(!isOpen())
    {
        qWarning("SetMode: SPI device not open!");
        return false;
    }
    else
    {
        if(ioctl(handle(), SPI_IOC_WR_MODE, &mode) < 0)
            qWarning("QSPIDevice::SetMode failed!");
    }
    return true;
}

bool QSPIDevice::setLSBFirst(bool lsbFirst)
{
    Q_D(QSPIDevice);
    qInfo("SPI LSBFirst %u...", lsbFirst);
    bool bOK = true;
    if(!isOpen())
    {
        qWarning("SetLSBFirst: SPI device not open!");
        bOK = false;
    }
    else
    {
        __u8 lsb = (__u8)lsbFirst;
        bOK = ioctl(handle(), SPI_IOC_WR_LSB_FIRST, &lsb) >= 0;
        if(!bOK)
        {
            qWarning("QSPIDevice::SetLSBFirst failed!");
            if(lsbFirst)
                qInfo("Software bit reversing is used.");
            d->bSWReverseRequired = lsbFirst;
            bOK = true;
        }
    }
    return bOK;
}

bool QSPIDevice::setBitsPerWord(quint8 bitsPerWord)
{
    qInfo("SPI bits per word %u...", bitsPerWord);
    bool bOK = true;
    if(!isOpen())
    {
        qWarning("SetBitsPerWord: SPI device not open!");
        bOK = false;
    }
    else
    {
        __u8 bits = (__u8)bitsPerWord;
        bOK = ioctl(handle(), SPI_IOC_WR_BITS_PER_WORD, &bits) >= 0;
        if(!bOK)
            qWarning("QSPIDevice::setBitsPerWord failed!");
    }
    return bOK;
}

bool QSPIDevice::setBitSpeed(quint32 bitSpeedHz)
{
    qInfo("SPI bitspeed %u Hz...", bitSpeedHz);
    bool bOK = true;
    if(!isOpen())
    {
        qWarning("QSPIDevice::setBitSpeed: device not open!");
        bOK = false;
    }
    else
    {
        __u32 hz = (__u32)bitSpeedHz;
        bOK = ioctl(handle(), SPI_IOC_WR_MAX_SPEED_HZ, &hz) >= 0;
        if(!bOK)
            qWarning("QSPIDevice::setBitSpeed failed!");
    }
    return bOK;
}

bool QSPIDevice::sendReceive(QByteArray &dataSend, QByteArray &dataReceive)
{
    bool bOK = true;
    if(!isOpen())
    {
        qWarning("QSPIDevice::sendReceive: device not open!");
        bOK = false;
    }
    if(dataSend.size() == 0)
    {
        qWarning("QSPIDevice::sendReceive: dataSend is empty!");
        bOK = false;
    }
    if(bOK)
    {
        dataReceive.resize(dataSend.size());
        struct spi_ioc_transfer	spi_io;
        memset(&spi_io, 0, sizeof spi_io);
        spi_io.tx_buf = (__u64)dataSend.constData();
        spi_io.rx_buf = (__u64)dataReceive.data();
        spi_io.len = dataSend.size();
        if(ioctl(handle(), SPI_IOC_MESSAGE(1), &spi_io) < 0)
        {
            qWarning("QSPIDevice::sendReceive: SPI_IOC_MESSAGE failed!");
            bOK = false;
        }
    }
    return bOK;
}

// taken from http://stackoverflow.com/questions/2602823/in-c-c-whats-the-simplest-way-to-reverse-the-order-of-bits-in-a-byte
static quint8 reverse(quint8 b) {
   b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
   b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
   b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
   return b;
}

qint64 QSPIDevice::readData(char *data, qint64 maxlen)
{
    Q_D(QSPIDevice);
    qint64 bytesRead = QFile::readData(data, maxlen);
    if(d->bSWReverseRequired)
    {
        for(qint64 iByte=0; iByte<bytesRead; iByte++)
            data[iByte] = (char)reverse((quint8)data[iByte]);
    }
    return bytesRead;
}

qint64 QSPIDevice::writeData(const char *data, qint64 len)
{
    Q_D(QSPIDevice);
    qint64 bytesWritten = -1;
    if(d->bSWReverseRequired)
    {
        char *copyData = new char[len];
        if(copyData)
        {
            for(qint64 iByte=0; iByte<len; iByte++)
                copyData[iByte] = (char)reverse((quint8)data[iByte]);
            bytesWritten = QFile::writeData(copyData, len);
            delete[] copyData;
        }
    }
    else
        bytesWritten = QFile::writeData(data, len);
    return bytesWritten;
}
