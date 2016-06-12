#ifndef FTDI_H
#define FTDI_H

#include "ftd2xx.h"
#include <boost/circular_buffer.hpp>

namespace ftdi {
    // Buffer used by FTDI for recieving data
    extern boost::circular_buffer<BYTE> rxBuffer;

    __int8  init(DWORD devIndex, DWORD baud, DWORD latency, FT_HANDLE &handle);
    __int8  read(FT_HANDLE &handle);
    void    close(FT_HANDLE &handle);
    __int8  write(BYTE* packet, BYTE length, FT_HANDLE &handle);
    __int8  FTError(FT_STATUS &status);
}

#endif // FTDI_H
