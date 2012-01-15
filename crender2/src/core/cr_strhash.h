#ifndef CR_STRHASH_H
#define CR_STRHASH_H

#include "cr_platform.h"

// http://chrissavoie.com/articles-mainmenu-29/15/65-hash2

// the depth of the hashing
#define CR_HASH_DEPTH 64

// randomly generated constants.  The bottom half has to be FFFF or
// else the entire hash loses some strength
static const cr_uint32 CR_HASH_CONSTANTS[CR_HASH_DEPTH+1] =
{
    0xFFA739F8, 0x536438DA, 0x49AE2CF9, 0x156A1AA3, 0x9F43358D, 0xF0E725CD, 0x3D30C908, 0x986059ED, 
    0xC5EAEB32, 0x290D24E1, 0xE3494514, 0xE814C918, 0x50CA24A3, 0xC3D7DEC0, 0x1BF647A9, 0xA29DF87C, 
    0xD8081164, 0x2319889B, 0x2544E9B6, 0xADB0F1EA, 0x97E8D5D0, 0xEABAC79B, 0x412B2DC7, 0x17AEB1CC, 
    0x43907772, 0x937CC8B3, 0x74363679, 0x97AD8E89, 0x0417BA93, 0x625E909C, 0x1D40F02B, 0xCCA7322E, 
    0x0898C513, 0x0BB8C81A, 0xA87F21F1, 0xA35A0A4A, 0x4FE29A7C, 0x90938883, 0xE47419CD, 0xF9F75218, 
    0x47DFC554, 0x4AEDEC81, 0xF52B6820, 0x8EA51AE3, 0xA2ADC1B1, 0x4AE76916, 0xAC7D5E5E, 0x046DF6A6, 
    0x4B27FD13, 0x073732E0, 0x44904502, 0xB7AE28F2, 0xBB827F6D, 0x675E339E, 0x67627CC0, 0xC1A88214, 
    0x7DBD9184, 0x89C453F2, 0xD238FEFF, 0xD8EB4876, 0x407CF47D, 0x93BA1D67, 0x3190F189, 0x43CFD445, 
    0x70627EB0, 
};

// multiplication constants, this allows an abstract use
// of the string length
static const cr_uint32 CR_HASH_MULTS[CR_HASH_DEPTH+1] =
{
     33,  34,  35,  36,  37,  38,  39,  40, 
     41,  42,  43,  44,  45,  46,  47,  48, 
     49,  50,  51,  52,  53,  54,  55,  56, 
     57,  58,  59,  60,  61,  62,  63,  64, 
     65,  66,  67,  68,  69,  70,  71,  72, 
     73,  74,  75,  76,  77,  78,  79,  80, 
     81,  82,  83,  84,  85,  86,  87,  88, 
     89,  90,  91,  92,  93,  94,  95,  96, 
     97, 
};

#define CR_HASH_RECURSE_00(string, value) CR_HASH_FUNCTION((*(string+1) == 0 ? CR_HASH_CONSTANTS[0] : CR_HASH_MULTS[0] * CR_HASH_RECURSE_01(string+1, *(string+1))), value)
#define CR_HASH_RECURSE_01(string, value) CR_HASH_FUNCTION((*(string+1) == 0 ? CR_HASH_CONSTANTS[1] : CR_HASH_MULTS[1] * CR_HASH_RECURSE_02(string+1, *(string+1))), value)
#define CR_HASH_RECURSE_02(string, value) CR_HASH_FUNCTION((*(string+1) == 0 ? CR_HASH_CONSTANTS[2] : CR_HASH_MULTS[2] * CR_HASH_RECURSE_03(string+1, *(string+1))), value)
#define CR_HASH_RECURSE_03(string, value) CR_HASH_FUNCTION((*(string+1) == 0 ? CR_HASH_CONSTANTS[3] : CR_HASH_MULTS[3] * CR_HASH_RECURSE_04(string+1, *(string+1))), value)
#define CR_HASH_RECURSE_04(string, value) CR_HASH_FUNCTION((*(string+1) == 0 ? CR_HASH_CONSTANTS[4] : CR_HASH_MULTS[4] * CR_HASH_RECURSE_05(string+1, *(string+1))), value)
#define CR_HASH_RECURSE_05(string, value) CR_HASH_FUNCTION((*(string+1) == 0 ? CR_HASH_CONSTANTS[5] : CR_HASH_MULTS[5] * CR_HASH_RECURSE_06(string+1, *(string+1))), value)
#define CR_HASH_RECURSE_06(string, value) CR_HASH_FUNCTION((*(string+1) == 0 ? CR_HASH_CONSTANTS[6] : CR_HASH_MULTS[6] * CR_HASH_RECURSE_07(string+1, *(string+1))), value)
#define CR_HASH_RECURSE_07(string, value) CR_HASH_FUNCTION((*(string+1) == 0 ? CR_HASH_CONSTANTS[7] : CR_HASH_MULTS[7] * CR_HASH_RECURSE_08(string+1, *(string+1))), value)
#define CR_HASH_RECURSE_08(string, value) CR_HASH_FUNCTION((*(string+1) == 0 ? CR_HASH_CONSTANTS[8] : CR_HASH_MULTS[8] * CR_HASH_RECURSE_09(string+1, *(string+1))), value)
#define CR_HASH_RECURSE_09(string, value) CR_HASH_FUNCTION((*(string+1) == 0 ? CR_HASH_CONSTANTS[9] : CR_HASH_MULTS[9] * CR_HASH_RECURSE_10(string+1, *(string+1))), value)
#define CR_HASH_RECURSE_10(string, value) CR_HASH_FUNCTION((*(string+1) == 0 ? CR_HASH_CONSTANTS[10] : CR_HASH_MULTS[10] * CR_HASH_RECURSE_11(string+1, *(string+1))), value)
#define CR_HASH_RECURSE_11(string, value) CR_HASH_FUNCTION((*(string+1) == 0 ? CR_HASH_CONSTANTS[11] : CR_HASH_MULTS[11] * CR_HASH_RECURSE_12(string+1, *(string+1))), value)
#define CR_HASH_RECURSE_12(string, value) CR_HASH_FUNCTION((*(string+1) == 0 ? CR_HASH_CONSTANTS[12] : CR_HASH_MULTS[12] * CR_HASH_RECURSE_13(string+1, *(string+1))), value)
#define CR_HASH_RECURSE_13(string, value) CR_HASH_FUNCTION((*(string+1) == 0 ? CR_HASH_CONSTANTS[13] : CR_HASH_MULTS[13] * CR_HASH_RECURSE_14(string+1, *(string+1))), value)
#define CR_HASH_RECURSE_14(string, value) CR_HASH_FUNCTION((*(string+1) == 0 ? CR_HASH_CONSTANTS[14] : CR_HASH_MULTS[14] * CR_HASH_RECURSE_15(string+1, *(string+1))), value)
#define CR_HASH_RECURSE_15(string, value) CR_HASH_FUNCTION((*(string+1) == 0 ? CR_HASH_CONSTANTS[15] : CR_HASH_MULTS[15] * CR_HASH_RECURSE_16(string+1, *(string+1))), value)
#define CR_HASH_RECURSE_16(string, value) CR_HASH_FUNCTION((*(string+1) == 0 ? CR_HASH_CONSTANTS[16] : CR_HASH_MULTS[16] * CR_HASH_RECURSE_17(string+1, *(string+1))), value)
#define CR_HASH_RECURSE_17(string, value) CR_HASH_FUNCTION((*(string+1) == 0 ? CR_HASH_CONSTANTS[17] : CR_HASH_MULTS[17] * CR_HASH_RECURSE_18(string+1, *(string+1))), value)
#define CR_HASH_RECURSE_18(string, value) CR_HASH_FUNCTION((*(string+1) == 0 ? CR_HASH_CONSTANTS[18] : CR_HASH_MULTS[18] * CR_HASH_RECURSE_19(string+1, *(string+1))), value)
#define CR_HASH_RECURSE_19(string, value) CR_HASH_FUNCTION((*(string+1) == 0 ? CR_HASH_CONSTANTS[19] : CR_HASH_MULTS[19] * CR_HASH_RECURSE_20(string+1, *(string+1))), value)
#define CR_HASH_RECURSE_20(string, value) CR_HASH_FUNCTION((*(string+1) == 0 ? CR_HASH_CONSTANTS[20] : CR_HASH_MULTS[20] * CR_HASH_RECURSE_21(string+1, *(string+1))), value)
#define CR_HASH_RECURSE_21(string, value) CR_HASH_FUNCTION((*(string+1) == 0 ? CR_HASH_CONSTANTS[21] : CR_HASH_MULTS[21] * CR_HASH_RECURSE_22(string+1, *(string+1))), value)
#define CR_HASH_RECURSE_22(string, value) CR_HASH_FUNCTION((*(string+1) == 0 ? CR_HASH_CONSTANTS[22] : CR_HASH_MULTS[22] * CR_HASH_RECURSE_23(string+1, *(string+1))), value)
#define CR_HASH_RECURSE_23(string, value) CR_HASH_FUNCTION((*(string+1) == 0 ? CR_HASH_CONSTANTS[23] : CR_HASH_MULTS[23] * CR_HASH_RECURSE_24(string+1, *(string+1))), value)
#define CR_HASH_RECURSE_24(string, value) CR_HASH_FUNCTION((*(string+1) == 0 ? CR_HASH_CONSTANTS[24] : CR_HASH_MULTS[24] * CR_HASH_RECURSE_25(string+1, *(string+1))), value)
#define CR_HASH_RECURSE_25(string, value) CR_HASH_FUNCTION((*(string+1) == 0 ? CR_HASH_CONSTANTS[25] : CR_HASH_MULTS[25] * CR_HASH_RECURSE_26(string+1, *(string+1))), value)
#define CR_HASH_RECURSE_26(string, value) CR_HASH_FUNCTION((*(string+1) == 0 ? CR_HASH_CONSTANTS[26] : CR_HASH_MULTS[26] * CR_HASH_RECURSE_27(string+1, *(string+1))), value)
#define CR_HASH_RECURSE_27(string, value) CR_HASH_FUNCTION((*(string+1) == 0 ? CR_HASH_CONSTANTS[27] : CR_HASH_MULTS[27] * CR_HASH_RECURSE_28(string+1, *(string+1))), value)
#define CR_HASH_RECURSE_28(string, value) CR_HASH_FUNCTION((*(string+1) == 0 ? CR_HASH_CONSTANTS[28] : CR_HASH_MULTS[28] * CR_HASH_RECURSE_29(string+1, *(string+1))), value)
#define CR_HASH_RECURSE_29(string, value) CR_HASH_FUNCTION((*(string+1) == 0 ? CR_HASH_CONSTANTS[29] : CR_HASH_MULTS[29] * CR_HASH_RECURSE_30(string+1, *(string+1))), value)
#define CR_HASH_RECURSE_30(string, value) CR_HASH_FUNCTION((*(string+1) == 0 ? CR_HASH_CONSTANTS[30] : CR_HASH_MULTS[30] * CR_HASH_RECURSE_31(string+1, *(string+1))), value)
#define CR_HASH_RECURSE_31(string, value) CR_HASH_FUNCTION((*(string+1) == 0 ? CR_HASH_CONSTANTS[31] : CR_HASH_MULTS[31] * CR_HASH_RECURSE_32(string+1, *(string+1))), value)
#define CR_HASH_RECURSE_32(string, value) CR_HASH_FUNCTION((*(string+1) == 0 ? CR_HASH_CONSTANTS[32] : CR_HASH_MULTS[32] * CR_HASH_RECURSE_33(string+1, *(string+1))), value)
#define CR_HASH_RECURSE_33(string, value) CR_HASH_FUNCTION((*(string+1) == 0 ? CR_HASH_CONSTANTS[33] : CR_HASH_MULTS[33] * CR_HASH_RECURSE_34(string+1, *(string+1))), value)
#define CR_HASH_RECURSE_34(string, value) CR_HASH_FUNCTION((*(string+1) == 0 ? CR_HASH_CONSTANTS[34] : CR_HASH_MULTS[34] * CR_HASH_RECURSE_35(string+1, *(string+1))), value)
#define CR_HASH_RECURSE_35(string, value) CR_HASH_FUNCTION((*(string+1) == 0 ? CR_HASH_CONSTANTS[35] : CR_HASH_MULTS[35] * CR_HASH_RECURSE_36(string+1, *(string+1))), value)
#define CR_HASH_RECURSE_36(string, value) CR_HASH_FUNCTION((*(string+1) == 0 ? CR_HASH_CONSTANTS[36] : CR_HASH_MULTS[36] * CR_HASH_RECURSE_37(string+1, *(string+1))), value)
#define CR_HASH_RECURSE_37(string, value) CR_HASH_FUNCTION((*(string+1) == 0 ? CR_HASH_CONSTANTS[37] : CR_HASH_MULTS[37] * CR_HASH_RECURSE_38(string+1, *(string+1))), value)
#define CR_HASH_RECURSE_38(string, value) CR_HASH_FUNCTION((*(string+1) == 0 ? CR_HASH_CONSTANTS[38] : CR_HASH_MULTS[38] * CR_HASH_RECURSE_39(string+1, *(string+1))), value)
#define CR_HASH_RECURSE_39(string, value) CR_HASH_FUNCTION((*(string+1) == 0 ? CR_HASH_CONSTANTS[39] : CR_HASH_MULTS[39] * CR_HASH_RECURSE_40(string+1, *(string+1))), value)
#define CR_HASH_RECURSE_40(string, value) CR_HASH_FUNCTION((*(string+1) == 0 ? CR_HASH_CONSTANTS[40] : CR_HASH_MULTS[40] * CR_HASH_RECURSE_41(string+1, *(string+1))), value)
#define CR_HASH_RECURSE_41(string, value) CR_HASH_FUNCTION((*(string+1) == 0 ? CR_HASH_CONSTANTS[41] : CR_HASH_MULTS[41] * CR_HASH_RECURSE_42(string+1, *(string+1))), value)
#define CR_HASH_RECURSE_42(string, value) CR_HASH_FUNCTION((*(string+1) == 0 ? CR_HASH_CONSTANTS[42] : CR_HASH_MULTS[42] * CR_HASH_RECURSE_43(string+1, *(string+1))), value)
#define CR_HASH_RECURSE_43(string, value) CR_HASH_FUNCTION((*(string+1) == 0 ? CR_HASH_CONSTANTS[43] : CR_HASH_MULTS[43] * CR_HASH_RECURSE_44(string+1, *(string+1))), value)
#define CR_HASH_RECURSE_44(string, value) CR_HASH_FUNCTION((*(string+1) == 0 ? CR_HASH_CONSTANTS[44] : CR_HASH_MULTS[44] * CR_HASH_RECURSE_45(string+1, *(string+1))), value)
#define CR_HASH_RECURSE_45(string, value) CR_HASH_FUNCTION((*(string+1) == 0 ? CR_HASH_CONSTANTS[45] : CR_HASH_MULTS[45] * CR_HASH_RECURSE_46(string+1, *(string+1))), value)
#define CR_HASH_RECURSE_46(string, value) CR_HASH_FUNCTION((*(string+1) == 0 ? CR_HASH_CONSTANTS[46] : CR_HASH_MULTS[46] * CR_HASH_RECURSE_47(string+1, *(string+1))), value)
#define CR_HASH_RECURSE_47(string, value) CR_HASH_FUNCTION((*(string+1) == 0 ? CR_HASH_CONSTANTS[47] : CR_HASH_MULTS[47] * CR_HASH_RECURSE_48(string+1, *(string+1))), value)
#define CR_HASH_RECURSE_48(string, value) CR_HASH_FUNCTION((*(string+1) == 0 ? CR_HASH_CONSTANTS[48] : CR_HASH_MULTS[48] * CR_HASH_RECURSE_49(string+1, *(string+1))), value)
#define CR_HASH_RECURSE_49(string, value) CR_HASH_FUNCTION((*(string+1) == 0 ? CR_HASH_CONSTANTS[49] : CR_HASH_MULTS[49] * CR_HASH_RECURSE_50(string+1, *(string+1))), value)
#define CR_HASH_RECURSE_50(string, value) CR_HASH_FUNCTION((*(string+1) == 0 ? CR_HASH_CONSTANTS[50] : CR_HASH_MULTS[50] * CR_HASH_RECURSE_51(string+1, *(string+1))), value)
#define CR_HASH_RECURSE_51(string, value) CR_HASH_FUNCTION((*(string+1) == 0 ? CR_HASH_CONSTANTS[51] : CR_HASH_MULTS[51] * CR_HASH_RECURSE_52(string+1, *(string+1))), value)
#define CR_HASH_RECURSE_52(string, value) CR_HASH_FUNCTION((*(string+1) == 0 ? CR_HASH_CONSTANTS[52] : CR_HASH_MULTS[52] * CR_HASH_RECURSE_53(string+1, *(string+1))), value)
#define CR_HASH_RECURSE_53(string, value) CR_HASH_FUNCTION((*(string+1) == 0 ? CR_HASH_CONSTANTS[53] : CR_HASH_MULTS[53] * CR_HASH_RECURSE_54(string+1, *(string+1))), value)
#define CR_HASH_RECURSE_54(string, value) CR_HASH_FUNCTION((*(string+1) == 0 ? CR_HASH_CONSTANTS[54] : CR_HASH_MULTS[54] * CR_HASH_RECURSE_55(string+1, *(string+1))), value)
#define CR_HASH_RECURSE_55(string, value) CR_HASH_FUNCTION((*(string+1) == 0 ? CR_HASH_CONSTANTS[55] : CR_HASH_MULTS[55] * CR_HASH_RECURSE_56(string+1, *(string+1))), value)
#define CR_HASH_RECURSE_56(string, value) CR_HASH_FUNCTION((*(string+1) == 0 ? CR_HASH_CONSTANTS[56] : CR_HASH_MULTS[56] * CR_HASH_RECURSE_57(string+1, *(string+1))), value)
#define CR_HASH_RECURSE_57(string, value) CR_HASH_FUNCTION((*(string+1) == 0 ? CR_HASH_CONSTANTS[57] : CR_HASH_MULTS[57] * CR_HASH_RECURSE_58(string+1, *(string+1))), value)
#define CR_HASH_RECURSE_58(string, value) CR_HASH_FUNCTION((*(string+1) == 0 ? CR_HASH_CONSTANTS[58] : CR_HASH_MULTS[58] * CR_HASH_RECURSE_59(string+1, *(string+1))), value)
#define CR_HASH_RECURSE_59(string, value) CR_HASH_FUNCTION((*(string+1) == 0 ? CR_HASH_CONSTANTS[59] : CR_HASH_MULTS[59] * CR_HASH_RECURSE_60(string+1, *(string+1))), value)
#define CR_HASH_RECURSE_60(string, value) CR_HASH_FUNCTION((*(string+1) == 0 ? CR_HASH_CONSTANTS[60] : CR_HASH_MULTS[60] * CR_HASH_RECURSE_61(string+1, *(string+1))), value)
#define CR_HASH_RECURSE_61(string, value) CR_HASH_FUNCTION((*(string+1) == 0 ? CR_HASH_CONSTANTS[61] : CR_HASH_MULTS[61] * CR_HASH_RECURSE_62(string+1, *(string+1))), value)
#define CR_HASH_RECURSE_62(string, value) CR_HASH_FUNCTION((*(string+1) == 0 ? CR_HASH_CONSTANTS[62] : CR_HASH_MULTS[62] * CR_HASH_RECURSE_63(string+1, *(string+1))), value)
#define CR_HASH_RECURSE_63(string, value) CR_HASH_FUNCTION((*(string+1) == 0 ? CR_HASH_CONSTANTS[63] : CR_HASH_MULTS[63] * CR_HASH_RECURSE_64(string+1, *(string+1))), value)
#define CR_HASH_RECURSE_64(string, value) CR_HASH_CONSTANTS[64]

// The following is the function used for hashing
// Do NOT use NEXTHASH more than once, it will cause
// N-Squared expansion and make compilation very slow
// If not impossible
#define CR_HASH_FUNCTION(next, value) (value + 33) + ((11 * value * (value << 3)) ^ (next))

// finally the macro used to generate the hash
#define cr_strhash(string) CR_HASH_RECURSE_00(string, *string)

#endif // CR_STRHASH
