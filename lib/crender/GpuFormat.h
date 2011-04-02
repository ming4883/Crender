#ifndef __CRENDER_GPUFORMAT_H__
#define __CRENDER_GPUFORMAT_H__

typedef enum CrGpuFormat
{
	CrGpuFormat_UnormR8G8B8A8,
	CrGpuFormat_UnormR8,
	CrGpuFormat_FloatR16,
	CrGpuFormat_FloatR32,
	CrGpuFormat_FloatR32G32,
	CrGpuFormat_FloatR32G32B32,
	CrGpuFormat_FloatR16G16B16A16,
	CrGpuFormat_FloatR32G32B32A32,
	CrGpuFormat_Depth = 0x00010000,
	CrGpuFormat_Depth16 = 0x00010001,
	CrGpuFormat_Depth32 = 0x00010002,
	CrGpuFormat_Depth24Stencil8 = 0x00010003,

} CrGpuFormat;

#endif	// __CRENDER_GPUFORMAT_H__