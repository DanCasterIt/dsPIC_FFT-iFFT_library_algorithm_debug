/* Constant Definitions */
#define FOSC    (80000000)
#define FP (FOSC/2)
#define BAUDRATE 19600
#define BRGVAL ((FP/BAUDRATE)/16)-1
#define BUFFER_LENGHT 128
#define BUFFER_LENGHT_LOG (log(BUFFER_LENGHT) / log(2))
#define FFTTWIDCOEFFS_IN_PROGMEM 1
//------------DEBUG DEFINES------------
#define TRANSFORM_FUNCTION_NO_TEST 1 //1 FOR DEBUG MODE (debug messages)