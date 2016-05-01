/*
 * PSP Software Development Kit - http://www.pspdev.org
 * -----------------------------------------------------------------------
 * Licensed under the BSD license, see LICENSE in PSPSDK root for details.
 *
 * psppower.h - Prototypes for the scePower library.
 *
 * Copyright (c) 2005 Marcus R. Brown <mrbrown@ocgnet.org>
 * Copyright (c) 2005 James Forshaw <tyranid@gmail.com>
 * Copyright (c) 2005 John Kelley <ps2dev@kelley.ca>
 * Copyright (c) 2005 David Perry <tias_dp@hotmail.com>
 *
 * $Id: psppower.h 2409 2008-07-14 00:30:58Z iwn $
 */
#ifndef __POWER_H__
#define __POWER_H__

#include <pspkerneltypes.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Power callback flags
 */
 /*indicates the power switch it pushed, putting the unit into suspend mode*/
#define PSP_POWER_CB_POWER_SWITCH	0x80000000
/*indicates the hold switch is on*/
#define PSP_POWER_CB_HOLD_SWITCH	0x40000000
/*what is standby mode?*/
#define PSP_POWER_CB_STANDBY		0x00080000
/*indicates the resume process has been completed (only seems to be triggered when another event happens)*/
#define PSP_POWER_CB_RESUME_COMPLETE	0x00040000
/*indicates the unit is resuming from suspend mode*/
#define PSP_POWER_CB_RESUMING		0x00020000
/*indicates the unit is suspending, seems to occur due to inactivity*/
#define PSP_POWER_CB_SUSPENDING		0x00010000
/*indicates the unit is plugged into an AC outlet*/
#define PSP_POWER_CB_AC_POWER		0x00001000
/*indicates the battery charge level is low*/
#define PSP_POWER_CB_BATTERY_LOW	0x00000100
/*indicates there is a battery present in the unit*/
#define PSP_POWER_CB_BATTERY_EXIST	0x00000080
/*unknown*/
#define PSP_POWER_CB_BATTPOWER		0x0000007F

/**
 * Power tick flags
 */
/* All */
#define PSP_POWER_TICK_ALL	0
/* Suspend */
#define PSP_POWER_TICK_SUSPEND	1
/* Display */
#define PSP_POWER_TICK_DISPLAY	6

/**
 * Power Callback Function Definition
 *
 * @param unknown - unknown function, appears to cycle between 1,2 and 3
 * @param powerInfo - combination of PSP_POWER_CB_ flags
 */
typedef void (*powerCallback_t)(int unknown, int powerInfo);

/**
 * Register Power Callback Function
 *
 * @param slot - slot of the callback in the list, 0 to 15, pass -1 to get an auto assignment.
 * @param cbid - callback id from calling sceKernelCreateCallback
 *
 * @returns 0 on success, the slot number if -1 is passed, < 0 on error.
 */
int scePowerRegisterCallback(int slot, SceUID cbid);

/**
 * Unregister Power Callback Function
 *
 * @param slot - slot of the callback
 *
 * @returns 0 on success, < 0 on error.
 */
int scePowerUnregisterCallback(int slot);

/**
 * Refresh battery infos
 * @note:
 * It perfectly works in kernel mode,
 * (infos are updated after 2 seconds)
 * strange results in vsh or user mode
 * |->(update time is different 3-13 seconds)
 *
 * @return always 0
 */
int scePowerBatteryUpdateInfo(void);

/**
 * Check if unit is plugged in
 *
 * @returns 1 if plugged in, 0 if not plugged in, < 0 on error.
 */
int scePowerIsPowerOnline(void);

/**
 * Check if a battery is present
 *
 * @returns 1 if battery present, 0 if battery not present, < 0 on error.
 */
int scePowerIsBatteryExist(void);

/**
 * Check if the battery is charging
 *
 * @returns 1 if battery charging, 0 if battery not charging, < 0 on error.
 */
int scePowerIsBatteryCharging(void);

/**
 * Get the status of the battery charging
 */
int scePowerGetBatteryChargingStatus(void);

/**
 * Check if the battery is low
 *
 * @returns 1 if the battery is low, 0 if the battery is not low, < 0 on error.
 */
int scePowerIsLowBattery(void);

/**
 * Get battery remain capacity as integer
 *
 * @returns battery remain capacity (mAh)
 */
int scePowerGetBatteryRemainCapacity(void);

/**
 * Get battery total capacity as integer
 *
 * @returns battery total capacity (mAh)
 */
int scePowerGetBatteryFullCapacity(void);

/**
 * Unknow function
 *
 * @return always 216 in all of my tests
 */
int scePowerGetLowBatteryCapacity(void);

/**
 * Get battery life as integer percent
 *
 * @returns Battery charge percentage (0-100), < 0 on error.
 */
int scePowerGetBatteryLifePercent(void);

/* 
 * Get battery life as time
 *
 * @returns Battery life in minutes, < 0 on error.
 */
int scePowerGetBatteryLifeTime(void);

/**
 * Get temperature of the battery
 */
int scePowerGetBatteryTemp(void);

/**
 * Get battery volt level
 */
int scePowerGetBatteryVolt(void);

/**
 * Unknow function
 *
 * return always 0 and set to 0 unk1
 */
int scePowerGetBatteryElec(int* unk1);

/**
 * Unknow, maybe return the maximum brightness level available,
 * probably the function pathed by M33 to get always
 * 4 brightness level
 *
 * @return always 4 in all of my tests
 */
int scePowerGetBacklightMaximum(void);

/**
 * Set CPU Frequency
 * @param cpufreq - new CPU frequency, valid values are 1 - 333
 */
int scePowerSetCpuClockFrequency(int cpufreq);

/**
 * Set Bus Frequency
 * @param busfreq - new BUS frequency, valid values are 1 - 167
 */
int scePowerSetBusClockFrequency(int busfreq);

/**
 * Alias for scePowerGetCpuClockFrequencyInt
 * @returns frequency as int
 */
int scePowerGetCpuClockFrequency(void);

/**
 * Get CPU Frequency as Integer
 * @returns frequency as int
 */
int scePowerGetCpuClockFrequencyInt(void);

/**
 * Get CPU Frequency as Float
 * @returns frequency as float
 */
float scePowerGetCpuClockFrequencyFloat(void);

/**
 * Alias for scePowerGetBusClockFrequencyInt
 * @returns frequency as int
 */
int scePowerGetBusClockFrequency(void);

/**
 * Get Bus fequency as Integer
 * @returns frequency as int
 */
int scePowerGetBusClockFrequencyInt(void);

/**
 * Get Bus frequency as Float
 * @returns frequency as float
 */
float scePowerGetBusClockFrequencyFloat(void);

/**
 * Get Pll frequency as Int
 * @returns frequency as int
 */
int scePowerGetPllClockFrequencyInt(void);

/**
 * Get Pll frequency as Float
 * @returns frequency as float
 */
float scePowerGetPllClockFrequencyFloat(void);

/**
 * Set Clock Frequencies
 *
 * @param pllfreq - pll frequency, valid from 19-333
 * @param cpufreq - cpu frequency, valid from 1-333
 * @param busfreq - bus frequency, valid from 1-167
 * 
 * and:
 * 
 * cpufreq <= pllfreq
 * busfreq*2 <= pllfreq
 *
 * @returns < 0 on error, otherwise 0
 */
int scePowerSetClockFrequency(int pllfreq, int cpufreq, int busfreq);

/**
 * Set clocks frequencies
 *
 * @note: Alias for scePowerSetClockFrequency, used in the psx emu
 *
 * @returns < 0 on error, otherwise 0
 */
int scePower_545A7F3C(int pllfreq, int cpufreq, int busfreq);

/**
 * Set clocks frequencies
 *
 * @note: Another alias for scePowerSetClockFrequency
 *
 * @returns < 0 on error, otherwise 0
 */
int scePower_A4E93389(int pllfreq, int cpufreq, int busfreq);

/**
 * Set clocks frequencies
 *
 * @note: Incredible, another alias for scePowerSetClockFrequency
 *
 * @returns < 0 on error, otherwise 0
 */
int scePower_EBD177D6(int pllfreq, int cpufreq, int busfreq);

/**
 * Lock power switch
 *
 * Note: if the power switch is toggled while locked
 * it will fire immediately after being unlocked.
 *
 * @param unknown - pass 0
 *
 * @returns 0 on success, < 0 on error.
 */
int scePowerLock(int unknown);

/**
 * Unlock power switch
 *
 * @param unknown - pass 0
 *
 * @returns 0 on success, < 0 on error.
 */
int scePowerUnlock(int unknown);

/**
 * Generate a power tick, preventing unit from 
 * powering off and turning off display.
 *
 * @param type - Either PSP_POWER_TICK_ALL, PSP_POWER_TICK_SUSPEND or PSP_POWER_TICK_DISPLAY
 *
 * @returns 0 on success, < 0 on error.
 */
int scePowerTick(int type);

/**
 * Get Idle timer
 *
 */
int scePowerGetIdleTimer(void);

/**
 * Enable Idle timer
 *
 * @param unknown - pass 0
 */
int scePowerIdleTimerEnable(int unknown);

/**
 * Disable Idle timer
 *
 * @param unknown - pass 0
 */
int scePowerIdleTimerDisable(int unknown);

/**
 * Get the total ammount of resume times
 *
 * @return the total resume times
 */
int scePowerGetResumeCount(void);

/**
 * Request the PSP to go into standby
 *
 * @return 0 always
 */
int scePowerRequestStandby(void);

/**
 * Request the PSP to go into suspend
 *
 * @returns 0 always
 */
int scePowerRequestSuspend(void);


/**
 * Request for PSP reboot
 *
 * @param unk1  - Unknow, pass 0
 */
void scePowerRequestColdReset(int unk1);

/**
 * Wait for the completion of the previous request
 *
 * @note:
 * Between the request for a op and it's execution
 * there's a "natural" delay time, with this function
 * you can stop the execution and wait for the
 * request execution
 *
 * @returns ???
 */
int scePowerWaitRequestCompletion(void);

/**
 * Check for a request uncompleted
 *
 * @returns 1 if a request is found,
 * otherwise 0
 */
int scePowerIsRequest(void);

/**
 * Unknow, probably delete a previous request,
 * but doesn't work
 *
 * @returns ???
 */
int scePowerCancelRequest(void);

/**
 * Unknow, set something that is unknowed
 *
 * @param unk1  - Unknow --> 0 or 1 or 2 or 3 ( I think )
 */
void scePowerSetPowerSwMode(int unk1);

/**
 * Unknow, get ??? something
 *
 * @returns 0 or 1 or 2 or 3, default to 2
 */
int scePowerGetPowerSwMode(void);

#ifdef __cplusplus
}
#endif

#endif
