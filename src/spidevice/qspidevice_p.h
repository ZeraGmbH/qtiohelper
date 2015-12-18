#ifndef QSPIDEVICE_P_H
#define QSPIDEVICE_P_H

#include "qspidevice.h"

class QSPIDevicePrivate
{
public:
    QSPIDevicePrivate();
    virtual ~QSPIDevicePrivate();
    /**
      At least beaglebone's SPI does not support LSBFirst. We implement a
      software fallback with the assumption that MSBFirst is default. So
      when LSBFirst cannot be set, bSWReverseRequired is set and the
      read/write buffers are reversed.
      */
    bool bSWReverseRequired;
};

#endif // QSPIDEVICE_P_H

