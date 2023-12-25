
#ifndef HC165_H
#define HC165_H

#include <stdio.h>
#include "em_gpio.h"
#include "stdint.h"
#include "stdarg.h"
#include "sl_udelay.h"

#define DATA_PIN  8    // PA8 Pin connected to 74HC165 data pin (SER)
#define CLOCK_PIN 5    // PC5 Pin connected to 74HC165 clock pin (SRCLK)
#define LATCH_PIN 6    // PC6 Pin connected to 74HC165 latch pin (RCLK)
#define NUM_BITS  16   // Number of bits in total (2 shift registers)

int read_hc165(void);

#define BTN_MASK_UP        0x0001
#define BTN_MASK_DOWN      0x0002
#define BTN_MASK_LEFT      0x0004
#define BTN_MASK_RIGHT     0x0008
#define BTN_MASK_SET       0x0010
#define BTN_MASK_ENTER     0x0020
#define BTN_MASK_ESC       0x0040
#define BTN_MASK_BLUETOOTH 0x0080

#define BTN_UP        1
#define BTN_DOWN      2
#define BTN_LEFT      3
#define BTN_RIGHT     4
#define BTN_SET       5
#define BTN_ENTER     6
#define BTN_ESC       7
#define BTN_BLUETOOTH 8

#define BTN_PRESSED_STATE   0
#define BTN_RELEASED_STATE  1
#define BTN_PRESSED_EVENT   2
#define BTN_RELEASED_EVENT  3
#define BTN_PRESSED_HOLD   4
#define BTN_RELEASED_HOLD  5

typedef struct {
    uint8_t state;
    uint8_t history;
    uint8_t key;
} Button_t;

#define DI_MASK_DI1 0x0100
#define DI_MASK_DI2 0x0200
#define DI_MASK_DI3 0x0400
#define DI_MASK_DI4 0x0800
#define DI_MASK_DI5 0x1000

#define DI_STATE_OPEN  0
#define DI_STATE_CLOSE 1

typedef struct {
    uint8_t state;
    uint8_t history;
} DI_t;

extern Button_t btnUp;
extern Button_t btnDown;
extern Button_t btnLeft;
extern Button_t btnRight;
extern Button_t btnSet;
extern Button_t btnEnter;
extern Button_t btnEsc;
extern Button_t btnBluetooth;
extern Button_t *btn_array[];

extern DI_t di1;
extern DI_t di2;
extern DI_t di3;
extern DI_t di4;
extern DI_t di5;
extern DI_t *di_array[];

void btnAndDiPoll(void);
uint8_t getButton(void);
int checkButtonPressed(void);

uint8_t pack_DI_bytes(void);

#endif



