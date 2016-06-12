#ifndef MSGDEFINITIONS
    #define MSGDEFINITIONS
    #include <stdio.h>

    /* ----<>---------------------------------------------------------------
            Message Headers
    ----------------------------------------------------------------------*/
    #define packetHeader        0x55

    /************GEN***************/
    #define progByte_mh         0x11
    #define readByte_mh         0x12
    #define chipErase_mh        0x1F

    /* ----<>---------------------------------------------------------------
            Special Types
    ----------------------------------------------------------------------*/
    typedef union
    {
        uint8_t packet[4];
        float data;
    } float_ut;

    /* ----<>---------------------------------------------------------------
            Message Structs
    ----------------------------------------------------------------------*/
    typedef struct progByte_msg
    {
        unsigned __int8     Header;
        unsigned __int8     msgHeader;
        unsigned __int8     addrLow;
        unsigned __int8     addrHigh;
        unsigned __int8     data;
    } progByte_t;

    typedef struct readByte_msg
    {
        unsigned __int8     Header;
        unsigned __int8     msgHeader;
        unsigned __int8     addrLow;
        unsigned __int8     addrHigh;
        unsigned __int8     padding;
    } readByte_t;

    typedef struct chipErase_msg
    {
        unsigned __int8     Header;
        unsigned __int8     msgHeader;
        unsigned __int8     padding0;
        unsigned __int8     padding1;
        unsigned __int8     padding2;
    } chipErase_t;

    typedef struct flashResp_msg
    {
        unsigned __int8     Header;
        unsigned __int8     msgHeader;
        unsigned __int8     response;
    } flashResp_t;

#endif // MSGDEFINITIONS

