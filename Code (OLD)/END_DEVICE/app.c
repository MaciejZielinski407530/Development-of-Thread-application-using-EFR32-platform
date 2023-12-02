
/***************************************************************************//**
 * @file
 * @brief Core application logic.
 *******************************************************************************
 * # License
 * <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/
// Define module name for Power Manager debuging feature.
#define CURRENT_MODULE_NAME    "OPENTHREAD_SAMPLE_APP"


#include <assert.h>
#include <openthread-core-config.h>
#include <openthread/config.h>

#include <openthread/cli.h>
#include <openthread/diag.h>
#include <openthread/tasklet.h>
#include <openthread/thread.h>
#include "openthread-system.h"
#include "app.h"
#include "app_function.h"

#include "reset_util.h"
#include "em_cmu.h"
#include "em_emu.h"

#include "sl_component_catalog.h"
#ifdef SL_CATALOG_POWER_MANAGER_PRESENT
#include "sl_power_manager.h"
#endif

#if (defined(SL_CATALOG_BTN0_PRESENT) || defined(SL_CATALOG_BTN1_PRESENT))
#include "sl_button.h"
#include "sl_simple_button.h"
#endif

#define JOINER_PSKd "J01NU5"
// Define Commissioner PSKd
#define COMMISSIONER_PSKd "J01NU5"
// Define Commissioner Timeout [s]
#define COMMISSIONER_TIMEOUT 100


void initUdp(void);
extern void otAppCliInit(otInstance *aInstance);

volatile uint32_t      msTickCount;
static uint32_t        identify_time;
static uint32_t        join_time;
static otInstance *    sInstance       = NULL;
static bool            sButtonPressed  = false;
static bool            sStayAwake      = true;
static bool            thread_st       = false;
extern bool            identified;


void SysTick_Handler(void){

  msTickCount++;

}

uint32_t getSysTick_time(void){
  return msTickCount;
}

uint32_t getJoinTime(void){
  return join_time;
}

otInstance *otGetInstance(void)
{
    return sInstance;
}

#if (defined(SL_CATALOG_BTN0_PRESENT) || defined(SL_CATALOG_BTN1_PRESENT))
void sl_button_on_change(const sl_button_t *handle)
{
    if (sl_button_get_state(handle) == SL_SIMPLE_BUTTON_PRESSED)
    {
        sButtonPressed = true;
        otSysEventSignalPending();
    }
}
#endif

void sl_ot_rtos_application_tick(void)
{
    if (sButtonPressed)
    {
        otCliOutputFormat("przycisk\n");
        sButtonPressed = false;
        sStayAwake     = !sStayAwake;
        if (sStayAwake)
        {
#ifdef SL_CATALOG_POWER_MANAGER_PRESENT
            sl_power_manager_add_em_requirement(SL_POWER_MANAGER_EM1);
#endif
        }
        else
        {
#ifdef SL_CATALOG_POWER_MANAGER_PRESENT
            sl_power_manager_remove_em_requirement(SL_POWER_MANAGER_EM1);
#endif
        }
    }
}

/*
 * Provide, if required an "otPlatLog()" function
 */
#if OPENTHREAD_CONFIG_LOG_OUTPUT == OPENTHREAD_CONFIG_LOG_OUTPUT_APP
void otPlatLog(otLogLevel aLogLevel, otLogRegion aLogRegion, const char *aFormat, ...)
{
    OT_UNUSED_VARIABLE(aLogLevel);
    OT_UNUSED_VARIABLE(aLogRegion);
    OT_UNUSED_VARIABLE(aFormat);

    va_list ap;
    va_start(ap, aFormat);
    otCliPlatLogv(aLogLevel, aLogRegion, aFormat, ap);
    va_end(ap);
}
#endif

void sl_ot_create_instance(void)
{
#if OPENTHREAD_CONFIG_MULTIPLE_INSTANCE_ENABLE
    size_t   otInstanceBufferLength = 0;
    uint8_t *otInstanceBuffer       = NULL;

    // Call to query the buffer size
    (void)otInstanceInit(NULL, &otInstanceBufferLength);

    // Call to allocate the buffer
    otInstanceBuffer = (uint8_t *)malloc(otInstanceBufferLength);
    assert(otInstanceBuffer);

    // Initialize OpenThread with the buffer
    sInstance = otInstanceInit(otInstanceBuffer, &otInstanceBufferLength);
#else
    sInstance = otInstanceInitSingle();
#endif
    assert(sInstance);
}

void sl_ot_cli_init(void)
{
    otAppCliInit(sInstance);
}

void JoinerCallback(otError aError, void *aContext){

  if(otJoinerGetState(sInstance) == OT_JOINER_STATE_IDLE && otThreadGetDeviceRole(sInstance) == OT_DEVICE_ROLE_DISABLED ){
      assert(otJoinerStart(sInstance,JOINER_PSKd, NULL, NULL, NULL, NULL, NULL,JoinerCallback,NULL) == OT_ERROR_NONE);
  }

  otCliOutputFormat("Joiner state: %d\n",otJoinerGetState(sInstance));
}
/**************************************************************************//**
 * Application Init.
 *****************************************************************************/

void app_init(void)
{
    OT_SETUP_RESET_JUMP(argv);

    initUdp();
    SysTick_Config(CMU_ClockFreqGet( cmuClock_CORE )/1000);

    assert(otIp6SetEnabled(sInstance, true) == OT_ERROR_NONE);

    assert(otJoinerStart(sInstance,JOINER_PSKd, NULL, NULL, NULL, NULL, NULL,JoinerCallback,NULL) == OT_ERROR_NONE);

}

/**************************************************************************//**
 * Application Process Action.
 *****************************************************************************/
void app_process_action(void)
{
    otTaskletsProcess(sInstance);
    otSysProcessDrivers(sInstance);

    // Start Thread after joined to network
    if(otJoinerGetState(sInstance) == OT_JOINER_STATE_JOINED && thread_st == false){
        assert(otThreadSetEnabled(sInstance, true) == OT_ERROR_NONE);
        thread_st = true;
        join_time = msTickCount;
    }

    // First Identification and start Commissioner
    if ((msTickCount - identify_time)> 20000 && identified == false && get_dev_state() != 'D'){
        identify_request();
        identify_time = msTickCount;
        otCommissionerStart(sInstance, NULL, NULL, NULL);
    }

    // Update identification info if something changed
    if((msTickCount - identify_time)> 20000 && send_dev_info_correct() == false && identified == true){
        identify_request();
        identify_time = msTickCount;
    }

    // Start adding Joiner with the press of a button
    if (sButtonPressed)
        {
            sButtonPressed = false;
            if(otCommissionerGetState(sInstance) == OT_COMMISSIONER_STATE_ACTIVE){
                otCommissionerAddJoiner(sInstance, NULL, COMMISSIONER_PSKd, COMMISSIONER_TIMEOUT);
            }else{
                otCliOutputFormat("Commissioner is not active\n");
            }
        }

}

/**************************************************************************//**
 * Application Exit.
 *****************************************************************************/
void app_exit(void)
{
    otInstanceFinalize(sInstance);
#if OPENTHREAD_CONFIG_MULTIPLE_INSTANCE_ENABLE
    free(otInstanceBuffer);
#endif
    // TO DO : pseudo reset?
}
