#include <stdio.h>
#include "ftdi.h"

namespace ftdi {
    boost::circular_buffer<BYTE> rxBuffer(65536);

    __int8 FTError(FT_STATUS status)
    {
        if (status != FT_OK)
        {
            printf("FTDI Error: \n");
            //FT_Close(handle);

            switch (status)
            {
            case 1:
                printf("Invalid handle! \n");
                break;
            case 2:
                printf("Device not found! \n");
                break;
            case 3:
                printf("Device not opened! \n");
                break;
            default:
                printf("Unknown error! \n");
            }
           return status;
        }
        return 0;
    }

    __int8  init(DWORD devIndex, DWORD baud, DWORD latency, FT_HANDLE &handle)
    {
        FT_STATUS status = -1;
        char buffer[64] = {0}; // more than enough room!
        int error = -1;

        // Find device and open the handle
        status = FT_ListDevices((PVOID)devIndex, buffer, FT_LIST_BY_INDEX | FT_OPEN_BY_SERIAL_NUMBER);
        if(status == FT_OK)
        {
            status = FT_OpenEx(buffer, FT_OPEN_BY_SERIAL_NUMBER, &handle);

            if(status == FT_OK)
            {
                printf("FTDI::READ:: Device opened successful\n");
            } else
            {
                printf("FTDI::READ:: FT_OpenEx Failed\n");
                return 0;
            }
        } else
        {
            printf("FTDI::READ:: Ft_ListDevices Failed\n");
            return 0;
        }

        error = FTError( FT_ResetDevice(handle) ); if (error) {return error;}
        error = FTError( FT_SetBaudRate(handle, baud) ); if (error) {return error;}
        error = FTError( FT_SetDataCharacteristics(handle, FT_BITS_8, FT_STOP_BITS_1, FT_PARITY_NONE) ); if (error) {return error;}
        error = FTError( FT_SetFlowControl(handle, (USHORT)FT_FLOW_NONE, (UCHAR)FT_FLOW_XON_XOFF, (UCHAR)FT_FLOW_XON_XOFF) ); if (error) {return error;}
        error = FTError( FT_SetUSBParameters(handle, 64, 0) ); if (error) {return error;}
        error = FTError( FT_SetLatencyTimer(handle, latency) ); if (error) {return error;}
        error = FTError( FT_Purge(handle, FT_PURGE_RX | FT_PURGE_TX) ); if (error) {return error;}
        error = FTError( FT_SetDtr(handle) ); if (error) {return error;}
        error = FTError( FT_SetRts(handle) ); if (error) {return error;}
        error = FTError( FT_SetTimeouts(handle, 5, 5) ); if (error) {return error;}

        return 1;
    }

    __int8 read(FT_HANDLE &handle)
    {
    /* int read(void)
     *  input:
     *      none
     *  output:
     *      status of read
     *
     * The purpose of the ftdi::read() function is to pull all of the data from the FTDI recieve buffer
     *  and store them in a buffer on the pc side. Any processing of the data will happen elsewhere. This
     *  is simply a getter function.
     */
        // variables for use with FTDI functions
        DWORD rxBytes, txBytes, eventWord, bytesRead;
        FT_STATUS status;
        BYTE tempBuffer[65535];

        // rxBytes holds number of characters in the recieve que. txByte holds the number of characters in
        //  the transmist que. eventWord is a pointer to the current state of the event status.
        FTError(FT_GetStatus(handle, &rxBytes, &txBytes, &eventWord));
        //qDebug("FTDI:: buffer bytes: %d", rxBytes);
        if(rxBytes > 0)
        {
            // All of the data in the recieve buffer will be put into tempBuffer. tempBuffer is the maximum size
            //  that the recieve buffer could possibly hold. rxBytes will tell FT_READ how many bytes it should
            //  read. bytesRead will hold the number of bytes that were actually read.
            status = FT_Read(handle, tempBuffer, rxBytes, &bytesRead);

            // While there are still bytes to read
            //  * We need this loop because FT_READ is limited by FT_SetUSBParameters (see startFTDI above) to
            //      64 bytes. This polls until all the bytes have been read.
            // !!! This may need to be reworked to avoid locking up. !!!
            while(rxBytes < bytesRead);

            if(status == FT_OK)
            {
                //qDebug("tempBuffer: \n");
                // Store all of the data into the ring buffer
                for(unsigned __int16 i = 0; i < rxBytes; i++)
                {
                    ftdi::rxBuffer.push_back(tempBuffer[i]);
                    //qDebug("data[%d]: 0x%02X  |   buffer[%d] : 0x%02X\n", i, tempBuffer[i], i, rxBuffer[i]);
                }
                //qDebug("~~~~~ rxBuffer.size(): %d ~~~~~\n", ftdi::rxBuffer.size());
                //FT_Close(handle);
                return 1;
                //if(bufRx.dataOverFlow) qDebug("dataOverFlow");;
            }
            else
            {
                printf("FTDI::READ:: read failed\n");
                //FT_Close(handle);
                return 0;
            }
        }

        return 0;
    }

    __int8 write(unsigned __int8 *packet, unsigned __int8 length, FT_HANDLE &handle){
        // variables for use with FTDI functions
        DWORD bytesSent;

        FTError(FT_Write(handle, packet, length, &bytesSent));

        return bytesSent;
    }

    void close(FT_HANDLE &handle)
    {
        FT_Close(handle);
    }
}
