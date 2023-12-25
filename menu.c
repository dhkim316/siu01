#include <stdio.h>
#include "menu.h"
#include "lcd.h"
#include "hc165.h"
#include "app.h"
#include "at24c256.h"
#include "sl_udelay.h"
#include "isl1208.h"
#include "sensor.h"
#include "event_log.h"
//global variable
menuIndex_t m_menu;

conFigSensor_t mSensorConfig;
ConFigMode_t mModeConfig;
ConfigNetwork_t mNetworkConfig;
ConfigDateTime_t mDateTimeConfig;
passCode_t mPassCode;
char tPassCode[5]={'0','0','0','0', 0};

const conFigSensor_t cSensorConfig = {.cfg = {'X','X','X','X','X','O','O','O'}};
const ConFigMode_t cModeConfig = {.cfg = {'X','O'}};   //server X, client O
const ConfigNetwork_t cNetworkConfig = {.eth.mac = {0x94,0xDF,0x58,0xAA,0xBB,0xCC},
                                        .eth.ip =  {192,168,  1, 55},
                                        .eth.sn =  {255,255,255,  0},
                                        .eth.gw =  {192,168,  1,  1},
                                        .eth.dns = {  8,  8,  8,  8},
                                        .eth.dhcp = NETINFO_STATIC,
                                        .port = MODBUS_PORT,
                                        .remote_ip =  {192,168,1,99},
                                        .modbus = {100, 20000},    //REMOTE DEVICE ADDRESS, REGISTER ADDRESS
                                        .siuAddress = 99
                                         };

static ConfigNetwork_t bkupNetworkConfig;

void backupNetworkConfig(void) {
    memcpy(&bkupNetworkConfig, &mNetworkConfig, sizeof(ConfigNetwork_t));
}
void restoreNetworkConfig(void) {
    memcpy(&mNetworkConfig, &bkupNetworkConfig, sizeof(ConfigNetwork_t));
}

const passCode_t cPassCode = { .level1 = {'1', '2', '3', '4', 0},
                               .level2 = {'6', '2', '2', '2', 0}};
void upDateMenuId(void) {
    MenuNode* menu = getMenu();
    m_menu.currentId = menu->id;         //save current menu id;
}

void selectMenuDisplay(const MenuNode* menu) {  //버튼 동작이 있을때만 불려지는 함수
    Debug_printBT("I%d,S%d,P%d", menu->id,m_menu.stage, m_menu.passLevel);
    static int cnt = 0;
    if(!((m_menu.currentId == 301) && (m_menu.stage == STAGE3))) {
        printf_lcd(1,1,"%s", menu->title);
        printf_lcd(1,2,"%s", menu->message);    //head line message
    }
    m_menu.currentId = menu->id;         //save current menu id;
    if((m_menu.currentId == 130) || (m_menu.currentId == 131)) getDateTime(); //시간을 읽어와서 표시
    if((m_menu.currentId >= 10) && (m_menu.currentId <= 13)) printf_lcd(13, 1,"LV.%d", m_menu.passLevel);
    applyMenuValuetoLcd();
}

const passCodeSet_t passCodePack = {.position = {7,8,9,10},
                                    .index ={0,1,2,3} };

const ipSet_t ipPack   = {.position = {1,2,3, 5,6,7, 9,10,11, 13,14,15}, 
                          .digit = {100,10,1, 100,10,1, 100,10,1, 100,10,1},
                          .index = {  0, 0,0,   1, 1,1,   2, 2,2,   3, 3,3}
                         };    //값을 조정하기 위한 틀

const intSet_t intPack = {.position = {1,2,3,4,5},
                           .digit = {10000, 1000, 100, 10, 1}
                         };
// const intSet2_t int2Pack = {.position = {3,4,5,6,7,       11,12,13,14,15},
//                            .digit = {10000,1000,100,10,1, 10000,1000,100,10,1},
//                            .index = {0,0,0,0,0,           1,1,1,1,1}
//                            };

const intSet2_t int2Pack = {.position = {4,5,6,      11,12,13,14,15},
                           .digit =     {100,10,1, 10000,1000,100,10,1},
                           .index =     {0,0,0,           1,1,1,1,1}
                           };

const intSet_t uint8Pack = {.position = {1,2,3},
                            .digit = {100, 10, 1}
                           };                          

const dateTimeSet_t datePack = {.position = {3,4,6,7,9,10},
                                .digit = {10,1,10,1,10,1},
                                .index = {0,0,1,1,2,2}
                               };

const dateTimeSet_t timePack = {.position = {1,2,4,5,7,8},
                                .digit = {10,1,10,1,10,1},
                                .index = {0,0,1,1,2,2}
                               };

const MenuNode menu0   = {"GREEN INFRA TECH", "SIU-01          ", 0,0};

const MenuNode menu1   = {DEVICE__INFO___VER, "SETTING         ", 4,1};
const MenuNode menu10  = {"SETTING         ", "SENSOR SET      ", 6,10};
const MenuNode menu100 = {"SENSOR SET      ", "[O] VI          ", 0,100};
const MenuNode menu101 = {"SENSOR SET      ", "[O] CO          ", 0,101};
const MenuNode menu102 = {"SENSOR SET      ", "[O] NO          ", 0,102};
const MenuNode menu103 = {"SENSOR SET      ", "[O] NO2         ", 0,103};
const MenuNode menu104 = {"SENSOR SET      ", "[O] NOX         ", 0,104};
const MenuNode menu105 = {"SENSOR SET      ", "[O] Veloicty    ", 0,105};

const MenuNode menu11  = {"SETTING         ", "MODE SET        ", 2,11};
const MenuNode menu110 = {"MODE SET        ", "[O] SERVER      ", 0,110};
const MenuNode menu111 = {"MODE SET        ", "[X] CLIENT      ", 0,111};

const MenuNode menu12  = {"SETTING         ", "NETWORK SET     ", 7,12};
const MenuNode menu120 = {"IP              ", "192.168.001.055 ", 0,120};
const MenuNode menu121 = {"SUBNET MASK     ", "255.255.255.000 ", 0,121};
const MenuNode menu122 = {"GATE WAY        ", "192.168.001.001 ", 0,122};
const MenuNode menu123 = {"REMOTE IP       ", "192.168.001.099 ", 0,123};
const MenuNode menu124 = {"PORT            ", "9000            ", 0,124};
const MenuNode menu125 = {"REMOTE MODBUS   ", "DA:100 RA:10020 ", 0,125};
const MenuNode menu126 = {"SIU-01 ADRESS   ", "099             ", 0,126};

const MenuNode menu13  = {"SETTING         ", "DATE & TIME SET ", 2,13};
const MenuNode menu130 = {"DATE SET        ", "2023-07-21      ", 0,130};
const MenuNode menu131 = {"TIME SET        ", "12:30:45        ", 0,131};

const MenuNode menu14 =  {"    PASSWORD    ", "      ****      ", 0,14};

const MenuNode menu2   = {DEVICE__INFO___VER, "MEASUREMENT     ", 7,2};
const MenuNode menu20  = {"MEASUREMENT     ", "VI ...          ", 0,20};
const MenuNode menu21  = {"MEASUREMENT     ", "CO ...          ", 0,21};
const MenuNode menu22  = {"MEASUREMENT     ", "NO ...          ", 0,22};
const MenuNode menu23  = {"MEASUREMENT     ", "NO2 ...         ", 0,23};
const MenuNode menu24  = {"MEASUREMENT     ", "NOX ...         ", 0,24};
const MenuNode menu25  = {"MEASUREMENT     ", "Velocity ...    ", 0,25};
const MenuNode menu26  = {"MEASUREMENT     ", "DI ...          ", 0,26};

const MenuNode menu3   = {DEVICE__INFO___VER, "EVENT LIST/CLEAR", 2,3};
const MenuNode menu30  = {"EVENT LIST      ", "                ", 1,30};
const MenuNode menu301 = {"EVENT           ", "                ", 0,301};
const MenuNode menu31  = {"EVENT CLEAR     ", "      ****      ", 0,31};

// const MenuNode *menu0123_array[] = { &menu0, &menu1, &menu2, &menu3 };
// const MenuNode *menu1x_array[]   = { &menu10, &menu11, &menu12, &menu13 };
// const MenuNode *menu10x_array[]  = { &menu100, &menu101, &menu102, &menu103, &menu104, &menu105 };
// const MenuNode *menu11x_array[]  = { &menu110, &menu111 };
// const MenuNode *menu12x_array[]  = { &menu120, &menu121, &menu122, &menu123, &menu124, &menu125, &menu126 };
// const MenuNode *menu13x_array[]  = { &menu130, &menu131 };
// const MenuNode *menu2x_array[]   = { &menu20, &menu21, &menu22, &menu23, &menu24, &menu25, &menu26 };
// const MenuNode *menu3x_array[]   = { &menu30 };

//메뉴 초기화
void initMenu(void) {
    memset(m_menu.index, NONE, sizeof(m_menu.index));
    m_menu.index[STAGE1]=HEAD_LINE_INDEX;
    m_menu.stage = STAGE1;
    m_menu.operationMode = OPERATION_MODE_SHOPPING;
    m_menu.digit = NONE;
    m_menu.x = 0;
    clearPassCode();
    selectMenuDisplay(&menu0);
    backupNetworkConfig();
}

// m --- sm --- ssm
// 0     1      2
// menu function call procedure
// btnAndDiPoll() --> menuProcess()

// 메뉴둘러보기
int menuProcess(void) {
    static int prevButton=0;
    cursor(OFF);
    m_menu.x=0;
    int btn = menuselector();
    if(btn) {
        selectMenuDisplay(getMenu());
        upDateMenuId();
        if((m_menu.currentId == 301)) {
            if(btn == BTN_UP) {
                Debug_printBT("BTN UP");
                onClick();
                if(prevButton == BTN_DOWN)
                    onClick();
            }
            if(btn == BTN_DOWN) {
                onReverseClick();
                Debug_printBT("BTN DOWN");
                if(prevButton == BTN_UP)
                    onReverseClick();
            }
            if(btn == BTN_RIGHT) {
                if(vm.startFlag == 0) {
                    Debug_printBT("BTN RIGHT");
                    onClick();
                    prevButton = BTN_UP;    //fake button to bug fix
                    return;
                }
            }
            prevButton = btn;
        } 
        else {
            vm.startFlag = 0;
        }
    }
    else {
        applyMenuValuetoLcd();
    }
}

void timeDisplay(void) {
    if((m_menu.currentId == 0)) {
        getDateTime(); 
        printf_lcd(9,2, "%02d:%02d:%02d", mDateTimeConfig.time[0], mDateTimeConfig.time[1], mDateTimeConfig.time[2]);
    }
}
int menuselector(void) {
    int button = getButton();
    switch (button) {
        case BTN_DOWN:
            moveMenuIndexDown();
            break;
        case BTN_UP:
            moveMenuIndexUp();
            break;
        case BTN_RIGHT:
            moveMenuStageRight();
            break;
        case BTN_LEFT:
            moveMenuStageLeft();
            break;
        default:
            break;
        case BTN_SET:
            if(isSettingMenu()) m_menu.operationMode = OPERATION_MODE_SETTING;
            break;
        // case BTN_ENTER:
        //     break;
        case BTN_ESC:
            m_menu.operationMode = OPERATION_MODE_SHOPPING;
            break;
        // case BTN_BLUETOOTH:
        //     break;
    }
    return button;
}

//스테이지와 인덱스에따른 메뉴포인터 
const MenuNode * getMenu(void) {
    if (m_menu.index[STAGE1] == HEAD_LINE_INDEX) return &menu0;
    if (m_menu.passLevel != 0 ) {
        if ((m_menu.index[STAGE1] == SETTING_INDEX) && (m_menu.index[STAGE2] == NONE) && (m_menu.index[STAGE3] == NONE)) return &menu1;
        if ((m_menu.index[STAGE1] == SETTING_INDEX) && (m_menu.index[STAGE2] == SENSOR_SET) && (m_menu.index[STAGE3] == NONE)) return &menu10;
        if ((m_menu.index[STAGE1] == SETTING_INDEX) && (m_menu.index[STAGE2] == SENSOR_SET) && (m_menu.index[STAGE3] == VI_INDEX)) return &menu100;
        if ((m_menu.index[STAGE1] == SETTING_INDEX) && (m_menu.index[STAGE2] == SENSOR_SET) && (m_menu.index[STAGE3] == CO_INDEX)) return &menu101;
        if ((m_menu.index[STAGE1] == SETTING_INDEX) && (m_menu.index[STAGE2] == SENSOR_SET) && (m_menu.index[STAGE3] == NO_INDEX)) return &menu102;
        if ((m_menu.index[STAGE1] == SETTING_INDEX) && (m_menu.index[STAGE2] == SENSOR_SET) && (m_menu.index[STAGE3] == NO2_INDEX)) return &menu103;
        if ((m_menu.index[STAGE1] == SETTING_INDEX) && (m_menu.index[STAGE2] == SENSOR_SET) && (m_menu.index[STAGE3] == NOX_INDEX)) return &menu104;
        if ((m_menu.index[STAGE1] == SETTING_INDEX) && (m_menu.index[STAGE2] == SENSOR_SET) && (m_menu.index[STAGE3] == Velocity_INDEX)) return &menu105;

        if ((m_menu.index[STAGE1] == SETTING_INDEX) && (m_menu.index[STAGE2] == MODE_SET) && (m_menu.index[STAGE3] == NONE)) return &menu11;
        if ((m_menu.index[STAGE1] == SETTING_INDEX) && (m_menu.index[STAGE2] == MODE_SET) && (m_menu.index[STAGE3] == SERVER_INDEX)) return &menu110;
        if ((m_menu.index[STAGE1] == SETTING_INDEX) && (m_menu.index[STAGE2] == MODE_SET) && (m_menu.index[STAGE3] == CLIENT_INDEX)) return &menu111;

        if ((m_menu.index[STAGE1] == SETTING_INDEX) && (m_menu.index[STAGE2] == NETWORK_SET) && (m_menu.index[STAGE3] == NONE)) return &menu12;
        if ((m_menu.index[STAGE1] == SETTING_INDEX) && (m_menu.index[STAGE2] == NETWORK_SET) && (m_menu.index[STAGE3] == IP_INDEX)) return &menu120;
        if ((m_menu.index[STAGE1] == SETTING_INDEX) && (m_menu.index[STAGE2] == NETWORK_SET) && (m_menu.index[STAGE3] == SUBNET_INDEX)) return &menu121;
        if ((m_menu.index[STAGE1] == SETTING_INDEX) && (m_menu.index[STAGE2] == NETWORK_SET) && (m_menu.index[STAGE3] == GATEWAT_INDEX)) return &menu122;
        if ((m_menu.index[STAGE1] == SETTING_INDEX) && (m_menu.index[STAGE2] == NETWORK_SET) && (m_menu.index[STAGE3] == PORT_INDEX)) return &menu123;
        if ((m_menu.index[STAGE1] == SETTING_INDEX) && (m_menu.index[STAGE2] == NETWORK_SET) && (m_menu.index[STAGE3] == REMOTE_IP_INDEX)) return &menu124;
        if ((m_menu.index[STAGE1] == SETTING_INDEX) && (m_menu.index[STAGE2] == NETWORK_SET) && (m_menu.index[STAGE3] == SENSOR_ADDRESS_INDEX)) return &menu125;
        if ((m_menu.index[STAGE1] == SETTING_INDEX) && (m_menu.index[STAGE2] == NETWORK_SET) && (m_menu.index[STAGE3] == SIU_ADDRESS_INDEX)) return &menu126;
        
        if ((m_menu.index[STAGE1] == SETTING_INDEX) && (m_menu.index[STAGE2] == DATEnTIME_SET) && (m_menu.index[STAGE3] == NONE)) return &menu13;
        if ((m_menu.index[STAGE1] == SETTING_INDEX) && (m_menu.index[STAGE2] == DATEnTIME_SET) && (m_menu.index[STAGE3] == DATE_INDEX)) return &menu130;
        if ((m_menu.index[STAGE1] == SETTING_INDEX) && (m_menu.index[STAGE2] == DATEnTIME_SET) && (m_menu.index[STAGE3] == TIME_INDEX)) return &menu131;
    }
    else {
        if ((m_menu.index[STAGE1] == SETTING_INDEX) && ((m_menu.currentId == 1) || (m_menu.currentId == 14)) && (m_menu.stage == 1)) { 
            m_menu.index[STAGE2] = NONE;    //protect worong behavior
            m_menu.index[STAGE3] == NONE;   //protect worong behavior
            return &menu14;
        }
        if ((m_menu.index[STAGE1] == SETTING_INDEX) && (m_menu.stage == 0)) { 
            m_menu.index[STAGE2] = NONE;    //protect worong behavior
            m_menu.index[STAGE3] == NONE;   //protect worong behavior
            return &menu1;
        }
    }
    if ((m_menu.index[STAGE1] == MEASUREMENT_INDEX) && (m_menu.index[STAGE2] == NONE) && (m_menu.index[STAGE3] == NONE)) return &menu2;
    if ((m_menu.index[STAGE1] == MEASUREMENT_INDEX) && (m_menu.index[STAGE2] == VI_INDEX) && (m_menu.index[STAGE3] == NONE)) return &menu20;
    if ((m_menu.index[STAGE1] == MEASUREMENT_INDEX) && (m_menu.index[STAGE2] == CO_INDEX) && (m_menu.index[STAGE3] == NONE)) return &menu21;
    if ((m_menu.index[STAGE1] == MEASUREMENT_INDEX) && (m_menu.index[STAGE2] == NO_INDEX) && (m_menu.index[STAGE3] == NONE)) return &menu22;
    if ((m_menu.index[STAGE1] == MEASUREMENT_INDEX) && (m_menu.index[STAGE2] == NO2_INDEX) && (m_menu.index[STAGE3] == NONE)) return &menu23;
    if ((m_menu.index[STAGE1] == MEASUREMENT_INDEX) && (m_menu.index[STAGE2] == NOX_INDEX) && (m_menu.index[STAGE3] == NONE)) return &menu24;
    if ((m_menu.index[STAGE1] == MEASUREMENT_INDEX) && (m_menu.index[STAGE2] == Velocity_INDEX) && (m_menu.index[STAGE3] == NONE)) return &menu25;
    if ((m_menu.index[STAGE1] == MEASUREMENT_INDEX) && (m_menu.index[STAGE2] == DI_INDEX) && (m_menu.index[STAGE3] == NONE)) return &menu26;

    if ((m_menu.index[STAGE1] == EVENT_LIST_INDEX) && (m_menu.index[STAGE2] == NONE) && (m_menu.index[STAGE3] == NONE)) return &menu3;
    if ((m_menu.index[STAGE1] == EVENT_LIST_INDEX) && (m_menu.index[STAGE2] == EVENT_LIST_SUB_INDEX) && (m_menu.index[STAGE3] == NONE)) return &menu30;
    if ((m_menu.index[STAGE1] == EVENT_LIST_INDEX) && (m_menu.index[STAGE2] == EVENT_LIST_SUB_INDEX) && (m_menu.index[STAGE3] == EVENT_LIST_SUB_EVENT_INDEX)) return &menu301;
    if ((m_menu.index[STAGE1] == EVENT_LIST_INDEX) && (m_menu.index[STAGE2] == EVENT_CLEAR_SUB_INDEX) && (m_menu.index[STAGE3] == NONE)) return &menu31;

    return &menu0;
}

void limitStage(uint8_t limit) {
    if(m_menu.stage >= limit) m_menu.stage = limit;
}

void limitX(uint8_t limit) {
    if(m_menu.x == NONE) m_menu.x = limit;
    if(m_menu.x > limit) m_menu.x = 0;
}

int isSettingMenu(void) {
    if((m_menu.currentId >= 100) && (m_menu.currentId <= 150)) return 1;  //setting menu;
    if((m_menu.currentId == 14)) return 1;  //setting PASSWORD
    if((m_menu.currentId == 31)) return 1;  //EVENT CLEAR
    return 0;
}

void increasePosition(void) {
    m_menu.x++;
    if((m_menu.currentId >= 120) && (m_menu.currentId <= 123)) limitX(11); 
    if((m_menu.currentId == 124)) limitX(4); 
    if((m_menu.currentId == 125)) limitX(7); 
    if((m_menu.currentId == 126)) limitX(2); 
    if((m_menu.currentId == 130) || (m_menu.currentId == 131)) limitX(5);
    if((m_menu.currentId == 14)) limitX(3); //password
    if((m_menu.currentId == 31)) limitX(3); //password

}

void decreasePosition(void) {
    m_menu.x--;
    if((m_menu.currentId >= 120) && (m_menu.currentId <= 123)) limitX(11); 
    if((m_menu.currentId == 124)) limitX(4); 
    if((m_menu.currentId == 125)) limitX(7); 
    if((m_menu.currentId == 126)) limitX(2); 
    if((m_menu.currentId == 130) || (m_menu.currentId == 131)) limitX(5); 
    if((m_menu.currentId == 14)) limitX(3); //password
    if((m_menu.currentId == 31)) limitX(3); //password
}

void limitUint8(int *ip) {
    if(*ip > 255) *ip = 0;
    if(*ip < 0  ) *ip = 255;
}

void limitDateTime(int *dt, uint8_t limit) {
    if(*dt > limit) *dt = 0;
    if(*dt < 0) *dt = limit;
}

void increaseDate(uint8_t *d) {
    int val;
    val = d[datePack.index[m_menu.x]]; //select one if date[0..2] year, month, day
    val = val + datePack.digit[m_menu.x];
    if(datePack.index[m_menu.x] == 0) limitDateTime(&val, 99);  //year  0..99
    if(datePack.index[m_menu.x] == 1) limitDateTime(&val, 11);  //month 0..11
    if(datePack.index[m_menu.x] == 2) limitDateTime(&val, 30);  //day   0..30
    d[datePack.index[m_menu.x]] = val;
}

void decreaseDate(uint8_t *d) {
    int val;
    val = d[datePack.index[m_menu.x]]; //select one if date[0..2] year, month, day
    val = val - datePack.digit[m_menu.x];
    if(datePack.index[m_menu.x] == 0) limitDateTime(&val, 99);  //year  0..99
    if(datePack.index[m_menu.x] == 1) limitDateTime(&val, 11);  //month 0..11
    if(datePack.index[m_menu.x] == 2) limitDateTime(&val, 30);  //day   0..30
    d[datePack.index[m_menu.x]] = val;
}

void increaseTime(uint8_t *t) {
    int val;
    val = t[timePack.index[m_menu.x]]; //select one if time[0..2] hour, minute, second
    val = val + timePack.digit[m_menu.x];
    if(timePack.index[m_menu.x] == 0) limitDateTime(&val, 23);  //hour
    if(timePack.index[m_menu.x] == 1) limitDateTime(&val, 59);  //minute
    if(timePack.index[m_menu.x] == 2) limitDateTime(&val, 59);  //second
    t[timePack.index[m_menu.x]] = val;
}

void decreaseTime(uint8_t *t) {
    int val;
    val = t[timePack.index[m_menu.x]]; //select one if time[0..2] hour, minute, second
    val = val - timePack.digit[m_menu.x];
    if(timePack.index[m_menu.x] == 0) limitDateTime(&val, 23);  //hour
    if(timePack.index[m_menu.x] == 1) limitDateTime(&val, 59);  //minute
    if(timePack.index[m_menu.x] == 2) limitDateTime(&val, 59);  //second
    t[timePack.index[m_menu.x]] = val;
}

void increaseIp(uint8_t *ip) {
    int ipVal;
    ipVal = ip[ipPack.index[m_menu.x]]; //select one if ip[0..3]
    ipVal = ipVal + ipPack.digit[m_menu.x];
    limitUint8(&ipVal);
    ip[ipPack.index[m_menu.x]]=ipVal;
}

void decreaseIp(uint8_t *ip) {
    int ipVal;
    ipVal = ip[ipPack.index[m_menu.x]]; //select on of ip[0..3]
    ipVal = ipVal - ipPack.digit[m_menu.x];
    limitUint8(&ipVal);
    ip[ipPack.index[m_menu.x]]=ipVal;
}

void limitInt(int *i) {
    if(*i > 65535) *i = 0;
    if(*i < 0  )   *i = 65535;
}

void increaseInt(uint16_t *i){
    int val;
    val = *i;
    val = val + intPack.digit[m_menu.x];
    limitInt(&val);
    *i = val;
}

void decreaseInt(uint16_t *i){
    int val;
    val = *i;
    val = val - intPack.digit[m_menu.x];
    limitInt(&val);
    *i = val;
}

void increaseInt2(uint16_t *Int2) {
    int Int2Val;
    Int2Val = Int2[int2Pack.index[m_menu.x]]; //select one if Int2[0..1]
    Int2Val = Int2Val + int2Pack.digit[m_menu.x];
    if(int2Pack.index[m_menu.x] == 0)   //slave address
        limitUint8(&Int2Val);
    else
        limitInt(&Int2Val);             //start address
    Int2[int2Pack.index[m_menu.x]]=Int2Val;
}

void decreaseInt2(uint16_t *Int2) {
    int Int2Val;
    Int2Val = Int2[int2Pack.index[m_menu.x]]; //select one if Int2[0..3]
    Int2Val = Int2Val - int2Pack.digit[m_menu.x];
    if(int2Pack.index[m_menu.x] == 0)   //slave address
        limitUint8(&Int2Val);
    else
        limitInt(&Int2Val);             //start address
    Int2[int2Pack.index[m_menu.x]]=Int2Val;
}

void increaseUint8(uint8_t *i8) {
    int i;
    i = *i8;
    i = i + uint8Pack.digit[m_menu.x];
    limitUint8(&i);
    *i8 = i;
}

void decreaseUint8(uint8_t *i8) {
    int i;
    i = *i8;
    i = i - uint8Pack.digit[m_menu.x];
    limitUint8(&i);
    *i8 = i;
}

void limitPassCode(char *p) {
    if(*p > '9') *p = '0';
    if(*p < '0') *p = '9';
}

void printEventClear(void) {
    printf_lcd(1,2, "Clear...        ");
    sl_udelay_wait(700000); //hold 700ms
}

void clearPassCode(void) {
    m_menu.passLevel = 0;
    memset(tPassCode, '0', 4); 
}

void increasePassCode(void)
{
    char c;
    c = tPassCode[passCodePack.index[m_menu.x]]; //select on of passcode [0..3]
    c++;
    limitPassCode(&c);
    tPassCode[passCodePack.index[m_menu.x]]=c;
    if(strcmp(tPassCode, mPassCode.level1) == 0) m_menu.passLevel = 1;
    if(strcmp(tPassCode, mPassCode.level2) == 0) m_menu.passLevel = 2;

    if(m_menu.passLevel) {
        m_menu.operationMode = OPERATION_MODE_SHOPPING;
        if(m_menu.index[STAGE1] == SETTING_INDEX) {
            m_menu.index[m_menu.stage] = DATEnTIME_SET;
        }
        if(m_menu.index[STAGE1] == EVENT_LIST_INDEX) {
            clearAllEventLog();
            printEventClear();
            clearPassCode();
            //todo event clear
            m_menu.index[m_menu.stage] = NONE;
        }
        selectMenuDisplay(getMenu());
    }
}

void decreasePassCode(void)
{
    char c;
    c = tPassCode[passCodePack.index[m_menu.x]]; //select on of passcode [0..3]
    c--;
    limitPassCode(&c);
    tPassCode[passCodePack.index[m_menu.x]]=c;
    if(strcmp(tPassCode, mPassCode.level1) == 0) m_menu.passLevel = 1;
    if(strcmp(tPassCode, mPassCode.level2) == 0) m_menu.passLevel = 2;

    if(m_menu.passLevel) { 
        m_menu.operationMode = OPERATION_MODE_SHOPPING;
        if(m_menu.index[STAGE1] == SETTING_INDEX)
            m_menu.index[m_menu.stage] = DATEnTIME_SET;
        if(m_menu.index[STAGE1] == EVENT_LIST_INDEX) {
            printEventClear();
            clearPassCode();
            //todo event clear
            m_menu.index[m_menu.stage] = NONE;
        }
        selectMenuDisplay(getMenu());
    }
}

void printIp(uint8_t *ip) {
    printf_lcd(1,2, "%03d.%03d.%03d.%03d",ip[0],ip[1],ip[2],ip[3]);
}

void printPasscode(void) {
    printf_lcd(1,2,"      ****      ");
    if(m_menu.operationMode == OPERATION_MODE_SETTING)
        printf_lcd(passCodePack.position[m_menu.x], 2, "%c", tPassCode[m_menu.x]);
}

static uint8_t mEthUpdate = 0;

void ethUpdate(void) {
    if(mEthUpdate) {
        w5500_spi_init();
        w5500_ip_init();
        mEthUpdate = 0;
    } 
}

// 세팅 값을 변경시키는
int setMenu(void) {
    cursor(ON);
    uint8_t button = getButton();
    switch (button) {
        case BTN_UP:
            if((m_menu.currentId >= 100) && (m_menu.currentId <= 105)) setOX(&mSensorConfig.cfg[m_menu.currentId%100], 2,2);
            if((m_menu.currentId >= 110) && (m_menu.currentId <= 111)) setOX(&mModeConfig.cfg[m_menu.currentId%110], 2,2);
            if((m_menu.currentId == 120)) increaseIp(mNetworkConfig.eth.ip);
            if((m_menu.currentId == 121)) increaseIp(mNetworkConfig.eth.sn);
            if((m_menu.currentId == 122)) increaseIp(mNetworkConfig.eth.gw);
            if((m_menu.currentId == 123)) increaseIp(mNetworkConfig.remote_ip);
            if((m_menu.currentId == 124)) increaseInt(&mNetworkConfig.port);
            if((m_menu.currentId == 125)) increaseInt2(mNetworkConfig.modbus);
            if((m_menu.currentId == 126)) increaseUint8(&mNetworkConfig.siuAddress);
            if((m_menu.currentId == 130)) increaseDate(mDateTimeConfig.date);
            if((m_menu.currentId == 131)) increaseTime(mDateTimeConfig.time);
            if((m_menu.currentId == 14)) increasePassCode();
            if((m_menu.currentId == 31)) increasePassCode();
            break;
        case BTN_DOWN:
            if((m_menu.currentId >= 100) && (m_menu.currentId <= 105)) setOX(&mSensorConfig.cfg[m_menu.currentId%100], 2,2);
            if((m_menu.currentId >= 110) && (m_menu.currentId <= 111)) setOX(&mModeConfig.cfg[m_menu.currentId%110], 2,2);
            if((m_menu.currentId == 120)) decreaseIp(mNetworkConfig.eth.ip);
            if((m_menu.currentId == 121)) decreaseIp(mNetworkConfig.eth.sn);
            if((m_menu.currentId == 122)) decreaseIp(mNetworkConfig.eth.gw);
            if((m_menu.currentId == 123)) decreaseIp(mNetworkConfig.remote_ip);
            if((m_menu.currentId == 124)) decreaseInt(&mNetworkConfig.port);
            if((m_menu.currentId == 125)) decreaseInt2(mNetworkConfig.modbus);
            if((m_menu.currentId == 126)) decreaseUint8(&mNetworkConfig.siuAddress);
            if((m_menu.currentId == 130)) decreaseDate(mDateTimeConfig.date);
            if((m_menu.currentId == 131)) decreaseTime(mDateTimeConfig.time);
            if((m_menu.currentId == 14)) decreasePassCode();
            if((m_menu.currentId == 31)) decreasePassCode();
            break;
        case BTN_LEFT:
            if((m_menu.currentId >= 100) && (m_menu.currentId <= 105)) m_menu.operationMode = OPERATION_MODE_SHOPPING;
            if((m_menu.currentId >= 110) && (m_menu.currentId <= 111)) m_menu.operationMode = OPERATION_MODE_SHOPPING;
            if((m_menu.currentId >= 120) && (m_menu.currentId <= 126)) decreasePosition();
            if((m_menu.currentId == 130) || (m_menu.currentId == 131)) decreasePosition();
            if((m_menu.currentId == 14)) decreasePosition();
            if((m_menu.currentId == 31)) decreasePosition();
            break;
        case BTN_RIGHT:
            if((m_menu.currentId >= 120) && (m_menu.currentId <= 126)) increasePosition();
            if((m_menu.currentId == 130) || (m_menu.currentId == 131)) increasePosition();
            if((m_menu.currentId == 14)) increasePosition();
            if((m_menu.currentId == 31)) increasePosition();
            break;
        case BTN_ESC:
            m_menu.operationMode = OPERATION_MODE_SHOPPING;
            if(mEthUpdate == 0) {
                restoreNetworkConfig();//when ESC without Save
            }
            break;
        case BTN_ENTER:
            if(m_menu.currentId == 130)  { setDate(); getDateTime(); };
            if(m_menu.currentId == 131)  { setTime(); getDateTime(); };
            lput(16,2,'*');
            saveSystemConfig();
            loadSystemConfig();
            backupNetworkConfig();
            if((m_menu.currentId >= 120) && (m_menu.currentId <= 124)) {
                mEthUpdate = 1;
            }
            sl_udelay_wait(100000);  //wait 0.1 second
            lput(16,2,' ');
            break;
    }
    return button;
}

void setProcess(void) {
    if(setMenu()) applyMenuValuetoLcd();
}

//변경된 세팅 값을 표시하도롣
void applyMenuValuetoLcd(void) {
    upDateMenuId();
    if((m_menu.currentId >= 100) && (m_menu.currentId <= 105)) lput(2,2, mSensorConfig.cfg[m_menu.currentId%100]);
    if((m_menu.currentId >= 110) && (m_menu.currentId <= 111)) lput(2,2, mModeConfig.cfg[m_menu.currentId%110]);
    if((m_menu.currentId == 120)) printIp(mNetworkConfig.eth.ip);
    if((m_menu.currentId == 121)) printIp(mNetworkConfig.eth.sn);
    if((m_menu.currentId == 122)) printIp(mNetworkConfig.eth.gw);
    if((m_menu.currentId == 123)) printIp(mNetworkConfig.remote_ip);
    if((m_menu.currentId == 124)) printf_lcd(1,2,"%05d", mNetworkConfig.port);
    if((m_menu.currentId == 125)) printf_lcd(1,2,"DA:%03d RA:%05d", mNetworkConfig.modbus[0], mNetworkConfig.modbus[1]);
    if((m_menu.currentId == 126)) printf_lcd(1,2,"%03d", mNetworkConfig.siuAddress);
    if((m_menu.currentId == 130)) printf_lcd(3,2, "%02d-%02d-%02d", mDateTimeConfig.date[0], mDateTimeConfig.date[1]+1, mDateTimeConfig.date[2]+1);
    if((m_menu.currentId == 131)) printf_lcd(1,2, "%02d:%02d:%02d", mDateTimeConfig.time[0], mDateTimeConfig.time[1], mDateTimeConfig.time[2]);
    if((m_menu.currentId >= 20) && (m_menu.currentId <= 26)) displayMeasuremet(m_menu.currentId);
    if((m_menu.currentId == 14)) printPasscode(); 
    if((m_menu.currentId == 31)) printPasscode(); 
}

//세팅모드에서 커서를 켜고 끄는,
void cursor(uint8_t onOff) {
    upDateMenuId();
    if((m_menu.currentId >= 100) && (m_menu.currentId <= 105)) cursor_xy(2, 2, onOff);
    if((m_menu.currentId >= 110) && (m_menu.currentId <= 111)) cursor_xy(2, 2, onOff);
    if((m_menu.currentId >= 120) && (m_menu.currentId <= 123)) cursor_xy(ipPack.position[m_menu.x], 2, onOff);
    if((m_menu.currentId == 124)) cursor_xy(intPack.position[m_menu.x], 2, onOff);
    if((m_menu.currentId == 125)) cursor_xy(int2Pack.position[m_menu.x], 2, onOff);
    if((m_menu.currentId == 126)) cursor_xy(uint8Pack.position[m_menu.x], 2, onOff);
    if((m_menu.currentId == 130)) cursor_xy(datePack.position[m_menu.x], 2, onOff);
    if((m_menu.currentId == 131)) cursor_xy(timePack.position[m_menu.x], 2, onOff);
    if((m_menu.currentId == 14)) cursor_xy(passCodePack.position[m_menu.x], 2, onOff);
    if((m_menu.currentId == 31)) cursor_xy(passCodePack.position[m_menu.x], 2, onOff);
}

//OX 형태의 값을 수정하는
void setOX(uint8_t *ref, uint8_t x, uint8_t y) {
    if(*ref == 'O') { 
        *ref = 'X';
        if(ref == &mModeConfig.cfg[0]) mModeConfig.cfg[1] = 'O';  //toggle server, client
        if(ref == &mModeConfig.cfg[1]) mModeConfig.cfg[0] = 'O';  
    }
    else {
        *ref = 'O';
        if(ref == &mModeConfig.cfg[0]) mModeConfig.cfg[1] = 'X';  
        if(ref == &mModeConfig.cfg[1]) mModeConfig.cfg[0] = 'X';  
    }
    lput(x,y, *ref);
}

//메뉴의 스테이지를 저단으로
void moveMenuStageLeft(void) {
    if(m_menu.stage) m_menu.stage--;
    if(m_menu.stage == STAGE1) {
        m_menu.index[STAGE2] = NONE;
        m_menu.index[STAGE3] = NONE;
        clearPassCode();
    }
    if(m_menu.stage == STAGE2) {
        m_menu.index[STAGE3] = NONE;
    }
}

//메뉴의 스테이지를 고단으로
void moveMenuStageRight(void) {
    m_menu.stage++;
    if(m_menu.index[STAGE1] == SETTING_INDEX) { 
        if(m_menu.passLevel == 0) limitStage(STAGE2);   //blocking setting stage
        else limitStage(STAGE3);
    }
    else if(m_menu.index[STAGE1] == MEASUREMENT_INDEX) limitStage(STAGE2);
    else if(m_menu.index[STAGE1] == EVENT_LIST_INDEX) limitStage(STAGE3);
    else limitStage(STAGE1);

    if(m_menu.stage == STAGE1) {
        m_menu.index[STAGE2] = NONE;
        m_menu.index[STAGE3] = NONE;
    }
    if(m_menu.stage == STAGE2) {
        if(m_menu.index[STAGE2] == NONE) m_menu.index[STAGE2] = 0;
        m_menu.index[STAGE3] = NONE;
    }
    if(m_menu.stage == STAGE3) {
        if(m_menu.index[STAGE3] == NONE) m_menu.index[STAGE3] = 0;
    }
}

//스테이지 상의 메뉴 인덱스를 올리기 
void moveMenuIndexUp(void) {
    if(m_menu.index[m_menu.stage] == NONE) m_menu.index[m_menu.stage] = 0; //set to base point
    else {
        m_menu.index[m_menu.stage]--;

        //chck undeflower every menu.
        if(m_menu.stage == STAGE1) {
            if(m_menu.index[m_menu.stage] == NONE) m_menu.index[STAGE1] = NUM_OF_TOP_MENU-1;
            m_menu.index[STAGE2] = NONE;
            m_menu.index[STAGE3] = NONE;
        }

        if(m_menu.index[STAGE1] == SETTING_INDEX) {
            if(m_menu.stage == STAGE2) {
                if(m_menu.index[m_menu.stage] == NONE) {
                    if(m_menu.passLevel == 2)
                        m_menu.index[STAGE2] = menu1.numberOfsub-1;//date, network, mode, sensor
                    else
                        m_menu.index[STAGE2] = menu1.numberOfsub-3; //only date, network
                }
            }
            if(m_menu.stage == STAGE3) {
                if(m_menu.index[STAGE2] == SENSOR_SET) {
                    if(m_menu.index[m_menu.stage] == NONE) m_menu.index[STAGE3] = menu10.numberOfsub-1;
                }
                if(m_menu.index[STAGE2] == MODE_SET) {
                    if(m_menu.index[m_menu.stage] == NONE) m_menu.index[STAGE3] = menu11.numberOfsub-1;
                }
                if(m_menu.index[STAGE2] == NETWORK_SET) {
                    if(m_menu.index[m_menu.stage] == NONE) m_menu.index[STAGE3] = menu12.numberOfsub-1;
                }
                if(m_menu.index[STAGE2] == DATEnTIME_SET) {
                    if(m_menu.index[m_menu.stage] == NONE) m_menu.index[STAGE3] = menu13.numberOfsub-1;
                }
            }
        }

        if(m_menu.index[STAGE1] == MEASUREMENT_INDEX) {
            if(m_menu.stage == STAGE2) {
                if(m_menu.index[m_menu.stage] == NONE) m_menu.index[STAGE2] = menu2.numberOfsub-1;
            }
        }

        if(m_menu.index[STAGE1] == EVENT_LIST_INDEX) {
            if(m_menu.stage == STAGE2) {
                if(m_menu.index[m_menu.stage] == NONE) m_menu.index[STAGE2] = menu3.numberOfsub-1;
            }
            if(m_menu.stage == STAGE3) {
                if(m_menu.index[m_menu.stage] == NONE) m_menu.index[STAGE3] = menu30.numberOfsub-1;
            }
        }
    }
}

//스테이지 상의 메뉴 인덱스를 내리기 
void moveMenuIndexDown(void) {
    if(m_menu.index[m_menu.stage] == NONE) m_menu.index[m_menu.stage] = 0; //set to base point
    else m_menu.index[m_menu.stage]++;

    if(m_menu.stage == STAGE1) {
        m_menu.index[STAGE1] %= NUM_OF_TOP_MENU;
        m_menu.index[STAGE2] = NONE;
        m_menu.index[STAGE3] = NONE;
    }

    if(m_menu.index[STAGE1] == SETTING_INDEX) {
        if(m_menu.stage == STAGE2) {
            if(m_menu.passLevel == 2)
                m_menu.index[m_menu.stage] %= menu1.numberOfsub;   //date, network, mode, sensor
            else
                m_menu.index[m_menu.stage] %= menu1.numberOfsub-2; //only date, network
        }
        if(m_menu.stage == STAGE3) {
            if(m_menu.index[STAGE2] == SENSOR_SET) {
                m_menu.index[m_menu.stage] %= menu10.numberOfsub;
            }
            if(m_menu.index[STAGE2] == MODE_SET) {
                m_menu.index[m_menu.stage] %= menu11.numberOfsub;
            }
            if(m_menu.index[STAGE2] == NETWORK_SET) {
                m_menu.index[m_menu.stage] %= menu12.numberOfsub;
            }
            if(m_menu.index[STAGE2] == DATEnTIME_SET) {
                m_menu.index[m_menu.stage] %= menu13.numberOfsub;
            }
        }
    }
    if(m_menu.index[STAGE1] == MEASUREMENT_INDEX) {
        if(m_menu.stage == STAGE2) {
            m_menu.index[m_menu.stage] %= menu2.numberOfsub;
        }
    }
    if(m_menu.index[STAGE1] == EVENT_LIST_INDEX) {
        if(m_menu.stage == STAGE2) {
            m_menu.index[m_menu.stage] %= menu3.numberOfsub;
        }
        if(m_menu.stage == STAGE3) {
            m_menu.index[m_menu.stage] %= menu30.numberOfsub;
        }
    }
}

 
void saveOffsetToEEPROM(void) {
    float offset[OFFSET_APPLY_MEMBER];
    for(int i=0; i<OFFSET_APPLY_MEMBER; i++) {
        offset[i] = RTU_fx[i].offset;
    }
    writeEEPROM(EEP_ADDR_OFFSET_VALUE, offset, sizeof(offset));
}

void loadOffsetFromEEPROM(void) {
    float offset[OFFSET_APPLY_MEMBER];
    readEEPROM(EEP_ADDR_OFFSET_VALUE, offset, sizeof(offset));

    for(int i=0; i<OFFSET_APPLY_MEMBER; i++) {
        RTU_fx[i].offset = offset[i];
    }
}

void saveDefaultOffsetToEEPROM(void) {
    float offset[6];
    for(int i=0; i<6; i++) {
        offset[i] = 0.0;
    }
    writeEEPROM(EEP_ADDR_OFFSET_VALUE, offset, sizeof(offset));
}

void saveSystemConfig(void) {
    writeEEPROM(EEP_ADDR_NETWORK_CONFIG , &mNetworkConfig, sizeof(mNetworkConfig));
    writeEEPROM(EEP_ADDR_SENSOR_CONFIG, &mSensorConfig, sizeof(mSensorConfig));
    writeEEPROM(EEP_ADDR_MODE_CONFIG, &mModeConfig, sizeof(mModeConfig));
    writeEEPROM(EEP_ADDR_PASSWORD, &mPassCode, sizeof(mPassCode));
    saveOffsetToEEPROM();
}

void loadSystemConfig(void) {
    readEEPROM(EEP_ADDR_NETWORK_CONFIG , &mNetworkConfig, sizeof(mNetworkConfig));
    readEEPROM(EEP_ADDR_SENSOR_CONFIG, &mSensorConfig, sizeof(mSensorConfig));
    readEEPROM(EEP_ADDR_MODE_CONFIG, &mModeConfig, sizeof(mModeConfig));
    readEEPROM(EEP_ADDR_PASSWORD, &mPassCode, sizeof(mPassCode));
    memcpy(&gWIZNETINFO, &mNetworkConfig.eth, sizeof(wiz_NetInfo));
    memcpy(gSERVERNETINFO.ip, mNetworkConfig.remote_ip, 4);
    gSERVERNETINFO.port = mNetworkConfig.port;
    getSensorConfigForPollingAndDisplay();  //get O,X
    loadOffsetFromEEPROM();
}

void saveDefultConfig(void) {
    writeEEPROM(EEP_ADDR_NETWORK_CONFIG, &cNetworkConfig, sizeof(cNetworkConfig));
    writeEEPROM(EEP_ADDR_SENSOR_CONFIG, &cSensorConfig, sizeof(cSensorConfig));
    writeEEPROM(EEP_ADDR_MODE_CONFIG, &cModeConfig, sizeof(cModeConfig));
    writeEEPROM(EEP_ADDR_PASSWORD, &cPassCode, sizeof(cPassCode));
    saveDefaultOffsetToEEPROM();
    loadSystemConfig();
}

void setDate(void) {
    struct rtc_time stime;
//현재 조정중인 날짜 데이터
    stime.tm_year = mDateTimeConfig.date[0];
    stime.tm_mon  = mDateTimeConfig.date[1]+1;
    stime.tm_mday = mDateTimeConfig.date[2]+1;
    stime.tm_wday = 1;

//시간 조정이 아니므로 시간은 현재 시간을 읽어서 적용한다
    getDateTime();
    stime.tm_hour = mDateTimeConfig.time[0];
    stime.tm_min  = mDateTimeConfig.time[1];
    stime.tm_sec  = mDateTimeConfig.time[2];
    isl1208_i2c_set_time(&stime);
}

void setTime(void) {
    struct rtc_time stime;
//현재 조정중인 시간 데이터
    stime.tm_hour = mDateTimeConfig.time[0];
    stime.tm_min  = mDateTimeConfig.time[1];
    stime.tm_sec  = mDateTimeConfig.time[2];

//날짜 조정이 아니므로 날짜는 현재 날짜를 읽어서 적용한다
    getDateTime();
    stime.tm_year = mDateTimeConfig.date[0];
    stime.tm_mon  = mDateTimeConfig.date[1];
    stime.tm_mday = mDateTimeConfig.date[2];
    stime.tm_wday = 1;
    isl1208_i2c_set_time(&stime);
}

//날짜 시간 가져오기

struct rtc_time rtcTime;
void getDateTime(void) {
    isl1208_i2c_read_time(&rtcTime);
    mDateTimeConfig.date[0] = rtcTime.tm_year;
    mDateTimeConfig.date[1] = rtcTime.tm_mon-1;
    mDateTimeConfig.date[2] = rtcTime.tm_mday-1;
    mDateTimeConfig.time[0] = rtcTime.tm_hour;
    mDateTimeConfig.time[1] = rtcTime.tm_min;
    mDateTimeConfig.time[2] = rtcTime.tm_sec;
}

char* uint32toFloatStr(uint32_t value, char *str)
{
  float floatValue = convertToFloat(value);
  sprintf(str, "%0.2f", floatValue);
  return str;
}

void displayMeasuremet(int menuId)
{
    char str[32];
    if(! isPollingRTUfree()) return;

    switch(menuId) {
        case 20:
            if(RTU_fx[V_VI_POLLING_INDEX].flag =='O') {
                printf_lcd(1,1, "VI              ");
                if(RTU_fx[V_VI_POLLING_INDEX].value != NaN)
                    printf_lcd(1,2, "%s ", uint32toFloatStr(RTU_fx[V_VI_POLLING_INDEX].value, str));
                else 
                    printf_lcd(1,2, "..... ");
                viUnitDisplay();

            }
            else {
                printf_lcd(1,1, "VI              ");
                printf_lcd(1,2, "-----           ");
                viUnitDisplay();
            }
            break;
        case 21:
            if(RTU_fx[V_CO_POLLING_INDEX].flag =='O') {
                printf_lcd(1,1, "CO              ");
                if(RTU_fx[V_CO_POLLING_INDEX].value != NaN)
                    printf_lcd(1,2, "%s ppm            ", uint32toFloatStr(RTU_fx[V_CO_POLLING_INDEX].value, str));
                else 
                    printf_lcd(1,2, "..... ppm       ");
            }
            else {
                printf_lcd(1,1, "CO              ");
                printf_lcd(1,2, "----- ppm       ");
            }
            break;
        case 22:
            if(RTU_fx[V_NO_POLLING_INDEX].flag =='O') {
                printf_lcd(1,1, "NO              ");
                if(RTU_fx[V_NO_POLLING_INDEX].value != NaN)
                    printf_lcd(1,2, "%s ppm", uint32toFloatStr(RTU_fx[V_NO_POLLING_INDEX].value, str));
                else 
                    printf_lcd(1,2, "..... ppm       ");
            }
            else {
                printf_lcd(1,1, "NO              ");
                printf_lcd(1,2, "----- ppm       ");
            }
            break;
        case 23:
            if(RTU_fx[V_NO2_POLLING_INDEX].flag =='O') {
                printf_lcd(1,1, "NO2             ");
                if(RTU_fx[V_NO2_POLLING_INDEX].value != NaN)
                    printf_lcd(1,2, "%s ppm         ", uint32toFloatStr(RTU_fx[V_NO2_POLLING_INDEX].value, str));
                else 
                    printf_lcd(1,2, "..... ppm       ");
            }
            else {
                printf_lcd(1,1, "NO2             ");
                printf_lcd(1,2, "----- ppm       ");
            }
            break;
        case 24:
            if(RTU_fx[V_NOX_POLLING_INDEX].flag =='O') {
                printf_lcd(1,1, "NOX             ");
                if(RTU_fx[V_NOX_POLLING_INDEX].value != NaN)
                    printf_lcd(1,2, "%s ppm        ", uint32toFloatStr(RTU_fx[V_NOX_POLLING_INDEX].value, str));
                else
                    printf_lcd(1,2, "..... ppm       ");
            }
            else {
                printf_lcd(1,1, "NOX             ");
                printf_lcd(1,2, "----- ppm       ");
            }
            break;
        case 25:
            // if(RTU_fx[VELOCITY_ACQUISITION_INDEX].flag =='O') printf_lcd(1,2, "Velocity %s", uint32toFloatStr(RTU_fx[VELOCITY_ACQUISITION_INDEX].value, str));
            if(RTU_fx[A_U_AXIS_POLLING_INDEX].flag =='O') {
                printf_lcd(1,1, "Velocity        ");
                if(RTU_fx[A_U_AXIS_POLLING_INDEX].value != NaN)
                    printf_lcd(1,2, "%s m/s        ", uint32toFloatStr(RTU_fx[A_U_AXIS_POLLING_INDEX].value, str));
                else
                    printf_lcd(1,2, "..... m/s      ");
            }
            else {
                printf_lcd(1,1, "Velocity        ");
                printf_lcd(1,2, "----- m/s       ");
            }
            break;
        case 26:
        {
            int diIndex = 0;
            int i;
            //메모리에 상태 채우기
            for (i = 0; i < 9; i++) {
                if (i % 2 == 0) {
                    str[i] = di_array[diIndex % 5]->state + '0';
                    diIndex++;
                } else {
                    str[i] = '-';
                }
            }
            str[i] = 0;
            //표시하기
            printf_lcd(1,1, "DI 1-2-3-4-5    ");
            printf_lcd(1,2, "   %s", str);
        }
            break;
        default:
            break;
    }
}

void autoReturnToHome(void) {
    static int start = 0;
    static int accTimer = 0;
    static int prevMenu;
    if(start == 0) {
        prevMenu = m_menu.currentId;
        start = 1;
        return;
    }
    if(prevMenu == m_menu.currentId) {  //menu의 변화가 없는 상태에서 10분이상 진행되면 초기화면으로 복귀 23.10.6
        accTimer++;
        if(accTimer >= TEN_MINUTE) {
            if(m_menu.currentId != 0) initMenu();
            accTimer = 0;
        }
    }
    else {  //메뉴에 변화가 있으면 누적 타이머 초기화
        accTimer = 0;
    }
    prevMenu = m_menu.currentId;
}