	.set noreorder

#include "pspstub.s"

	STUB_START "ku_bridge",0x40090000,0x00070005
	STUB_FUNC  0xBBB01C12,init_ku_bridge
	STUB_FUNC  0x13475607,kuImposeGetParam
	STUB_FUNC  0xB9D58B81,kuImposeSetParam
	STUB_FUNC  0x0CCE70FC,kuCtrlSetSamplingCycle
	STUB_FUNC  0x83E3C13E,kuCtrlSetSamplingMode
	STUB_FUNC  0x983E0EA4,kuCtrlPeekBufferPositive
	STUB_FUNC  0x20D2A21C,kuCtrlReadBufferPositive
	STUB_END
