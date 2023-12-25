#ifndef EVENT_LOG_H
#define EVENT_LOG_H

#include "stdint.h"

#define MAX_EVENT_COUNT 32

#define VICONOX_ST1_LOG_INDEX 0
#define VICONOX_ST2_LOG_INDEX 1
#define AIRFLOW_ST1_LOG_INDEX 2
#define AIRFLOW_ST2_LOG_INDEX 3

#define VICONOX_ST1_LOG_MASK 0B1111111101101010
#define VICONOX_ST2_LOG_MASK 0B0000001111100000
#define AIRFLOW_ST1_LOG_MASK 0B0000001111101000
#define AIRFLOW_ST2_LOG_MASK 0B0000000000000000

typedef struct {
    uint8_t year;   // 년 (0~255)
    uint8_t month;  // 월 (1~12, 하지만 8비트에는 제한 없음)
    uint8_t day;    // 일 (1~31, 하지만 8비트에는 제한 없음)
    uint8_t hour;   // 시 (0~23)
    uint8_t minute; // 분 (0~59)
    uint8_t index;  // 0:18(VICONOX 10590), 1:19(VICONOX 10591), 2:38(AIRFLOW 10590), 3:39(AIRFLOW 10591)
    uint16_t event;
} DateTimeEventData_t;

typedef struct {
    uint16_t st[4]; // 0:18(VICONOX 10590), 1:19(VICONOX 10591), 2:38(AIRFLOW 10590), 3:39(AIRFLOW 10591)
} UnitStatus_t;

typedef struct {
    int startFlag;                     // 
    int currentBit;         // 현재 확인 중인 비트 위치
    int currentDataIndex;   // 현재 확인 중인 데이터 인덱스
    DateTimeEventData_t eventLogData;
} EventListViewManager_t;

uint16_t extractEvent(uint16_t prevData, uint16_t newData);
void printStringsBasedOnBits(uint16_t data, char *strings[], int size);
void clearAllEventLog(void);
void loadEventLogData(uint16_t index, DateTimeEventData_t *eventData);
void saveEventLogData(void);
void loadEventLogCounter(void);
void saveEventLogCounter(void);

// extern const char *cEventStr18[16];
// extern const char *cEventStr19[16];
// extern const char *cEventStr38[16];
// extern const char *cEventStr39[16];

extern DateTimeEventData_t mDateTimeEventData;
extern UnitStatus_t mUnitStatus;
extern uint16_t mEventLogCounter;

extern EventListViewManager_t vm;

void onClick(void);
void onReverseClick(void);

#endif