#include "processing.h"
#include <stdio.h>

namespace coms
{
    /*
     *
     */
    unsigned __int8 sendPacket(unsigned __int8* packet, unsigned __int8 length, FT_HANDLE handle)
    {
//        printf("PACKET:\n");
//        for(int i = 0; i < length; ++i)
//        {
//            printf("%d\n", packet[i]);
//        }
        return ftdi::write(packet, length, handle);
    }

    /*
     * retrievePackets
     * input:
     *  no input
     * output:
     *  returns a status int
     *
     * Description:
     * This functionn is used to pull the packets out of the raw data (rxBuffer).
     *  This will store packets that hold data and will handle the processing
     *  of ack messages.
     */
    unsigned __int8 retrievePackets(void)
    {
        BYTE packet[32]; //allocate more than enough data
        unsigned int i, packetLength;

        while(!ftdi::rxBuffer.empty())
        {
            // The first part is to bring a packetHeader to the front of the buffer.
            while(!ftdi::rxBuffer.empty())
            {
                // clear the packet holder
                std::fill( packet, packet + sizeof( packet ), 0 );

                // TODO: Put some sort of watchdog timer in here to protect from threadlock

                // Find the start of a packet
                packet[0] = ftdi::rxBuffer.front();
                if(packet[0] == packetHeader)
                {
                    // If we find a header, leave it IN the buffer, but break the loop.
                    //printf("!!!!! packetHeader found !!!!!\n");
                    break;
                }
                // If the front of the buffer isn't a header, then it's probably garbage
                //  data. There should be very limited scenarios where the first BYTE read
                //  is NOT a header, but this should handle all of that.
                ftdi::rxBuffer.pop_front();
            }

            // If either the buffer is empty or there is not a complete message in the buffer,
            //  we should return to get more data. I had this inside the loop but I moved it here
            //  to catch anything that made it by the while loop.
            if(/*ftdi::rxBuffer.empty()) ||*/ ftdi::rxBuffer.size() < 3)
            {
                //printf("The buffer is either empty or not long enough\n");
                return 0;
            }

            // After the packet header, we need to look at the message header. This will
            //  tell us how long the messsage should be. Knowing this, we can determine if
            //  the entire message is in the buffer. If we call the length of the message L,
            //  then the next L BYTES should not have a packetHeader and L + 1 should be a
            //  packet header. If this is the case, we have the entire message. If not, then
            //  we need to figure out what's wrong.

            // Read the next byte (message header) and get the expected size
            packetLength = packetSizeLookup(ftdi::rxBuffer[1]);
            //printf("messageHeader: 0x%X\n", ftdi::rxBuffer[1]);
            //printf("packetLength: %d\n", packetLength);

            // Now we need to make sure that the data in the buffer matches what we expect.
            //  This loop will make sure that there isn't a packetHeader too soon, and that
            //  the packet we are looking at is the right length. If it isn't then we need
            //  to determine if its garbage data, or there is data missing.
            if(/*packetLength*/3 > ftdi::rxBuffer.size()){
                // There is not enoguh data in the buffer, read more and try again.
                //qDebug("packetLength > rxBuffer.size()\n");
                return 0;
            }

            // Load the suspected packet into a buffer
            for(i = 0; i < /*packetLength*/3; ++i)
            {
                packet[i] = ftdi::rxBuffer[i];
            }

            // If the data passes the crc check, then we need to pack it up and send it to get
            //  processed. If it does not, we need to drop the packetHeader and try again.
//            if(crcCheck(packet, packetLength))
//            {
//                // The CRC check was successful so we pull the packet out of the buffer
                for(i = 0; i < /*packetLength*/3; ++i)
                {
                    ftdi::rxBuffer.pop_front();
                }
                //printf("===== Packet Recieved! =====\n");
//            }
//            else
//            {
//                // The CRC check failed, so drop off packetHeader and try again.
//                ftdi::rxBuffer.pop_front();
//                printf("***** CRC FAILED *****\n");
//                continue;
//            }

            // At this point, packet[] now has the packet in it. Everything up to this point was
            //  used to locate and confirm possible packets in the buffer. Now that we have one,
            //  we need to process it. There are two scenarios. If it is a ack packet, then we
            //  simply set the flag it's meant for and move on. If we got anything else, most often
            //  a superpacket, then we pull the data we want out of it and store it on a stack.
            processPacket(packet);
        }

        return 0;
    }

    /*
     * processPacket
     * input:
     *  TODO
     * output:
     *  no output
     *
     * Description:
     *  This function is used to process the packets that retrievePackets() finds. There are really only
     *      two ways this can go. The first is that it recieves an ack message. If this occurs, then we
     *      simply find which one it is and configure accordingly. The only other option is that we are
     *      inputting a superPacket. If this is the case, then we will hand it off to the appropriate
     *      handler.
     */
    void processPacket(BYTE* packet)
    {
        switch(packet[1])
        {
            case(progByte_mh):
                progByteHandler((flashResp_t*)packet);
            break;

            case(readByte_mh):
                readByteHandler((flashResp_t*)packet);
            break;

            case(chipErase_mh):
                chipEraseHandler((flashResp_t*)packet);
            break;
        }
    }

    unsigned __int8 packetSizeLookup(BYTE msgHeader)
    {
        /*
         *Originally all message types were polled using sizeof.  Such a method would return the byte lenght.
         *Unfortunately the current version of mingw has a bug in sizeof.  sizeof inquries to bitfield objects
         *Return erroneous lengths.  As a result all static values have been hard coded.
        */
        switch(msgHeader)
        {
            case (progByte_mh):
                return 5;
            break;

            case (readByte_mh):
                return 5;
            break;

            case (chipErase_mh):
                return 5;
            break;

            default:
                return 0;
        }
    }

    /*
     * There will one day be a better place for this function, but I am not working on that now so I will
     *  just place it here.
     */
    unsigned __int8 buffer[32];
    unsigned __int8 bufferItr = 0;

    void clearBuffer(void)
    {
        bufferItr = 0;
        std::memset(buffer, 0xFF, 32);
    }

    void printBuffer(void)
    {
        printf("Buffer Contents:\n");

        for(int i = 0; i < 32; ++i)
        {
            printf("0x%02X - 0x%02X\n", i, (unsigned char)buffer[i]);
        }
    }

    __int8 verifyBuffer(unsigned __int8* progBuffer)
    {
        for(int i = 0; i < 32; ++i)
        {
            if(progBuffer[i] != buffer[i])
            {
                return 0;
            }
        }
        return 1;
    }

    __int8 blankCheck(void)
    {
        for(int i = 0; i < 32; ++i)
        {
            if(buffer[i] != 0xFF)
            {
                return 0;
            }
        }
        return 1;
    }

    void clearPending(void)
    {
        coms::flashPending = 0;
    }

    void setPending(void)
    {
        coms::flashPending = 1;
    }

    __int8 polling(void)
    {
        return coms::flashPending;
    }


    unsigned __int8 progByteHandler(flashResp_t* packet)
    {
        clearPending();

        return 1;
    }

    unsigned __int8 readByteHandler(flashResp_t* packet)
    {
        //printf("Read : %#04x\n", packet->response);
        buffer[bufferItr++] = packet->response;
        clearPending();
        return 1;
    }

    unsigned __int8 chipEraseHandler(flashResp_t* packet)
    {
        clearPending();
        return 1;
    }
}
