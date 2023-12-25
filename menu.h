#ifndef MENU_H
#define MENU_H
#include "stdint.h"
#include "wizchip_conf.h"

// #define DEVICE__INFO___VER   "SIU-01   V231006"
// #define DEVICE__INFO___VER   "SIU-01   V231020"
// #define DEVICE__INFO___VER   "SIU-01   V231024"
// #define DEVICE__INFO___VER   "SIU-01   V231026"
// #define DEVICE__INFO___VER   "SIU-01   V231110"
#define DEVICE__INFO___VER   "SIU-01  V231110A"

#define ON 1
#define OFF 0

// 메뉴 구조체 정의
typedef struct MenuNode {
    char* title;
    char* message;
    uint8_t numberOfsub;
    uint16_t id;
} MenuNode;

#define STAGE1 0
#define STAGE2 1
#define STAGE3 2
#define STAGE4 3

#define NUM_OF_TOP_MENU 4

#define HEAD_LINE_INDEX 0
#define SETTING_INDEX 1
#define MEASUREMENT_INDEX 2
#define EVENT_LIST_INDEX 3

// #define SENSOR_SET 0
// #define MODE_SET 1
// #define NETWORK_SET 2
// #define DATEnTIME_SET 3

#define DATEnTIME_SET 0
#define NETWORK_SET 1
#define MODE_SET 2
#define SENSOR_SET 3

#define OPERATION_MODE_SHOPPING 0
#define OPERATION_MODE_SETTING 1

typedef struct {
    uint8_t level1[5];  //4byte string
    uint8_t level2[5];  //4byte string
} passCode_t;

typedef struct {
    uint8_t stage;      //0,1,2
    uint8_t index[3];
    uint16_t currentId;  //current menu id
    uint8_t operationMode;
    uint8_t x;
    uint16_t digit;
    uint8_t passLevel;
} menuIndex_t;
// stage 0 ---> index[0] 0..2
// stage 1 ---> 

typedef struct {
    uint8_t position[12];
    uint8_t digit[12];
    uint8_t index[12];
} ipSet_t;

typedef struct {
    uint8_t position[5];
    uint16_t digit[5];
} intSet_t;

typedef struct {
    uint8_t position[8];
    uint16_t digit[8];
    uint8_t index[8];
} intSet2_t;

typedef struct {
    uint8_t position[4];
    uint16_t index[4];
} passCodeSet_t;


#define NONE 0xFF

#define VI_INDEX 0
#define CO_INDEX 1
#define NO_INDEX 2
#define NO2_INDEX 3
#define NOX_INDEX 4
#define Velocity_INDEX 5
#define DI_INDEX 6

typedef struct {
    // uint8_t vi;
    // uint8_t co;
    // uint8_t no;
    // uint8_t no2;
    // uint8_t nox;
    // uint8_t velocity;
    uint8_t cfg[8];
} conFigSensor_t;


#define SERVER_INDEX 0
#define CLIENT_INDEX 1

typedef struct {
    uint8_t cfg[2]; //0..server 1..client
} ConFigMode_t;

#define IP_INDEX 0
#define SUBNET_INDEX 1
#define GATEWAT_INDEX 2
#define REMOTE_IP_INDEX 3
#define PORT_INDEX 4
#define SENSOR_ADDRESS_INDEX 5
#define SIU_ADDRESS_INDEX 6

#define REMOTE_MODEBUS_SLAVE_ADDRESS_INDEX 0    //리모트의 국번이 들어있는 위치
#define REMOTE_MODEBUS_REGISTER_ADDRESS_INDEX 1    //리모트의 레제스터 시작주소가 들어있는 위치

typedef struct {
    wiz_NetInfo eth;    //mac, ip, sn, gw, dns(8.8.8.8), dhcp(1)
    uint16_t port;
    uint8_t remote_ip[4];   //< Remote IP Address
    uint16_t modbus[2];  //0=remote slave address, 1=remote register start address
    uint8_t siuAddress;
} ConfigNetwork_t;

#define DATE_INDEX 0
#define TIME_INDEX 1

#define YEAR_INDEX 0
#define MONTH_INDEX 1
#define DAY_INDEX 2

#define HOUR_INDEX 0
#define MINUTE_INDEX 1
#define SECOND_INDEX 2

typedef struct {
    uint8_t position[6];
    uint8_t digit[6];
    uint8_t index[6];
} dateTimeSet_t;

typedef struct {
    uint8_t date[3];   //yy-mm-dd
    uint8_t time[3];   //hh:mm:ss 
} ConfigDateTime_t;


// #define VI_INDEX 0
// #define CO_INDEX 1
// #define NO_INDEX 2
// #define NO2_INDEX 3
// #define NOX_INDEX 4
// #define Velocity_INDEX 5
// #define DI_INDEX 6 ---- use bit0 ~ 4
typedef struct {    //arry data to uint32_t and will be convert Float index 0~5
    uint32_t measurement[7];
} Measurement_t;

#define EVENT_LIST_SUB_INDEX 0
#define EVENT_CLEAR_SUB_INDEX 1

#define EVENT_LIST_SUB_EVENT_INDEX 0

extern menuIndex_t m_menu;
extern conFigSensor_t mSensorConfig;
extern ConFigMode_t mModeConfig;
extern ConfigNetwork_t mNetworkConfig;
extern passCode_t mPassCode;

void initMenu(void);
void selectMenuDisplay(const MenuNode* menu);
int menuProcess(void);
int menuselector(void);
const MenuNode * getMenu(void);
void limitStage(uint8_t limit);
int isSettingMenu(void);
int setMenu(void);
void applyMenuValuetoLcd(void);
void setOX(uint8_t *ref, uint8_t x, uint8_t y);
void moveMenuIndexDown(void);
void moveMenuIndexUp(void);
void moveMenuStageRight(void);
void moveMenuStageLeft(void);
void cursor(uint8_t onOff);
void setProcess(void);
void saveSystemConfig(void);
void loadSystemConfig(void);
void saveDefultConfig(void);
void setDate(void);
void setTime(void);
void getDateTime(void);
void timeDisplay(void);
void displayMeasuremet(int menuId);
void upDateMenuId(void);
void clearPassCode(void);

void saveOffsetToEEPROM(void);
void loadOffsetFromEEPROM(void);

extern struct rtc_time rtcTime;

#define TEN_MINUTE 30000   //*20msec = 600 sec
// #define TEN_MINUTE 6000 //*20msec = 120 sec

void autoReturnToHome(void);
void ethUpdate(void);   //update network parameter

#endif
