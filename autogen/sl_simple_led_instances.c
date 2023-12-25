/***************************************************************************//**
 * @file
 * @brief LED Driver Instances
 *******************************************************************************
 * # License
 * <b>Copyright 2019 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc.  Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement.  This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/

#include "sl_simple_led.h"
#include "em_gpio.h"
#include "sl_simple_led_client_config.h"
#include "sl_simple_led_comm_config.h"
#include "sl_simple_led_fault_config.h"

sl_simple_led_context_t simple_client_context = {
  .port = SL_SIMPLE_LED_CLIENT_PORT,
  .pin = SL_SIMPLE_LED_CLIENT_PIN,
  .polarity = SL_SIMPLE_LED_CLIENT_POLARITY,
};

const sl_led_t sl_led_client = {
  .context = &simple_client_context,
  .init = sl_simple_led_init,
  .turn_on = sl_simple_led_turn_on,
  .turn_off = sl_simple_led_turn_off,
  .toggle = sl_simple_led_toggle,
  .get_state = sl_simple_led_get_state,
};
sl_simple_led_context_t simple_comm_context = {
  .port = SL_SIMPLE_LED_COMM_PORT,
  .pin = SL_SIMPLE_LED_COMM_PIN,
  .polarity = SL_SIMPLE_LED_COMM_POLARITY,
};

const sl_led_t sl_led_comm = {
  .context = &simple_comm_context,
  .init = sl_simple_led_init,
  .turn_on = sl_simple_led_turn_on,
  .turn_off = sl_simple_led_turn_off,
  .toggle = sl_simple_led_toggle,
  .get_state = sl_simple_led_get_state,
};
sl_simple_led_context_t simple_fault_context = {
  .port = SL_SIMPLE_LED_FAULT_PORT,
  .pin = SL_SIMPLE_LED_FAULT_PIN,
  .polarity = SL_SIMPLE_LED_FAULT_POLARITY,
};

const sl_led_t sl_led_fault = {
  .context = &simple_fault_context,
  .init = sl_simple_led_init,
  .turn_on = sl_simple_led_turn_on,
  .turn_off = sl_simple_led_turn_off,
  .toggle = sl_simple_led_toggle,
  .get_state = sl_simple_led_get_state,
};

const sl_led_t *sl_simple_led_array[] = {
  &sl_led_client,
  &sl_led_comm,
  &sl_led_fault
};

void sl_simple_led_init_instances(void)
{
  sl_led_init(&sl_led_client);
  sl_led_init(&sl_led_comm);
  sl_led_init(&sl_led_fault);
}
