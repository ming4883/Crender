#ifndef __CRENDER_GPUFORMAT_H__
#define __CRENDER_GPUFORMAT_H__

typedef enum CrGpuFormat
{
	// uncompressed formats
	CrGpuFormat_UnormR8G8B8A8,
	CrGpuFormat_UnormR8,
	CrGpuFormat_UnormA8,
	CrGpuFormat_UnormR5G5B5A1,
	CrGpuFormat_UnormR5G6B5,
	CrGpuFormat_UnormR4G4B4A4,
	CrGpuFormat_FloatR16,
	CrGpuFormat_FloatR32,
	CrGpuFormat_FloatR16G16,
	CrGpuFormat_FloatR32G32,
	CrGpuFormat_FloatR32G32B32,
	CrGpuFormat_FloatR16G16B16A16,
	CrGpuFormat_FloatR32G32B32A32,

	// compressed formats
	CrGpuFormat_Compressed	= 0x00010000,
	CrGpuFormat_CompPVRTC2	= 0x00010001, // PowerVR texture compression 2-bpp
	CrGpuFormat_CompPVRTC4	= 0x00010002, // PowerVR texture compression 4-bpp
	CrGpuFormat_CompETC		= 0x00010003, // Ericsson texture compression
	CrGpuFormat_CompDXT1	= 0x00010004, // DirectX's DXT1 compression
	CrGpuFormat_CompDXT5	= 0x00010005, // DirectX's DXT5 compression

	// depth formats
	CrGpuFormat_Depth = 0x00020000,
	CrGpuFormat_Depth16 = 0x00020001,
	CrGpuFormat_Depth32 = 0x00020002,
	CrGpuFormat_Depth24Stencil8 = 0x00020003,

} CrGpuFormat;

#endif	// __CRENDER_GPUFORMAT_H__
