
#include "event_log.h"
#include "at24c256.h"
#include "lcd.h"

DateTimeEventData_t mDateTimeEventData;
UnitStatus_t mUnitStatus;

uint16_t mEventLogCounter;

void initUnitStatus(void) {//이벤트 초기화 함수
    DateTimeEventData_t eventData;
    int found[4] = {0}; // 각 index가 발견되었는지 여부를 저장하기 위한 배열
    int foundCount = 0; // 발견된 index의 수
    
    memset(&mUnitStatus, 0, sizeof(mUnitStatus));
    
    loadEventLogCounter();
    if(mEventLogCounter == 0) return;

    for(int i = mEventLogCounter-1; i >= 0  && foundCount < 4; i--) {
        loadEventLogData(i, &eventData);
        if (found[eventData.index] == 0) {
            mUnitStatus.st[eventData.index] = eventData.event; // 최신 데이터 저장
            found[eventData.index] = 1; // 해당 index가 발견되었다고 표시
            foundCount++;    
        }
    }
}

void saveEventLogCounter(void) {

    writeEEPROM(EEP_ADDR_EVENT_LOG_COUNTER, &mEventLogCounter, sizeof(mEventLogCounter));
}

void loadEventLogCounter(void){
    readEEPROM(EEP_ADDR_EVENT_LOG_COUNTER, &mEventLogCounter, sizeof(mEventLogCounter));
    if(mEventLogCounter == 0XFFFF) { 
        mEventLogCounter = 0;
        saveEventLogCounter();
    }
    if(mEventLogCounter > MAX_EVENT_COUNT) mEventLogCounter = MAX_EVENT_COUNT;
}

void saveEventLogData(void) {
    if(mEventLogCounter < MAX_EVENT_COUNT) {
        writeEEPROM(EEP_ADDR_EVENT_LOG_DATA_BASE + (mEventLogCounter*sizeof(DateTimeEventData_t)), &mDateTimeEventData, sizeof(DateTimeEventData_t));
        mEventLogCounter++;
        saveEventLogCounter();    
    }
}

void loadEventLogData(uint16_t index, DateTimeEventData_t *eventData) {
    readEEPROM(EEP_ADDR_EVENT_LOG_DATA_BASE + (index*sizeof(DateTimeEventData_t)), eventData, sizeof(DateTimeEventData_t));
}

void clearAllEventLog(void) {
    mEventLogCounter = 0;
    saveEventLogCounter();
}

uint16_t extractEvent(uint16_t prevData, uint16_t newData) {
    uint16_t diffData = prevData ^ newData;
    uint16_t setBits = ~prevData & newData;
    uint16_t changedBitsData = diffData & setBits;
    return changedBitsData;
}

//for log display mask
const uint16_t cLogMask[4] = {VICONOX_ST1_LOG_MASK, VICONOX_ST2_LOG_MASK, AIRFLOW_ST1_LOG_MASK, AIRFLOW_ST2_LOG_MASK};
const char cSnesorInitial[4] = {'V', 'V', 'A', 'A'};

const char *cEventStr18[16] = {
    "OVERRIDE ACTIVE ",
    "MEMORY FL       ",
    "SETUP ACTIVE    ",
    "RTD FL          ",
    "TEMP OUT OF RG  ",
    "HS FL           ",
    "COMMS CRC FL    ",
    "HI-NO2 OFFSET   ",
    "STEPPER MOTOR FL",
    "CHOPPER SPEED ER",
    "HI IR SIGNAL    ",
    "LO IR SIGNAL    ",
    "GAS WHEEL FL    ",
    "VIS FL          ",
    "IR FL           ",
    "TX NOT DETECTED " 
};

const char *cEventStr19[16] = {
    "HIGH HUMIDITY   ", 
    "SN SATURATED NOW",
    "SN SATURATED    ",
    "DATA INVALID   V",
    "ALARM ACTIVE   V",
    "HI MEAS-VIS     ",
    "HI REF-VIS/NO2  ",
    "LO REF-VIS/NO2  ",
    "HI MEAS-NO2     ",
    "HI REF GAIN     ",
    "POWER UP        ",
    "MAINTENANCE MODE",
    "HIGH OPACITY    ",
    "DIRTY OPTICS    ",
    "STABILISING     ",
    "CO/NO BEAM BLOCK"
};

const char *cEventStr38[16] = {
    "SPARE3800       ",
    "SPARE3801       ",
    "SPARE3802       ",
    "Memory FL       ",
    "TEMP OUT OF RG  ",
    "RTD FL          ",
    "Gill Checksum ER",
    "Gill Timeout ER ",
    "Gill Internal ER",
    "Comms CRC FL    ",
    "SPARE3810       ",
    "SPARE3811       ",
    "Stabilising     ",
    "Override Active ",
    "SPARE3814       ",
    "SPARE3815       "
};

const char *cEventStr39[16] = {
    "SPARE3900       ",
    "SPARE3901       ",
    "SPARE3902       ",
    "Data Invalid   A",
    "Alarm Active   A",
    "SPARE3905       ",
    "SPARE3906       ",
    "SPARE3907       ",
    "SPARE3908       ",
    "SPARE3909       ",
    "SPARE3910       ",
    "SPARE3911       ",
    "SPARE3912       ",
    "SPARE3913       ",
    "SPARE3914       ",
    "SPARE3915       "
};

const char** cEventStrArray[4] = {
    cEventStr18,
    cEventStr19,
    cEventStr38,
    cEventStr39
};

void printStringsBasedOnBits(uint16_t data, char *strings[], int size) {
    for (int i = 0; i < size; i++) {
        if (data & (1 << i)) {
            printf("%s\n", strings[i]);
        }
    }
}

// int currentBit = 0;         // 현재 확인 중인 비트 위치
// int currentDataIndex = 0;   // 현재 확인 중인 데이터 인덱스
// DateTimeEventData_t eventLogData;

EventListViewManager_t vm = {.startFlag = 0, .currentBit=0, };

void onClick(void) {
    // Ensure vm is defined and properly initialized
    if (vm.startFlag == 0 ) {
        loadEventLogCounter();    
        if(mEventLogCounter == 0) {
            printf_lcd(1,1,"NO EVENT LIST   ");
            printf_lcd(1,2,"                ");
            return;
        }
        vm.currentDataIndex = mEventLogCounter;
        vm.currentDataIndex--;
        loadEventLogData(vm.currentDataIndex, &vm.eventLogData);
        vm.startFlag = 1;
        vm.currentBit = 0;
    }

    // Iterate while there is data
    while (vm.currentDataIndex >= 0) {
        vm.eventLogData.event = vm.eventLogData.event & cLogMask[vm.eventLogData.index];    //add 23.10.22
        if (vm.eventLogData.event & (1 << vm.currentBit)) {
            printf_lcd(1,1,"%c:%02d-%02d-%02d/%02d:%02d", cSnesorInitial[vm.eventLogData.index], vm.eventLogData.year, vm.eventLogData.month+1, vm.eventLogData.day+1, vm.eventLogData.hour, vm.eventLogData.minute);
            printf_lcd(1,2,"%s", cEventStrArray[vm.eventLogData.index][vm.currentBit]);
            Debug_printBT("index=%d, bit=%d, logid=%d", vm.currentDataIndex, vm.currentBit, vm.eventLogData.index);
            
            // Move to the next bit or data index
            vm.currentBit++;
            if (vm.currentBit >= 16) {
                vm.currentBit = 0;
                vm.currentDataIndex--;
                if (vm.currentDataIndex >= 0) {
                    loadEventLogData(vm.currentDataIndex, &vm.eventLogData);
                } else {
                    // Handle the case when we have read all data
                    // vm.startFlag = 0;
                    vm.currentBit = 0;
                    loadEventLogCounter();    
                    vm.currentDataIndex = mEventLogCounter;
                    vm.currentDataIndex--;
                    loadEventLogData(vm.currentDataIndex, &vm.eventLogData);
                }
            }
            break;
        }
        
        // If current bit is 0, move to the next bit or data index
        vm.currentBit++;
        if (vm.currentBit >= 16) {
            vm.currentBit = 0;
            vm.currentDataIndex--;
            if (vm.currentDataIndex >= 0) {
                loadEventLogData(vm.currentDataIndex, &vm.eventLogData);
            } else {
                // Handle the case when we have read all data
                // vm.startFlag = 0;
                vm.currentBit = 0;
                loadEventLogCounter();    
                vm.currentDataIndex = mEventLogCounter;
                vm.currentDataIndex--;
                loadEventLogData(vm.currentDataIndex, &vm.eventLogData);
            }
        }
    }
}

void onReverseClick(void) {
    // 초기화 확인
    if (vm.startFlag == 0) {
        loadEventLogCounter();
        if(mEventLogCounter == 0) {
            printf_lcd(1,1,"NO EVENT LIST   ");
            printf_lcd(1,2,"                ");
            return;
        }
        vm.startFlag = 1;
        vm.currentBit = 15; // 비트 위치를 마지막으로 설정
        vm.currentDataIndex = 0; // 데이터 인덱스를 처음으로 설정
        if (vm.currentDataIndex < mEventLogCounter) {
            loadEventLogData(vm.currentDataIndex, &vm.eventLogData);
        }
    }

    // 데이터가 있을 동안 반복
    while (vm.currentDataIndex < mEventLogCounter) {
        //todo bit mask here
        vm.eventLogData.event = vm.eventLogData.event & cLogMask[vm.eventLogData.index];    //add 23.10.22
        if (vm.eventLogData.event & (1 << vm.currentBit)) {
            printf_lcd(1,1,"%c:%02d-%02d-%02d/%02d:%02d", cSnesorInitial[vm.eventLogData.index], vm.eventLogData.year, vm.eventLogData.month+1, vm.eventLogData.day+1, vm.eventLogData.hour, vm.eventLogData.minute);
            printf_lcd(1,2,"%s", cEventStrArray[vm.eventLogData.index][vm.currentBit]);
            Debug_printBT("index=%d, bit=%d, logid=%d", vm.currentDataIndex, vm.currentBit, vm.eventLogData.index);

            // 다음 비트 또는 데이터 인덱스로 이동
            vm.currentBit--;
            if (vm.currentBit < 0) {
                vm.currentBit = 15;
                vm.currentDataIndex++;
                if (vm.currentDataIndex < mEventLogCounter) {
                    loadEventLogData(vm.currentDataIndex, &vm.eventLogData);
                } else {
                    // 모든 데이터를 읽은 경우 처리
                    vm.currentBit = 15;
                    loadEventLogCounter();
                    vm.currentDataIndex = 0;
                    loadEventLogData(vm.currentDataIndex, &vm.eventLogData);
                }
            }
            break;
        }

        // 현재 비트가 0이면 다음 비트 또는 데이터 인덱스로 이동
        vm.currentBit--;
        if (vm.currentBit < 0) {
            vm.currentBit = 15;
            vm.currentDataIndex++;
            if (vm.currentDataIndex < mEventLogCounter) {
                loadEventLogData(vm.currentDataIndex, &vm.eventLogData);
            } else {
                // 모든 데이터를 읽은 경우 처리
                vm.currentBit = 15;
                loadEventLogCounter();
                vm.currentDataIndex = 0;
                loadEventLogData(vm.currentDataIndex, &vm.eventLogData);
            }
        }
    }
}
