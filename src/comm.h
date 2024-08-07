
#ifndef _COMM_H
#define _COMM_H

#ifdef __cplusplus
extern "C"
{
#endif

    typedef uint32_t SYS_TICK_t;

    /*
        MANAGE WIFI
    */

#define C_AP_LIST_MAX  10 // numero massimo di AP visualizzati nella lista dopo la scansione


#define BIT_INIT_OK 0x80
#define BIT_SCAN_ERROR  0x01
#define BIT_SCAN_COMPLETED 0x02
#define BIT_CONNECT 0x04

    typedef enum
    {
        WIFI_INIT = 0,
        WIFI_WAIT,
        WIFI_SCAN,
        WIFI_CONNECTION,
        WIFI_RECONNECTION,
        WIFI_FUNC_MAX
    } WIFI_FUNC_t;

    typedef struct
    {
        uint8_t statusFlags;
        WIFI_FUNC_t function;
        WIFI_FUNC_t oldfunction;
        SYS_TICK_t tickStart;
        SYS_TICK_t tickWait;
        SYS_TICK_t tickCount;
        uint8_t numAP;  // numero AP trovati con scansione
        unsigned char SSID[64+1];   // la lunghezza massima di DDIS e pass 
        unsigned char pass[64+1];   // è 64 caratteri. +1 è il terminatore 

    } WIFI_taskData_t;

    extern WIFI_taskData_t WIFI_taskData;
    WIFI_taskData_t *wifiTaskInit(void);
    void manage_WIFI(void);

    /*
        manage connection
    */
    typedef enum
    {
        CONN_INIT = 0,
        CONN_WAIT,
        CONN_SCAN,
        CONN_CONNECTION,
        CONN_RECONNECTION,
        CONN_FUNC_MAX
    } CONN_FUNC_t;

    typedef struct
    {
        uint8_t statusFlags;
        CONN_FUNC_t function;
        CONN_FUNC_t oldfunction;
        SYS_TICK_t tickStart;
        SYS_TICK_t tickWait;
        SYS_TICK_t tickCount;

    } CONN_taskData_t;

    extern CONN_taskData_t CONN_taskData;
    CONN_taskData_t *CONN_taskInit(void);
    void manage_connection(void);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif
