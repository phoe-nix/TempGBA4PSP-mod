  .set noreorder

# include "pspimport.s"

  IMPORT_START "sceCtrl_driver", 0x00010000

  IMPORT_FUNC  "sceCtrl_driver", 0x6D74BF08, sceCtrlSetSamplingCycle371
  IMPORT_FUNC  "sceCtrl_driver", 0x6A2774F3, sceCtrlSetSamplingCycle

  IMPORT_FUNC  "sceCtrl_driver", 0x28E71A16, sceCtrlSetSamplingMode371
  IMPORT_FUNC  "sceCtrl_driver", 0x1F4011E6, sceCtrlSetSamplingMode

  IMPORT_FUNC  "sceCtrl_driver", 0x454455AC, sceCtrlReadBufferPositive371
  IMPORT_FUNC  "sceCtrl_driver", 0x1F803938, sceCtrlReadBufferPositive

  IMPORT_FUNC  "sceCtrl_driver", 0xC4AAD55F, sceCtrlPeekBufferPositive371
  IMPORT_FUNC  "sceCtrl_driver", 0x3A622550, sceCtrlReekBufferPositive

