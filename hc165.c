#include "hc165.h"

int read_hc165(void) 
{
    static char init=0;
    if(init == 0) {        
        GPIO_PinModeSet(gpioPortA, DATA_PIN,  gpioModeInputPull,1);
        GPIO_PinModeSet(gpioPortC, CLOCK_PIN, gpioModePushPull, 0); 
        GPIO_PinModeSet(gpioPortC, LATCH_PIN, gpioModePushPull, 0); 
        init = 1;
    }

    // Read data from shift registers
    GPIO_PinOutClear(gpioPortC, LATCH_PIN);
    GPIO_PinOutSet(gpioPortC, LATCH_PIN);

    int data = 0;  // Variable to store the received data

    for (int i = 0; i < NUM_BITS; i++) {

        int bit = GPIO_PinInGet(gpioPortA, DATA_PIN);
        data |= (bit << (NUM_BITS - 1 - i));  // Store the bit in the data variable
        GPIO_PinOutClear(gpioPortC, CLOCK_PIN);
        GPIO_PinOutSet(gpioPortC, CLOCK_PIN);
    }
    return data;
}

Button_t btnUp;
Button_t btnDown;
Button_t btnLeft;
Button_t btnRight;
Button_t btnSet;
Button_t btnEnter;
Button_t btnEsc;
Button_t btnBluetooth;

Button_t *btn_array[] = {
    &btnUp,
    &btnDown,
    &btnLeft,
    &btnRight,
    &btnSet,
    &btnEnter,
    &btnEsc,
    &btnBluetooth
};

DI_t di1;
DI_t di2;
DI_t di3;
DI_t di4;
DI_t di5;

DI_t *di_array[] = {
    &di1,
    &di2,
    &di3,
    &di4,
    &di5
};

void btnAndDiPoll(void)
{
    uint16_t btnAndDi;

    //move button state to history
    for(int i = 0; i < 8; i++ ) {
        btn_array[i]->history = btn_array[i]->state;
    }
    //move di state to history
    for(int i = 0; i < 5; i++ ) {
        di_array[i]->history = di_array[i]->state;
    }

    btnAndDi = read_hc165();

    //save current button state
    for(int i = 0; i < 8; i++ ) {
        if(~btnAndDi & (1<<i)) {
            btn_array[i]->state = BTN_PRESSED_STATE;
        }
        else {
            btn_array[i]->state = BTN_RELEASED_STATE;
        }
    }

    for(int i = 0; i < 8; i++ ) {
        if((btn_array[i]->history == BTN_RELEASED_STATE) && (btn_array[i]->state == BTN_PRESSED_STATE)) {
            btn_array[i]->key = BTN_PRESSED_EVENT;
        }
        if((btn_array[i]->history == BTN_PRESSED_STATE) && (btn_array[i]->state == BTN_RELEASED_STATE)) {
            btn_array[i]->key = BTN_RELEASED_EVENT;
        }
        if((btn_array[i]->history == BTN_PRESSED_STATE) && (btn_array[i]->state == BTN_PRESSED_STATE)) {
            btn_array[i]->key = BTN_PRESSED_HOLD;
        }
        if((btn_array[i]->history == BTN_RELEASED_STATE) && (btn_array[i]->state == BTN_RELEASED_STATE)) {
            btn_array[i]->key = BTN_RELEASED_HOLD;
        }
    }

    //save current di state
    for(int i = 0; i < 5; i++ ) {
        if(btnAndDi & (0x0100<<i)) {
            di_array[i]->state = DI_STATE_CLOSE;
        }
        else {
            di_array[i]->state = DI_STATE_OPEN;
        }
    }
}

int checkButtonPressed(void)
{
    for(int i=0; i < 8; i++) {
        if (btn_array[i]->key == BTN_PRESSED_EVENT) return 1;
    }    
    return 0;
}

uint8_t getButton(void) {
    if (btn_array[0]->key == BTN_PRESSED_EVENT) return BTN_UP;
    if (btn_array[1]->key == BTN_PRESSED_EVENT) return BTN_DOWN;
    if (btn_array[2]->key == BTN_PRESSED_EVENT) return BTN_LEFT;
    if (btn_array[3]->key == BTN_PRESSED_EVENT) return BTN_RIGHT;
    if (btn_array[4]->key == BTN_PRESSED_EVENT) return BTN_SET;
    if (btn_array[5]->key == BTN_PRESSED_EVENT) return BTN_ENTER;
    if (btn_array[6]->key == BTN_PRESSED_EVENT) return BTN_ESC;
    if (btn_array[7]->key == BTN_PRESSED_EVENT) return BTN_BLUETOOTH;
    return 0;   //no button event;
}

uint8_t pack_DI_bytes(void) {
    uint8_t result = 0;
    for (int i = 0; i < 5; i++) {
        result |= (di_array[i]->state & 1) << i;
    }
    return result;
}
