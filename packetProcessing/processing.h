#ifndef PROCESSING
    #define PROCESSING

    #include <boost/lockfree/spsc_queue.hpp>
    #include "../ftdiDriver/ftdi.h"
    #include "msgdefinitions.h"

    namespace coms
    {

       unsigned __int8  sendPacket(unsigned __int8* packet, unsigned __int8 length, FT_HANDLE handle);
       unsigned __int8  retrievePackets(void);
       void             processPacket(unsigned __int8* packet);
       unsigned __int8  packetSizeLookup(unsigned __int8 msgHeader);

       // Handlers for flash
       unsigned __int8  progByteHandler(flashResp_t* packet);
       unsigned __int8  readByteHandler(flashResp_t* packet);
       unsigned __int8  chipEraseHandler(flashResp_t* packet);

       // Polling system
       static uint8_t flashPending = 0;

       void clearBuffer(void);
       void printBuffer(void);
       __int8 verifyBuffer(unsigned __int8* progBuffer);
       __int8 blankCheck(void);

       void clearPending(void);
       void setPending(void);
       __int8 polling(void);
    }

#endif // PROCESSING

