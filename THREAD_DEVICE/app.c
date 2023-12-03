
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

#include <assert.h>
#include <sys/time.h>
#include <stdint.h>
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

// Define JOINER PSKd
#define JOINER_PSKd "J01NU5"


void initUdp(void);
extern void otAppCliInit(otInstance *aInstance);

volatile uint32_t      msTickCount;
static uint32_t        join_time;
static otInstance *    sInstance       = NULL;
static bool            thread_st       = false;



void SysTick_Handler(void)
{
  msTickCount++;
}

uint32_t getSysTick_time(void)
{
  return msTickCount;
}

uint32_t getJoinTime(void)
{
  return join_time;
}

otInstance *otGetInstance(void)
{
    return sInstance;
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

    srand(time(NULL));

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
        initUdp();
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

}
