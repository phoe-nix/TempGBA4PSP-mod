/******************************************************************************

  ku_bridge.prx

******************************************************************************/

#include <pspsdk.h>
#include <pspimpose_driver.h>
#include <pspctrl.h>


PSP_MODULE_INFO("ku_bridge", PSP_MODULE_KERNEL, 1, 1);
PSP_MAIN_THREAD_ATTR(0);


/******************************************************************************
  prototypes
******************************************************************************/

int sceCtrlSetSamplingCycle371(int cycle);
int sceCtrlSetSamplingMode371(int mode);

int sceCtrlReadBufferPositive371(SceCtrlData *pad_data, int count);
int sceCtrlPeekBufferPositive371(SceCtrlData *pad_data, int count);


/******************************************************************************
  local variables
******************************************************************************/

static int (*__sceCtrlPeekBufferPositive)(SceCtrlData *pad_data, int count);
static int (*__sceCtrlReadBufferPositive)(SceCtrlData *pad_data, int count);

static int (*__sceCtrlSetSamplingCycle)(int cycle);
static int (*__sceCtrlSetSamplingMode)(int mode);

/******************************************************************************
  functions
******************************************************************************/

int kuImposeGetParam(SceImposeParam param)
{
  int k1 = pspSdkSetK1(0);
  int ret = sceImposeGetParam(param);
  pspSdkSetK1(k1);
  return ret;
}

int kuImposeSetParam(SceImposeParam param, int value)
{
  int k1 = pspSdkSetK1(0);
  int ret = sceImposeSetParam(param, value);
  pspSdkSetK1(k1);
  return ret;
}


int kuCtrlSetSamplingCycle(int cycle)
{
  if (__sceCtrlSetSamplingCycle)
  {
    int k1 = pspSdkSetK1(0);
    int ret = (*__sceCtrlSetSamplingCycle)(cycle);
    pspSdkSetK1(k1);
    return ret;
  }

  return -1;
}

int kuCtrlSetSamplingMode(int mode)
{
  if (__sceCtrlSetSamplingMode)
  {
    int k1 = pspSdkSetK1(0);
    int ret = (*__sceCtrlSetSamplingMode)(mode);
    pspSdkSetK1(k1);
    return ret;
  }

  return -1;
}


int kuCtrlPeekBufferPositive(SceCtrlData *pad_data, int count)
{
  if (__sceCtrlPeekBufferPositive)
  {
    int k1 = pspSdkSetK1(0);
    int ret = (*__sceCtrlPeekBufferPositive)(pad_data, count);
    pspSdkSetK1(k1);
    return ret;
  }

  return -1;
}

int kuCtrlReadBufferPositive(SceCtrlData *pad_data, int count)
{
  if (__sceCtrlReadBufferPositive)
  {
    int k1 = pspSdkSetK1(0);
    int ret = (*__sceCtrlReadBufferPositive)(pad_data, count);
    pspSdkSetK1(k1);
    return ret;
  }

  return -1;
}


void init_ku_bridge(int devkit_version)
{
  if (devkit_version < 0x03070110)
  {
    __sceCtrlSetSamplingCycle = sceCtrlSetSamplingCycle;
    __sceCtrlSetSamplingMode  = sceCtrlSetSamplingMode;

    __sceCtrlPeekBufferPositive = sceCtrlPeekBufferPositive;
    __sceCtrlReadBufferPositive = sceCtrlReadBufferPositive;
  }
  else
  {
    __sceCtrlSetSamplingCycle = sceCtrlSetSamplingCycle371;
    __sceCtrlSetSamplingMode  = sceCtrlSetSamplingMode371;

    __sceCtrlPeekBufferPositive = sceCtrlPeekBufferPositive371;
    __sceCtrlReadBufferPositive = sceCtrlReadBufferPositive371;
  }
}


int module_start(SceSize args, void *argp)
{
  __sceCtrlSetSamplingCycle = NULL;
  __sceCtrlSetSamplingMode  = NULL;

  __sceCtrlPeekBufferPositive = NULL;
  __sceCtrlReadBufferPositive = NULL;

  return 0;
}

int module_stop(void)
{
  return 0;
}


