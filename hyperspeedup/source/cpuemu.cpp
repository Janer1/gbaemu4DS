#include "main.h"


extern "C" void swiHalt(void);

#include <stdio.h>
#include <stdlib.h>
#include <nds/memory.h>//#include <memory.h> ichfly
#include <nds/ndstypes.h>
#include <nds/memory.h>
#include <nds/bios.h>
#include <nds/system.h>
#include <nds/arm9/math.h>
#include <nds/arm9/video.h>
#include <nds/arm9/videoGL.h>
#include <nds/arm9/trig_lut.h>
#include <nds/arm9/sassert.h>
#include <stdarg.h>
#include <string.h>

#include "GBA.h"
#include "GBAinline.h"
#include "Globals.h"
#include "Gfx.h"
#include "EEprom.h"
#include "Flash.h"
#include "Sound.h"
#include "Sram.h"
#include "bios.h"
#include "unzip.h"
#include "Cheats.h"
#include "NLS.h"
#include "elf.h"
#include "Util.h"
#include "Port.h"
#include "agbprint.h"


void CPUWriteMemory(u32 addr, u32 value);
void CPUWriteHalfWord(u32 addr, u16 value);
void CPUWriteByte (u32 addr, u8  value);

u32 CPUReadMemory(u32 addr);
u16 CPUReadHalfWordSigned(u32 addr);
u8  CPUReadByte (u32 addr);


#define DEV_VERSION

#define patchmyregsfromromtoromaddr //parts todo

bool disableMessage = false;


//ichfly i use VisualBoyAdvance instead of normal funktions because i know them











extern u8 cpuBitsSet[256];












#define C_CORE

#define UPDATE_OLD_myregs \
		   if (debugger_last) { \
			   sprintf(oldbuffer,"%08x", armState ? myregs[15].I - 4 : myregs[15].I - 4); \
			   for (xxx=0; xxx<18; xxx++){ \
				   oldmyregs[xxx]=myregs[xxx].I; \
			   } \
		   } 


#ifdef C_CORE
#define NEG(i) ((i) >> 31)
#define POS(i) ((~(i)) >> 31)
#define ADDCARRY(a, b, c) \
  C_FLAG = ((NEG(a) & NEG(b)) |\
            (NEG(a) & POS(c)) |\
            (NEG(b) & POS(c))) ? true : false;
#define ADDOVERFLOW(a, b, c) \
  V_FLAG = ((NEG(a) & NEG(b) & POS(c)) |\
            (POS(a) & POS(b) & NEG(c))) ? true : false;
#define SUBCARRY(a, b, c) \
  C_FLAG = ((NEG(a) & POS(b)) |\
            (NEG(a) & POS(c)) |\
            (POS(b) & POS(c))) ? true : false;
#define SUBOVERFLOW(a, b, c)\
  V_FLAG = ((NEG(a) & POS(b) & POS(c)) |\
            (POS(a) & NEG(b) & NEG(c))) ? true : false;
#define ADD_RD_RS_RN \
   {\
     u32 lhs = myregs[source].I;\
     u32 rhs = value;\
     u32 res = lhs + rhs;\
     myregs[dest].I = res;\
     Z_FLAG = (res == 0) ? true : false;\
     N_FLAG = NEG(res) ? true : false;\
     ADDCARRY(lhs, rhs, res);\
     ADDOVERFLOW(lhs, rhs, res);\
   }
#define ADD_RD_RS_O3 \
   {\
     u32 lhs = myregs[source].I;\
     u32 rhs = value;\
     u32 res = lhs + rhs;\
     myregs[dest].I = res;\
     Z_FLAG = (res == 0) ? true : false;\
     N_FLAG = NEG(res) ? true : false;\
     ADDCARRY(lhs, rhs, res);\
     ADDOVERFLOW(lhs, rhs, res);\
   }
#define ADD_RN_O8(d) \
   {\
     u32 lhs = myregs[(d)].I;\
     u32 rhs = (opcode & 255);\
     u32 res = lhs + rhs;\
     myregs[(d)].I = res;\
     Z_FLAG = (res == 0) ? true : false;\
     N_FLAG = NEG(res) ? true : false;\
     ADDCARRY(lhs, rhs, res);\
     ADDOVERFLOW(lhs, rhs, res);\
   }
#define CMN_RD_RS \
   {\
     u32 lhs = myregs[dest].I;\
     u32 rhs = value;\
     u32 res = lhs + rhs;\
     Z_FLAG = (res == 0) ? true : false;\
     N_FLAG = NEG(res) ? true : false;\
     ADDCARRY(lhs, rhs, res);\
     ADDOVERFLOW(lhs, rhs, res);\
   }
#define ADC_RD_RS \
   {\
     u32 lhs = myregs[dest].I;\
     u32 rhs = value;\
     u32 res = lhs + rhs + (u32)C_FLAG;\
     myregs[dest].I = res;\
     Z_FLAG = (res == 0) ? true : false;\
     N_FLAG = NEG(res) ? true : false;\
     ADDCARRY(lhs, rhs, res);\
     ADDOVERFLOW(lhs, rhs, res);\
   }
#define SUB_RD_RS_RN \
   {\
     u32 lhs = myregs[source].I;\
     u32 rhs = value;\
     u32 res = lhs - rhs;\
     myregs[dest].I = res;\
     Z_FLAG = (res == 0) ? true : false;\
     N_FLAG = NEG(res) ? true : false;\
     SUBCARRY(lhs, rhs, res);\
     SUBOVERFLOW(lhs, rhs, res);\
   }
#define SUB_RD_RS_O3 \
   {\
     u32 lhs = myregs[source].I;\
     u32 rhs = value;\
     u32 res = lhs - rhs;\
     myregs[dest].I = res;\
     Z_FLAG = (res == 0) ? true : false;\
     N_FLAG = NEG(res) ? true : false;\
     SUBCARRY(lhs, rhs, res);\
     SUBOVERFLOW(lhs, rhs, res);\
   }
#define SUB_RN_O8(d) \
   {\
     u32 lhs = myregs[(d)].I;\
     u32 rhs = (opcode & 255);\
     u32 res = lhs - rhs;\
     myregs[(d)].I = res;\
     Z_FLAG = (res == 0) ? true : false;\
     N_FLAG = NEG(res) ? true : false;\
     SUBCARRY(lhs, rhs, res);\
     SUBOVERFLOW(lhs, rhs, res);\
   }
#define CMP_RN_O8(d) \
   {\
     u32 lhs = myregs[(d)].I;\
     u32 rhs = (opcode & 255);\
     u32 res = lhs - rhs;\
     Z_FLAG = (res == 0) ? true : false;\
     N_FLAG = NEG(res) ? true : false;\
     SUBCARRY(lhs, rhs, res);\
     SUBOVERFLOW(lhs, rhs, res);\
   }
#define SBC_RD_RS \
   {\
     u32 lhs = myregs[dest].I;\
     u32 rhs = value;\
     u32 res = lhs - rhs - !((u32)C_FLAG);\
     myregs[dest].I = res;\
     Z_FLAG = (res == 0) ? true : false;\
     N_FLAG = NEG(res) ? true : false;\
     SUBCARRY(lhs, rhs, res);\
     SUBOVERFLOW(lhs, rhs, res);\
   }
#define LSL_RD_RM_I5 \
   {\
     C_FLAG = (myregs[source].I >> (32 - shift)) & 1 ? true : false;\
     value = myregs[source].I << shift;\
   }
#define LSL_RD_RS \
   {\
     C_FLAG = (myregs[dest].I >> (32 - value)) & 1 ? true : false;\
     value = myregs[dest].I << value;\
   }
#define LSR_RD_RM_I5 \
   {\
     C_FLAG = (myregs[source].I >> (shift - 1)) & 1 ? true : false;\
     value = myregs[source].I >> shift;\
   }
#define LSR_RD_RS \
   {\
     C_FLAG = (myregs[dest].I >> (value - 1)) & 1 ? true : false;\
     value = myregs[dest].I >> value;\
   }
#define ASR_RD_RM_I5 \
   {\
     C_FLAG = ((s32)myregs[source].I >> (int)(shift - 1)) & 1 ? true : false;\
     value = (s32)myregs[source].I >> (int)shift;\
   }
#define ASR_RD_RS \
   {\
     C_FLAG = ((s32)myregs[dest].I >> (int)(value - 1)) & 1 ? true : false;\
     value = (s32)myregs[dest].I >> (int)value;\
   }
#define ROR_RD_RS \
   {\
     C_FLAG = (myregs[dest].I >> (value - 1)) & 1 ? true : false;\
     value = ((myregs[dest].I << (32 - value)) |\
              (myregs[dest].I >> value));\
   }
#define NEG_RD_RS \
   {\
     u32 lhs = myregs[source].I;\
     u32 rhs = 0;\
     u32 res = rhs - lhs;\
     myregs[dest].I = res;\
     Z_FLAG = (res == 0) ? true : false;\
     N_FLAG = NEG(res) ? true : false;\
     SUBCARRY(rhs, lhs, res);\
     SUBOVERFLOW(rhs, lhs, res);\
   }
#define CMP_RD_RS \
   {\
     u32 lhs = myregs[dest].I;\
     u32 rhs = value;\
     u32 res = lhs - rhs;\
     Z_FLAG = (res == 0) ? true : false;\
     N_FLAG = NEG(res) ? true : false;\
     SUBCARRY(lhs, rhs, res);\
     SUBOVERFLOW(lhs, rhs, res);\
   }
#else
#ifdef __GNUC__
#ifdef __POWERPC__
            #define ADD_RD_RS_RN \
            {										\
                myregsister int Flags;					\
                myregsister int Result;				\
                asm volatile("addco. %0, %2, %3\n"	\
                            "mcrxr cr1\n"			\
                            "mfcr %1\n"				\
                            : "=r" (Result), 		\
                              "=r" (Flags)			\
                            : "r" (myregs[source].I), 	\
                              "r" (value)			\
                            );						\
                myregs[dest].I = Result;				\
                Z_FLAG = (Flags >> 29) & 1;			\
                N_FLAG = (Flags >> 31) & 1;			\
                C_FLAG = (Flags >> 25) & 1;			\
                V_FLAG = (Flags >> 26) & 1;			\
            }
            #define ADD_RD_RS_O3 ADD_RD_RS_RN
            #define ADD_RN_O8(d) \
            {\
                myregsister int Flags;					\
                myregsister int Result;				\
                asm volatile("addco. %0, %2, %3\n"	\
                            "mcrxr cr1\n"			\
                            "mfcr %1\n"				\
                            : "=r" (Result), 		\
                              "=r" (Flags)			\
                            : "r" (myregs[(d)].I), 	\
                              "r" (opcode & 255)	\
                            );						\
                myregs[(d)].I = Result;				\
                Z_FLAG = (Flags >> 29) & 1;			\
                N_FLAG = (Flags >> 31) & 1;			\
                C_FLAG = (Flags >> 25) & 1;			\
                V_FLAG = (Flags >> 26) & 1;			\
            }
            #define CMN_RD_RS \
            {\
                myregsister int Flags;					\
                myregsister int Result;				\
                asm volatile("addco. %0, %2, %3\n"	\
                            "mcrxr cr1\n"			\
                            "mfcr %1\n"				\
                            : "=r" (Result), 		\
                              "=r" (Flags)			\
                            : "r" (myregs[dest].I), 	\
                              "r" (value)			\
                            );						\
                Z_FLAG = (Flags >> 29) & 1;			\
                N_FLAG = (Flags >> 31) & 1;			\
                C_FLAG = (Flags >> 25) & 1;			\
                V_FLAG = (Flags >> 26) & 1;			\
            }
            #define ADC_RD_RS \
            {\
                myregsister int Flags;					\
                myregsister int Result;				\
                asm volatile("mtspr xer, %4\n"		\
                             "addeo. %0, %2, %3\n"	\
                             "mcrxr cr1\n"			\
                             "mfcr	%1\n"			\
                             : "=r" (Result),		\
                               "=r" (Flags)			\
                             : "r" (myregs[dest].I),	\
                               "r" (value),			\
                               "r" (C_FLAG << 29)	\
                             );						\
                myregs[dest].I = Result;				\
                Z_FLAG = (Flags >> 29) & 1;			\
                N_FLAG = (Flags >> 31) & 1;			\
                C_FLAG = (Flags >> 25) & 1;			\
                V_FLAG = (Flags >> 26) & 1;			\
            }
            #define SUB_RD_RS_RN \
            {\
                myregsister int Flags;					\
                myregsister int Result;				\
                asm volatile("subco. %0, %2, %3\n"	\
                            "mcrxr cr1\n"			\
                            "mfcr %1\n"				\
                            : "=r" (Result), 		\
                              "=r" (Flags)			\
                            : "r" (myregs[source].I), 	\
                              "r" (value)			\
                            );						\
                myregs[dest].I = Result;				\
                Z_FLAG = (Flags >> 29) & 1;			\
                N_FLAG = (Flags >> 31) & 1;			\
                C_FLAG = (Flags >> 25) & 1;			\
                V_FLAG = (Flags >> 26) & 1;			\
            }
            #define SUB_RD_RS_O3 SUB_RD_RS_RN
            #define SUB_RN_O8(d) \
            {\
                myregsister int Flags;					\
                myregsister int Result;				\
                asm volatile("subco. %0, %2, %3\n"	\
                            "mcrxr cr1\n"			\
                            "mfcr %1\n"				\
                            : "=r" (Result), 		\
                              "=r" (Flags)			\
                            : "r" (myregs[(d)].I), 	\
                              "r" (opcode & 255)	\
                            );						\
                myregs[(d)].I = Result;				\
                Z_FLAG = (Flags >> 29) & 1;			\
                N_FLAG = (Flags >> 31) & 1;			\
                C_FLAG = (Flags >> 25) & 1;			\
                V_FLAG = (Flags >> 26) & 1;			\
            }
            #define CMP_RN_O8(d) \
            {\
                myregsister int Flags;					\
                myregsister int Result;				\
                asm volatile("subco. %0, %2, %3\n"	\
                            "mcrxr cr1\n"			\
                            "mfcr %1\n"				\
                            : "=r" (Result), 		\
                              "=r" (Flags)			\
                            : "r" (myregs[(d)].I), 	\
                              "r" (opcode & 255)	\
                            );						\
                Z_FLAG = (Flags >> 29) & 1;			\
                N_FLAG = (Flags >> 31) & 1;			\
                C_FLAG = (Flags >> 25) & 1;			\
                V_FLAG = (Flags >> 26) & 1;			\
            }
            #define SBC_RD_RS \
            {\
                myregsister int Flags;					\
                myregsister int Result;				\
                asm volatile("mtspr xer, %4\n"		\
                             "subfeo. %0, %3, %2\n"	\
                             "mcrxr cr1\n"			\
                             "mfcr	%1\n"			\
                             : "=r" (Result),		\
                               "=r" (Flags)			\
                             : "r" (myregs[dest].I),	\
                               "r" (value),			\
                               "r" (C_FLAG << 29) 	\
                             );						\
                myregs[dest].I = Result;				\
                Z_FLAG = (Flags >> 29) & 1;			\
                N_FLAG = (Flags >> 31) & 1;			\
                C_FLAG = (Flags >> 25) & 1;			\
                V_FLAG = (Flags >> 26) & 1;			\
            }
            #define LSL_RD_RM_I5 \
            {\
                C_FLAG = (myregs[source].I >> (32 - shift)) & 1 ? true : false;\
                value = myregs[source].I << shift;\
            }
            #define LSL_RD_RS \
            {\
                C_FLAG = (myregs[dest].I >> (32 - value)) & 1 ? true : false;\
                value = myregs[dest].I << value;\
            }
            #define LSR_RD_RM_I5 \
            {\
                C_FLAG = (myregs[source].I >> (shift - 1)) & 1 ? true : false;\
                value = myregs[source].I >> shift;\
            }
            #define LSR_RD_RS \
            {\
                C_FLAG = (myregs[dest].I >> (value - 1)) & 1 ? true : false;\
                value = myregs[dest].I >> value;\
            }
            #define ASR_RD_RM_I5 \
            {\
                C_FLAG = ((s32)myregs[source].I >> (int)(shift - 1)) & 1 ? true : false;\
                value = (s32)myregs[source].I >> (int)shift;\
            }
            #define ASR_RD_RS \
            {\
                C_FLAG = ((s32)myregs[dest].I >> (int)(value - 1)) & 1 ? true : false;\
                value = (s32)myregs[dest].I >> (int)value;\
            }
            #define ROR_RD_RS \
            {\
                C_FLAG = (myregs[dest].I >> (value - 1)) & 1 ? true : false;\
                value = ((myregs[dest].I << (32 - value)) |\
                        (myregs[dest].I >> value));\
            }
            #define NEG_RD_RS \
            {\
                myregsister int Flags;					\
                myregsister int Result;				\
                asm volatile("subfco. %0, %2, %3\n"	\
                            "mcrxr cr1\n"			\
                            "mfcr %1\n"				\
                            : "=r" (Result), 		\
                              "=r" (Flags)			\
                            : "r" (myregs[source].I), 	\
                              "r" (0)				\
                            );						\
                myregs[dest].I = Result;				\
                Z_FLAG = (Flags >> 29) & 1;			\
                N_FLAG = (Flags >> 31) & 1;			\
                C_FLAG = (Flags >> 25) & 1;			\
                V_FLAG = (Flags >> 26) & 1;			\
            }
            #define CMP_RD_RS \
            {\
                myregsister int Flags;					\
                myregsister int Result;				\
                asm volatile("subco. %0, %2, %3\n"	\
                            "mcrxr cr1\n"			\
                            "mfcr %1\n"				\
                            : "=r" (Result), 		\
                              "=r" (Flags)			\
                            : "r" (myregs[dest].I), 	\
                              "r" (value)			\
                            );						\
                Z_FLAG = (Flags >> 29) & 1;			\
                N_FLAG = (Flags >> 31) & 1;			\
                C_FLAG = (Flags >> 25) & 1;			\
                V_FLAG = (Flags >> 26) & 1;			\
            }
#else
#define ADD_RD_RS_RN \
     asm ("add %1, %%ebx;"\
          "setsb N_FLAG;"\
          "setzb Z_FLAG;"\
          "setcb C_FLAG;"\
          "setob V_FLAG;"\
          : "=b" (myregs[dest].I)\
          : "r" (value), "b" (myregs[source].I));
#define ADD_RD_RS_O3 \
     asm ("add %1, %%ebx;"\
          "setsb N_FLAG;"\
          "setzb Z_FLAG;"\
          "setcb C_FLAG;"\
          "setob V_FLAG;"\
          : "=b" (myregs[dest].I)\
          : "r" (value), "b" (myregs[source].I));
#define ADD_RN_O8(d) \
     asm ("add %1, %%ebx;"\
          "setsb N_FLAG;"\
          "setzb Z_FLAG;"\
          "setcb C_FLAG;"\
          "setob V_FLAG;"\
          : "=b" (myregs[(d)].I)\
          : "r" (opcode & 255), "b" (myregs[(d)].I));
#define CMN_RD_RS \
     asm ("add %0, %1;"\
          "setsb N_FLAG;"\
          "setzb Z_FLAG;"\
          "setcb C_FLAG;"\
          "setob V_FLAG;"\
          : \
          : "r" (value), "r" (myregs[dest].I):"1");
#define ADC_RD_RS \
     asm ("bt $0, C_FLAG;"\
          "adc %1, %%ebx;"\
          "setsb N_FLAG;"\
          "setzb Z_FLAG;"\
          "setcb C_FLAG;"\
          "setob V_FLAG;"\
          : "=b" (myregs[dest].I)\
          : "r" (value), "b" (myregs[dest].I));
#define SUB_RD_RS_RN \
     asm ("sub %1, %%ebx;"\
          "setsb N_FLAG;"\
          "setzb Z_FLAG;"\
          "setncb C_FLAG;"\
          "setob V_FLAG;"\
          : "=b" (myregs[dest].I)\
          : "r" (value), "b" (myregs[source].I));
#define SUB_RD_RS_O3 \
     asm ("sub %1, %%ebx;"\
          "setsb N_FLAG;"\
          "setzb Z_FLAG;"\
          "setncb C_FLAG;"\
          "setob V_FLAG;"\
          : "=b" (myregs[dest].I)\
          : "r" (value), "b" (myregs[source].I));
#define SUB_RN_O8(d) \
     asm ("sub %1, %%ebx;"\
          "setsb N_FLAG;"\
          "setzb Z_FLAG;"\
          "setncb C_FLAG;"\
          "setob V_FLAG;"\
          : "=b" (myregs[(d)].I)\
          : "r" (opcode & 255), "b" (myregs[(d)].I));
#define CMP_RN_O8(d) \
     asm ("sub %0, %1;"\
          "setsb N_FLAG;"\
          "setzb Z_FLAG;"\
          "setncb C_FLAG;"\
          "setob V_FLAG;"\
          : \
          : "r" (opcode & 255), "r" (myregs[(d)].I) : "1");
#define SBC_RD_RS \
     asm volatile ("bt $0, C_FLAG;"\
                   "cmc;"\
                   "sbb %1, %%ebx;"\
                   "setsb N_FLAG;"\
                   "setzb Z_FLAG;"\
                   "setncb C_FLAG;"\
                   "setob V_FLAG;"\
                   : "=b" (myregs[dest].I)\
                   : "r" (value), "b" (myregs[dest].I) : "cc", "memory");
#define LSL_RD_RM_I5 \
       asm ("shl %%cl, %%eax;"\
            "setcb C_FLAG;"\
            : "=a" (value)\
            : "a" (myregs[source].I), "c" (shift));
#define LSL_RD_RS \
         asm ("shl %%cl, %%eax;"\
              "setcb C_FLAG;"\
              : "=a" (value)\
              : "a" (myregs[dest].I), "c" (value));
#define LSR_RD_RM_I5 \
       asm ("shr %%cl, %%eax;"\
            "setcb C_FLAG;"\
            : "=a" (value)\
            : "a" (myregs[source].I), "c" (shift));
#define LSR_RD_RS \
         asm ("shr %%cl, %%eax;"\
              "setcb C_FLAG;"\
              : "=a" (value)\
              : "a" (myregs[dest].I), "c" (value));
#define ASR_RD_RM_I5 \
     asm ("sar %%cl, %%eax;"\
          "setcb C_FLAG;"\
          : "=a" (value)\
          : "a" (myregs[source].I), "c" (shift));
#define ASR_RD_RS \
         asm ("sar %%cl, %%eax;"\
              "setcb C_FLAG;"\
              : "=a" (value)\
              : "a" (myregs[dest].I), "c" (value));
#define ROR_RD_RS \
         asm ("ror %%cl, %%eax;"\
              "setcb C_FLAG;"\
              : "=a" (value)\
              : "a" (myregs[dest].I), "c" (value));
#define NEG_RD_RS \
     asm ("neg %%ebx;"\
          "setsb N_FLAG;"\
          "setzb Z_FLAG;"\
          "setncb C_FLAG;"\
          "setob V_FLAG;"\
          : "=b" (myregs[dest].I)\
          : "b" (myregs[source].I));
#define CMP_RD_RS \
     asm ("sub %0, %1;"\
          "setsb N_FLAG;"\
          "setzb Z_FLAG;"\
          "setncb C_FLAG;"\
          "setob V_FLAG;"\
          : \
          : "r" (value), "r" (myregs[dest].I):"1");
#endif
#else
#define ADD_RD_RS_RN \
   {\
     __asm mov eax, source\
     __asm mov ebx, dword ptr [OFFSET myregs+4*eax]\
     __asm add ebx, value\
     __asm mov eax, dest\
     __asm mov dword ptr [OFFSET myregs+4*eax], ebx\
     __asm sets byte ptr N_FLAG\
     __asm setz byte ptr Z_FLAG\
     __asm setc byte ptr C_FLAG\
     __asm seto byte ptr V_FLAG\
   }
#define ADD_RD_RS_O3 \
   {\
     __asm mov eax, source\
     __asm mov ebx, dword ptr [OFFSET myregs+4*eax]\
     __asm add ebx, value\
     __asm mov eax, dest\
     __asm mov dword ptr [OFFSET myregs+4*eax], ebx\
     __asm sets byte ptr N_FLAG\
     __asm setz byte ptr Z_FLAG\
     __asm setc byte ptr C_FLAG\
     __asm seto byte ptr V_FLAG\
   }
#define ADD_RN_O8(d) \
   {\
     __asm mov ebx, opcode\
     __asm and ebx, 255\
     __asm add dword ptr [OFFSET myregs+4*(d)], ebx\
     __asm sets byte ptr N_FLAG\
     __asm setz byte ptr Z_FLAG\
     __asm setc byte ptr C_FLAG\
     __asm seto byte ptr V_FLAG\
   }
#define CMN_RD_RS \
     {\
       __asm mov eax, dest\
       __asm mov ebx, dword ptr [OFFSET myregs+4*eax]\
       __asm add ebx, value\
       __asm sets byte ptr N_FLAG\
       __asm setz byte ptr Z_FLAG\
       __asm setc byte ptr C_FLAG\
       __asm seto byte ptr V_FLAG\
     }
#define ADC_RD_RS \
     {\
       __asm mov ebx, dest\
       __asm mov ebx, dword ptr [OFFSET myregs+4*ebx]\
       __asm bt word ptr C_FLAG, 0\
       __asm adc ebx, value\
       __asm mov eax, dest\
       __asm mov dword ptr [OFFSET myregs+4*eax], ebx\
       __asm sets byte ptr N_FLAG\
       __asm setz byte ptr Z_FLAG\
       __asm setc byte ptr C_FLAG\
       __asm seto byte ptr V_FLAG\
     }
#define SUB_RD_RS_RN \
   {\
     __asm mov eax, source\
     __asm mov ebx, dword ptr [OFFSET myregs+4*eax]\
     __asm sub ebx, value\
     __asm mov eax, dest\
     __asm mov dword ptr [OFFSET myregs+4*eax], ebx\
     __asm sets byte ptr N_FLAG\
     __asm setz byte ptr Z_FLAG\
     __asm setnc byte ptr C_FLAG\
     __asm seto byte ptr V_FLAG\
   }
#define SUB_RD_RS_O3 \
   {\
     __asm mov eax, source\
     __asm mov ebx, dword ptr [OFFSET myregs+4*eax]\
     __asm sub ebx, value\
     __asm mov eax, dest\
     __asm mov dword ptr [OFFSET myregs+4*eax], ebx\
     __asm sets byte ptr N_FLAG\
     __asm setz byte ptr Z_FLAG\
     __asm setnc byte ptr C_FLAG\
     __asm seto byte ptr V_FLAG\
   }
#define SUB_RN_O8(d) \
   {\
     __asm mov ebx, opcode\
     __asm and ebx, 255\
     __asm sub dword ptr [OFFSET myregs + 4*(d)], ebx\
     __asm sets byte ptr N_FLAG\
     __asm setz byte ptr Z_FLAG\
     __asm setnc byte ptr C_FLAG\
     __asm seto byte ptr V_FLAG\
   }
#define CMP_RN_O8(d) \
   {\
     __asm mov eax, dword ptr [OFFSET myregs+4*(d)]\
     __asm mov ebx, opcode\
     __asm and ebx, 255\
     __asm sub eax, ebx\
     __asm sets byte ptr N_FLAG\
     __asm setz byte ptr Z_FLAG\
     __asm setnc byte ptr C_FLAG\
     __asm seto byte ptr V_FLAG\
   }
#define SBC_RD_RS \
     {\
       __asm mov ebx, dest\
       __asm mov ebx, dword ptr [OFFSET myregs + 4*ebx]\
       __asm mov eax, value\
       __asm bt word ptr C_FLAG, 0\
       __asm cmc\
       __asm sbb ebx, eax\
       __asm mov eax, dest\
       __asm mov dword ptr [OFFSET myregs + 4*eax], ebx\
       __asm sets byte ptr N_FLAG\
       __asm setz byte ptr Z_FLAG\
       __asm setnc byte ptr C_FLAG\
       __asm seto byte ptr V_FLAG\
     }
#define LSL_RD_RM_I5 \
     {\
       __asm mov eax, source\
       __asm mov eax, dword ptr [OFFSET myregs + 4 * eax]\
       __asm mov cl, byte ptr shift\
       __asm shl eax, cl\
       __asm mov value, eax\
       __asm setc byte ptr C_FLAG\
     }
#define LSL_RD_RS \
         {\
           __asm mov eax, dest\
           __asm mov eax, dword ptr [OFFSET myregs + 4 * eax]\
           __asm mov cl, byte ptr value\
           __asm shl eax, cl\
           __asm mov value, eax\
           __asm setc byte ptr C_FLAG\
         }
#define LSR_RD_RM_I5 \
     {\
       __asm mov eax, source\
       __asm mov eax, dword ptr [OFFSET myregs + 4 * eax]\
       __asm mov cl, byte ptr shift\
       __asm shr eax, cl\
       __asm mov value, eax\
       __asm setc byte ptr C_FLAG\
     }
#define LSR_RD_RS \
         {\
           __asm mov eax, dest\
           __asm mov eax, dword ptr [OFFSET myregs + 4 * eax]\
           __asm mov cl, byte ptr value\
           __asm shr eax, cl\
           __asm mov value, eax\
           __asm setc byte ptr C_FLAG\
         }
#define ASR_RD_RM_I5 \
     {\
       __asm mov eax, source\
       __asm mov eax, dword ptr [OFFSET myregs + 4*eax]\
       __asm mov cl, byte ptr shift\
       __asm sar eax, cl\
       __asm mov value, eax\
       __asm setc byte ptr C_FLAG\
     }
#define ASR_RD_RS \
         {\
           __asm mov eax, dest\
           __asm mov eax, dword ptr [OFFSET myregs + 4*eax]\
           __asm mov cl, byte ptr value\
           __asm sar eax, cl\
           __asm mov value, eax\
           __asm setc byte ptr C_FLAG\
         }
#define ROR_RD_RS \
         {\
           __asm mov eax, dest\
           __asm mov eax, dword ptr [OFFSET myregs + 4*eax]\
           __asm mov cl, byte ptr value\
           __asm ror eax, cl\
           __asm mov value, eax\
           __asm setc byte ptr C_FLAG\
         }
#define NEG_RD_RS \
     {\
       __asm mov ebx, source\
       __asm mov ebx, dword ptr [OFFSET myregs+4*ebx]\
       __asm neg ebx\
       __asm mov eax, dest\
       __asm mov dword ptr [OFFSET myregs+4*eax],ebx\
       __asm sets byte ptr N_FLAG\
       __asm setz byte ptr Z_FLAG\
       __asm setnc byte ptr C_FLAG\
       __asm seto byte ptr V_FLAG\
     }
#define CMP_RD_RS \
     {\
       __asm mov eax, dest\
       __asm mov ebx, dword ptr [OFFSET myregs+4*eax]\
       __asm sub ebx, value\
       __asm sets byte ptr N_FLAG\
       __asm setz byte ptr Z_FLAG\
       __asm setnc byte ptr C_FLAG\
       __asm seto byte ptr V_FLAG\
     }
#endif
#endif


















































































#ifdef BKPT_SUPPORT
#define CONSOLE_OUTPUT(a,b) \
    extern void (*dbgOutput)(char *, u32);\
    if((opcode == 0xe0000000) && (myregs[0].I == 0xC0DED00D)) {\
      dbgOutput((a), (b));\
    }
#else
#define CONSOLE_OUTPUT(a,b)
#endif

#define OP_AND \
      myregs[dest].I = myregs[(opcode>>16)&15].I & value;\
      CONSOLE_OUTPUT(NULL,myregs[2].I);

#define OP_ANDS \
      myregs[dest].I = myregs[(opcode>>16)&15].I & value;\
      \
      N_FLAG = (myregs[dest].I & 0x80000000) ? true : false;\
      Z_FLAG = (myregs[dest].I) ? false : true;\
      C_FLAG = C_OUT;

#define OP_EOR \
      myregs[dest].I = myregs[(opcode>>16)&15].I ^ value;

#define OP_EORS \
      myregs[dest].I = myregs[(opcode>>16)&15].I ^ value;\
      \
      N_FLAG = (myregs[dest].I & 0x80000000) ? true : false;\
      Z_FLAG = (myregs[dest].I) ? false : true;\
      C_FLAG = C_OUT;
//#ifdef C_CORE
#define NEG(i) ((i) >> 31)
#define POS(i) ((~(i)) >> 31)
#define ADDCARRY(a, b, c) \
  C_FLAG = ((NEG(a) & NEG(b)) |\
            (NEG(a) & POS(c)) |\
            (NEG(b) & POS(c))) ? true : false;
#define ADDOVERFLOW(a, b, c) \
  V_FLAG = ((NEG(a) & NEG(b) & POS(c)) |\
            (POS(a) & POS(b) & NEG(c))) ? true : false;
#define SUBCARRY(a, b, c) \
  C_FLAG = ((NEG(a) & POS(b)) |\
            (NEG(a) & POS(c)) |\
            (POS(b) & POS(c))) ? true : false;
#define SUBOVERFLOW(a, b, c)\
  V_FLAG = ((NEG(a) & POS(b) & POS(c)) |\
            (POS(a) & NEG(b) & NEG(c))) ? true : false;
#define OP_SUB \
    {\
      myregs[dest].I = myregs[base].I - value;\
    }
#define OP_SUBS \
   {\
     u32 lhs = myregs[base].I;\
     u32 rhs = value;\
     u32 res = lhs - rhs;\
     myregs[dest].I = res;\
     Z_FLAG = (res == 0) ? true : false;\
     N_FLAG = NEG(res) ? true : false;\
     SUBCARRY(lhs, rhs, res);\
     SUBOVERFLOW(lhs, rhs, res);\
   }
#define OP_RSB \
    {\
      myregs[dest].I = value - myregs[base].I;\
    }
#define OP_RSBS \
   {\
     u32 lhs = myregs[base].I;\
     u32 rhs = value;\
     u32 res = rhs - lhs;\
     myregs[dest].I = res;\
     Z_FLAG = (res == 0) ? true : false;\
     N_FLAG = NEG(res) ? true : false;\
     SUBCARRY(rhs, lhs, res);\
     SUBOVERFLOW(rhs, lhs, res);\
   }
#define OP_ADD \
    {\
      myregs[dest].I = myregs[base].I + value;\
    }
#define OP_ADDS \
   {\
     u32 lhs = myregs[base].I;\
     u32 rhs = value;\
     u32 res = lhs + rhs;\
     myregs[dest].I = res;\
     Z_FLAG = (res == 0) ? true : false;\
     N_FLAG = NEG(res) ? true : false;\
     ADDCARRY(lhs, rhs, res);\
     ADDOVERFLOW(lhs, rhs, res);\
   }
#define OP_ADC \
    {\
      myregs[dest].I = myregs[base].I + value + (u32)C_FLAG;\
    }
#define OP_ADCS \
   {\
     u32 lhs = myregs[base].I;\
     u32 rhs = value;\
     u32 res = lhs + rhs + (u32)C_FLAG;\
     myregs[dest].I = res;\
     Z_FLAG = (res == 0) ? true : false;\
     N_FLAG = NEG(res) ? true : false;\
     ADDCARRY(lhs, rhs, res);\
     ADDOVERFLOW(lhs, rhs, res);\
   }
#define OP_SBC \
    {\
      myregs[dest].I = myregs[base].I - value - !((u32)C_FLAG);\
    }
#define OP_SBCS \
   {\
     u32 lhs = myregs[base].I;\
     u32 rhs = value;\
     u32 res = lhs - rhs - !((u32)C_FLAG);\
     myregs[dest].I = res;\
     Z_FLAG = (res == 0) ? true : false;\
     N_FLAG = NEG(res) ? true : false;\
     SUBCARRY(lhs, rhs, res);\
     SUBOVERFLOW(lhs, rhs, res);\
   }
#define OP_RSC \
    {\
      myregs[dest].I = value - myregs[base].I - !((u32)C_FLAG);\
    }
#define OP_RSCS \
   {\
     u32 lhs = myregs[base].I;\
     u32 rhs = value;\
     u32 res = rhs - lhs - !((u32)C_FLAG);\
     myregs[dest].I = res;\
     Z_FLAG = (res == 0) ? true : false;\
     N_FLAG = NEG(res) ? true : false;\
     SUBCARRY(rhs, lhs, res);\
     SUBOVERFLOW(rhs, lhs, res);\
   }
#define OP_CMP \
   {\
     u32 lhs = myregs[base].I;\
     u32 rhs = value;\
     u32 res = lhs - rhs;\
     Z_FLAG = (res == 0) ? true : false;\
     N_FLAG = NEG(res) ? true : false;\
     SUBCARRY(lhs, rhs, res);\
     SUBOVERFLOW(lhs, rhs, res);\
   }
#define OP_CMN \
   {\
     u32 lhs = myregs[base].I;\
     u32 rhs = value;\
     u32 res = lhs + rhs;\
     Z_FLAG = (res == 0) ? true : false;\
     N_FLAG = NEG(res) ? true : false;\
     ADDCARRY(lhs, rhs, res);\
     ADDOVERFLOW(lhs, rhs, res);\
   }

#define LOGICAL_LSL_myregs \
   {\
     u32 v = myregs[opcode & 0x0f].I;\
     C_OUT = (v >> (32 - shift)) & 1 ? true : false;\
     value = v << shift;\
   }
#define LOGICAL_LSR_myregs \
   {\
     u32 v = myregs[opcode & 0x0f].I;\
     C_OUT = (v >> (shift - 1)) & 1 ? true : false;\
     value = v >> shift;\
   }
#define LOGICAL_ASR_myregs \
   {\
     u32 v = myregs[opcode & 0x0f].I;\
     C_OUT = ((s32)v >> (int)(shift - 1)) & 1 ? true : false;\
     value = (s32)v >> (int)shift;\
   }
#define LOGICAL_ROR_myregs \
   {\
     u32 v = myregs[opcode & 0x0f].I;\
     C_OUT = (v >> (shift - 1)) & 1 ? true : false;\
     value = ((v << (32 - shift)) |\
              (v >> shift));\
   }
#define LOGICAL_RRX_myregs \
   {\
     u32 v = myregs[opcode & 0x0f].I;\
     shift = (int)C_FLAG;\
     C_OUT = (v  & 1) ? true : false;\
     value = ((v >> 1) |\
              (shift << 31));\
   }
#define LOGICAL_ROR_IMM \
   {\
     u32 v = opcode & 0xff;\
     C_OUT = (v >> (shift - 1)) & 1 ? true : false;\
     value = ((v << (32 - shift)) |\
              (v >> shift));\
   }
#define ARITHMETIC_LSL_myregs \
   {\
     u32 v = myregs[opcode & 0x0f].I;\
     value = v << shift;\
   }
#define ARITHMETIC_LSR_myregs \
   {\
     u32 v = myregs[opcode & 0x0f].I;\
     value = v >> shift;\
   }
#define ARITHMETIC_ASR_myregs \
   {\
     u32 v = myregs[opcode & 0x0f].I;\
     value = (s32)v >> (int)shift;\
   }
#define ARITHMETIC_ROR_myregs \
   {\
     u32 v = myregs[opcode & 0x0f].I;\
     value = ((v << (32 - shift)) |\
              (v >> shift));\
   }
#define ARITHMETIC_RRX_myregs \
   {\
     u32 v = myregs[opcode & 0x0f].I;\
     shift = (int)C_FLAG;\
     value = ((v >> 1) |\
              (shift << 31));\
   }
#define ARITHMETIC_ROR_IMM \
   {\
     u32 v = opcode & 0xff;\
     value = ((v << (32 - shift)) |\
              (v >> shift));\
   }
#define ROR_IMM_MSR \
   {\
     u32 v = opcode & 0xff;\
     value = ((v << (32 - shift)) |\
              (v >> shift));\
   }
#define ROR_VALUE \
   {\
     value = ((value << (32 - shift)) |\
              (value >> shift));\
   }
#define RCR_VALUE \
   {\
     shift = (int)C_FLAG;\
     value = ((value >> 1) |\
              (shift << 31));\
   }
//#else
/*#ifdef __GNUC__
        #ifdef __POWERPC__
            #define OP_SUB \
                {\
                myregs[dest].I = myregs[base].I - value;\
                }
            #define OP_SUBS \
            {\
                myregsister int Flags;                             \
                myregsister int Result;                            \
                asm volatile("subco. %0, %2, %3\n"              \
                            "mcrxr cr1\n"                       \
                            "mfcr %1\n"                         \
                            : "=r" (Result),                    \
                              "=r" (Flags)                      \
                            : "r" (myregs[base].I),                \
                              "r" (value)                       \
                            );                                  \
                myregs[dest].I = Result;                           \
                Z_FLAG = (Flags >> 29) & 1;                     \
                N_FLAG = (Flags >> 31) & 1;                     \
                C_FLAG = (Flags >> 25) & 1;                     \
                V_FLAG = (Flags >> 26) & 1;                     \
            }
            #define OP_RSB \
                {\
                myregs[dest].I = value - myregs[base].I;\
                }
            #define OP_RSBS \
            {\
                myregsister int Flags;                             \
                myregsister int Result;                            \
                asm volatile("subfco. %0, %2, %3\n"             \
                            "mcrxr cr1\n"                       \
                            "mfcr %1\n"                         \
                            : "=r" (Result),                    \
                              "=r" (Flags)                      \
                            : "r" (myregs[base].I),                \
                              "r" (value)                       \
                            );                                  \
                myregs[dest].I = Result;                           \
                Z_FLAG = (Flags >> 29) & 1;                     \
                N_FLAG = (Flags >> 31) & 1;                     \
                C_FLAG = (Flags >> 25) & 1;                     \
                V_FLAG = (Flags >> 26) & 1;                     \
            }
            #define OP_ADD \
                {\
                myregs[dest].I = myregs[base].I + value;\
                }

            #define OP_ADDS \
            {\
                myregsister int Flags;                             \
                myregsister int Result;                            \
                asm volatile("addco. %0, %2, %3\n"              \
                            "mcrxr cr1\n"                       \
                            "mfcr %1\n"                         \
                            : "=r" (Result),                    \
                              "=r" (Flags)                      \
                            : "r" (myregs[base].I),                \
                              "r" (value)                       \
                            );                                  \
                myregs[dest].I = Result;                           \
                Z_FLAG = (Flags >> 29) & 1;                     \
                N_FLAG = (Flags >> 31) & 1;                     \
                C_FLAG = (Flags >> 25) & 1;                     \
                V_FLAG = (Flags >> 26) & 1;                     \
            }
            #define OP_ADC \
            {\
                myregs[dest].I = myregs[base].I + value + (u32)C_FLAG;\
            }
            #define OP_ADCS \
            {\
                myregsister int Flags;                             \
                myregsister int Result;                            \
                asm volatile("mtspr xer, %4\n"                  \
                             "addeo. %0, %2, %3\n"              \
                             "mcrxr cr1\n"                      \
                             "mfcr      %1\n"                   \
                             : "=r" (Result),                   \
                               "=r" (Flags)                     \
                             : "r" (myregs[base].I),               \
                               "r" (value),                     \
                               "r" (C_FLAG << 29)               \
                             );                                 \
                myregs[dest].I = Result;                           \
                Z_FLAG = (Flags >> 29) & 1;                     \
                N_FLAG = (Flags >> 31) & 1;                     \
                C_FLAG = (Flags >> 25) & 1;                     \
                V_FLAG = (Flags >> 26) & 1;                     \
            }
            #define OP_SBC \
                {\
                myregs[dest].I = myregs[base].I - value - (C_FLAG^1);\
                }
            #define OP_SBCS \
            {\
                myregsister int Flags;                             \
                myregsister int Result;                            \
                asm volatile("mtspr xer, %4\n"                  \
                             "subfeo. %0, %3, %2\n"             \
                             "mcrxr cr1\n"                      \
                             "mfcr      %1\n"                   \
                             : "=r" (Result),                   \
                               "=r" (Flags)                     \
                             : "r" (myregs[base].I),               \
                               "r" (value),                     \
                               "r" (C_FLAG << 29)               \
                             );                                 \
                myregs[dest].I = Result;                           \
                Z_FLAG = (Flags >> 29) & 1;                     \
                N_FLAG = (Flags >> 31) & 1;                     \
                C_FLAG = (Flags >> 25) & 1;                     \
                V_FLAG = (Flags >> 26) & 1;                     \
            }
            #define OP_RSC \
                {\
                myregs[dest].I = value - myregs[base].I - (C_FLAG^1);\
                }
            #define OP_RSCS \
            {\
                myregsister int Flags;                             \
                myregsister int Result;                            \
                asm volatile("mtspr xer, %4\n"                  \
                             "subfeo. %0, %2, %3\n"             \
                             "mcrxr cr1\n"                      \
                             "mfcr      %1\n"                   \
                             : "=r" (Result),                   \
                               "=r" (Flags)                     \
                             : "r" (myregs[base].I),               \
                               "r" (value),                     \
                               "r" (C_FLAG << 29)               \
                             );                                 \
                myregs[dest].I = Result;                           \
                Z_FLAG = (Flags >> 29) & 1;                     \
                N_FLAG = (Flags >> 31) & 1;                     \
                C_FLAG = (Flags >> 25) & 1;                     \
                V_FLAG = (Flags >> 26) & 1;                     \
            }
            #define OP_CMP \
            {\
                myregsister int Flags;                             \
                myregsister int Result;                            \
                asm volatile("subco. %0, %2, %3\n"              \
                            "mcrxr cr1\n"                       \
                            "mfcr %1\n"                         \
                            : "=r" (Result),                    \
                              "=r" (Flags)                      \
                            : "r" (myregs[base].I),                \
                              "r" (value)                       \
                            );                                  \
                Z_FLAG = (Flags >> 29) & 1;                     \
                N_FLAG = (Flags >> 31) & 1;                     \
                C_FLAG = (Flags >> 25) & 1;                     \
                V_FLAG = (Flags >> 26) & 1;                     \
            }
            #define OP_CMN \
            {\
                myregsister int Flags;                             \
                myregsister int Result;                            \
                asm volatile("addco. %0, %2, %3\n"              \
                            "mcrxr cr1\n"                       \
                            "mfcr %1\n"                         \
                            : "=r" (Result),                    \
                              "=r" (Flags)                      \
                            : "r" (myregs[base].I),                \
                              "r" (value)                       \
                            );                                  \
                Z_FLAG = (Flags >> 29) & 1;                     \
                N_FLAG = (Flags >> 31) & 1;                     \
                C_FLAG = (Flags >> 25) & 1;                     \
                V_FLAG = (Flags >> 26) & 1;                     \
            }
            */
            #define LOGICAL_LSL_myregs \
            {\
                u32 v = myregs[opcode & 0x0f].I;\
                C_OUT = (v >> (32 - shift)) & 1 ? true : false;\
                value = v << shift;\
            }
            #define LOGICAL_LSR_myregs \
            {\
                u32 v = myregs[opcode & 0x0f].I;\
                C_OUT = (v >> (shift - 1)) & 1 ? true : false;\
                value = v >> shift;\
            }
            #define LOGICAL_ASR_myregs \
            {\
                u32 v = myregs[opcode & 0x0f].I;\
                C_OUT = ((s32)v >> (int)(shift - 1)) & 1 ? true : false;\
                value = (s32)v >> (int)shift;\
            }
            #define LOGICAL_ROR_myregs \
            {\
                u32 v = myregs[opcode & 0x0f].I;\
                C_OUT = (v >> (shift - 1)) & 1 ? true : false;\
                value = ((v << (32 - shift)) |\
                        (v >> shift));\
            }
            #define LOGICAL_RRX_myregs \
            {\
                u32 v = myregs[opcode & 0x0f].I;\
                shift = (int)C_FLAG;\
                C_OUT = (v  & 1) ? true : false;\
                value = ((v >> 1) |\
                        (shift << 31));\
            }
            #define LOGICAL_ROR_IMM \
            {\
                u32 v = opcode & 0xff;\
                C_OUT = (v >> (shift - 1)) & 1 ? true : false;\
                value = ((v << (32 - shift)) |\
                        (v >> shift));\
            }
            #define ARITHMETIC_LSL_myregs \
            {\
                u32 v = myregs[opcode & 0x0f].I;\
                value = v << shift;\
            }
            #define ARITHMETIC_LSR_myregs \
            {\
                u32 v = myregs[opcode & 0x0f].I;\
                value = v >> shift;\
            }
            #define ARITHMETIC_ASR_myregs \
            {\
                u32 v = myregs[opcode & 0x0f].I;\
                value = (s32)v >> (int)shift;\
            }
            #define ARITHMETIC_ROR_myregs \
            {\
                u32 v = myregs[opcode & 0x0f].I;\
                value = ((v << (32 - shift)) |\
                        (v >> shift));\
            }
            #define ARITHMETIC_RRX_myregs \
            {\
                u32 v = myregs[opcode & 0x0f].I;\
                shift = (int)C_FLAG;\
                value = ((v >> 1) |\
                        (shift << 31));\
            }
            #define ARITHMETIC_ROR_IMM \
            {\
                u32 v = opcode & 0xff;\
                value = ((v << (32 - shift)) |\
                        (v >> shift));\
            }
            #define ROR_IMM_MSR \
            {\
                u32 v = opcode & 0xff;\
                value = ((v << (32 - shift)) |\
                        (v >> shift));\
            }
            #define ROR_VALUE \
            {\
                value = ((value << (32 - shift)) |\
                        (value >> shift));\
            }
            #define RCR_VALUE \
            {\
                shift = (int)C_FLAG;\
                value = ((value >> 1) |\
                        (shift << 31));\
            }
/*#else
#define OP_SUB \
     asm ("sub %1, %%ebx;"\
                  : "=b" (myregs[dest].I)\
                  : "r" (value), "b" (myregs[base].I));

#define OP_SUBS \
     asm ("sub %1, %%ebx;"\
          "setsb N_FLAG;"\
          "setzb Z_FLAG;"\
          "setncb C_FLAG;"\
          "setob V_FLAG;"\
                  : "=b" (myregs[dest].I)\
                  : "r" (value), "b" (myregs[base].I));

#define OP_RSB \
            asm  ("sub %1, %%ebx;"\
                 : "=b" (myregs[dest].I)\
                 : "r" (myregs[base].I), "b" (value));

#define OP_RSBS \
            asm  ("sub %1, %%ebx;"\
                  "setsb N_FLAG;"\
                  "setzb Z_FLAG;"\
                  "setncb C_FLAG;"\
                  "setob V_FLAG;"\
                 : "=b" (myregs[dest].I)\
                 : "r" (myregs[base].I), "b" (value));

#define OP_ADD \
            asm  ("add %1, %%ebx;"\
                 : "=b" (myregs[dest].I)\
                 : "r" (value), "b" (myregs[base].I));

#define OP_ADDS \
            asm  ("add %1, %%ebx;"\
                  "setsb N_FLAG;"\
                  "setzb Z_FLAG;"\
                  "setcb C_FLAG;"\
                  "setob V_FLAG;"\
                 : "=b" (myregs[dest].I)\
                 : "r" (value), "b" (myregs[base].I));

#define OP_ADC \
            asm  ("bt $0, C_FLAG;"\
                  "adc %1, %%ebx;"\
                 : "=b" (myregs[dest].I)\
                 : "r" (value), "b" (myregs[base].I));

#define OP_ADCS \
            asm  ("bt $0, C_FLAG;"\
                  "adc %1, %%ebx;"\
                  "setsb N_FLAG;"\
                  "setzb Z_FLAG;"\
                  "setcb C_FLAG;"\
                  "setob V_FLAG;"\
                 : "=b" (myregs[dest].I)\
                 : "r" (value), "b" (myregs[base].I));

#define OP_SBC \
            asm  ("bt $0, C_FLAG;"\
                  "cmc;"\
                  "sbb %1, %%ebx;"\
                 : "=b" (myregs[dest].I)\
                 : "r" (value), "b" (myregs[base].I));

#define OP_SBCS \
            asm  ("bt $0, C_FLAG;"\
                  "cmc;"\
                  "sbb %1, %%ebx;"\
                  "setsb N_FLAG;"\
                  "setzb Z_FLAG;"\
                  "setncb C_FLAG;"\
                  "setob V_FLAG;"\
                 : "=b" (myregs[dest].I)\
                 : "r" (value), "b" (myregs[base].I));
#define OP_RSC \
            asm  ("bt $0, C_FLAG;"\
                  "cmc;"\
                  "sbb %1, %%ebx;"\
                 : "=b" (myregs[dest].I)\
                 : "r" (myregs[base].I), "b" (value));

#define OP_RSCS \
            asm  ("bt $0, C_FLAG;"\
                  "cmc;"\
                  "sbb %1, %%ebx;"\
                  "setsb N_FLAG;"\
                  "setzb Z_FLAG;"\
                  "setncb C_FLAG;"\
                  "setob V_FLAG;"\
                 : "=b" (myregs[dest].I)\
                 : "r" (myregs[base].I), "b" (value));
#define OP_CMP \
            asm  ("sub %0, %1;"\
                  "setsb N_FLAG;"\
                  "setzb Z_FLAG;"\
                  "setncb C_FLAG;"\
                  "setob V_FLAG;"\
                 :\
                 : "r" (value), "r" (myregs[base].I));

#define OP_CMN \
            asm  ("add %0, %1;"\
                  "setsb N_FLAG;"\
                  "setzb Z_FLAG;"\
                  "setcb C_FLAG;"\
                  "setob V_FLAG;"\
                 : \
                 : "r" (value), "r" (myregs[base].I));
#define LOGICAL_LSL_myregs \
       asm("shl %%cl, %%eax;"\
           "setcb %%cl;"\
           : "=a" (value), "=c" (C_OUT)\
           : "a" (myregs[opcode & 0x0f].I), "c" (shift));

#define LOGICAL_LSR_myregs \
       asm("shr %%cl, %%eax;"\
           "setcb %%cl;"\
           : "=a" (value), "=c" (C_OUT)\
           : "a" (myregs[opcode & 0x0f].I), "c" (shift));

#define LOGICAL_ASR_myregs \
       asm("sar %%cl, %%eax;"\
           "setcb %%cl;"\
           : "=a" (value), "=c" (C_OUT)\
           : "a" (myregs[opcode & 0x0f].I), "c" (shift));

#define LOGICAL_ROR_myregs \
       asm("ror %%cl, %%eax;"\
           "setcb %%cl;"\
           : "=a" (value), "=c" (C_OUT)\
           : "a" (myregs[opcode & 0x0f].I), "c" (shift));       

#define LOGICAL_RRX_myregs \
       asm("bt $0, C_FLAG;"\
           "rcr $1, %%eax;"\
           "setcb %%cl;"\
           : "=a" (value), "=c" (C_OUT)\
           : "a" (myregs[opcode & 0x0f].I));       

#define LOGICAL_ROR_IMM \
       asm("ror %%cl, %%eax;"\
           "setcb %%cl;"\
           : "=a" (value), "=c" (C_OUT)\
           : "a" (opcode & 0xff), "c" (shift));
#define ARITHMETIC_LSL_myregs \
       asm("\
             shl %%cl, %%eax;"\
           : "=a" (value)\
           : "a" (myregs[opcode & 0x0f].I), "c" (shift));

#define ARITHMETIC_LSR_myregs \
       asm("\
             shr %%cl, %%eax;"\
           : "=a" (value)\
           : "a" (myregs[opcode & 0x0f].I), "c" (shift));

#define ARITHMETIC_ASR_myregs \
       asm("\
             sar %%cl, %%eax;"\
           : "=a" (value)\
           : "a" (myregs[opcode & 0x0f].I), "c" (shift));

#define ARITHMETIC_ROR_myregs \
       asm("\
             ror %%cl, %%eax;"\
           : "=a" (value)\
           : "a" (myregs[opcode & 0x0f].I), "c" (shift));       

#define ARITHMETIC_RRX_myregs \
       asm("\
             bt $0, C_FLAG;\
             rcr $1, %%eax;"\
           : "=a" (value)\
           : "a" (myregs[opcode & 0x0f].I));       

#define ARITHMETIC_ROR_IMM \
       asm("\
             ror %%cl, %%eax;"\
           : "=a" (value)\
           : "a" (opcode & 0xff), "c" (shift));
#define ROR_IMM_MSR \
      asm ("ror %%cl, %%eax;"\
           : "=a" (value)\
           : "a" (opcode & 0xFF), "c" (shift));
#define ROR_VALUE \
      asm("ror %%cl, %0"\
          : "=r" (value)\
          : "r" (value), "c" (shift));
#define RCR_VALUE \
      asm("bt $0, C_FLAG;"\
          "rcr $1, %0"\
          : "=r" (value)\
          : "r" (value));
#endif
#else
#define OP_SUB \
      {\
        __asm mov ebx, base\
        __asm mov ebx, dword ptr [OFFSET myregs+4*ebx]\
        __asm sub ebx, value\
        __asm mov eax, dest\
        __asm mov dword ptr [OFFSET myregs+4*eax], ebx\
      }

#define OP_SUBS \
      {\
        __asm mov ebx, base\
        __asm mov ebx, dword ptr [OFFSET myregs+4*ebx]\
        __asm sub ebx, value\
        __asm mov eax, dest\
        __asm mov dword ptr [OFFSET myregs+4*eax], ebx\
        __asm sets byte ptr N_FLAG\
        __asm setz byte ptr Z_FLAG\
        __asm setnc byte ptr C_FLAG\
        __asm seto byte ptr V_FLAG\
      }

#define OP_RSB \
      {\
        __asm mov ebx, base\
        __asm mov ebx, dword ptr [OFFSET myregs+4*ebx]\
        __asm mov eax, value\
        __asm sub eax, ebx\
        __asm mov ebx, dest\
        __asm mov dword ptr [OFFSET myregs+4*ebx], eax\
      }

#define OP_RSBS \
      {\
        __asm mov ebx, base\
        __asm mov ebx, dword ptr [OFFSET myregs+4*ebx]\
        __asm mov eax, value\
        __asm sub eax, ebx\
        __asm mov ebx, dest\
        __asm mov dword ptr [OFFSET myregs+4*ebx], eax\
        __asm sets byte ptr N_FLAG\
        __asm setz byte ptr Z_FLAG\
        __asm setnc byte ptr C_FLAG\
        __asm seto byte ptr V_FLAG\
      }

#define OP_ADD \
      {\
        __asm mov ebx, base\
        __asm mov ebx, dword ptr [OFFSET myregs+4*ebx]\
        __asm add ebx, value\
        __asm mov eax, dest\
        __asm mov dword ptr [OFFSET myregs+4*eax], ebx\
      }

#define OP_ADDS \
      {\
        __asm mov ebx, base\
        __asm mov ebx, dword ptr [OFFSET myregs+4*ebx]\
        __asm add ebx, value\
        __asm mov eax, dest\
        __asm mov dword ptr [OFFSET myregs+4*eax], ebx\
        __asm sets byte ptr N_FLAG\
        __asm setz byte ptr Z_FLAG\
        __asm setc byte ptr C_FLAG\
        __asm seto byte ptr V_FLAG\
      }

#define OP_ADC \
      {\
        __asm mov ebx, base\
        __asm mov ebx, dword ptr [OFFSET myregs+4*ebx]\
        __asm bt word ptr C_FLAG, 0\
        __asm adc ebx, value\
        __asm mov eax, dest\
        __asm mov dword ptr [OFFSET myregs+4*eax], ebx\
      }

#define OP_ADCS \
      {\
        __asm mov ebx, base\
        __asm mov ebx, dword ptr [OFFSET myregs+4*ebx]\
        __asm bt word ptr C_FLAG, 0\
        __asm adc ebx, value\
        __asm mov eax, dest\
        __asm mov dword ptr [OFFSET myregs+4*eax], ebx\
        __asm sets byte ptr N_FLAG\
        __asm setz byte ptr Z_FLAG\
        __asm setc byte ptr C_FLAG\
        __asm seto byte ptr V_FLAG\
      }

#define OP_SBC \
      {\
        __asm mov ebx, base\
        __asm mov ebx, dword ptr [OFFSET myregs + 4*ebx]\
        __asm mov eax, value\
        __asm bt word ptr C_FLAG, 0\
        __asm cmc\
        __asm sbb ebx, eax\
        __asm mov eax, dest\
        __asm mov dword ptr [OFFSET myregs + 4*eax], ebx\
      }

#define OP_SBCS \
      {\
        __asm mov ebx, base\
        __asm mov ebx, dword ptr [OFFSET myregs + 4*ebx]\
        __asm mov eax, value\
        __asm bt word ptr C_FLAG, 0\
        __asm cmc\
        __asm sbb ebx, eax\
        __asm mov eax, dest\
        __asm mov dword ptr [OFFSET myregs + 4*eax], ebx\
        __asm sets byte ptr N_FLAG\
        __asm setz byte ptr Z_FLAG\
        __asm setnc byte ptr C_FLAG\
        __asm seto byte ptr V_FLAG\
      }
#define OP_RSC \
      {\
        __asm mov ebx, value\
        __asm mov eax, base\
        __asm mov eax, dword ptr[OFFSET myregs + 4*eax]\
        __asm bt word ptr C_FLAG, 0\
        __asm cmc\
        __asm sbb ebx, eax\
        __asm mov eax, dest\
        __asm mov dword ptr [OFFSET myregs + 4*eax], ebx\
      }

#define OP_RSCS \
      {\
        __asm mov ebx, value\
        __asm mov eax, base\
        __asm mov eax, dword ptr[OFFSET myregs + 4*eax]\
        __asm bt word ptr C_FLAG, 0\
        __asm cmc\
        __asm sbb ebx, eax\
        __asm mov eax, dest\
        __asm mov dword ptr [OFFSET myregs + 4*eax], ebx\
        __asm sets byte ptr N_FLAG\
        __asm setz byte ptr Z_FLAG\
        __asm setnc byte ptr C_FLAG\
        __asm seto byte ptr V_FLAG\
      }
#define OP_CMP \
     {\
       __asm mov eax, base\
       __asm mov ebx, dword ptr [OFFSET myregs+4*eax]\
       __asm sub ebx, value\
       __asm sets byte ptr N_FLAG\
       __asm setz byte ptr Z_FLAG\
       __asm setnc byte ptr C_FLAG\
       __asm seto byte ptr V_FLAG\
     }

#define OP_CMN \
     {\
       __asm mov eax, base\
       __asm mov ebx, dword ptr [OFFSET myregs+4*eax]\
       __asm add ebx, value\
       __asm sets byte ptr N_FLAG\
       __asm setz byte ptr Z_FLAG\
       __asm setc byte ptr C_FLAG\
       __asm seto byte ptr V_FLAG\
     }
#define LOGICAL_LSL_myregs \
        __asm mov eax, opcode\
        __asm and eax, 0x0f\
        __asm mov eax, dword ptr [OFFSET myregs + 4 * eax]\
        __asm mov cl, byte ptr shift\
        __asm shl eax, cl\
        __asm mov value, eax\
        __asm setc byte ptr C_OUT

#define LOGICAL_LSR_myregs \
        __asm mov eax, opcode\
        __asm and eax, 0x0f\
        __asm mov eax, dword ptr [OFFSET myregs + 4 * eax]\
        __asm mov cl, byte ptr shift\
        __asm shr eax, cl\
        __asm mov value, eax\
        __asm setc byte ptr C_OUT

#define LOGICAL_ASR_myregs \
        __asm mov eax, opcode\
        __asm and eax, 0x0f\
        __asm mov eax, dword ptr [OFFSET myregs + 4 * eax]\
        __asm mov cl, byte ptr shift\
        __asm sar eax, cl\
        __asm mov value, eax\
        __asm setc byte ptr C_OUT

#define LOGICAL_ROR_myregs \
        __asm mov eax, opcode\
        __asm and eax, 0x0F\
        __asm mov eax, dword ptr [OFFSET myregs + 4*eax]\
        __asm mov cl, byte ptr shift\
        __asm ror eax, cl\
        __asm mov value, eax\
        __asm setc byte ptr C_OUT

#define LOGICAL_RRX_myregs \
        __asm mov eax, opcode\
        __asm and eax, 0x0F\
        __asm mov eax, dword ptr [OFFSET myregs + 4*eax]\
        __asm bt word ptr C_OUT, 0\
        __asm rcr eax, 1\
        __asm mov value, eax\
        __asm setc byte ptr C_OUT

#define LOGICAL_ROR_IMM \
        __asm mov eax, opcode\
        __asm and eax, 0xff\
        __asm mov cl, byte ptr shift\
        __asm ror eax, cl\
        __asm mov value, eax\
        __asm setc byte ptr C_OUT
#define ARITHMETIC_LSL_myregs \
        __asm mov eax, opcode\
        __asm and eax, 0x0f\
        __asm mov eax, dword ptr [OFFSET myregs + 4 * eax]\
        __asm mov cl, byte ptr shift\
        __asm shl eax, cl\
        __asm mov value, eax

#define ARITHMETIC_LSR_myregs \
        __asm mov eax, opcode\
        __asm and eax, 0x0f\
        __asm mov eax, dword ptr [OFFSET myregs + 4 * eax]\
        __asm mov cl, byte ptr shift\
        __asm shr eax, cl\
        __asm mov value, eax

#define ARITHMETIC_ASR_myregs \
        __asm mov eax, opcode\
        __asm and eax, 0x0f\
        __asm mov eax, dword ptr [OFFSET myregs + 4 * eax]\
        __asm mov cl, byte ptr shift\
        __asm sar eax, cl\
        __asm mov value, eax

#define ARITHMETIC_ROR_myregs \
        __asm mov eax, opcode\
        __asm and eax, 0x0F\
        __asm mov eax, dword ptr [OFFSET myregs + 4*eax]\
        __asm mov cl, byte ptr shift\
        __asm ror eax, cl\
        __asm mov value, eax

#define ARITHMETIC_RRX_myregs \
        __asm mov eax, opcode\
        __asm and eax, 0x0F\
        __asm mov eax, dword ptr [OFFSET myregs + 4*eax]\
        __asm bt word ptr C_FLAG, 0\
        __asm rcr eax, 1\
        __asm mov value, eax

#define ARITHMETIC_ROR_IMM \
        __asm mov eax, opcode\
        __asm and eax, 0xff\
        __asm mov cl, byte ptr shift\
        __asm ror eax, cl\
        __asm mov value, eax
#define ROR_IMM_MSR \
      {\
        __asm mov eax, opcode\
        __asm and eax, 0xff\
        __asm mov cl, byte ptr shift\
        __asm ror eax, CL\
        __asm mov value, eax\
      }
#define ROR_VALUE \
      {\
        __asm mov cl, byte ptr shift\
        __asm ror dword ptr value, cl\
      }
#define RCR_VALUE \
      {\
        __asm mov cl, byte ptr shift\
        __asm bt word ptr C_FLAG, 0\
        __asm rcr dword ptr value, 1\
      }
#endif
#endif*/

#define OP_TST \
      u32 res = myregs[base].I & value;\
      N_FLAG = (res & 0x80000000) ? true : false;\
      Z_FLAG = (res) ? false : true;\
      C_FLAG = C_OUT;

#define OP_TEQ \
      u32 res = myregs[base].I ^ value;\
      N_FLAG = (res & 0x80000000) ? true : false;\
      Z_FLAG = (res) ? false : true;\
      C_FLAG = C_OUT;

#define OP_ORR \
    myregs[dest].I = myregs[base].I | value;

#define OP_ORRS \
    myregs[dest].I = myregs[base].I | value;\
    N_FLAG = (myregs[dest].I & 0x80000000) ? true : false;\
    Z_FLAG = (myregs[dest].I) ? false : true;\
    C_FLAG = C_OUT;

#define OP_MOV \
    myregs[dest].I = value;

#define OP_MOVS \
    myregs[dest].I = value;\
    N_FLAG = (myregs[dest].I & 0x80000000) ? true : false;\
    Z_FLAG = (myregs[dest].I) ? false : true;\
    C_FLAG = C_OUT;

#define OP_BIC \
    myregs[dest].I = myregs[base].I & (~value);

#define OP_BICS \
    myregs[dest].I = myregs[base].I & (~value);\
    N_FLAG = (myregs[dest].I & 0x80000000) ? true : false;\
    Z_FLAG = (myregs[dest].I) ? false : true;\
    C_FLAG = C_OUT;

#define OP_MVN \
    myregs[dest].I = ~value;

#define OP_MVNS \
    myregs[dest].I = ~value; \
    N_FLAG = (myregs[dest].I & 0x80000000) ? true : false;\
    Z_FLAG = (myregs[dest].I) ? false : true;\
    C_FLAG = C_OUT;

#define CASE_16(BASE) \
  case BASE:\
  case BASE+1:\
  case BASE+2:\
  case BASE+3:\
  case BASE+4:\
  case BASE+5:\
  case BASE+6:\
  case BASE+7:\
  case BASE+8:\
  case BASE+9:\
  case BASE+10:\
  case BASE+11:\
  case BASE+12:\
  case BASE+13:\
  case BASE+14:\
  case BASE+15:

#define CASE_256(BASE) \
  CASE_16(BASE)\
  CASE_16(BASE+0x10)\
  CASE_16(BASE+0x20)\
  CASE_16(BASE+0x30)\
  CASE_16(BASE+0x40)\
  CASE_16(BASE+0x50)\
  CASE_16(BASE+0x60)\
  CASE_16(BASE+0x70)\
  CASE_16(BASE+0x80)\
  CASE_16(BASE+0x90)\
  CASE_16(BASE+0xa0)\
  CASE_16(BASE+0xb0)\
  CASE_16(BASE+0xc0)\
  CASE_16(BASE+0xd0)\
  CASE_16(BASE+0xe0)\
  CASE_16(BASE+0xf0)

#define LOGICAL_DATA_OPCODE(OPCODE, OPCODE2, BASE) \
  case BASE: \
  case BASE+8:\
    {\
      /* OP Rd,Rb,Rm LSL # */ \
      int base = (opcode >> 16) & 0x0F;\
      int shift = (opcode >> 7) & 0x1F;\
      int dest = (opcode>>12) & 15;\
      bool C_OUT = C_FLAG;\
      u32 value;\
      \ 
      if(shift) {\
        LOGICAL_LSL_myregs\
      } else {\
        value = myregs[opcode & 0x0F].I;\
      }\
      if(dest == 15) {\ 
        OPCODE2\
/*todo*/\
       if(armState) {\
          myregs[15].I &= 0xFFFFFFFC;\
          armNextPC = myregs[15].I;\
          myregs[15].I += 4;\ 
        } else {\
          myregs[15].I &= 0xFFFFFFFE;\
          armNextPC = myregs[15].I;\
          myregs[15].I += 2;\ 
        }\
      }\
 else {\
        OPCODE \
      }\
    }\
    break;\
  case BASE+2:\
  case BASE+10:\
    {\
       /* OP Rd,Rb,Rm LSR # */ \
      int base = (opcode >> 16) & 0x0F;\
      int shift = (opcode >> 7) & 0x1F;\
      int dest = (opcode>>12) & 15;\
      bool C_OUT = C_FLAG;\
      u32 value;\ 
      if(shift) {\
        LOGICAL_LSR_myregs\
      } else {\
        value = 0;\
        C_OUT = (myregs[opcode & 0x0F].I & 0x80000000) ? true : false;\
      }\
      \
      if(dest == 15) {\ 
        OPCODE2\
/*todo*/\
        if(armState) {\
          myregs[15].I &= 0xFFFFFFFC;\
          armNextPC = myregs[15].I;\
          myregs[15].I += 4;\ 
        } else {\
          myregs[15].I &= 0xFFFFFFFE;\
          armNextPC = myregs[15].I;\
          myregs[15].I += 2;\ 
        }\
      } else {\
        OPCODE \
      }\
    }\
    break;\
  case BASE+4:\
  case BASE+12:\
    {\
       /* OP Rd,Rb,Rm ASR # */\
      int base = (opcode >> 16) & 0x0F;\
      int shift = (opcode >> 7) & 0x1F;\
      int dest = (opcode>>12) & 15;\
      bool C_OUT = C_FLAG;\
      u32 value;\ 
      if(shift) {\
        LOGICAL_ASR_myregs\
      } else {\
        if(myregs[opcode & 0x0F].I & 0x80000000){\
          value = 0xFFFFFFFF;\
          C_OUT = true;\
        } else {\
          value = 0;\
          C_OUT = false;\
        }                   \
      }\
      \
      if(dest == 15) {\ 
        OPCODE2\
/*todo*/\
        if(armState) {\
          myregs[15].I &= 0xFFFFFFFC;\
          armNextPC = myregs[15].I;\
          myregs[15].I += 4;\ 
        } else {\
          myregs[15].I &= 0xFFFFFFFE;\
          armNextPC = myregs[15].I;\
          myregs[15].I += 2;\ 
        }\
      } else {\
        OPCODE \
      }\
    }\
    break;\
  case BASE+6:\
  case BASE+14:\
    {\
       /* OP Rd,Rb,Rm ROR # */\
      int base = (opcode >> 16) & 0x0F;\
      int shift = (opcode >> 7) & 0x1F;\
      int dest = (opcode>>12) & 15;\
      bool C_OUT = C_FLAG;\
      u32 value;\ 
      if(shift) {\
        LOGICAL_ROR_myregs\
      } else {\
        LOGICAL_RRX_myregs\
      }\
      if(dest == 15) {\ 
        OPCODE2\
/*todo*/\
        if(armState) {\
          myregs[15].I &= 0xFFFFFFFC;\
          armNextPC = myregs[15].I;\
          myregs[15].I += 4;\ 
        } else {\
          myregs[15].I &= 0xFFFFFFFE;\
          armNextPC = myregs[15].I;\
          myregs[15].I += 2;\ 
        }\
      } else {\
        OPCODE \
      }\
    }\
    break;\
  case BASE+1:\
    {\
       /* OP Rd,Rb,Rm LSL Rs */\
      int base = (opcode >> 16) & 0x0F;\
      int shift = myregs[(opcode >> 8)&15].B.B0;\
      int dest = (opcode>>12) & 15;\
      bool C_OUT = C_FLAG;\
      u32 value;\ 
      if(shift) {\
        if(shift == 32) {\
          value = 0;\
          C_OUT = (myregs[opcode & 0x0F].I & 1 ? true : false);\
        } else if(shift < 32) {\
           LOGICAL_LSL_myregs\
        } else {\
          value = 0;\
          C_OUT = false;\
        }\
      } else {\
        value = myregs[opcode & 0x0F].I;\
      }\
      if(dest == 15) {\ 
        OPCODE2\
/*todo*/\
        if(armState) {\
          myregs[15].I &= 0xFFFFFFFC;\
          armNextPC = myregs[15].I;\
          myregs[15].I += 4;\ 
        } else {\
          myregs[15].I &= 0xFFFFFFFE;\
          armNextPC = myregs[15].I;\
          myregs[15].I += 2;\ 
        }\
      } else {\
        OPCODE \
      }\
    }\
    break;\
  case BASE+3:\
    {\
       /* OP Rd,Rb,Rm LSR Rs */ \
      int base = (opcode >> 16) & 0x0F;\
      int shift = myregs[(opcode >> 8)&15].B.B0;\
      int dest = (opcode>>12) & 15;\
      bool C_OUT = C_FLAG;\
      u32 value;\ 
      if(shift) {\
        if(shift == 32) {\
          value = 0;\
          C_OUT = (myregs[opcode & 0x0F].I & 0x80000000 ? true : false);\
        } else if(shift < 32) {\
            LOGICAL_LSR_myregs\
        } else {\
          value = 0;\
          C_OUT = false;\
        }\
      } else {\
        value = myregs[opcode & 0x0F].I;\
      }\
      if(dest == 15) {\ 
        OPCODE2\
/*todo*/\
        if(armState) {\
          myregs[15].I &= 0xFFFFFFFC;\
          armNextPC = myregs[15].I;\
          myregs[15].I += 4;\ 
        } else {\
          myregs[15].I &= 0xFFFFFFFE;\
          armNextPC = myregs[15].I;\
          myregs[15].I += 2;\ 
        }\
      } else {\
        OPCODE \
      }\
    }\
    break;\
  case BASE+5:\
    {\
       /* OP Rd,Rb,Rm ASR Rs */ \
      int base = (opcode >> 16) & 0x0F;\
      int shift = myregs[(opcode >> 8)&15].B.B0;\
      int dest = (opcode>>12) & 15;\
      bool C_OUT = C_FLAG;\
      u32 value;\ 
      if(shift < 32) {\
        if(shift) {\
          LOGICAL_ASR_myregs\
        } else {\
          value = myregs[opcode & 0x0F].I;\
        }\
      } else {\
        if(myregs[opcode & 0x0F].I & 0x80000000){\
          value = 0xFFFFFFFF;\
          C_OUT = true;\
        } else {\
          value = 0;\
          C_OUT = false;\
        }\
      }\
      if(dest == 15) {\ 
        OPCODE2\
/*todo*/\
        if(armState) {\
          myregs[15].I &= 0xFFFFFFFC;\
          armNextPC = myregs[15].I;\
          myregs[15].I += 4;\ 
        } else {\
          myregs[15].I &= 0xFFFFFFFE;\
          armNextPC = myregs[15].I;\
          myregs[15].I += 2;\ 
        }\
      } else {\
        OPCODE \
      }\
    }\
    break;\
  case BASE+7:\
    {\
       /* OP Rd,Rb,Rm ROR Rs */\
      int base = (opcode >> 16) & 0x0F;\
      int shift = myregs[(opcode >> 8)&15].B.B0;\
      int dest = (opcode>>12) & 15;\
      bool C_OUT = C_FLAG;\
      u32 value;\ 
      if(shift) {\
        shift &= 0x1f;\
        if(shift) {\
          LOGICAL_ROR_myregs\
        } else {\
          value = myregs[opcode & 0x0F].I;\
          C_OUT = (value & 0x80000000 ? true : false);\
        }\
      } else {\
        value = myregs[opcode & 0x0F].I;\
        C_OUT = (value & 0x80000000 ? true : false);\
      }\
      if(dest == 15) {\ 
        OPCODE2\
/*todo*/\
        if(armState) {\
          myregs[15].I &= 0xFFFFFFFC;\
          armNextPC = myregs[15].I;\
          myregs[15].I += 4;\ 
        } else {\
          myregs[15].I &= 0xFFFFFFFE;\
          armNextPC = myregs[15].I;\
          myregs[15].I += 2;\ 
        }\
      } else {\
        OPCODE \
      }\
    }\
    break;\
  case BASE+0x200:\
  case BASE+0x201:\
  case BASE+0x202:\
  case BASE+0x203:\
  case BASE+0x204:\
  case BASE+0x205:\
  case BASE+0x206:\
  case BASE+0x207:\
  case BASE+0x208:\
  case BASE+0x209:\
  case BASE+0x20a:\
  case BASE+0x20b:\
  case BASE+0x20c:\
  case BASE+0x20d:\
  case BASE+0x20e:\
  case BASE+0x20f:\
    {\
      int shift = (opcode & 0xF00) >> 7;\
      int base = (opcode >> 16) & 0x0F;\
      int dest = (opcode >> 12) & 0x0F;\
      bool C_OUT = C_FLAG;\
      u32 value;\ 
      if(shift) {\
        LOGICAL_ROR_IMM\
      } else {\
        value = opcode & 0xff;\
      }\
      if(dest == 15) {\ 
        OPCODE2\
/*todo*/\
        if(armState) {\
          myregs[15].I &= 0xFFFFFFFC;\
          armNextPC = myregs[15].I;\
          myregs[15].I += 4;\ 
        } else {\
          myregs[15].I &= 0xFFFFFFFE;\
          armNextPC = myregs[15].I;\
          myregs[15].I += 2;\ 
        }\
      } else {\
        OPCODE \
      }\
    }\
    break;

#define LOGICAL_DATA_OPCODE_WITHOUT_base(OPCODE, OPCODE2, BASE) \
  case BASE: \
  case BASE+8:\
    {\
      /* OP Rd,Rb,Rm LSL # */ \
      int shift = (opcode >> 7) & 0x1F;\
      int dest = (opcode>>12) & 15;\
      bool C_OUT = C_FLAG;\
      u32 value;\ 
      \
      if(shift) {\
        LOGICAL_LSL_myregs\
      } else {\
        value = myregs[opcode & 0x0F].I;\
      }\
      if(dest == 15) {\ 
        OPCODE2\
/*todo*/\
        if(armState) {\
          myregs[15].I &= 0xFFFFFFFC;\
          armNextPC = myregs[15].I;\
          myregs[15].I += 4;\ 
        } else {\
          myregs[15].I &= 0xFFFFFFFE;\
          armNextPC = myregs[15].I;\
          myregs[15].I += 2;\ 
        }\
      } else {\
        OPCODE \
      }\
    }\
    break;\
  case BASE+2:\
  case BASE+10:\
    {\
       /* OP Rd,Rb,Rm LSR # */ \
      int shift = (opcode >> 7) & 0x1F;\
      int dest = (opcode>>12) & 15;\
      bool C_OUT = C_FLAG;\
      u32 value;\ 
      if(shift) {\
        LOGICAL_LSR_myregs\
      } else {\
        value = 0;\
        C_OUT = (myregs[opcode & 0x0F].I & 0x80000000) ? true : false;\
      }\
      \
      if(dest == 15) {\ 
        OPCODE2\
/*todo*/\
        if(armState) {\
          myregs[15].I &= 0xFFFFFFFC;\
          armNextPC = myregs[15].I;\
          myregs[15].I += 4;\ 
        } else {\
          myregs[15].I &= 0xFFFFFFFE;\
          armNextPC = myregs[15].I;\
          myregs[15].I += 2;\ 
        }\
      } else {\
        OPCODE \
      }\
    }\
    break;\
  case BASE+4:\
  case BASE+12:\
    {\
       /* OP Rd,Rb,Rm ASR # */\
      int shift = (opcode >> 7) & 0x1F;\
      int dest = (opcode>>12) & 15;\
      bool C_OUT = C_FLAG;\
      u32 value;\ 
      if(shift) {\
        LOGICAL_ASR_myregs\
      } else {\
        if(myregs[opcode & 0x0F].I & 0x80000000){\
          value = 0xFFFFFFFF;\
          C_OUT = true;\
        } else {\
          value = 0;\
          C_OUT = false;\
        }                   \
      }\
      \
      if(dest == 15) {\ 
        OPCODE2\
/*todo*/\
        if(armState) {\
          myregs[15].I &= 0xFFFFFFFC;\
          armNextPC = myregs[15].I;\
          myregs[15].I += 4;\ 
        } else {\
          myregs[15].I &= 0xFFFFFFFE;\
          armNextPC = myregs[15].I;\
          myregs[15].I += 2;\ 
        }\
      } else {\
        OPCODE \
      }\
    }\
    break;\
  case BASE+6:\
  case BASE+14:\
    {\
       /* OP Rd,Rb,Rm ROR # */\
      int shift = (opcode >> 7) & 0x1F;\
      int dest = (opcode>>12) & 15;\
      bool C_OUT = C_FLAG;\
      u32 value;\ 
      if(shift) {\
        LOGICAL_ROR_myregs\
      } else {\
        LOGICAL_RRX_myregs\
      }\
      if(dest == 15) {\ 
        OPCODE2\
/*todo*/\
        if(armState) {\
          myregs[15].I &= 0xFFFFFFFC;\
          armNextPC = myregs[15].I;\
          myregs[15].I += 4;\ 
        } else {\
          myregs[15].I &= 0xFFFFFFFE;\
          armNextPC = myregs[15].I;\
          myregs[15].I += 2;\ 
        }\
      } else {\
        OPCODE \
      }\
    }\
    break;\
  case BASE+1:\
    {\
       /* OP Rd,Rb,Rm LSL Rs */\
      int shift = myregs[(opcode >> 8)&15].B.B0;\
      int dest = (opcode>>12) & 15;\
      bool C_OUT = C_FLAG;\
      u32 value;\ 
      if(shift) {\
        if(shift == 32) {\
          value = 0;\
          C_OUT = (myregs[opcode & 0x0F].I & 1 ? true : false);\
        } else if(shift < 32) {\
           LOGICAL_LSL_myregs\
        } else {\
          value = 0;\
          C_OUT = false;\
        }\
      } else {\
        value = myregs[opcode & 0x0F].I;\
      }\
      if(dest == 15) {\ 
        OPCODE2\
/*todo*/\
        if(armState) {\
          myregs[15].I &= 0xFFFFFFFC;\
          armNextPC = myregs[15].I;\
          myregs[15].I += 4;\ 
        } else {\
          myregs[15].I &= 0xFFFFFFFE;\
          armNextPC = myregs[15].I;\
          myregs[15].I += 2;\ 
        }\
      } else {\
        OPCODE \
      }\
    }\
    break;\
  case BASE+3:\
    {\
       /* OP Rd,Rb,Rm LSR Rs */ \
      int shift = myregs[(opcode >> 8)&15].B.B0;\
      int dest = (opcode>>12) & 15;\
      bool C_OUT = C_FLAG;\
      u32 value;\ 
      if(shift) {\
        if(shift == 32) {\
          value = 0;\
          C_OUT = (myregs[opcode & 0x0F].I & 0x80000000 ? true : false);\
        } else if(shift < 32) {\
            LOGICAL_LSR_myregs\
        } else {\
          value = 0;\
          C_OUT = false;\
        }\
      } else {\
        value = myregs[opcode & 0x0F].I;\
      }\
      if(dest == 15) {\ 
        OPCODE2\
/*todo*/\
        if(armState) {\
          myregs[15].I &= 0xFFFFFFFC;\
          armNextPC = myregs[15].I;\
          myregs[15].I += 4;\ 
        } else {\
          myregs[15].I &= 0xFFFFFFFE;\
          armNextPC = myregs[15].I;\
          myregs[15].I += 2;\ 
        }\
      } else {\
        OPCODE \
      }\
    }\
    break;\
  case BASE+5:\
    {\
       /* OP Rd,Rb,Rm ASR Rs */ \
      int shift = myregs[(opcode >> 8)&15].B.B0;\
      int dest = (opcode>>12) & 15;\
      bool C_OUT = C_FLAG;\
      u32 value;\ 
      if(shift < 32) {\
        if(shift) {\
          LOGICAL_ASR_myregs\
        } else {\
          value = myregs[opcode & 0x0F].I;\
        }\
      } else {\
        if(myregs[opcode & 0x0F].I & 0x80000000){\
          value = 0xFFFFFFFF;\
          C_OUT = true;\
        } else {\
          value = 0;\
          C_OUT = false;\
        }\
      }\
      if(dest == 15) {\ 
        OPCODE2\
/*todo*/\
        if(armState) {\
          myregs[15].I &= 0xFFFFFFFC;\
          armNextPC = myregs[15].I;\
          myregs[15].I += 4;\ 
        } else {\
          myregs[15].I &= 0xFFFFFFFE;\
          armNextPC = myregs[15].I;\
          myregs[15].I += 2;\ 
        }\
      } else {\
        OPCODE \
      }\
    }\
    break;\
  case BASE+7:\
    {\
       /* OP Rd,Rb,Rm ROR Rs */\
      int shift = myregs[(opcode >> 8)&15].B.B0;\
      int dest = (opcode>>12) & 15;\
      bool C_OUT = C_FLAG;\
      u32 value;\ 
      if(shift) {\
        shift &= 0x1f;\
        if(shift) {\
          LOGICAL_ROR_myregs\
        } else {\
          value = myregs[opcode & 0x0F].I;\
          C_OUT = (value & 0x80000000 ? true : false);\
        }\
      } else {\
        value = myregs[opcode & 0x0F].I;\
        C_OUT = (value & 0x80000000 ? true : false);\
      }\
      if(dest == 15) {\ 
        OPCODE2\
/*todo*/\
        if(armState) {\
          myregs[15].I &= 0xFFFFFFFC;\
          armNextPC = myregs[15].I;\
          myregs[15].I += 4;\ 
        } else {\
          myregs[15].I &= 0xFFFFFFFE;\
          armNextPC = myregs[15].I;\
          myregs[15].I += 2;\ 
        }\
      } else {\
        OPCODE \
      }\
    }\
    break;\
  case BASE+0x200:\
  case BASE+0x201:\
  case BASE+0x202:\
  case BASE+0x203:\
  case BASE+0x204:\
  case BASE+0x205:\
  case BASE+0x206:\
  case BASE+0x207:\
  case BASE+0x208:\
  case BASE+0x209:\
  case BASE+0x20a:\
  case BASE+0x20b:\
  case BASE+0x20c:\
  case BASE+0x20d:\
  case BASE+0x20e:\
  case BASE+0x20f:\
    {\
      int shift = (opcode & 0xF00) >> 7;\
      int dest = (opcode >> 12) & 0x0F;\
      bool C_OUT = C_FLAG;\
      u32 value;\ 
      if(shift) {\
        LOGICAL_ROR_IMM\
      } else {\
        value = opcode & 0xff;\
      }\
      if(dest == 15) {\ 
        OPCODE2\
/*todo*/\
        if(armState) {\
          myregs[15].I &= 0xFFFFFFFC;\
          armNextPC = myregs[15].I;\
          myregs[15].I += 4;\ 
        } else {\
          myregs[15].I &= 0xFFFFFFFE;\
          armNextPC = myregs[15].I;\
          myregs[15].I += 2;\ 
        }\
      } else {\
        OPCODE \
      }\
    }\
    break;

#define ARITHMETIC_DATA_OPCODE(OPCODE, OPCODE2, BASE) \
  case BASE:\
  case BASE+8:\
    {\
      /* OP Rd,Rb,Rm LSL # */\
      int base = (opcode >> 16) & 0x0F;\
      int shift = (opcode >> 7) & 0x1F;\
      int dest = (opcode>>12) & 15;\
      u32 value;\ 
      if(shift) {\
        ARITHMETIC_LSL_myregs\
      } else {\
        value = myregs[opcode & 0x0F].I;\
      }\
      if(dest == 15) {\ 
        OPCODE2\
/*todo*/\
        if(armState) {\
          myregs[15].I &= 0xFFFFFFFC;\
          armNextPC = myregs[15].I;\
          myregs[15].I += 4;\ 
        } else {\
          myregs[15].I &= 0xFFFFFFFE;\
          armNextPC = myregs[15].I;\
          myregs[15].I += 2;\ 
        }\
      } else {\
        OPCODE \
      }\
    }\
    break;\
  case BASE+2:\
  case BASE+10:\
    {\
      /* OP Rd,Rb,Rm LSR # */\
      int base = (opcode >> 16) & 0x0F;\
      int shift = (opcode >> 7) & 0x1F;\
      int dest = (opcode>>12) & 15;\
      u32 value;\ 
      if(shift) {\
        ARITHMETIC_LSR_myregs\
      } else {\
        value = 0;\
      }\
      if(dest == 15) {\ 
        OPCODE2\
/*todo*/\
        if(armState) {\
          myregs[15].I &= 0xFFFFFFFC;\
          armNextPC = myregs[15].I;\
          myregs[15].I += 4;\ 
        } else {\
          myregs[15].I &= 0xFFFFFFFE;\
          armNextPC = myregs[15].I;\
          myregs[15].I += 2;\ 
        }\
      } else {\
        OPCODE \
      }\
    }\
    break;\
  case BASE+4:\
  case BASE+12:\
    {\
      /* OP Rd,Rb,Rm ASR # */\
      int base = (opcode >> 16) & 0x0F;\
      int shift = (opcode >> 7) & 0x1F;\
      int dest = (opcode>>12) & 15;\
      u32 value;\ 
      if(shift) {\
        ARITHMETIC_ASR_myregs\
      } else {\
        if(myregs[opcode & 0x0F].I & 0x80000000){\
          value = 0xFFFFFFFF;\
        } else value = 0;\
      }\
      if(dest == 15) {\ 
        OPCODE2\
/*todo*/\
        if(armState) {\
          myregs[15].I &= 0xFFFFFFFC;\
          armNextPC = myregs[15].I;\
          myregs[15].I += 4;\ 
        } else {\
          myregs[15].I &= 0xFFFFFFFE;\
          armNextPC = myregs[15].I;\
          myregs[15].I += 2;\ 
        }\
      } else {\
        OPCODE \
      }\
    }\
    break;\
  case BASE+6:\
  case BASE+14:\
    {\
      /* OP Rd,Rb,Rm ROR # */\
      int base = (opcode >> 16) & 0x0F;\
      int shift = (opcode >> 7) & 0x1F;\
      int dest = (opcode>>12) & 15;\
      u32 value;\ 
      if(shift) {\
         ARITHMETIC_ROR_myregs\
      } else {\
         ARITHMETIC_RRX_myregs\
      }\
      if(dest == 15) {\ 
        OPCODE2\
/*todo*/\
        if(armState) {\
          myregs[15].I &= 0xFFFFFFFC;\
          armNextPC = myregs[15].I;\
          myregs[15].I += 4;\ 
        } else {\
          myregs[15].I &= 0xFFFFFFFE;\
          armNextPC = myregs[15].I;\
          myregs[15].I += 2;\ 
        }\
      } else {\
        OPCODE \
      }\
    }\
    break;\
  case BASE+1:\
    {\
      /* OP Rd,Rb,Rm LSL Rs */\
      int base = (opcode >> 16) & 0x0F;\
      int shift = myregs[(opcode >> 8)&15].B.B0;\
      int dest = (opcode>>12) & 15;\
      u32 value;\ 
      if(shift) {\
        if(shift == 32) {\
          value = 0;\
        } else if(shift < 32) {\
           ARITHMETIC_LSL_myregs\
        } else value = 0;\
      } else {\
        value = myregs[opcode & 0x0F].I;\
      }\
      if(dest == 15) {\ 
        OPCODE2\
/*todo*/\
        if(armState) {\
          myregs[15].I &= 0xFFFFFFFC;\
          armNextPC = myregs[15].I;\
          myregs[15].I += 4;\ 
        } else {\
          myregs[15].I &= 0xFFFFFFFE;\
          armNextPC = myregs[15].I;\
          myregs[15].I += 2;\ 
        }\
      } else {\
        OPCODE \
      }\
    }\
    break;\
  case BASE+3:\
    {\
      /* OP Rd,Rb,Rm LSR Rs */\
      int base = (opcode >> 16) & 0x0F;\
      int shift = myregs[(opcode >> 8)&15].B.B0;\
      int dest = (opcode>>12) & 15;\
      u32 value;\ 
      if(shift) {\
        if(shift == 32) {\
          value = 0;\
        } else if(shift < 32) {\
           ARITHMETIC_LSR_myregs\
        } else value = 0;\
      } else {\
        value = myregs[opcode & 0x0F].I;\
      }\
      if(dest == 15) {\ 
        OPCODE2\
/*todo*/\
        if(armState) {\
          myregs[15].I &= 0xFFFFFFFC;\
          armNextPC = myregs[15].I;\
          myregs[15].I += 4;\ 
        } else {\
          myregs[15].I &= 0xFFFFFFFE;\
          armNextPC = myregs[15].I;\
          myregs[15].I += 2;\ 
        }\
      } else {\
        OPCODE \
      }\
    }\
    break;\
  case BASE+5:\
    {\
      /* OP Rd,Rb,Rm ASR Rs */\
      int base = (opcode >> 16) & 0x0F;\
      int shift = myregs[(opcode >> 8)&15].B.B0;\
      int dest = (opcode>>12) & 15;\
      u32 value;\ 
      if(shift < 32) {\
        if(shift) {\
           ARITHMETIC_ASR_myregs\
        } else {\
          value = myregs[opcode & 0x0F].I;\
        }\
      } else {\
        if(myregs[opcode & 0x0F].I & 0x80000000){\
          value = 0xFFFFFFFF;\
        } else value = 0;\
      }\
      if(dest == 15) {\ 
        OPCODE2\
/*todo*/\
        if(armState) {\
          myregs[15].I &= 0xFFFFFFFC;\
          armNextPC = myregs[15].I;\
          myregs[15].I += 4;\ 
        } else {\
          myregs[15].I &= 0xFFFFFFFE;\
          armNextPC = myregs[15].I;\
          myregs[15].I += 2;\ 
        }\
      } else {\
        OPCODE \
      }\
    }\
    break;\
  case BASE+7:\
    {\
      /* OP Rd,Rb,Rm ROR Rs */\
      int base = (opcode >> 16) & 0x0F;\
      int shift = myregs[(opcode >> 8)&15].B.B0;\
      int dest = (opcode>>12) & 15;\
      u32 value;\ 
      if(shift) {\
        shift &= 0x1f;\
        if(shift) {\
           ARITHMETIC_ROR_myregs\
        } else {\
           value = myregs[opcode & 0x0F].I;\
        }\
      } else {\
        value = myregs[opcode & 0x0F].I;\
      }\
      if(dest == 15) {\ 
        OPCODE2\
/*todo*/\
        if(armState) {\
          myregs[15].I &= 0xFFFFFFFC;\
          armNextPC = myregs[15].I;\
          myregs[15].I += 4;\ 
        } else {\
          myregs[15].I &= 0xFFFFFFFE;\
          armNextPC = myregs[15].I;\
          myregs[15].I += 2;\ 
        }\
      } else {\
        OPCODE \
      }\
    }\
    break;\
  case BASE+0x200:\
  case BASE+0x201:\
  case BASE+0x202:\
  case BASE+0x203:\
  case BASE+0x204:\
  case BASE+0x205:\
  case BASE+0x206:\
  case BASE+0x207:\
  case BASE+0x208:\
  case BASE+0x209:\
  case BASE+0x20a:\
  case BASE+0x20b:\
  case BASE+0x20c:\
  case BASE+0x20d:\
  case BASE+0x20e:\
  case BASE+0x20f:\
    {\
      int shift = (opcode & 0xF00) >> 7;\
      int base = (opcode >> 16) & 0x0F;\
      int dest = (opcode >> 12) & 0x0F;\
      u32 value;\ 
      {\
        ARITHMETIC_ROR_IMM\
      }\
      if(dest == 15) {\ 
        OPCODE2\
/*todo*/\
        if(armState) {\
          myregs[15].I &= 0xFFFFFFFC;\
          armNextPC = myregs[15].I;\
          myregs[15].I += 4;\ 
        } else {\
          myregs[15].I &= 0xFFFFFFFE;\
          armNextPC = myregs[15].I;\
          myregs[15].I += 2;\ 
        }\
      } else {\
        OPCODE \
      }\
    }\
    break;\
















































void emuInstrARM(u32 opcode, s32 *R)
{

reg_pair* myregs = (reg_pair*)R;
	
  int cond = opcode >> 28;
  // suggested optimization for frequent cases
  bool cond_res;
  if(cond == 0x0e) {
    cond_res = true;
  } else {
    switch(cond) { 
    case 0x00: // EQ 
      cond_res = Z_FLAG;
      break;
    case 0x01: // NE
      cond_res = !Z_FLAG;
      break; 
    case 0x02: // CS
      cond_res = C_FLAG;
      break;
    case 0x03: // CC
      cond_res = !C_FLAG;
      break;
    case 0x04: // MI
      cond_res = N_FLAG;
      break;
    case 0x05: // PL
      cond_res = !N_FLAG;
      break;
    case 0x06: // VS
      cond_res = V_FLAG;
      break;
    case 0x07: // VC
      cond_res = !V_FLAG;
      break;
    case 0x08: // HI
      cond_res = C_FLAG && !Z_FLAG;
      break;
    case 0x09: // LS
      cond_res = !C_FLAG || Z_FLAG;
      break;
    case 0x0A: // GE
      cond_res = N_FLAG == V_FLAG;
      break;
    case 0x0B: // LT
      cond_res = N_FLAG != V_FLAG;
      break;
    case 0x0C: // GT
      cond_res = !Z_FLAG &&(N_FLAG == V_FLAG);
      break;    
    case 0x0D: // LE
      cond_res = Z_FLAG || (N_FLAG != V_FLAG);
      break; 
    case 0x0E: 
      cond_res = true; 
      break;
    case 0x0F:
    default:
      // ???
      cond_res = false;
      break;
    }
  }
  
if(cond_res) {
  switch(((opcode>>16)&0xFF0) | ((opcode>>4)&0x0F)) {
    LOGICAL_DATA_OPCODE_WITHOUT_base(OP_AND,  OP_AND, 0x000);
    LOGICAL_DATA_OPCODE_WITHOUT_base(OP_ANDS, OP_AND, 0x010);
  case 0x009:
    {
      // MUL Rd, Rm, Rs
      int dest = (opcode >> 16) & 0x0F;
      int mult = (opcode & 0x0F);
      u32 rs = myregs[(opcode >> 8) & 0x0F].I;
      myregs[dest].I = myregs[mult].I * rs;
      if(((s32)rs)<0)
         rs = ~rs;

    }
    break;
  case 0x019:
    {
      // MULS Rd, Rm, Rs
      int dest = (opcode >> 16) & 0x0F;
      int mult = (opcode & 0x0F);
      u32 rs = myregs[(opcode >> 8) & 0x0F].I;
      myregs[dest].I = myregs[mult].I * rs;
      N_FLAG = (myregs[dest].I & 0x80000000) ? true : false;
      Z_FLAG = (myregs[dest].I) ? false : true;
      if(((s32)rs)<0)
        rs = ~rs;
    }
    break;
  case 0x00b:
  case 0x02b:
    {
      // STRH Rd, [Rn], -Rm
 
      int base = (opcode >> 16) & 0x0F;
      int dest = (opcode >> 12) & 0x0F;
      u32 address = myregs[base].I;
      int offset = myregs[opcode & 0x0F].I;
 
      CPUWriteHalfWord(address, myregs[dest].W.W0);
      address -= offset;
      myregs[base].I = address;
    }
    break;
  case 0x04b:
  case 0x06b:
    {
      // STRH Rd, [Rn], #-offset
 
      int base = (opcode >> 16) & 0x0F;
      int dest = (opcode >> 12) & 0x0F;
      u32 address = myregs[base].I;
      int offset = (opcode & 0x0F) | ((opcode >> 4) & 0xF0);
 
      CPUWriteHalfWord(address, myregs[dest].W.W0);
      address -= offset;
      myregs[base].I = address;
    }
    break;
  case 0x08b:
  case 0x0ab:
    {
      // STRH Rd, [Rn], Rm
 
      int base = (opcode >> 16) & 0x0F;
      int dest = (opcode >> 12) & 0x0F;
      u32 address = myregs[base].I;
      int offset = myregs[opcode & 0x0F].I;
 
      CPUWriteHalfWord(address, myregs[dest].W.W0);
      address += offset;
      myregs[base].I = address;
    }
    break;
  case 0x0cb:
  case 0x0eb:
    {
      // STRH Rd, [Rn], #offset
 
      int base = (opcode >> 16) & 0x0F;
      int dest = (opcode >> 12) & 0x0F;
      u32 address = myregs[base].I;
      int offset = (opcode & 0x0F) | ((opcode >> 4) & 0xF0);
 
      CPUWriteHalfWord(address, myregs[dest].W.W0);
      address += offset;
      myregs[base].I = address;
    }
    break;
  case 0x10b:
    {
      // STRH Rd, [Rn, -Rm]
 
      int base = (opcode >> 16) & 0x0F;
      int dest = (opcode >> 12) & 0x0F;
      u32 address = myregs[base].I - myregs[opcode & 0x0F].I;
 
      CPUWriteHalfWord(address, myregs[dest].W.W0);
    }
    break;
  case 0x12b:
    {
      // STRH Rd, [Rn, -Rm]!
 
      int base = (opcode >> 16) & 0x0F;
      int dest = (opcode >> 12) & 0x0F;
      u32 address = myregs[base].I - myregs[opcode & 0x0F].I;
 
      CPUWriteHalfWord(address, myregs[dest].W.W0);
      myregs[base].I = address;
    }
    break;
  case 0x14b:
    {
      // STRH Rd, [Rn, -#offset]
 
      int base = (opcode >> 16) & 0x0F;
      int dest = (opcode >> 12) & 0x0F;
      u32 address = myregs[base].I - ((opcode & 0x0F)|((opcode>>4)&0xF0));
 
      CPUWriteHalfWord(address, myregs[dest].W.W0);
    }
    break;
  case 0x16b:
    {
      // STRH Rd, [Rn, -#offset]!
 
      int base = (opcode >> 16) & 0x0F;
      int dest = (opcode >> 12) & 0x0F;
      u32 address = myregs[base].I - ((opcode & 0x0F)|((opcode>>4)&0xF0));
 
      CPUWriteHalfWord(address, myregs[dest].W.W0);
      myregs[base].I = address;
    }
    break;
  case 0x18b:
    {
      // STRH Rd, [Rn, Rm]
 
      int base = (opcode >> 16) & 0x0F;
      int dest = (opcode >> 12) & 0x0F;
      u32 address = myregs[base].I + myregs[opcode & 0x0F].I;
 
      CPUWriteHalfWord(address, myregs[dest].W.W0);
    }
    break;
  case 0x1ab:
    {
      // STRH Rd, [Rn, Rm]!
 
      int base = (opcode >> 16) & 0x0F;
      int dest = (opcode >> 12) & 0x0F;
      u32 address = myregs[base].I + myregs[opcode & 0x0F].I;
 
      CPUWriteHalfWord(address, myregs[dest].W.W0);
      myregs[base].I = address;
    }
    break;
  case 0x1cb:
    {
      // STRH Rd, [Rn, #offset]
 
      int base = (opcode >> 16) & 0x0F;
      int dest = (opcode >> 12) & 0x0F;
      u32 address = myregs[base].I + ((opcode & 0x0F)|((opcode>>4)&0xF0));
 
      CPUWriteHalfWord(address, myregs[dest].W.W0);
    }
    break;
  case 0x1eb:
    {
      // STRH Rd, [Rn, #offset]!
 
      int base = (opcode >> 16) & 0x0F;
      int dest = (opcode >> 12) & 0x0F;
      u32 address = myregs[base].I + ((opcode & 0x0F)|((opcode>>4)&0xF0));
 
      CPUWriteHalfWord(address, myregs[dest].W.W0);
      myregs[base].I = address;
    }
    break;
  case 0x01b:
  case 0x03b:
    {
      // LDRH Rd, [Rn], -Rm
 
      int base = (opcode >> 16) & 0x0F;
      int dest = (opcode >> 12) & 0x0F;
      u32 address = myregs[base].I;
      int offset = myregs[opcode & 0x0F].I;
      myregs[dest].I = CPUReadHalfWord(address);
      if(dest != base) {
        address -= offset;
        myregs[base].I = address;
      }
       
      if(dest == 15) {
        myregs[15].I &= 0xFFFFFFFC;
        armNextPC = myregs[15].I;
        myregs[15].I += 4;
        //ARM_PREFETCH;
         
      }
 
    }
    break;
  case 0x05b:
  case 0x07b:
    {
      // LDRH Rd, [Rn], #-offset
 
      int base = (opcode >> 16) & 0x0F;
      int dest = (opcode >> 12) & 0x0F;
      u32 address = myregs[base].I;
      int offset = (opcode & 0x0F) | ((opcode >> 4) & 0xF0);
      myregs[dest].I = CPUReadHalfWord(address);
      if(dest != base) {
        address -= offset;
        myregs[base].I = address;
      }
       
      if(dest == 15) {
        myregs[15].I &= 0xFFFFFFFC;
        armNextPC = myregs[15].I;
        myregs[15].I += 4;
        //ARM_PREFETCH;
         
      }
 
    }
    break;
  case 0x09b:
  case 0x0bb:
    {
      // LDRH Rd, [Rn], Rm
 
      int base = (opcode >> 16) & 0x0F;
      int dest = (opcode >> 12) & 0x0F;
      u32 address = myregs[base].I;
      int offset = myregs[opcode & 0x0F].I;
      myregs[dest].I = CPUReadHalfWord(address);
      if(dest != base) {
        address += offset;
        myregs[base].I = address;
      }
       
      if(dest == 15) {
        myregs[15].I &= 0xFFFFFFFC;
        armNextPC = myregs[15].I;
        myregs[15].I += 4;
        //ARM_PREFETCH;
         
      }
 
    }
    break;
  case 0x0db:
  case 0x0fb:
    {
      // LDRH Rd, [Rn], #offset
 
      int base = (opcode >> 16) & 0x0F;
      int dest = (opcode >> 12) & 0x0F;
      u32 address = myregs[base].I;
      int offset = (opcode & 0x0F) | ((opcode >> 4) & 0xF0);
      myregs[dest].I = CPUReadHalfWord(address);
      if(dest != base) {
        address += offset;
        myregs[base].I = address;
      }
       
      if(dest == 15) {
        myregs[15].I &= 0xFFFFFFFC;
        armNextPC = myregs[15].I;
        myregs[15].I += 4;
        //ARM_PREFETCH;
         
      }
 
    }
    break;
  case 0x11b:
    {
      // LDRH Rd, [Rn, -Rm]
 
      int base = (opcode >> 16) & 0x0F;
      int dest = (opcode >> 12) & 0x0F;
      u32 address = myregs[base].I - myregs[opcode & 0x0F].I;
      myregs[dest].I = CPUReadHalfWord(address);
       
      if(dest == 15) {
        myregs[15].I &= 0xFFFFFFFC;
        armNextPC = myregs[15].I;
        myregs[15].I += 4;
        //ARM_PREFETCH;
         
      }
 
    }
    break;
  case 0x13b:
    {
      // LDRH Rd, [Rn, -Rm]!
 
      int base = (opcode >> 16) & 0x0F;
      int dest = (opcode >> 12) & 0x0F;
      u32 address = myregs[base].I - myregs[opcode & 0x0F].I;
      myregs[dest].I = CPUReadHalfWord(address);
      if(dest != base)
        myregs[base].I = address;
       
      if(dest == 15) {
        myregs[15].I &= 0xFFFFFFFC;
        armNextPC = myregs[15].I;
        myregs[15].I += 4;
        //ARM_PREFETCH;
         
      }
 
    }
    break;
  case 0x15b:
    {
      // LDRH Rd, [Rn, -#offset]
 
      int base = (opcode >> 16) & 0x0F;
      int dest = (opcode >> 12) & 0x0F;
      u32 address = myregs[base].I - ((opcode & 0x0F)|((opcode>>4)&0xF0));
      myregs[dest].I = CPUReadHalfWord(address);
       
      if(dest == 15) {
        myregs[15].I &= 0xFFFFFFFC;
        armNextPC = myregs[15].I;
        myregs[15].I += 4;
        //ARM_PREFETCH;
         
      }
 
    }
    break;
  case 0x17b:
    {
      // LDRH Rd, [Rn, -#offset]!
 
      int base = (opcode >> 16) & 0x0F;
      int dest = (opcode >> 12) & 0x0F;
      u32 address = myregs[base].I - ((opcode & 0x0F)|((opcode>>4)&0xF0));
      myregs[dest].I = CPUReadHalfWord(address);
      if(dest != base)
        myregs[base].I = address;
       
      if(dest == 15) {
        myregs[15].I &= 0xFFFFFFFC;
        armNextPC = myregs[15].I;
        myregs[15].I += 4;
        //ARM_PREFETCH;
         
      }
 
    }
    break;
  case 0x19b:
    {
      // LDRH Rd, [Rn, Rm]
 
      int base = (opcode >> 16) & 0x0F;
      int dest = (opcode >> 12) & 0x0F;
      u32 address = myregs[base].I + myregs[opcode & 0x0F].I;
      myregs[dest].I = CPUReadHalfWord(address);
       
      if(dest == 15) {
        myregs[15].I &= 0xFFFFFFFC;
        armNextPC = myregs[15].I;
        myregs[15].I += 4;
        //ARM_PREFETCH;
         
      }
 
    }
    break;
  case 0x1bb:
    {
      // LDRH Rd, [Rn, Rm]!
 
      int base = (opcode >> 16) & 0x0F;
      int dest = (opcode >> 12) & 0x0F;
      u32 address = myregs[base].I + myregs[opcode & 0x0F].I;
      myregs[dest].I = CPUReadHalfWord(address);
      if(dest != base)
        myregs[base].I = address;
       
      if(dest == 15) {
        myregs[15].I &= 0xFFFFFFFC;
        armNextPC = myregs[15].I;
        myregs[15].I += 4;
        //ARM_PREFETCH;
         
      }
 
    }
    break;
  case 0x1db:
    {
      // LDRH Rd, [Rn, #offset]
 
      int base = (opcode >> 16) & 0x0F;
      int dest = (opcode >> 12) & 0x0F;
      u32 address = myregs[base].I + ((opcode & 0x0F)|((opcode>>4)&0xF0));
      myregs[dest].I = CPUReadHalfWord(address);
       
      if(dest == 15) {
        myregs[15].I &= 0xFFFFFFFC;
        armNextPC = myregs[15].I;
        myregs[15].I += 4;
        //ARM_PREFETCH;
         
      }
 
    }
    break;
  case 0x1fb:
    {
      // LDRH Rd, [Rn, #offset]!
 
      int base = (opcode >> 16) & 0x0F;
      int dest = (opcode >> 12) & 0x0F;
      u32 address = myregs[base].I + ((opcode & 0x0F)|((opcode>>4)&0xF0));
      myregs[dest].I = CPUReadHalfWord(address);
      if(dest != base)
        myregs[base].I = address;
       
      if(dest == 15) {
        myregs[15].I &= 0xFFFFFFFC;
        armNextPC = myregs[15].I;
        myregs[15].I += 4;
        //ARM_PREFETCH;
         
      }
 
    }
    break;
  case 0x01d:
  case 0x03d:
    {
      // LDRSB Rd, [Rn], -Rm
 
      int base = (opcode >> 16) & 0x0F;
      int dest = (opcode >> 12) & 0x0F;
      u32 address = myregs[base].I;
      int offset = myregs[opcode & 0x0F].I;
      myregs[dest].I = (s8)CPUReadByte(address);
      if(dest != base) {
        address -= offset;
        myregs[base].I = address;
      }
       
      if(dest == 15) {
        myregs[15].I &= 0xFFFFFFFC;
        armNextPC = myregs[15].I;
        myregs[15].I += 4;
        //ARM_PREFETCH;
         
      }
 
    }
    break;
  case 0x05d:
  case 0x07d:
    {
      // LDRSB Rd, [Rn], #-offset
 
      int base = (opcode >> 16) & 0x0F;
      int dest = (opcode >> 12) & 0x0F;
      u32 address = myregs[base].I;
      int offset = (opcode & 0x0F) | ((opcode >> 4) & 0xF0);
      myregs[dest].I = (s8)CPUReadByte(address);
      if(dest != base) {
        address -= offset;
        myregs[base].I = address;
      }
       
      if(dest == 15) {
        myregs[15].I &= 0xFFFFFFFC;
        armNextPC = myregs[15].I;
        myregs[15].I += 4;
        //ARM_PREFETCH;
         
      }
 
    }
    break;
  case 0x09d:
  case 0x0bd:
    {
      // LDRSB Rd, [Rn], Rm
 
      int base = (opcode >> 16) & 0x0F;
      int dest = (opcode >> 12) & 0x0F;
      u32 address = myregs[base].I;
      int offset = myregs[opcode & 0x0F].I;
      myregs[dest].I = (s8)CPUReadByte(address);
      if(dest != base) {
        address += offset;
        myregs[base].I = address;
      }
       
      if(dest == 15) {
        myregs[15].I &= 0xFFFFFFFC;
        armNextPC = myregs[15].I;
        myregs[15].I += 4;
        //ARM_PREFETCH;
         
      }
 
    }
    break;
  case 0x0dd:
  case 0x0fd:
    {
      // LDRSB Rd, [Rn], #offset
 
      int base = (opcode >> 16) & 0x0F;
      int dest = (opcode >> 12) & 0x0F;
      u32 address = myregs[base].I;
      int offset = (opcode & 0x0F) | ((opcode >> 4) & 0xF0);
      myregs[dest].I = (s8)CPUReadByte(address);
      if(dest != base) {
        address += offset;
        myregs[base].I = address;
      }
       
      if(dest == 15) {
        myregs[15].I &= 0xFFFFFFFC;
        armNextPC = myregs[15].I;
        myregs[15].I += 4;
        //ARM_PREFETCH;
         
      }
 
    }
    break;
  case 0x11d:
    {
      // LDRSB Rd, [Rn, -Rm]
 
      int base = (opcode >> 16) & 0x0F;
      int dest = (opcode >> 12) & 0x0F;
      u32 address = myregs[base].I - myregs[opcode & 0x0F].I;
      myregs[dest].I = (s8)CPUReadByte(address);
       
      if(dest == 15) {
        myregs[15].I &= 0xFFFFFFFC;
        armNextPC = myregs[15].I;
        myregs[15].I += 4;
        //ARM_PREFETCH;
         
      }
 
    }
    break;
  case 0x13d:
    {
      // LDRSB Rd, [Rn, -Rm]!
 
      int base = (opcode >> 16) & 0x0F;
      int dest = (opcode >> 12) & 0x0F;
      u32 address = myregs[base].I - myregs[opcode & 0x0F].I;
      myregs[dest].I = (s8)CPUReadByte(address);
      if(dest != base)
        myregs[base].I = address;
       
      if(dest == 15) {
        myregs[15].I &= 0xFFFFFFFC;
        armNextPC = myregs[15].I;
        myregs[15].I += 4;
        //ARM_PREFETCH;
         
      }
 
    }
    break;
  case 0x15d:
    {
      // LDRSB Rd, [Rn, -#offset]
 
      int base = (opcode >> 16) & 0x0F;
      int dest = (opcode >> 12) & 0x0F;
      u32 address = myregs[base].I - ((opcode & 0x0F)|((opcode>>4)&0xF0));
      myregs[dest].I = (s8)CPUReadByte(address);
       
      if(dest == 15) {
        myregs[15].I &= 0xFFFFFFFC;
        armNextPC = myregs[15].I;
        myregs[15].I += 4;
        //ARM_PREFETCH;
         
      }
 
    }
    break;
  case 0x17d:
    {
      // LDRSB Rd, [Rn, -#offset]!
 
      int base = (opcode >> 16) & 0x0F;
      int dest = (opcode >> 12) & 0x0F;
      u32 address = myregs[base].I - ((opcode & 0x0F)|((opcode>>4)&0xF0));
      myregs[dest].I = (s8)CPUReadByte(address);
      if(dest != base)
        myregs[base].I = address;
       
      if(dest == 15) {
        myregs[15].I &= 0xFFFFFFFC;
        armNextPC = myregs[15].I;
        myregs[15].I += 4;
        //ARM_PREFETCH;
         
      }
 
    }
    break;
  case 0x19d:
    {
      // LDRSB Rd, [Rn, Rm]
 
      int base = (opcode >> 16) & 0x0F;
      int dest = (opcode >> 12) & 0x0F;
      u32 address = myregs[base].I + myregs[opcode & 0x0F].I;
      myregs[dest].I = (s8)CPUReadByte(address);
       
      if(dest == 15) {
        myregs[15].I &= 0xFFFFFFFC;
        armNextPC = myregs[15].I;
        myregs[15].I += 4;
        //ARM_PREFETCH;
         
      }
 
    }
    break;
  case 0x1bd:
    {
      // LDRSB Rd, [Rn, Rm]!
 
      int base = (opcode >> 16) & 0x0F;
      int dest = (opcode >> 12) & 0x0F;
      u32 address = myregs[base].I + myregs[opcode & 0x0F].I;
      myregs[dest].I = (s8)CPUReadByte(address);
      if(dest != base)
        myregs[base].I = address;
       
      if(dest == 15) {
        myregs[15].I &= 0xFFFFFFFC;
        armNextPC = myregs[15].I;
        myregs[15].I += 4;
        //ARM_PREFETCH;
         
      }
 
    }
    break;
  case 0x1dd:
    {
      // LDRSB Rd, [Rn, #offset]
 
      int base = (opcode >> 16) & 0x0F;
      int dest = (opcode >> 12) & 0x0F;
      u32 address = myregs[base].I + ((opcode & 0x0F)|((opcode>>4)&0xF0));
      myregs[dest].I = (s8)CPUReadByte(address);
       
      if(dest == 15) {
        myregs[15].I &= 0xFFFFFFFC;
        armNextPC = myregs[15].I;
        myregs[15].I += 4;
        //ARM_PREFETCH;
         
      }
 
    }
    break;
  case 0x1fd:
    {
      // LDRSB Rd, [Rn, #offset]!
 
      int base = (opcode >> 16) & 0x0F;
      int dest = (opcode >> 12) & 0x0F;
      u32 address = myregs[base].I + ((opcode & 0x0F)|((opcode>>4)&0xF0));
      myregs[dest].I = (s8)CPUReadByte(address);
      if(dest != base)
        myregs[base].I = address;
       
      if(dest == 15) {
        myregs[15].I &= 0xFFFFFFFC;
        armNextPC = myregs[15].I;
        myregs[15].I += 4;
        //ARM_PREFETCH;
         
      }
 
    }
    break;
  case 0x01f:
  case 0x03f:
    {
      // LDRSH Rd, [Rn], -Rm
 
      int base = (opcode >> 16) & 0x0F;
      int dest = (opcode >> 12) & 0x0F;
      u32 address = myregs[base].I;
      int offset = myregs[opcode & 0x0F].I;
      myregs[dest].I = (s16)CPUReadHalfWordSigned(address);
      if(dest != base) {
        address -= offset;
        myregs[base].I = address;
      }
       
      if(dest == 15) {
        myregs[15].I &= 0xFFFFFFFC;
        armNextPC = myregs[15].I;
        myregs[15].I += 4;
        //ARM_PREFETCH;
         
      }
 
    }
    break;
  case 0x05f:
  case 0x07f:
    {
      // LDRSH Rd, [Rn], #-offset
 
      int base = (opcode >> 16) & 0x0F;
      int dest = (opcode >> 12) & 0x0F;
      u32 address = myregs[base].I;
      int offset = (opcode & 0x0F) | ((opcode >> 4) & 0xF0);
      myregs[dest].I = (s16)CPUReadHalfWordSigned(address);
      if(dest != base) {
        address -= offset;
        myregs[base].I = address;
      }
       
      if(dest == 15) {
        myregs[15].I &= 0xFFFFFFFC;
        armNextPC = myregs[15].I;
        myregs[15].I += 4;
        //ARM_PREFETCH;
         
      }
 
    }
    break;
  case 0x09f:
  case 0x0bf:
    {
      // LDRSH Rd, [Rn], Rm
 
      int base = (opcode >> 16) & 0x0F;
      int dest = (opcode >> 12) & 0x0F;
      u32 address = myregs[base].I;
      int offset = myregs[opcode & 0x0F].I;
      myregs[dest].I = (s16)CPUReadHalfWordSigned(address);
      if(dest != base) {
        address += offset;
        myregs[base].I = address;
      }
       
      if(dest == 15) {
        myregs[15].I &= 0xFFFFFFFC;
        armNextPC = myregs[15].I;
        myregs[15].I += 4;
        //ARM_PREFETCH;
         
      }
 
    }
    break;
  case 0x0df:
  case 0x0ff:
    {
      // LDRSH Rd, [Rn], #offset
 
      int base = (opcode >> 16) & 0x0F;
      int dest = (opcode >> 12) & 0x0F;
      u32 address = myregs[base].I;
      int offset = (opcode & 0x0F) | ((opcode >> 4) & 0xF0);
      myregs[dest].I = (s16)CPUReadHalfWordSigned(address);
      if(dest != base) {
        address += offset;
        myregs[base].I = address;
      }
       
      if(dest == 15) {
        myregs[15].I &= 0xFFFFFFFC;
        armNextPC = myregs[15].I;
        myregs[15].I += 4;
        //ARM_PREFETCH;
         
      }
 
    }
    break;
  case 0x11f:
    {
      // LDRSH Rd, [Rn, -Rm]
 
      int base = (opcode >> 16) & 0x0F;
      int dest = (opcode >> 12) & 0x0F;
      u32 address = myregs[base].I - myregs[opcode & 0x0F].I;
      myregs[dest].I = (s16)CPUReadHalfWordSigned(address);
       
      if(dest == 15) {
        myregs[15].I &= 0xFFFFFFFC;
        armNextPC = myregs[15].I;
        myregs[15].I += 4;
        //ARM_PREFETCH;
         
      }
 
    }
    break;
  case 0x13f:
    {
      // LDRSH Rd, [Rn, -Rm]!
 
      int base = (opcode >> 16) & 0x0F;
      int dest = (opcode >> 12) & 0x0F;
      u32 address = myregs[base].I - myregs[opcode & 0x0F].I;
      myregs[dest].I = (s16)CPUReadHalfWordSigned(address);
      if(dest != base)
        myregs[base].I = address;
       
      if(dest == 15) {
        myregs[15].I &= 0xFFFFFFFC;
        armNextPC = myregs[15].I;
        myregs[15].I += 4;
        //ARM_PREFETCH;
         
      }
 
    }
    break;
  case 0x15f:
    {
      // LDRSH Rd, [Rn, -#offset]
 
      int base = (opcode >> 16) & 0x0F;
      int dest = (opcode >> 12) & 0x0F;
      u32 address = myregs[base].I - ((opcode & 0x0F)|((opcode>>4)&0xF0));
      myregs[dest].I = (s16)CPUReadHalfWordSigned(address);
       
      if(dest == 15) {
        myregs[15].I &= 0xFFFFFFFC;
        armNextPC = myregs[15].I;
        myregs[15].I += 4;
        //ARM_PREFETCH;
         
      }
 
    }
    break;
  case 0x17f:
    {
      // LDRSH Rd, [Rn, -#offset]!
 
      int base = (opcode >> 16) & 0x0F;
      int dest = (opcode >> 12) & 0x0F;
      u32 address = myregs[base].I - ((opcode & 0x0F)|((opcode>>4)&0xF0));
      myregs[dest].I = (s16)CPUReadHalfWordSigned(address);
      if(dest != base)
        myregs[base].I = address;
       
      if(dest == 15) {
        myregs[15].I &= 0xFFFFFFFC;
        armNextPC = myregs[15].I;
        myregs[15].I += 4;
        //ARM_PREFETCH;
         
      }
 
    }
    break;
  case 0x19f:
    {
      // LDRSH Rd, [Rn, Rm]
 
      int base = (opcode >> 16) & 0x0F;
      int dest = (opcode >> 12) & 0x0F;
      u32 address = myregs[base].I + myregs[opcode & 0x0F].I;
      myregs[dest].I = (s16)CPUReadHalfWordSigned(address);
       
      if(dest == 15) {
        myregs[15].I &= 0xFFFFFFFC;
        armNextPC = myregs[15].I;
        myregs[15].I += 4;
        //ARM_PREFETCH;
         
      }
 
    }
    break;
  case 0x1bf:
    {
      // LDRSH Rd, [Rn, Rm]!
 
      int base = (opcode >> 16) & 0x0F;
      int dest = (opcode >> 12) & 0x0F;
      u32 address = myregs[base].I + myregs[opcode & 0x0F].I;
      myregs[dest].I = (s16)CPUReadHalfWordSigned(address);
      if(dest != base)
        myregs[base].I = address;
       
      if(dest == 15) {
        myregs[15].I &= 0xFFFFFFFC;
        armNextPC = myregs[15].I;
        myregs[15].I += 4;
        //ARM_PREFETCH;
         
      }
 
    }
    break;
  case 0x1df:
    {
      // LDRSH Rd, [Rn, #offset]
 
      int base = (opcode >> 16) & 0x0F;
      int dest = (opcode >> 12) & 0x0F;
      u32 address = myregs[base].I + ((opcode & 0x0F)|((opcode>>4)&0xF0));
      myregs[dest].I = (s16)CPUReadHalfWordSigned(address);
       
      if(dest == 15) {
        myregs[15].I &= 0xFFFFFFFC;
        armNextPC = myregs[15].I;
        myregs[15].I += 4;
        //ARM_PREFETCH;
         
      }
 
    }
    break;
  case 0x1ff:
    {
      // LDRSH Rd, [Rn, #offset]!
 
      int base = (opcode >> 16) & 0x0F;
      int dest = (opcode >> 12) & 0x0F;
      u32 address = myregs[base].I + ((opcode & 0x0F)|((opcode>>4)&0xF0));
      myregs[dest].I = (s16)CPUReadHalfWordSigned(address);
      if(dest != base)
        myregs[base].I = address;
       
      if(dest == 15) {
        myregs[15].I &= 0xFFFFFFFC;
        armNextPC = myregs[15].I;
        myregs[15].I += 4;
        //ARM_PREFETCH;
         
      }
 
    }
    break;
    LOGICAL_DATA_OPCODE_WITHOUT_base(OP_EOR,  OP_EOR, 0x020);
    LOGICAL_DATA_OPCODE_WITHOUT_base(OP_EORS, OP_EOR, 0x030);
  case 0x029:
    {
      // MLA Rd, Rm, Rs, Rn
 
      int dest = (opcode >> 16) & 0x0F;
      int mult = (opcode & 0x0F);
      u32 rs = myregs[(opcode >> 8) & 0x0F].I;
      myregs[dest].I = myregs[mult].I * rs + myregs[(opcode>>12)&0x0f].I;
    }
    break;
  case 0x039:
    {
      // MLAS Rd, Rm, Rs, Rn
 
      int dest = (opcode >> 16) & 0x0F;
      int mult = (opcode & 0x0F);
      u32 rs = myregs[(opcode >> 8) & 0x0F].I;
      myregs[dest].I = myregs[mult].I * rs + myregs[(opcode>>12)&0x0f].I;
      N_FLAG = (myregs[dest].I & 0x80000000) ? true : false;
      Z_FLAG = (myregs[dest].I) ? false : true;
      if(((s32)rs)<0)
        rs = ~rs;

    }
    break;
    ARITHMETIC_DATA_OPCODE(OP_SUB,  OP_SUB, 0x040);
    ARITHMETIC_DATA_OPCODE(OP_SUBS, OP_SUB, 0x050);
    ARITHMETIC_DATA_OPCODE(OP_RSB,  OP_RSB, 0x060);
    ARITHMETIC_DATA_OPCODE(OP_RSBS, OP_RSB, 0x070);
    ARITHMETIC_DATA_OPCODE(OP_ADD,  OP_ADD, 0x080);
    ARITHMETIC_DATA_OPCODE(OP_ADDS, OP_ADD, 0x090);
  case 0x089:
    {
      // UMULL RdLo, RdHi, Rn, Rs
 
      u32 umult = myregs[(opcode & 0x0F)].I;
      u32 usource = myregs[(opcode >> 8) & 0x0F].I;
      int destLo = (opcode >> 12) & 0x0F;         
      int destHi = (opcode >> 16) & 0x0F;
      u64 uTemp = ((u64)umult)*((u64)usource);
      myregs[destLo].I = (u32)uTemp;
      myregs[destHi].I = (u32)(uTemp >> 32);
 
    }
    break;
  case 0x099:
    {
      // UMULLS RdLo, RdHi, Rn, Rs
 
      u32 umult = myregs[(opcode & 0x0F)].I;
      u32 usource = myregs[(opcode >> 8) & 0x0F].I;
      int destLo = (opcode >> 12) & 0x0F;         
      int destHi = (opcode >> 16) & 0x0F;
      u64 uTemp = ((u64)umult)*((u64)usource);
      myregs[destLo].I = (u32)uTemp;
      myregs[destHi].I = (u32)(uTemp >> 32);
      Z_FLAG = (uTemp) ? false : true;
      N_FLAG = (myregs[destHi].I & 0x80000000) ? true : false;

    }
    break;
    ARITHMETIC_DATA_OPCODE(OP_ADC,  OP_ADC, 0x0a0);
    ARITHMETIC_DATA_OPCODE(OP_ADCS, OP_ADC, 0x0b0);
  case 0x0a9:
    {
      // UMLAL RdLo, RdHi, Rn, Rs
 
      u32 umult = myregs[(opcode & 0x0F)].I;
      u32 usource = myregs[(opcode >> 8) & 0x0F].I;
      int destLo = (opcode >> 12) & 0x0F;         
      int destHi = (opcode >> 16) & 0x0F;
      u64 uTemp = (u64)myregs[destHi].I;
      uTemp <<= 32;
      uTemp |= (u64)myregs[destLo].I;
      uTemp += ((u64)umult)*((u64)usource);
      myregs[destLo].I = (u32)uTemp;
      myregs[destHi].I = (u32)(uTemp >> 32);

    }
    break;
  case 0x0b9:
    {
      // UMLALS RdLo, RdHi, Rn, Rs
 
      u32 umult = myregs[(opcode & 0x0F)].I;
      u32 usource = myregs[(opcode >> 8) & 0x0F].I;
      int destLo = (opcode >> 12) & 0x0F;         
      int destHi = (opcode >> 16) & 0x0F;
      u64 uTemp = (u64)myregs[destHi].I;
      uTemp <<= 32;
      uTemp |= (u64)myregs[destLo].I;
      uTemp += ((u64)umult)*((u64)usource);
      myregs[destLo].I = (u32)uTemp;
      myregs[destHi].I = (u32)(uTemp >> 32);
      Z_FLAG = (uTemp) ? false : true;
      N_FLAG = (myregs[destHi].I & 0x80000000) ? true : false;
    }
    break;
    ARITHMETIC_DATA_OPCODE(OP_SBC,  OP_SBC, 0x0c0);
    ARITHMETIC_DATA_OPCODE(OP_SBCS, OP_SBC, 0x0d0);
  case 0x0c9:
    {
      // SMULL RdLo, RdHi, Rm, Rs
      int destLo = (opcode >> 12) & 0x0F;         
      int destHi = (opcode >> 16) & 0x0F;
      u32 rs = myregs[(opcode >> 8) & 0x0F].I;
      s64 m = (s32)myregs[(opcode & 0x0F)].I;
      s64 s = (s32)rs;
      s64 sTemp = m*s;
      myregs[destLo].I = (u32)sTemp;
      myregs[destHi].I = (u32)(sTemp >> 32);
      if(((s32)rs) < 0)
        rs = ~rs;
    }
    break;
  case 0x0d9:
    {
      // SMULLS RdLo, RdHi, Rm, Rs
      int destLo = (opcode >> 12) & 0x0F;         
      int destHi = (opcode >> 16) & 0x0F;
      u32 rs = myregs[(opcode >> 8) & 0x0F].I;
      s64 m = (s32)myregs[(opcode & 0x0F)].I;
      s64 s = (s32)rs;
      s64 sTemp = m*s;
      myregs[destLo].I = (u32)sTemp;
      myregs[destHi].I = (u32)(sTemp >> 32);
      Z_FLAG = (sTemp) ? false : true;
      N_FLAG = (sTemp < 0) ? true : false;
      if(((s32)rs) < 0)
        rs = ~rs;
    }
    break;
    ARITHMETIC_DATA_OPCODE(OP_RSC,  OP_RSC, 0x0e0);
    ARITHMETIC_DATA_OPCODE(OP_RSCS, OP_RSC, 0x0f0);
  case 0x0e9:
    {
      // SMLAL RdLo, RdHi, Rm, Rs
      int destLo = (opcode >> 12) & 0x0F;         
      int destHi = (opcode >> 16) & 0x0F;
      u32 rs = myregs[(opcode >> 8) & 0x0F].I;
      s64 m = (s32)myregs[(opcode & 0x0F)].I;
      s64 s = (s32)rs;
      s64 sTemp = (u64)myregs[destHi].I;
      sTemp <<= 32;
      sTemp |= (u64)myregs[destLo].I;
      sTemp += m*s;
      myregs[destLo].I = (u32)sTemp;
      myregs[destHi].I = (u32)(sTemp >> 32);
      if(((s32)rs) < 0)
        rs = ~rs;
    }
    break;
  case 0x0f9:
    {
      // SMLALS RdLo, RdHi, Rm, Rs
      int destLo = (opcode >> 12) & 0x0F;         
      int destHi = (opcode >> 16) & 0x0F;
      u32 rs = myregs[(opcode >> 8) & 0x0F].I;
      s64 m = (s32)myregs[(opcode & 0x0F)].I;
      s64 s = (s32)rs;
      s64 sTemp = (u64)myregs[destHi].I;
      sTemp <<= 32;
      sTemp |= (u64)myregs[destLo].I;
      sTemp += m*s;
      myregs[destLo].I = (u32)sTemp;
      myregs[destHi].I = (u32)(sTemp >> 32);
      Z_FLAG = (sTemp) ? false : true;
      N_FLAG = (sTemp < 0) ? true : false;
      if(((s32)rs) < 0)
        rs = ~rs;
    }
    break;
    LOGICAL_DATA_OPCODE(OP_TST, OP_TST, 0x110);
  case 0x100:
    // MRS Rd, CPSR
    // TODO: check if right instruction....
    //CPUUpdateCPSR();
    myregs[(opcode >> 12) & 0x0F].I = myregs[16].I;
    break;
  case 0x109:
    {
      // SWP Rd, Rm, [Rn]
      u32 address = myregs[(opcode >> 16) & 15].I;
      u32 temp = CPUReadMemory(address);
      CPUWriteMemory(address, myregs[opcode&15].I);
      myregs[(opcode >> 12) & 15].I = temp;
 
    }
    break;
    LOGICAL_DATA_OPCODE(OP_TEQ, OP_TEQ, 0x130);
  case 0x120:
    {
      // MSR CPSR_fields, Rm
      //CPUUpdateCPSR();
      u32 value = myregs[opcode & 15].I;
      u32 newValue = myregs[16].I;
      if(armMode > 0x10) {
        if(opcode & 0x00010000)
          newValue = (newValue & 0xFFFFFF00) | (value & 0x000000FF);
        if(opcode & 0x00020000)
          newValue = (newValue & 0xFFFF00FF) | (value & 0x0000FF00);
        if(opcode & 0x00040000)
          newValue = (newValue & 0xFF00FFFF) | (value & 0x00FF0000);
      }
      if(opcode & 0x00080000)
        newValue = (newValue & 0x00FFFFFF) | (value & 0xFF000000);
      newValue |= 0x10;
      //CPUSwitchMode(newValue & 0x1f, false);
      myregs[16].I = newValue;
      ////CPUUpdateFlags();
      if(!armState) { // this should not be allowed, but it seems to work
        //THUMB_PREFETCH;
        myregs[15].I = armNextPC + 2;
      }
    }
    break;
  case 0x121:
    {
      // BX Rm
      // TODO: check if right instruction...
      int base = opcode & 0x0F;
      armState = myregs[base].I & 1 ? false : true;
      if(armState) {
        myregs[15].I = myregs[base].I & 0xFFFFFFFC;
        armNextPC = myregs[15].I;
        myregs[15].I += 4;
        //ARM_PREFETCH;
      } else {
        myregs[15].I = myregs[base].I & 0xFFFFFFFE;
        armNextPC = myregs[15].I;
        myregs[15].I += 2;
        //THUMB_PREFETCH;
      }
    }
    break;
    ARITHMETIC_DATA_OPCODE(OP_CMP, OP_CMP, 0x150);
  case 0x140:
    // MRS Rd, SPSR
    // TODO: check if right instruction...
    myregs[(opcode >> 12) & 0x0F].I = myregs[17].I;
    break;
  case 0x149:
    {
      // SWPB Rd, Rm, [Rn]
      u32 address = myregs[(opcode >> 16) & 15].I;
      u32 temp = CPUReadByte(address);
      CPUWriteByte(address, myregs[opcode&15].B.B0);
      myregs[(opcode>>12)&15].I = temp;
 
    }
    break;
    ARITHMETIC_DATA_OPCODE(OP_CMN, OP_CMN, 0x170);
  case 0x160:
    {
      // MSR SPSR_fields, Rm
      u32 value = myregs[opcode & 15].I;
      if(armMode > 0x10 && armMode < 0x1f) {
        if(opcode & 0x00010000)
          myregs[17].I = (myregs[17].I & 0xFFFFFF00) | (value & 0x000000FF);
        if(opcode & 0x00020000)
          myregs[17].I = (myregs[17].I & 0xFFFF00FF) | (value & 0x0000FF00);
        if(opcode & 0x00040000)
          myregs[17].I = (myregs[17].I & 0xFF00FFFF) | (value & 0x00FF0000);
        if(opcode & 0x00080000)
          myregs[17].I = (myregs[17].I & 0x00FFFFFF) | (value & 0xFF000000);
      }
    }
    break;
    LOGICAL_DATA_OPCODE             (OP_ORR,  OP_ORR, 0x180);
    LOGICAL_DATA_OPCODE             (OP_ORRS, OP_ORR, 0x190);
    LOGICAL_DATA_OPCODE_WITHOUT_base(OP_MOV,  OP_MOV, 0x1a0);
    LOGICAL_DATA_OPCODE_WITHOUT_base(OP_MOVS, OP_MOV, 0x1b0);
    LOGICAL_DATA_OPCODE             (OP_BIC,  OP_BIC, 0x1c0);
    LOGICAL_DATA_OPCODE             (OP_BICS, OP_BIC, 0x1d0);
    LOGICAL_DATA_OPCODE_WITHOUT_base(OP_MVN,  OP_MVN, 0x1e0);
    LOGICAL_DATA_OPCODE_WITHOUT_base(OP_MVNS, OP_MVN, 0x1f0);
#ifdef BKPT_SUPPORT
  case 0x127:
  case 0x7ff: // for GDB support
    extern void (*dbgSignal)(int,int);
    myregs[15].I -= 4;
    armNextPC -= 4;
    dbgSignal(5, (opcode & 0x0f)|((opcode>>4) & 0xfff0));
    return;
#endif
  case 0x320:
  case 0x321:
  case 0x322:
  case 0x323:
  case 0x324:
  case 0x325:
  case 0x326:
  case 0x327:
  case 0x328:
  case 0x329:
  case 0x32a:
  case 0x32b:
  case 0x32c:
  case 0x32d:
  case 0x32e:
  case 0x32f:
    {
      // MSR CPSR_fields, #
      //CPUUpdateCPSR();
      u32 value = opcode & 0xFF;
      int shift = (opcode & 0xF00) >> 7;
      if(shift) {
        ROR_IMM_MSR;
      }
      u32 newValue = myregs[16].I;
      if(armMode > 0x10) {
        if(opcode & 0x00010000)
          newValue = (newValue & 0xFFFFFF00) | (value & 0x000000FF);
        if(opcode & 0x00020000)
          newValue = (newValue & 0xFFFF00FF) | (value & 0x0000FF00);
        if(opcode & 0x00040000)
          newValue = (newValue & 0xFF00FFFF) | (value & 0x00FF0000);
      }
      if(opcode & 0x00080000)
        newValue = (newValue & 0x00FFFFFF) | (value & 0xFF000000);

      newValue |= 0x10;

      //CPUSwitchMode(newValue & 0x1f, false);
      myregs[16].I = newValue;
      //CPUUpdateFlags();
      if(!armState) { // this should not be allowed, but it seems to work
        //THUMB_PREFETCH;
        myregs[15].I = armNextPC + 2;
      }
    }
    break;
  case 0x360:
  case 0x361:
  case 0x362:
  case 0x363:
  case 0x364:
  case 0x365:
  case 0x366:
  case 0x367:
  case 0x368:
  case 0x369:
  case 0x36a:
  case 0x36b:
  case 0x36c:
  case 0x36d:
  case 0x36e:
  case 0x36f:
    {
      // MSR SPSR_fields, #
      if(armMode > 0x10 && armMode < 0x1f) {
        u32 value = opcode & 0xFF;
        int shift = (opcode & 0xF00) >> 7;
        if(shift) {
          ROR_IMM_MSR;
        }
        if(opcode & 0x00010000)
          myregs[17].I = (myregs[17].I & 0xFFFFFF00) | (value & 0x000000FF);
        if(opcode & 0x00020000)
          myregs[17].I = (myregs[17].I & 0xFFFF00FF) | (value & 0x0000FF00);
        if(opcode & 0x00040000)
          myregs[17].I = (myregs[17].I & 0xFF00FFFF) | (value & 0x00FF0000);
        if(opcode & 0x00080000)
          myregs[17].I = (myregs[17].I & 0x00FFFFFF) | (value & 0xFF000000);
      }
    }
  break;
  CASE_16(0x400)
  // T versions shouldn't be different on GBA      
  CASE_16(0x420)
    {
      // STR Rd, [Rn], -#
 
      int offset = opcode & 0xFFF;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = myregs[base].I;
      CPUWriteMemory(address, myregs[dest].I);
      myregs[base].I = address - offset;
 
    }
    break;
  CASE_16(0x480)
    // T versions shouldn't be different on GBA
  CASE_16(0x4a0)
    {
      // STR Rd, [Rn], #
 
      int offset = opcode & 0xFFF;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = myregs[base].I;
      CPUWriteMemory(address, myregs[dest].I);
      myregs[base].I = address + offset;
 
    }
    break;
  CASE_16(0x500)
    {
      // STR Rd, [Rn, -#]
 
      int offset = opcode & 0xFFF;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = myregs[base].I - offset;
      CPUWriteMemory(address, myregs[dest].I);
 
    }
    break;
  CASE_16(0x520)
    {
      // STR Rd, [Rn, -#]!
 
      int offset = opcode & 0xFFF;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = myregs[base].I - offset;
      myregs[base].I = address;
      CPUWriteMemory(address, myregs[dest].I);
 
    }
    break;
  CASE_16(0x580)
    {
      // STR Rd, [Rn, #]
 
      int offset = opcode & 0xFFF;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = myregs[base].I + offset;
      CPUWriteMemory(address, myregs[dest].I);
 
    }
    break;
  CASE_16(0x5a0)
    {
      // STR Rd, [Rn, #]!
 
      int offset = opcode & 0xFFF;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = myregs[base].I + offset;
      myregs[base].I = address;
      CPUWriteMemory(address, myregs[dest].I);
 
    }
    break;
  CASE_16(0x410)
    {
      // LDR Rd, [Rn], -#
 
      int offset = opcode & 0xFFF;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = myregs[base].I;
      myregs[dest].I = CPUReadMemory(address);
      if(dest != base)
        myregs[base].I -= offset;
       
      if(dest == 15) {
        myregs[15].I &= 0xFFFFFFFC;
        armNextPC = myregs[15].I;
        myregs[15].I += 4;
        //ARM_PREFETCH;
         
      }     }
    break;
  CASE_16(0x430)
    {
      // LDRT Rd, [Rn], -#
 
      int offset = opcode & 0xFFF;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = myregs[base].I;
      myregs[dest].I = CPUReadMemory(address);
      if(dest != base)
        myregs[base].I -= offset;
       
      if(dest == 15) {
        myregs[15].I &= 0xFFFFFFFC;
        armNextPC = myregs[15].I;
        myregs[15].I += 4;
        //ARM_PREFETCH;
         
      }     }
    break;
  CASE_16(0x490)
    {
      // LDR Rd, [Rn], #
 
      int offset = opcode & 0xFFF;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = myregs[base].I;
      myregs[dest].I = CPUReadMemory(address);
      if(dest != base)
        myregs[base].I += offset;
       
      if(dest == 15) {
        myregs[15].I &= 0xFFFFFFFC;
        armNextPC = myregs[15].I;
        myregs[15].I += 4;
        //ARM_PREFETCH;
         
      }     }
    break;
  CASE_16(0x4b0)
    {
      // LDRT Rd, [Rn], #
 
      int offset = opcode & 0xFFF;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = myregs[base].I;
      myregs[dest].I = CPUReadMemory(address);
      if(dest != base)
        myregs[base].I += offset;
       
      if(dest == 15) {
        myregs[15].I &= 0xFFFFFFFC;
        armNextPC = myregs[15].I;
        myregs[15].I += 4;
        //ARM_PREFETCH;
         
      }     }
    break;
  CASE_16(0x510)
    {
      // LDR Rd, [Rn, -#]
 
      int offset = opcode & 0xFFF;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = myregs[base].I - offset;
      myregs[dest].I = CPUReadMemory(address);
       
      if(dest == 15) {
        myregs[15].I &= 0xFFFFFFFC;
        armNextPC = myregs[15].I;
        myregs[15].I += 4;
        //ARM_PREFETCH;
         
      }     }
    break;
  CASE_16(0x530)
    {
      // LDR Rd, [Rn, -#]!
 
      int offset = opcode & 0xFFF;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = myregs[base].I - offset;
      myregs[dest].I = CPUReadMemory(address);
      if(dest != base)
        myregs[base].I = address;
       
      if(dest == 15) {
        myregs[15].I &= 0xFFFFFFFC;
        armNextPC = myregs[15].I;
        myregs[15].I += 4;
        //ARM_PREFETCH;
         
      }   }
    break;
  CASE_16(0x590)
    {
      // LDR Rd, [Rn, #]
 
      int offset = opcode & 0xFFF;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = myregs[base].I + offset;
      myregs[dest].I = CPUReadMemory(address);
       
      if(dest == 15) {
        myregs[15].I &= 0xFFFFFFFC;
        armNextPC = myregs[15].I;
        myregs[15].I += 4;
        //ARM_PREFETCH;
         
      }     }
    break;
  CASE_16(0x5b0)
    {
      // LDR Rd, [Rn, #]!
 
      int offset = opcode & 0xFFF;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = myregs[base].I + offset;
      myregs[dest].I = CPUReadMemory(address);
      if(dest != base)
        myregs[base].I = address;
       
      if(dest == 15) {
        myregs[15].I &= 0xFFFFFFFC;
        armNextPC = myregs[15].I;
        myregs[15].I += 4;
        //ARM_PREFETCH;
         
      }     }
    break;
  CASE_16(0x440)
    // T versions shouldn't be different on GBA      
  CASE_16(0x460)
    {
      // STRB Rd, [Rn], -#
 
      int offset = opcode & 0xFFF;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = myregs[base].I;
      CPUWriteByte(address, myregs[dest].B.B0);
      myregs[base].I = address - offset;
 
    }
    break;
  CASE_16(0x4c0)
    // T versions shouldn't be different on GBA
  CASE_16(0x4e0)
    {
      // STRB Rd, [Rn], #
 
      int offset = opcode & 0xFFF;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = myregs[base].I;
      CPUWriteByte(address, myregs[dest].B.B0);
      myregs[base].I = address + offset;
 
    }
    break;
  CASE_16(0x540)
    {
      // STRB Rd, [Rn, -#]
 
      int offset = opcode & 0xFFF;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = myregs[base].I - offset;
      CPUWriteByte(address, myregs[dest].B.B0);
 
    }
    break;
  CASE_16(0x560)
    {
      // STRB Rd, [Rn, -#]!
 
      int offset = opcode & 0xFFF;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = myregs[base].I - offset;
      myregs[base].I = address;
      CPUWriteByte(address, myregs[dest].B.B0);
 
    }
    break;
  CASE_16(0x5c0)
    {
      // STRB Rd, [Rn, #]
 
      int offset = opcode & 0xFFF;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = myregs[base].I + offset;
      CPUWriteByte(address, myregs[dest].B.B0);
 
    }
    break;
  CASE_16(0x5e0)
    {
      // STRB Rd, [Rn, #]!
 
      int offset = opcode & 0xFFF;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = myregs[base].I + offset;
      myregs[base].I = address;
      CPUWriteByte(address, myregs[dest].I);
 
    }
    break;
  CASE_16(0x450)
    // T versions shouldn't be different
  CASE_16(0x470)
    {
      // LDRB Rd, [Rn], -#
 
      int offset = opcode & 0xFFF;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = myregs[base].I;
      myregs[dest].I = CPUReadByte(address);
      if(dest != base)
        myregs[base].I -= offset;
       
      if(dest == 15) {
        myregs[15].I &= 0xFFFFFFFC;
        armNextPC = myregs[15].I;
        myregs[15].I += 4;
        //ARM_PREFETCH;
         
      }
 
    }
    break;
  CASE_16(0x4d0)
  CASE_16(0x4f0) // T versions should not be different
    {
      // LDRB Rd, [Rn], #
 
      int offset = opcode & 0xFFF;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = myregs[base].I;
      myregs[dest].I = CPUReadByte(address);
      if(dest != base)
        myregs[base].I += offset;
       
      if(dest == 15) {
        myregs[15].I &= 0xFFFFFFFC;
        armNextPC = myregs[15].I;
        myregs[15].I += 4;
        //ARM_PREFETCH;
         
      }
 
    }
    break;
  CASE_16(0x550)
    {
      // LDRB Rd, [Rn, -#]
 
      int offset = opcode & 0xFFF;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = myregs[base].I - offset;
      myregs[dest].I = CPUReadByte(address);
       
      if(dest == 15) {
        myregs[15].I &= 0xFFFFFFFC;
        armNextPC = myregs[15].I;
        myregs[15].I += 4;
        //ARM_PREFETCH;
         
      }
 
    }
    break;
  CASE_16(0x570)
    {
      // LDRB Rd, [Rn, -#]!
 
      int offset = opcode & 0xFFF;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = myregs[base].I - offset;
      myregs[dest].I = CPUReadByte(address);
      if(dest != base)
        myregs[base].I = address;
       
      if(dest == 15) {
        myregs[15].I &= 0xFFFFFFFC;
        armNextPC = myregs[15].I;
        myregs[15].I += 4;
        //ARM_PREFETCH;
         
      }
 
    }
    break;
  CASE_16(0x5d0)
    {
      // LDRB Rd, [Rn, #]
 
      int offset = opcode & 0xFFF;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = myregs[base].I + offset;
      myregs[dest].I = CPUReadByte(address);
       
      if(dest == 15) {
        myregs[15].I &= 0xFFFFFFFC;
        armNextPC = myregs[15].I;
        myregs[15].I += 4;
        //ARM_PREFETCH;
         
      }
 
    }
    break;
  CASE_16(0x5f0)
    {
      // LDRB Rd, [Rn, #]!
 
      int offset = opcode & 0xFFF;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = myregs[base].I + offset;
      myregs[dest].I = CPUReadByte(address);
      if(dest != base)
        myregs[base].I = address;
       
      if(dest == 15) {
        myregs[15].I &= 0xFFFFFFFC;
        armNextPC = myregs[15].I;
        myregs[15].I += 4;
        //ARM_PREFETCH;
         
      }
 
    }
    break;
  case 0x600:
  case 0x608:
    // T versions are the same
  case 0x620:
  case 0x628:
    {
      // STR Rd, [Rn], -Rm, LSL #
 
      int offset = myregs[opcode & 15].I << ((opcode>>7)& 31);
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = myregs[base].I;
      CPUWriteMemory(address, myregs[dest].I);
      myregs[base].I = address - offset;
 
    }
    break;
  case 0x602:
  case 0x60a:
    // T versions are the same
  case 0x622:
  case 0x62a:
    {
      // STR Rd, [Rn], -Rm, LSR #
 
      int shift = (opcode >> 7) & 31;
      int offset = shift ? myregs[opcode & 15].I >> shift : 0;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = myregs[base].I;
      CPUWriteMemory(address, myregs[dest].I);
      myregs[base].I = address - offset;
 
    }
    break;
  case 0x604:
  case 0x60c:
    // T versions are the same
  case 0x624:
  case 0x62c:
    {
      // STR Rd, [Rn], -Rm, ASR #
 
      int shift = (opcode >> 7) & 31;
      int offset;
      if(shift)
        offset = (int)((s32)myregs[opcode & 15].I >> shift);
      else if(myregs[opcode & 15].I & 0x80000000)
        offset = 0xFFFFFFFF;
      else
        offset = 0;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = myregs[base].I;
      CPUWriteMemory(address, myregs[dest].I);
      myregs[base].I = address - offset;
 
    }
    break;
  case 0x606:
  case 0x60e:
    // T versions are the same
  case 0x626:
  case 0x62e:
    {
      // STR Rd, [Rn], -Rm, ROR #
 
      int shift = (opcode >> 7) & 31;
      u32 value = myregs[opcode & 15].I;
      if(shift) {
        ROR_VALUE;
      } else {
        RCR_VALUE;
      }
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = myregs[base].I;
      CPUWriteMemory(address, myregs[dest].I);
      myregs[base].I = address - value;
 
    }
    break;
  case 0x680:
  case 0x688:
    // T versions are the same
  case 0x6a0:
  case 0x6a8:
    {
      // STR Rd, [Rn], Rm, LSL #
 
      int offset = myregs[opcode & 15].I << ((opcode>>7)& 31);
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = myregs[base].I;
      CPUWriteMemory(address, myregs[dest].I);
      myregs[base].I = address + offset;
 
    }
    break;
  case 0x682:
  case 0x68a:
    // T versions are the same
  case 0x6a2:
  case 0x6aa:
    {
      // STR Rd, [Rn], Rm, LSR #
 
      int shift = (opcode >> 7) & 31;
      int offset = shift ? myregs[opcode & 15].I >> shift : 0;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = myregs[base].I;
      CPUWriteMemory(address, myregs[dest].I);
      myregs[base].I = address + offset;
 
    }
    break;
  case 0x684:
  case 0x68c:
    // T versions are the same
  case 0x6a4:
  case 0x6ac:
    {
      // STR Rd, [Rn], Rm, ASR #
 
      int shift = (opcode >> 7) & 31;
      int offset;
      if(shift)
        offset = (int)((s32)myregs[opcode & 15].I >> shift);
      else if(myregs[opcode & 15].I & 0x80000000)
        offset = 0xFFFFFFFF;
      else
        offset = 0;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = myregs[base].I;
      CPUWriteMemory(address, myregs[dest].I);
      myregs[base].I = address + offset;
 
    }
    break;
  case 0x686:
  case 0x68e:
    // T versions are the same
  case 0x6a6:
  case 0x6ae:
    {
      // STR Rd, [Rn], Rm, ROR #
 
      int shift = (opcode >> 7) & 31;
      u32 value = myregs[opcode & 15].I;
      if(shift) {
        ROR_VALUE;
      } else {
        RCR_VALUE;
      }
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = myregs[base].I;
      CPUWriteMemory(address, myregs[dest].I);
      myregs[base].I = address + value;
 
    }
    break;
  case 0x700:
  case 0x708:
    {
      // STR Rd, [Rn, -Rm, LSL #]
 
      int offset = myregs[opcode & 15].I << ((opcode>>7)& 31);
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = myregs[base].I - offset;
      CPUWriteMemory(address, myregs[dest].I);
 
    }
    break;
  case 0x702:
  case 0x70a:
    {
      // STR Rd, [Rn, -Rm, LSR #]
 
      int shift = (opcode >> 7) & 31;
      int offset = shift ? myregs[opcode & 15].I >> shift : 0;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = myregs[base].I - offset;
      CPUWriteMemory(address, myregs[dest].I);
 
    }
    break;
  case 0x704:
  case 0x70c:
    {
      // STR Rd, [Rn, -Rm, ASR #]
 
      int shift = (opcode >> 7) & 31;
      int offset;
      if(shift)
        offset = (int)((s32)myregs[opcode & 15].I >> shift);
      else if(myregs[opcode & 15].I & 0x80000000)
        offset = 0xFFFFFFFF;
      else
        offset = 0;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = myregs[base].I - offset;
      CPUWriteMemory(address, myregs[dest].I);
 
    }
    break;
  case 0x706:
  case 0x70e:
    {
      // STR Rd, [Rn, -Rm, ROR #]
 
      int shift = (opcode >> 7) & 31;
      u32 value = myregs[opcode & 15].I;
      if(shift) {
        ROR_VALUE;
      } else {
        RCR_VALUE;
      }
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = myregs[base].I - value;
      CPUWriteMemory(address, myregs[dest].I);
 
    }
    break;
  case 0x720:
  case 0x728:
    {
      // STR Rd, [Rn, -Rm, LSL #]!
 
      int offset = myregs[opcode & 15].I << ((opcode>>7)& 31);
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = myregs[base].I - offset;
      myregs[base].I = address;
      CPUWriteMemory(address, myregs[dest].I);
 
    }
    break;
  case 0x722:
  case 0x72a:
    {
      // STR Rd, [Rn, -Rm, LSR #]!
 
      int shift = (opcode >> 7) & 31;
      int offset = shift ? myregs[opcode & 15].I >> shift : 0;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = myregs[base].I - offset;
      myregs[base].I = address;      
      CPUWriteMemory(address, myregs[dest].I);
 
    }
    break;
  case 0x724:
  case 0x72c:
    {
      // STR Rd, [Rn, -Rm, ASR #]!
 
      int shift = (opcode >> 7) & 31;
      int offset;
      if(shift)
        offset = (int)((s32)myregs[opcode & 15].I >> shift);
      else if(myregs[opcode & 15].I & 0x80000000)
        offset = 0xFFFFFFFF;
      else
        offset = 0;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = myregs[base].I - offset;
      myregs[base].I = address;
      CPUWriteMemory(address, myregs[dest].I);
 
    }
    break;
  case 0x726:
  case 0x72e:
    {
      // STR Rd, [Rn, -Rm, ROR #]!
 
      int shift = (opcode >> 7) & 31;
      u32 value = myregs[opcode & 15].I;
      if(shift) {
        ROR_VALUE;
      } else {
        RCR_VALUE;
      }
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = myregs[base].I - value;
      myregs[base].I = address;      
      CPUWriteMemory(address, myregs[dest].I);
 
    }
    break;
  case 0x780:
  case 0x788:
    {
      // STR Rd, [Rn, Rm, LSL #]
 
      int offset = myregs[opcode & 15].I << ((opcode>>7)& 31);
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = myregs[base].I + offset;
      CPUWriteMemory(address, myregs[dest].I);
 
    }
    break;
  case 0x782:
  case 0x78a:
    {
      // STR Rd, [Rn, Rm, LSR #]
 
      int shift = (opcode >> 7) & 31;
      int offset = shift ? myregs[opcode & 15].I >> shift : 0;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = myregs[base].I + offset;
      CPUWriteMemory(address, myregs[dest].I);
 
    }
    break;
  case 0x784:
  case 0x78c:
    {
      // STR Rd, [Rn, Rm, ASR #]
 
      int shift = (opcode >> 7) & 31;
      int offset;
      if(shift)
        offset = (int)((s32)myregs[opcode & 15].I >> shift);
      else if(myregs[opcode & 15].I & 0x80000000)
        offset = 0xFFFFFFFF;
      else
        offset = 0;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = myregs[base].I + offset;
      CPUWriteMemory(address, myregs[dest].I);
 
    }
    break;
  case 0x786:
  case 0x78e:
    {
      // STR Rd, [Rn, Rm, ROR #]
 
      int shift = (opcode >> 7) & 31;
      u32 value = myregs[opcode & 15].I;
      if(shift) {
        ROR_VALUE;
      } else {
        RCR_VALUE;
      }
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = myregs[base].I + value;
      CPUWriteMemory(address, myregs[dest].I);
 
    }
    break;
  case 0x7a0:
  case 0x7a8:
    {
      // STR Rd, [Rn, Rm, LSL #]!
 
      int offset = myregs[opcode & 15].I << ((opcode>>7)& 31);
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = myregs[base].I + offset;
      myregs[base].I = address;
      CPUWriteMemory(address, myregs[dest].I);
 
    }
    break;
  case 0x7a2:
  case 0x7aa:
    {
      // STR Rd, [Rn, Rm, LSR #]!
 
      int shift = (opcode >> 7) & 31;
      int offset = shift ? myregs[opcode & 15].I >> shift : 0;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = myregs[base].I + offset;
      myregs[base].I = address;      
      CPUWriteMemory(address, myregs[dest].I);
 
    }
    break;
  case 0x7a4:
  case 0x7ac:
    {
      // STR Rd, [Rn, Rm, ASR #]!
 
      int shift = (opcode >> 7) & 31;
      int offset;
      if(shift)
        offset = (int)((s32)myregs[opcode & 15].I >> shift);
      else if(myregs[opcode & 15].I & 0x80000000)
        offset = 0xFFFFFFFF;
      else
        offset = 0;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = myregs[base].I + offset;
      myregs[base].I = address;
      CPUWriteMemory(address, myregs[dest].I);
 
    }
    break;
  case 0x7a6:
  case 0x7ae:
    {
      // STR Rd, [Rn, Rm, ROR #]!
 
      int shift = (opcode >> 7) & 31;
      u32 value = myregs[opcode & 15].I;
      if(shift) {
        ROR_VALUE;
      } else {
        RCR_VALUE;
      }
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = myregs[base].I + value;
      myregs[base].I = address;      
      CPUWriteMemory(address, myregs[dest].I);
 
    }
    break;
  case 0x610:
  case 0x618:
    // T versions are the same
  case 0x630:
  case 0x638:
    {
      // LDR Rd, [Rn], -Rm, LSL #
 
      int offset = myregs[opcode & 15].I << ((opcode>>7)& 31);
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = myregs[base].I;
      myregs[dest].I = CPUReadMemory(address);
      if(dest != base)
        myregs[base].I = address - offset;
       
      if(dest == 15) {
        myregs[15].I &= 0xFFFFFFFC;
        armNextPC = myregs[15].I;
        myregs[15].I += 4;
        //ARM_PREFETCH;
         
      }     }
    break;
  case 0x612:
  case 0x61a:
    // T versions are the same
  case 0x632:
  case 0x63a:
    {
      // LDR Rd, [Rn], -Rm, LSR #
 
      int shift = (opcode >> 7) & 31;
      int offset = shift ? myregs[opcode & 15].I >> shift : 0;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = myregs[base].I;
      myregs[dest].I = CPUReadMemory(address);
      if(dest != base)
        myregs[base].I = address - offset;
       
      if(dest == 15) {
        myregs[15].I &= 0xFFFFFFFC;
        armNextPC = myregs[15].I;
        myregs[15].I += 4;
        //ARM_PREFETCH;
         
      }     }
    break;
  case 0x614:
  case 0x61c:
    // T versions are the same
  case 0x634:
  case 0x63c:
    {
      // LDR Rd, [Rn], -Rm, ASR #
 
      int shift = (opcode >> 7) & 31;
      int offset;
      if(shift)
        offset = (int)((s32)myregs[opcode & 15].I >> shift);
      else if(myregs[opcode & 15].I & 0x80000000)
        offset = 0xFFFFFFFF;
      else
        offset = 0;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = myregs[base].I;
      myregs[dest].I = CPUReadMemory(address);
      if(dest != base)
        myregs[base].I = address - offset;
       
      if(dest == 15) {
        myregs[15].I &= 0xFFFFFFFC;
        armNextPC = myregs[15].I;
        myregs[15].I += 4;
        //ARM_PREFETCH;
         
      }     }
    break;
  case 0x616:
  case 0x61e:
    // T versions are the same
  case 0x636:
  case 0x63e:
    {
      // LDR Rd, [Rn], -Rm, ROR #
 
      int shift = (opcode >> 7) & 31;
      u32 value = myregs[opcode & 15].I;
      if(shift) {
        ROR_VALUE;
      } else {
        RCR_VALUE;
      }
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = myregs[base].I;
      myregs[dest].I = CPUReadMemory(address);
      if(dest != base)
        myregs[base].I = address - value;
       
      if(dest == 15) {
        myregs[15].I &= 0xFFFFFFFC;
        armNextPC = myregs[15].I;
        myregs[15].I += 4;
        //ARM_PREFETCH;
         
      }     }
    break;
  case 0x690:
  case 0x698:
    // T versions are the same
  case 0x6b0:
  case 0x6b8:
    {
      // LDR Rd, [Rn], Rm, LSL #
 
      int offset = myregs[opcode & 15].I << ((opcode>>7)& 31);
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = myregs[base].I;
      myregs[dest].I = CPUReadMemory(address);
      if(dest != base)
        myregs[base].I = address + offset;
       
      if(dest == 15) {
        myregs[15].I &= 0xFFFFFFFC;
        armNextPC = myregs[15].I;
        myregs[15].I += 4;
        //ARM_PREFETCH;
         
      }     }
    break;
  case 0x692:
  case 0x69a:
    // T versions are the same
  case 0x6b2:
  case 0x6ba:
    {
      // LDR Rd, [Rn], Rm, LSR #
 
      int shift = (opcode >> 7) & 31;
      int offset = shift ? myregs[opcode & 15].I >> shift : 0;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = myregs[base].I;
      myregs[dest].I = CPUReadMemory(address);
      if(dest != base)
        myregs[base].I = address + offset;
       
      if(dest == 15) {
        myregs[15].I &= 0xFFFFFFFC;
        armNextPC = myregs[15].I;
        myregs[15].I += 4;
        //ARM_PREFETCH;
         
      }     }
    break;
  case 0x694:
  case 0x69c:
    // T versions are the same
  case 0x6b4:
  case 0x6bc:
    {
      // LDR Rd, [Rn], Rm, ASR #
 
      int shift = (opcode >> 7) & 31;
      int offset;
      if(shift)
        offset = (int)((s32)myregs[opcode & 15].I >> shift);
      else if(myregs[opcode & 15].I & 0x80000000)
        offset = 0xFFFFFFFF;
      else
        offset = 0;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = myregs[base].I;
      myregs[dest].I = CPUReadMemory(address);
      if(dest != base)
        myregs[base].I = address + offset;
       
      if(dest == 15) {
        myregs[15].I &= 0xFFFFFFFC;
        armNextPC = myregs[15].I;
        myregs[15].I += 4;
        //ARM_PREFETCH;
         
      }     }
    break;
  case 0x696:
  case 0x69e:
    // T versions are the same
  case 0x6b6:
  case 0x6be:
    {
      // LDR Rd, [Rn], Rm, ROR #
 
      int shift = (opcode >> 7) & 31;
      u32 value = myregs[opcode & 15].I;
      if(shift) {
        ROR_VALUE;
      } else {
        RCR_VALUE;
      }
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = myregs[base].I;
      myregs[dest].I = CPUReadMemory(address);
      if(dest != base)
        myregs[base].I = address + value;
       
      if(dest == 15) {
        myregs[15].I &= 0xFFFFFFFC;
        armNextPC = myregs[15].I;
        myregs[15].I += 4;
        //ARM_PREFETCH;
         
      }     }
    break;
  case 0x710:
  case 0x718:
    {
      // LDR Rd, [Rn, -Rm, LSL #]
 
      int offset = myregs[opcode & 15].I << ((opcode>>7)& 31);
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = myregs[base].I - offset;
      myregs[dest].I = CPUReadMemory(address);
       
      if(dest == 15) {
        myregs[15].I &= 0xFFFFFFFC;
        armNextPC = myregs[15].I;
        myregs[15].I += 4;
        //ARM_PREFETCH;
         
      }     }
    break;
  case 0x712:
  case 0x71a:
    {
      // LDR Rd, [Rn, -Rm, LSR #]
 
      int shift = (opcode >> 7) & 31;
      int offset = shift ? myregs[opcode & 15].I >> shift : 0;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = myregs[base].I - offset;
      myregs[dest].I = CPUReadMemory(address);
       
      if(dest == 15) {
        myregs[15].I &= 0xFFFFFFFC;
        armNextPC = myregs[15].I;
        myregs[15].I += 4;
        //ARM_PREFETCH;
         
      }     }
    break;
  case 0x714:
  case 0x71c:
    {
      // LDR Rd, [Rn, -Rm, ASR #]
 
      int shift = (opcode >> 7) & 31;
      int offset;
      if(shift)
        offset = (int)((s32)myregs[opcode & 15].I >> shift);
      else if(myregs[opcode & 15].I & 0x80000000)
        offset = 0xFFFFFFFF;
      else
        offset = 0;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = myregs[base].I - offset;
      myregs[dest].I = CPUReadMemory(address);
       
      if(dest == 15) {
        myregs[15].I &= 0xFFFFFFFC;
        armNextPC = myregs[15].I;
        myregs[15].I += 4;
        //ARM_PREFETCH;
         
      }     }
    break;
  case 0x716:
  case 0x71e:
    {
      // LDR Rd, [Rn, -Rm, ROR #]
 
      int shift = (opcode >> 7) & 31;
      u32 value = myregs[opcode & 15].I;
      if(shift) {
        ROR_VALUE;
      } else {
        RCR_VALUE;
      }
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = myregs[base].I - value;
      myregs[dest].I = CPUReadMemory(address);
       
      if(dest == 15) {
        myregs[15].I &= 0xFFFFFFFC;
        armNextPC = myregs[15].I;
        myregs[15].I += 4;
        //ARM_PREFETCH;
         
      }     }
    break;
  case 0x730:
  case 0x738:
    {
      // LDR Rd, [Rn, -Rm, LSL #]!
 
      int offset = myregs[opcode & 15].I << ((opcode>>7)& 31);
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = myregs[base].I - offset;
      myregs[dest].I = CPUReadMemory(address);
      if(dest != base)
        myregs[base].I = address;
       
      if(dest == 15) {
        myregs[15].I &= 0xFFFFFFFC;
        armNextPC = myregs[15].I;
        myregs[15].I += 4;
        //ARM_PREFETCH;
         
      }     }
    break;
  case 0x732:
  case 0x73a:
    {
      // LDR Rd, [Rn, -Rm, LSR #]!
 
      int shift = (opcode >> 7) & 31;
      int offset = shift ? myregs[opcode & 15].I >> shift : 0;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = myregs[base].I - offset;
      myregs[dest].I = CPUReadMemory(address);
      if(dest != base)
        myregs[base].I = address;      
       
      if(dest == 15) {
        myregs[15].I &= 0xFFFFFFFC;
        armNextPC = myregs[15].I;
        myregs[15].I += 4;
        //ARM_PREFETCH;
         
      }     }
    break;
  case 0x734:
  case 0x73c:
    {
      // LDR Rd, [Rn, -Rm, ASR #]!
 
      int shift = (opcode >> 7) & 31;
      int offset;
      if(shift)
        offset = (int)((s32)myregs[opcode & 15].I >> shift);
      else if(myregs[opcode & 15].I & 0x80000000)
        offset = 0xFFFFFFFF;
      else
        offset = 0;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = myregs[base].I - offset;
      myregs[dest].I = CPUReadMemory(address);
      if(dest != base)
        myregs[base].I = address;
       
      if(dest == 15) {
        myregs[15].I &= 0xFFFFFFFC;
        armNextPC = myregs[15].I;
        myregs[15].I += 4;
        //ARM_PREFETCH;
         
      }     }
    break;
  case 0x736:
  case 0x73e:
    {
      // LDR Rd, [Rn, -Rm, ROR #]!
 
      int shift = (opcode >> 7) & 31;
      u32 value = myregs[opcode & 15].I;
      if(shift) {
        ROR_VALUE;
      } else {
        RCR_VALUE;
      }
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = myregs[base].I - value;
      myregs[dest].I = CPUReadMemory(address);
      if(dest != base)
        myregs[base].I = address;      
       
      if(dest == 15) {
        myregs[15].I &= 0xFFFFFFFC;
        armNextPC = myregs[15].I;
        myregs[15].I += 4;
        //ARM_PREFETCH;
         
      }     }
    break;
  case 0x790:
  case 0x798:
    {
      // LDR Rd, [Rn, Rm, LSL #]
 
      int offset = myregs[opcode & 15].I << ((opcode>>7)& 31);
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = myregs[base].I + offset;
      myregs[dest].I = CPUReadMemory(address);
       
      if(dest == 15) {
        myregs[15].I &= 0xFFFFFFFC;
        armNextPC = myregs[15].I;
        myregs[15].I += 4;
        //ARM_PREFETCH;
         
      }     }
    break;
  case 0x792:
  case 0x79a:
    {
      // LDR Rd, [Rn, Rm, LSR #]
 
      int shift = (opcode >> 7) & 31;
      int offset = shift ? myregs[opcode & 15].I >> shift : 0;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = myregs[base].I + offset;
      myregs[dest].I = CPUReadMemory(address);
       
      if(dest == 15) {
        myregs[15].I &= 0xFFFFFFFC;
        armNextPC = myregs[15].I;
        myregs[15].I += 4;
        //ARM_PREFETCH;
         
      }     }
    break;
  case 0x794:
  case 0x79c:
    {
      // LDR Rd, [Rn, Rm, ASR #]
 
      int shift = (opcode >> 7) & 31;
      int offset;
      if(shift)
        offset = (int)((s32)myregs[opcode & 15].I >> shift);
      else if(myregs[opcode & 15].I & 0x80000000)
        offset = 0xFFFFFFFF;
      else
        offset = 0;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = myregs[base].I + offset;
      myregs[dest].I = CPUReadMemory(address);
       
      if(dest == 15) {
        myregs[15].I &= 0xFFFFFFFC;
        armNextPC = myregs[15].I;
        myregs[15].I += 4;
        //ARM_PREFETCH;
         
      }     }
    break;
  case 0x796:
  case 0x79e:
    {
      // LDR Rd, [Rn, Rm, ROR #]
 
      int shift = (opcode >> 7) & 31;
      u32 value = myregs[opcode & 15].I;
      if(shift) {
        ROR_VALUE;
      } else {
        RCR_VALUE;
      }
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = myregs[base].I + value;
      myregs[dest].I = CPUReadMemory(address);
       
      if(dest == 15) {
        myregs[15].I &= 0xFFFFFFFC;
        armNextPC = myregs[15].I;
        myregs[15].I += 4;
        //ARM_PREFETCH;
         
      }     }
    break;
  case 0x7b0:
  case 0x7b8:
    {
      // LDR Rd, [Rn, Rm, LSL #]!
 
      int offset = myregs[opcode & 15].I << ((opcode>>7)& 31);
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = myregs[base].I + offset;
      myregs[dest].I = CPUReadMemory(address);
      if(dest != base)
        myregs[base].I = address;
       
      if(dest == 15) {
        myregs[15].I &= 0xFFFFFFFC;
        armNextPC = myregs[15].I;
        myregs[15].I += 4;
        //ARM_PREFETCH;
         
      }     }
    break;
  case 0x7b2:
  case 0x7ba:
    {
      // LDR Rd, [Rn, Rm, LSR #]!
 
      int shift = (opcode >> 7) & 31;
      int offset = shift ? myregs[opcode & 15].I >> shift : 0;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = myregs[base].I + offset;
      myregs[dest].I = CPUReadMemory(address);
      if(dest != base)
        myregs[base].I = address;      
       
      if(dest == 15) {
        myregs[15].I &= 0xFFFFFFFC;
        armNextPC = myregs[15].I;
        myregs[15].I += 4;
        //ARM_PREFETCH;
         
      }     }
    break;
  case 0x7b4:
  case 0x7bc:
    {
      // LDR Rd, [Rn, Rm, ASR #]!
 
      int shift = (opcode >> 7) & 31;
      int offset;
      if(shift)
        offset = (int)((s32)myregs[opcode & 15].I >> shift);
      else if(myregs[opcode & 15].I & 0x80000000)
        offset = 0xFFFFFFFF;
      else
        offset = 0;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = myregs[base].I + offset;
      myregs[dest].I = CPUReadMemory(address);
      if(dest != base)
        myregs[base].I = address;
       
      if(dest == 15) {
        myregs[15].I &= 0xFFFFFFFC;
        armNextPC = myregs[15].I;
        myregs[15].I += 4;
        //ARM_PREFETCH;
         
      }     }
    break;
  case 0x7b6:
  case 0x7be:
    {
      // LDR Rd, [Rn, Rm, ROR #]!
 
      int shift = (opcode >> 7) & 31;
      u32 value = myregs[opcode & 15].I;
      if(shift) {
        ROR_VALUE;
      } else {
        RCR_VALUE;
      }
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = myregs[base].I + value;
      myregs[dest].I = CPUReadMemory(address);
      if(dest != base)
        myregs[base].I = address;
       
      if(dest == 15) {
        myregs[15].I &= 0xFFFFFFFC;
        armNextPC = myregs[15].I;
        myregs[15].I += 4;
        //ARM_PREFETCH;
         
      }     }
    break;
  case 0x640:
  case 0x648:
    // T versions are the same
  case 0x660:
  case 0x668:
    {
      // STRB Rd, [Rn], -Rm, LSL #
 
      int offset = myregs[opcode & 15].I << ((opcode>>7)& 31);
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = myregs[base].I;
      CPUWriteByte(address, myregs[dest].B.B0);
      myregs[base].I = address - offset;
 
    }
    break;
  case 0x642:
  case 0x64a:
    // T versions are the same
  case 0x662:
  case 0x66a:
    {
      // STRB Rd, [Rn], -Rm, LSR #
 
      int shift = (opcode >> 7) & 31;
      int offset = shift ? myregs[opcode & 15].I >> shift : 0;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = myregs[base].I;
      CPUWriteByte(address, myregs[dest].B.B0);
      myregs[base].I = address - offset;
 
    }
    break;
  case 0x644:
  case 0x64c:
    // T versions are the same
  case 0x664:
  case 0x66c:
    {
      // STRB Rd, [Rn], -Rm, ASR #
 
      int shift = (opcode >> 7) & 31;
      int offset;
      if(shift)
        offset = (int)((s32)myregs[opcode & 15].I >> shift);
      else if(myregs[opcode & 15].I & 0x80000000)
        offset = 0xFFFFFFFF;
      else
        offset = 0;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = myregs[base].I;
      CPUWriteByte(address, myregs[dest].B.B0);
      myregs[base].I = address - offset;
 
    }
    break;
  case 0x646:
  case 0x64e:
    // T versions are the same
  case 0x666:
  case 0x66e:
    {
      // STRB Rd, [Rn], -Rm, ROR #
 
      int shift = (opcode >> 7) & 31;
      u32 value = myregs[opcode & 15].I;
      if(shift) {
        ROR_VALUE;
      } else {
        RCR_VALUE;
      }
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = myregs[base].I;
      CPUWriteByte(address, myregs[dest].B.B0);
      myregs[base].I = address - value;
 
    }
    break;
  case 0x6c0:
  case 0x6c8:
    // T versions are the same
  case 0x6e0:
  case 0x6e8:
    {
      // STRB Rd, [Rn], Rm, LSL #
 
      int offset = myregs[opcode & 15].I << ((opcode>>7)& 31);
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = myregs[base].I;
      CPUWriteByte(address, myregs[dest].B.B0);
      myregs[base].I = address + offset;
 
    }
    break;
  case 0x6c2:
  case 0x6ca:
    // T versions are the same
  case 0x6e2:
  case 0x6ea:
    {
      // STRB Rd, [Rn], Rm, LSR #
 
      int shift = (opcode >> 7) & 31;
      int offset = shift ? myregs[opcode & 15].I >> shift : 0;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = myregs[base].I;
      CPUWriteByte(address, myregs[dest].B.B0);
      myregs[base].I = address + offset;
 
    }
    break;
  case 0x6c4:
  case 0x6cc:
    // T versions are the same
  case 0x6e4:
  case 0x6ec:
    {
      // STRB Rd, [Rn], Rm, ASR #
 
      int shift = (opcode >> 7) & 31;
      int offset;
      if(shift)
        offset = (int)((s32)myregs[opcode & 15].I >> shift);
      else if(myregs[opcode & 15].I & 0x80000000)
        offset = 0xFFFFFFFF;
      else
        offset = 0;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = myregs[base].I;
      CPUWriteByte(address, myregs[dest].B.B0);
      myregs[base].I = address + offset;
 
    }
    break;
  case 0x6c6:
  case 0x6ce:
    // T versions are the same
  case 0x6e6:
  case 0x6ee:
    {
      // STRB Rd, [Rn], Rm, ROR #
 
      int shift = (opcode >> 7) & 31;
      u32 value = myregs[opcode & 15].I;
      if(shift) {
        ROR_VALUE;
      } else {
        RCR_VALUE;
      }
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = myregs[base].I;
      CPUWriteByte(address, myregs[dest].B.B0);
      myregs[base].I = address + value;
 
    }
    break;
  case 0x740:
  case 0x748:
    {
      // STRB Rd, [Rn, -Rm, LSL #]
 
      int offset = myregs[opcode & 15].I << ((opcode>>7)& 31);
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = myregs[base].I - offset;
      CPUWriteByte(address, myregs[dest].B.B0);
 
    }
    break;
  case 0x742:
  case 0x74a:
    {
      // STRB Rd, [Rn, -Rm, LSR #]
 
      int shift = (opcode >> 7) & 31;
      int offset = shift ? myregs[opcode & 15].I >> shift : 0;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = myregs[base].I - offset;
      CPUWriteByte(address, myregs[dest].B.B0);
 
    }
    break;
  case 0x744:
  case 0x74c:
    {
      // STRB Rd, [Rn, -Rm, ASR #]
 
      int shift = (opcode >> 7) & 31;
      int offset;
      if(shift)
        offset = (int)((s32)myregs[opcode & 15].I >> shift);
      else if(myregs[opcode & 15].I & 0x80000000)
        offset = 0xFFFFFFFF;
      else
        offset = 0;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = myregs[base].I - offset;
      CPUWriteByte(address, myregs[dest].B.B0);
 
    }
    break;
  case 0x746:
  case 0x74e:
    {
      // STRB Rd, [Rn, -Rm, ROR #]
 
      int shift = (opcode >> 7) & 31;
      u32 value = myregs[opcode & 15].I;
      if(shift) {
        ROR_VALUE;
      } else {
        RCR_VALUE;
      }
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = myregs[base].I - value;
      CPUWriteByte(address, myregs[dest].B.B0);
 
    }
    break;
  case 0x760:
  case 0x768:
    {
      // STRB Rd, [Rn, -Rm, LSL #]!
 
      int offset = myregs[opcode & 15].I << ((opcode>>7)& 31);
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = myregs[base].I - offset;
      myregs[base].I = address;
      CPUWriteByte(address, myregs[dest].B.B0);
 
    }
    break;
  case 0x762:
  case 0x76a:
    {
      // STRB Rd, [Rn, -Rm, LSR #]!
 
      int shift = (opcode >> 7) & 31;
      int offset = shift ? myregs[opcode & 15].I >> shift : 0;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = myregs[base].I - offset;
      myregs[base].I = address;      
      CPUWriteByte(address, myregs[dest].B.B0);
 
    }
    break;
  case 0x764:
  case 0x76c:
    {
      // STRB Rd, [Rn, -Rm, ASR #]!
 
      int shift = (opcode >> 7) & 31;
      int offset;
      if(shift)
        offset = (int)((s32)myregs[opcode & 15].I >> shift);
      else if(myregs[opcode & 15].I & 0x80000000)
        offset = 0xFFFFFFFF;
      else
        offset = 0;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = myregs[base].I - offset;
      myregs[base].I = address;
      CPUWriteByte(address, myregs[dest].B.B0);
 
    }
    break;
  case 0x766:
  case 0x76e:
    {
      // STRB Rd, [Rn, -Rm, ROR #]!
 
      int shift = (opcode >> 7) & 31;
      u32 value = myregs[opcode & 15].I;
      if(shift) {
        ROR_VALUE;
      } else {
        RCR_VALUE;
      }
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = myregs[base].I - value;
      myregs[base].I = address;      
      CPUWriteByte(address, myregs[dest].B.B0);
 
    }
    break;
  case 0x7c0:
  case 0x7c8:
    {
      // STRB Rd, [Rn, Rm, LSL #]
 
      int offset = myregs[opcode & 15].I << ((opcode>>7)& 31);
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = myregs[base].I + offset;
      CPUWriteByte(address, myregs[dest].B.B0);
 
    }
    break;
  case 0x7c2:
  case 0x7ca:
    {
      // STRB Rd, [Rn, Rm, LSR #]
 
      int shift = (opcode >> 7) & 31;
      int offset = shift ? myregs[opcode & 15].I >> shift : 0;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = myregs[base].I + offset;
      CPUWriteByte(address, myregs[dest].B.B0);
 
    }
    break;
  case 0x7c4:
  case 0x7cc:
    {
      // STRB Rd, [Rn, Rm, ASR #]
 
      int shift = (opcode >> 7) & 31;
      int offset;
      if(shift)
        offset = (int)((s32)myregs[opcode & 15].I >> shift);
      else if(myregs[opcode & 15].I & 0x80000000)
        offset = 0xFFFFFFFF;
      else
        offset = 0;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = myregs[base].I + offset;
      CPUWriteByte(address, myregs[dest].B.B0);
 
    }
    break;
  case 0x7c6:
  case 0x7ce:
    {
      // STRB Rd, [Rn, Rm, ROR #]
 
      int shift = (opcode >> 7) & 31;
      u32 value = myregs[opcode & 15].I;
      if(shift) {
        ROR_VALUE;
      } else {
        RCR_VALUE;
      }
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = myregs[base].I + value;
      CPUWriteByte(address, myregs[dest].B.B0);
 
    }
    break;
  case 0x7e0:
  case 0x7e8:
    {
      // STRB Rd, [Rn, Rm, LSL #]!
 
      int offset = myregs[opcode & 15].I << ((opcode>>7)& 31);
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = myregs[base].I + offset;
      myregs[base].I = address;
      CPUWriteByte(address, myregs[dest].B.B0);
 
    }
    break;
  case 0x7e2:
  case 0x7ea:
    {
      // STRB Rd, [Rn, Rm, LSR #]!
 
      int shift = (opcode >> 7) & 31;
      int offset = shift ? myregs[opcode & 15].I >> shift : 0;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = myregs[base].I + offset;
      myregs[base].I = address;      
      CPUWriteByte(address, myregs[dest].B.B0);
 
    }
    break;
  case 0x7e4:
  case 0x7ec:
    {
      // STRB Rd, [Rn, Rm, ASR #]!
 
      int shift = (opcode >> 7) & 31;
      int offset;
      if(shift)
        offset = (int)((s32)myregs[opcode & 15].I >> shift);
      else if(myregs[opcode & 15].I & 0x80000000)
        offset = 0xFFFFFFFF;
      else
        offset = 0;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = myregs[base].I + offset;
      myregs[base].I = address;
      CPUWriteByte(address, myregs[dest].B.B0);
 
    }
    break;
  case 0x7e6:
  case 0x7ee:
    {
      // STRB Rd, [Rn, Rm, ROR #]!
 
      int shift = (opcode >> 7) & 31;
      u32 value = myregs[opcode & 15].I;
      if(shift) {
        ROR_VALUE;
      } else {
        RCR_VALUE;
      }
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = myregs[base].I + value;
      myregs[base].I = address;      
      CPUWriteByte(address, myregs[dest].B.B0);
 
    }
    break;
  case 0x650:
  case 0x658:
    // T versions are the same
  case 0x670:
  case 0x678:
    {
      // LDRB Rd, [Rn], -Rm, LSL #
 
      int offset = myregs[opcode & 15].I << ((opcode>>7)& 31);
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = myregs[base].I;
      myregs[dest].I = CPUReadByte(address);
      if(dest != base)
        myregs[base].I = address - offset;
       
      if(dest == 15) {
        myregs[15].I &= 0xFFFFFFFC;
        armNextPC = myregs[15].I;
        myregs[15].I += 4;
        //ARM_PREFETCH;
         
      }
 
    }
    break;
  case 0x652:
  case 0x65a:
    // T versions are the same
  case 0x672:
  case 0x67a:
    {
      // LDRB Rd, [Rn], -Rm, LSR #
 
      int shift = (opcode >> 7) & 31;
      int offset = shift ? myregs[opcode & 15].I >> shift : 0;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = myregs[base].I;
      myregs[dest].I = CPUReadByte(address);
      if(dest != base)
        myregs[base].I = address - offset;
       
      if(dest == 15) {
        myregs[15].I &= 0xFFFFFFFC;
        armNextPC = myregs[15].I;
        myregs[15].I += 4;
        //ARM_PREFETCH;
         
      }
 
    }
    break;
  case 0x654:
  case 0x65c:
    // T versions are the same
  case 0x674:
  case 0x67c:
    {
      // LDRB Rd, [Rn], -Rm, ASR #
 
      int shift = (opcode >> 7) & 31;
      int offset;
      if(shift)
        offset = (int)((s32)myregs[opcode & 15].I >> shift);
      else if(myregs[opcode & 15].I & 0x80000000)
        offset = 0xFFFFFFFF;
      else
        offset = 0;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = myregs[base].I;
      myregs[dest].I = CPUReadByte(address);
      if(dest != base)
        myregs[base].I = address - offset;
       
      if(dest == 15) {
        myregs[15].I &= 0xFFFFFFFC;
        armNextPC = myregs[15].I;
        myregs[15].I += 4;
        //ARM_PREFETCH;
         
      }
 
    }
    break;
  case 0x656:
  case 0x65e:
    // T versions are the same
  case 0x676:
  case 0x67e:
    {
      // LDRB Rd, [Rn], -Rm, ROR #
 
      int shift = (opcode >> 7) & 31;
      u32 value = myregs[opcode & 15].I;
      if(shift) {
        ROR_VALUE;
      } else {
        RCR_VALUE;
      }
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = myregs[base].I;
      myregs[dest].I = CPUReadByte(address);
      if(dest != base)
        myregs[base].I = address - value;
       
      if(dest == 15) {
        myregs[15].I &= 0xFFFFFFFC;
        armNextPC = myregs[15].I;
        myregs[15].I += 4;
        //ARM_PREFETCH;
         
      }
 
    }
    break;
  case 0x6d0:
  case 0x6d8:
    // T versions are the same
  case 0x6f0:
  case 0x6f8:
    {
      // LDRB Rd, [Rn], Rm, LSL #
 
      int offset = myregs[opcode & 15].I << ((opcode>>7)& 31);
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = myregs[base].I;
      myregs[dest].I = CPUReadByte(address);
      if(dest != base)
        myregs[base].I = address + offset;
       
      if(dest == 15) {
        myregs[15].I &= 0xFFFFFFFC;
        armNextPC = myregs[15].I;
        myregs[15].I += 4;
        //ARM_PREFETCH;
         
      }
 
    }
    break;
  case 0x6d2:
  case 0x6da:
    // T versions are the same
  case 0x6f2:
  case 0x6fa:
    {
      // LDRB Rd, [Rn], Rm, LSR #
 
      int shift = (opcode >> 7) & 31;
      int offset = shift ? myregs[opcode & 15].I >> shift : 0;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = myregs[base].I;
      myregs[dest].I = CPUReadByte(address);
      if(dest != base)
        myregs[base].I = address + offset;
       
      if(dest == 15) {
        myregs[15].I &= 0xFFFFFFFC;
        armNextPC = myregs[15].I;
        myregs[15].I += 4;
        //ARM_PREFETCH;
         
      }
 
    }
    break;
  case 0x6d4:
  case 0x6dc:
    // T versions are the same
  case 0x6f4:
  case 0x6fc:
    {
      // LDRB Rd, [Rn], Rm, ASR #
 
      int shift = (opcode >> 7) & 31;
      int offset;
      if(shift)
        offset = (int)((s32)myregs[opcode & 15].I >> shift);
      else if(myregs[opcode & 15].I & 0x80000000)
        offset = 0xFFFFFFFF;
      else
        offset = 0;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = myregs[base].I;
      myregs[dest].I = CPUReadByte(address);
      if(dest != base)
        myregs[base].I = address + offset;
       
      if(dest == 15) {
        myregs[15].I &= 0xFFFFFFFC;
        armNextPC = myregs[15].I;
        myregs[15].I += 4;
        //ARM_PREFETCH;
         
      }
 
    }
    break;
  case 0x6d6:
  case 0x6de:
    // T versions are the same
  case 0x6f6:
  case 0x6fe:
    {
      // LDRB Rd, [Rn], Rm, ROR #
 
      int shift = (opcode >> 7) & 31;
      u32 value = myregs[opcode & 15].I;
      if(shift) {
        ROR_VALUE;
      } else {
        RCR_VALUE;
      }
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = myregs[base].I;
      myregs[dest].I = CPUReadByte(address);
      if(dest != base)
        myregs[base].I = address + value;
       
      if(dest == 15) {
        myregs[15].I &= 0xFFFFFFFC;
        armNextPC = myregs[15].I;
        myregs[15].I += 4;
        //ARM_PREFETCH;
         
      }
 
    }
    break;
  case 0x750:
  case 0x758:
    {
      // LDRB Rd, [Rn, -Rm, LSL #]
 
      int offset = myregs[opcode & 15].I << ((opcode>>7)& 31);
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = myregs[base].I - offset;
      myregs[dest].I = CPUReadByte(address);
       
      if(dest == 15) {
        myregs[15].I &= 0xFFFFFFFC;
        armNextPC = myregs[15].I;
        myregs[15].I += 4;
        //ARM_PREFETCH;
         
      }
 
    }
    break;
  case 0x752:
  case 0x75a:
    {
      // LDRB Rd, [Rn, -Rm, LSR #]
 
      int shift = (opcode >> 7) & 31;
      int offset = shift ? myregs[opcode & 15].I >> shift : 0;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = myregs[base].I - offset;
      myregs[dest].I = CPUReadByte(address);
       
      if(dest == 15) {
        myregs[15].I &= 0xFFFFFFFC;
        armNextPC = myregs[15].I;
        myregs[15].I += 4;
        //ARM_PREFETCH;
         
      }
 
    }
    break;
  case 0x754:
  case 0x75c:
    {
      // LDRB Rd, [Rn, -Rm, ASR #]
 
      int shift = (opcode >> 7) & 31;
      int offset;
      if(shift)
        offset = (int)((s32)myregs[opcode & 15].I >> shift);
      else if(myregs[opcode & 15].I & 0x80000000)
        offset = 0xFFFFFFFF;
      else
        offset = 0;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = myregs[base].I - offset;
      myregs[dest].I = CPUReadByte(address);
       
      if(dest == 15) {
        myregs[15].I &= 0xFFFFFFFC;
        armNextPC = myregs[15].I;
        myregs[15].I += 4;
        //ARM_PREFETCH;
         
      }
 
    }
    break;
  case 0x756:
  case 0x75e:
    {
      // LDRB Rd, [Rn, -Rm, ROR #]
 
      int shift = (opcode >> 7) & 31;
      u32 value = myregs[opcode & 15].I;
      if(shift) {
        ROR_VALUE;
      } else {
        RCR_VALUE;
      }
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = myregs[base].I - value;
      myregs[dest].I = CPUReadByte(address);
       
      if(dest == 15) {
        myregs[15].I &= 0xFFFFFFFC;
        armNextPC = myregs[15].I;
        myregs[15].I += 4;
        //ARM_PREFETCH;
         
      }
 
    }
    break;
  case 0x770:
  case 0x778:
    {
      // LDRB Rd, [Rn, -Rm, LSL #]!
 
      int offset = myregs[opcode & 15].I << ((opcode>>7)& 31);
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = myregs[base].I - offset;
      myregs[dest].I = CPUReadByte(address);
      if(dest != base)
        myregs[base].I = address;
       
      if(dest == 15) {
        myregs[15].I &= 0xFFFFFFFC;
        armNextPC = myregs[15].I;
        myregs[15].I += 4;
        //ARM_PREFETCH;
         
      }
 
    }
    break;
  case 0x772:
  case 0x77a:
    {
      // LDRB Rd, [Rn, -Rm, LSR #]!
 
      int shift = (opcode >> 7) & 31;
      int offset = shift ? myregs[opcode & 15].I >> shift : 0;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = myregs[base].I - offset;
      myregs[dest].I = CPUReadByte(address);
      if(dest != base)
        myregs[base].I = address;      
       
      if(dest == 15) {
        myregs[15].I &= 0xFFFFFFFC;
        armNextPC = myregs[15].I;
        myregs[15].I += 4;
        //ARM_PREFETCH;
         
      }
 
    }
    break;
  case 0x774:
  case 0x77c:
    {
      // LDRB Rd, [Rn, -Rm, ASR #]!
 
      int shift = (opcode >> 7) & 31;
      int offset;
      if(shift)
        offset = (int)((s32)myregs[opcode & 15].I >> shift);
      else if(myregs[opcode & 15].I & 0x80000000)
        offset = 0xFFFFFFFF;
      else
        offset = 0;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = myregs[base].I - offset;
      myregs[dest].I = CPUReadByte(address);
      if(dest != base)
        myregs[base].I = address;
       
      if(dest == 15) {
        myregs[15].I &= 0xFFFFFFFC;
        armNextPC = myregs[15].I;
        myregs[15].I += 4;
        //ARM_PREFETCH;
         
      }
 
    }
    break;
  case 0x776:
  case 0x77e:
    {
      // LDRB Rd, [Rn, -Rm, ROR #]!
 
      int shift = (opcode >> 7) & 31;
      u32 value = myregs[opcode & 15].I;
      if(shift) {
        ROR_VALUE;
      } else {
        RCR_VALUE;
      }
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = myregs[base].I - value;
      myregs[dest].I = CPUReadByte(address);
      if(dest != base)
        myregs[base].I = address;      
       
      if(dest == 15) {
        myregs[15].I &= 0xFFFFFFFC;
        armNextPC = myregs[15].I;
        myregs[15].I += 4;
        //ARM_PREFETCH;
         
      }
 
    }
    break;
  case 0x7d0:
  case 0x7d8:
    {
      // LDRB Rd, [Rn, Rm, LSL #]
 
      int offset = myregs[opcode & 15].I << ((opcode>>7)& 31);
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = myregs[base].I + offset;
      myregs[dest].I = CPUReadByte(address);
       
      if(dest == 15) {
        myregs[15].I &= 0xFFFFFFFC;
        armNextPC = myregs[15].I;
        myregs[15].I += 4;
        //ARM_PREFETCH;
         
      }
 
    }
    break;
  case 0x7d2:
  case 0x7da:
    {
      // LDRB Rd, [Rn, Rm, LSR #]
 
      int shift = (opcode >> 7) & 31;
      int offset = shift ? myregs[opcode & 15].I >> shift : 0;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = myregs[base].I + offset;
      myregs[dest].I = CPUReadByte(address);
       
      if(dest == 15) {
        myregs[15].I &= 0xFFFFFFFC;
        armNextPC = myregs[15].I;
        myregs[15].I += 4;
        //ARM_PREFETCH;
         
      }
 
    }
    break;
  case 0x7d4:
  case 0x7dc:
    {
      // LDRB Rd, [Rn, Rm, ASR #]
 
      int shift = (opcode >> 7) & 31;
      int offset;
      if(shift)
        offset = (int)((s32)myregs[opcode & 15].I >> shift);
      else if(myregs[opcode & 15].I & 0x80000000)
        offset = 0xFFFFFFFF;
      else
        offset = 0;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = myregs[base].I + offset;
      myregs[dest].I = CPUReadByte(address);
       
      if(dest == 15) {
        myregs[15].I &= 0xFFFFFFFC;
        armNextPC = myregs[15].I;
        myregs[15].I += 4;
        //ARM_PREFETCH;
         
      }
 
    }
    break;
  case 0x7d6:
  case 0x7de:
    {
      // LDRB Rd, [Rn, Rm, ROR #]
 
      int shift = (opcode >> 7) & 31;
      u32 value = myregs[opcode & 15].I;
      if(shift) {
        ROR_VALUE;
      } else {
        RCR_VALUE;
      }
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = myregs[base].I + value;
      myregs[dest].I = CPUReadByte(address);
       
      if(dest == 15) {
        myregs[15].I &= 0xFFFFFFFC;
        armNextPC = myregs[15].I;
        myregs[15].I += 4;
        //ARM_PREFETCH;
         
      }
 
    }
    break;
  case 0x7f0:
  case 0x7f8:
    {
      // LDRB Rd, [Rn, Rm, LSL #]!
 
      int offset = myregs[opcode & 15].I << ((opcode>>7)& 31);
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = myregs[base].I + offset;
      myregs[dest].I = CPUReadByte(address);
      if(dest != base)
        myregs[base].I = address;
       
      if(dest == 15) {
        myregs[15].I &= 0xFFFFFFFC;
        armNextPC = myregs[15].I;
        myregs[15].I += 4;
        //ARM_PREFETCH;
         
      }
 
    }
    break;
  case 0x7f2:
  case 0x7fa:
    {
      // LDRB Rd, [Rn, Rm, LSR #]!
 
      int shift = (opcode >> 7) & 31;
      int offset = shift ? myregs[opcode & 15].I >> shift : 0;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = myregs[base].I + offset;
      myregs[dest].I = CPUReadByte(address);
      if(dest != base)
        myregs[base].I = address;      
       
      if(dest == 15) {
        myregs[15].I &= 0xFFFFFFFC;
        armNextPC = myregs[15].I;
        myregs[15].I += 4;
        //ARM_PREFETCH;
         
      }
 
    }
    break;
  case 0x7f4:
  case 0x7fc:
    {
      // LDRB Rd, [Rn, Rm, ASR #]!
 
      int shift = (opcode >> 7) & 31;
      int offset;
      if(shift)
        offset = (int)((s32)myregs[opcode & 15].I >> shift);
      else if(myregs[opcode & 15].I & 0x80000000)
        offset = 0xFFFFFFFF;
      else
        offset = 0;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = myregs[base].I + offset;
      myregs[dest].I = CPUReadByte(address);
      if(dest != base)
        myregs[base].I = address;
       
      if(dest == 15) {
        myregs[15].I &= 0xFFFFFFFC;
        armNextPC = myregs[15].I;
        myregs[15].I += 4;
        //ARM_PREFETCH;
         
      }
 
    }
    break;
  case 0x7f6:
  case 0x7fe:
    {
      // LDRB Rd, [Rn, Rm, ROR #]!
 
      int shift = (opcode >> 7) & 31;
      u32 value = myregs[opcode & 15].I;
      if(shift) {
        ROR_VALUE;
      } else {
        RCR_VALUE;
      }
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = myregs[base].I + value;
      myregs[dest].I = CPUReadByte(address);
      if(dest != base)
        myregs[base].I = address;      
       
      if(dest == 15) {
        myregs[15].I &= 0xFFFFFFFC;
        armNextPC = myregs[15].I;
        myregs[15].I += 4;
        //ARM_PREFETCH;
         
      }
 
    }
    break;
#define STMW_myregs(val,num) \
  if(opcode & (val)) {\
    CPUWriteMemory(address, myregs[(num)].I);\
    if(!offset) {\
      myregs[base].I = temp;\
       \
      offset = 1;\
    } else {\
       \
    }\
    address += 4;\
  }
#define STM_myregs(val,num) \
  if(opcode & (val)) {\
    CPUWriteMemory(address, myregs[(num)].I);\
    if(!offset) {\
       \
      offset = 1;\
    } else {\
       \
    }\
    address += 4;\
  }
    
  CASE_16(0x800)
    {
      // STMDA Rn, {Rlist}
 
      int base = (opcode & 0x000F0000) >> 16;
      u32 temp = myregs[base].I - 
        4 * (cpuBitsSet[opcode & 255] + cpuBitsSet[(opcode >> 8) & 255]);
      u32 address = (temp + 4) & 0xFFFFFFFC;
      int offset = 0;
      STM_myregs(1, 0);
      STM_myregs(2, 1);
      STM_myregs(4, 2);
      STM_myregs(8, 3);
      STM_myregs(16, 4);
      STM_myregs(32, 5);
      STM_myregs(64, 6);
      STM_myregs(128, 7);
      STM_myregs(256, 8);
      STM_myregs(512, 9);
      STM_myregs(1024, 10);
      STM_myregs(2048, 11);
      STM_myregs(4096, 12);
      STM_myregs(8192, 13);
      STM_myregs(16384, 14);
      if(opcode & 32768) {
        CPUWriteMemory(address, myregs[15].I+4);
 
      }
       
    }
    break;
  CASE_16(0x820)
    {
      // STMDA Rn!, {Rlist}
 
      int base = (opcode & 0x000F0000) >> 16;
      u32 temp = myregs[base].I - 
        4 * (cpuBitsSet[opcode & 255] + cpuBitsSet[(opcode >> 8) & 255]);
      u32 address = (temp+4) & 0xFFFFFFFC;
      int offset = 0;

      STMW_myregs(1, 0);
      STMW_myregs(2, 1);
      STMW_myregs(4, 2);
      STMW_myregs(8, 3);
      STMW_myregs(16, 4);
      STMW_myregs(32, 5);
      STMW_myregs(64, 6);
      STMW_myregs(128, 7);
      STMW_myregs(256, 8);
      STMW_myregs(512, 9);
      STMW_myregs(1024, 10);
      STMW_myregs(2048, 11);
      STMW_myregs(4096, 12);
      STMW_myregs(8192, 13);
      STMW_myregs(16384, 14);
      if(opcode & 32768) {
        CPUWriteMemory(address, myregs[15].I+4);
 
        myregs[base].I = temp;
      }
       
    }
  break;
  CASE_16(0x840)
    {
      // STMDA Rn, {Rlist}^
 
      int base = (opcode & 0x000F0000) >> 16;
      u32 temp = myregs[base].I - 
        4 * (cpuBitsSet[opcode & 255] + cpuBitsSet[(opcode >> 8) & 255]);
      u32 address = (temp+4) & 0xFFFFFFFC;
      int offset = 0;

      STM_myregs(1, 0);
      STM_myregs(2, 1);
      STM_myregs(4, 2);
      STM_myregs(8, 3);
      STM_myregs(16, 4);
      STM_myregs(32, 5);
      STM_myregs(64, 6);
      STM_myregs(128, 7);

      if(armMode == 0x11) {
        STM_myregs(256, R8_FIQ);
        STM_myregs(512, R9_FIQ);
        STM_myregs(1024, R10_FIQ);
        STM_myregs(2048, R11_FIQ);
        STM_myregs(4096, R12_FIQ);
      } else {
        STM_myregs(256, 8);
        STM_myregs(512, 9);
        STM_myregs(1024, 10);
        STM_myregs(2048, 11);
        STM_myregs(4096, 12);
      }

      if(armMode != 0x10 && armMode != 0x1f) {
        STM_myregs(8192, R13_USR);
        STM_myregs(16384, R14_USR);
      } else {
        STM_myregs(8192, 13);
        STM_myregs(16384, 14);
      }

      if(opcode & 32768) {
        CPUWriteMemory(address, myregs[15].I+4);
 
      }
       
    }
    break;
  CASE_16(0x860)
    {
      // STMDA Rn!, {Rlist}^
 
      int base = (opcode & 0x000F0000) >> 16;
      u32 temp = myregs[base].I - 
        4 * (cpuBitsSet[opcode & 255] + cpuBitsSet[(opcode >> 8) & 255]);
      u32 address = (temp+4) & 0xFFFFFFFC;
      int offset = 0;

      STMW_myregs(1, 0);
      STMW_myregs(2, 1);
      STMW_myregs(4, 2);
      STMW_myregs(8, 3);
      STMW_myregs(16, 4);
      STMW_myregs(32, 5);
      STMW_myregs(64, 6);
      STMW_myregs(128, 7);

      if(armMode == 0x11) {
        STMW_myregs(256, R8_FIQ);
        STMW_myregs(512, R9_FIQ);
        STMW_myregs(1024, R10_FIQ);
        STMW_myregs(2048, R11_FIQ);
        STMW_myregs(4096, R12_FIQ);
      } else {
        STMW_myregs(256, 8);
        STMW_myregs(512, 9);
        STMW_myregs(1024, 10);
        STMW_myregs(2048, 11);
        STMW_myregs(4096, 12);
      }

      if(armMode != 0x10 && armMode != 0x1f) {
        STMW_myregs(8192, R13_USR);
        STMW_myregs(16384, R14_USR);
      } else {
        STMW_myregs(8192, 13);
        STMW_myregs(16384, 14);
      }

      if(opcode & 32768) {
        CPUWriteMemory(address, myregs[15].I+4);
 
        myregs[base].I = temp;
      }
       
    }
    break;
  
  CASE_16(0x880)
    {
      // STMIA Rn, {Rlist}
 
      int base = (opcode & 0x000F0000) >> 16;
      u32 address = myregs[base].I & 0xFFFFFFFC;
      int offset = 0;
      STM_myregs(1, 0);
      STM_myregs(2, 1);
      STM_myregs(4, 2);
      STM_myregs(8, 3);
      STM_myregs(16, 4);
      STM_myregs(32, 5);
      STM_myregs(64, 6);
      STM_myregs(128, 7);
      STM_myregs(256, 8);
      STM_myregs(512, 9);
      STM_myregs(1024, 10);
      STM_myregs(2048, 11);
      STM_myregs(4096, 12);
      STM_myregs(8192, 13);
      STM_myregs(16384, 14);
      if(opcode & 32768) {
        CPUWriteMemory(address, myregs[15].I+4);
 
      }
       
    }
  break;
  CASE_16(0x8a0)
    {
      // STMIA Rn!, {Rlist}
 
      int base = (opcode & 0x000F0000) >> 16;
      u32 address = myregs[base].I & 0xFFFFFFFC;
      int offset = 0;
      u32 temp = myregs[base].I + 4*(cpuBitsSet[opcode & 0xFF] +
                                  cpuBitsSet[(opcode >> 8) & 255]);
      STMW_myregs(1, 0);
      STMW_myregs(2, 1);
      STMW_myregs(4, 2);
      STMW_myregs(8, 3);
      STMW_myregs(16, 4);
      STMW_myregs(32, 5);
      STMW_myregs(64, 6);
      STMW_myregs(128, 7);
      STMW_myregs(256, 8);
      STMW_myregs(512, 9);
      STMW_myregs(1024, 10);
      STMW_myregs(2048, 11);
      STMW_myregs(4096, 12);
      STMW_myregs(8192, 13);
      STMW_myregs(16384, 14);
      if(opcode & 32768) {
        CPUWriteMemory(address, myregs[15].I+4);
        if(!offset) {
          myregs[base].I = temp;
           
        } 
           
      }
       
    }
  break;
  CASE_16(0x8c0)
    {
      // STMIA Rn, {Rlist}^
 
      int base = (opcode & 0x000F0000) >> 16;
      u32 address = myregs[base].I & 0xFFFFFFFC;
      int offset = 0;
      STM_myregs(1, 0);
      STM_myregs(2, 1);
      STM_myregs(4, 2);
      STM_myregs(8, 3);
      STM_myregs(16, 4);
      STM_myregs(32, 5);
      STM_myregs(64, 6);
      STM_myregs(128, 7);
      if(armMode == 0x11) {
        STM_myregs(256, R8_FIQ);
        STM_myregs(512, R9_FIQ);
        STM_myregs(1024, R10_FIQ);
        STM_myregs(2048, R11_FIQ);
        STM_myregs(4096, R12_FIQ);
      } else {
        STM_myregs(256, 8);
        STM_myregs(512, 9);
        STM_myregs(1024, 10);
        STM_myregs(2048, 11);
        STM_myregs(4096, 12);      
      }
      if(armMode != 0x10 && armMode != 0x1f) {
        STM_myregs(8192, R13_USR);
        STM_myregs(16384, R14_USR);
      } else {
        STM_myregs(8192, 13);
        STM_myregs(16384, 14);
      }
      if(opcode & 32768) {
        CPUWriteMemory(address, myregs[15].I+4);
 
      }
       
    }
    break;
  CASE_16(0x8e0)
    {
      // STMIA Rn!, {Rlist}^
 
      int base = (opcode & 0x000F0000) >> 16;
      u32 address = myregs[base].I & 0xFFFFFFFC;
      int offset = 0;
      u32 temp = myregs[base].I + 4*(cpuBitsSet[opcode & 0xFF] +
                                  cpuBitsSet[(opcode >> 8) & 255]);    
      STMW_myregs(1, 0);
      STMW_myregs(2, 1);
      STMW_myregs(4, 2);
      STMW_myregs(8, 3);
      STMW_myregs(16, 4);
      STMW_myregs(32, 5);
      STMW_myregs(64, 6);
      STMW_myregs(128, 7);
      if(armMode == 0x11) {
        STMW_myregs(256, R8_FIQ);
        STMW_myregs(512, R9_FIQ);
        STMW_myregs(1024, R10_FIQ);
        STMW_myregs(2048, R11_FIQ);
        STMW_myregs(4096, R12_FIQ);
      } else {
        STMW_myregs(256, 8);
        STMW_myregs(512, 9);
        STMW_myregs(1024, 10);
        STMW_myregs(2048, 11);
        STMW_myregs(4096, 12);      
      }
      if(armMode != 0x10 && armMode != 0x1f) {
        STMW_myregs(8192, R13_USR);
        STMW_myregs(16384, R14_USR);
      } else {
        STMW_myregs(8192, 13);
        STMW_myregs(16384, 14);
      }
      if(opcode & 32768) {
        CPUWriteMemory(address, myregs[15].I+4);
        if(!offset) {
          myregs[base].I = temp;
           
        } 
           
      }
       
    }
    break;
    
  CASE_16(0x900)
    {
      // STMDB Rn, {Rlist}
 
      int base = (opcode & 0x000F0000) >> 16;
      u32 temp = myregs[base].I - 
        4 * (cpuBitsSet[opcode & 255] + cpuBitsSet[(opcode >> 8) & 255]);
      u32 address = temp & 0xFFFFFFFC;
      int offset = 0;
      STM_myregs(1, 0);
      STM_myregs(2, 1);
      STM_myregs(4, 2);
      STM_myregs(8, 3);
      STM_myregs(16, 4);
      STM_myregs(32, 5);
      STM_myregs(64, 6);
      STM_myregs(128, 7);
      STM_myregs(256, 8);
      STM_myregs(512, 9);
      STM_myregs(1024, 10);
      STM_myregs(2048, 11);
      STM_myregs(4096, 12);
      STM_myregs(8192, 13);
      STM_myregs(16384, 14);
      if(opcode & 32768) {
        CPUWriteMemory(address, myregs[15].I+4);
 
      }
       
    }
    break;
  CASE_16(0x920)
    {
      // STMDB Rn!, {Rlist}
 
      int base = (opcode & 0x000F0000) >> 16;
      u32 temp = myregs[base].I - 
        4 * (cpuBitsSet[opcode & 255] + cpuBitsSet[(opcode >> 8) & 255]);
      u32 address = temp & 0xFFFFFFFC;
      int offset = 0;

      STMW_myregs(1, 0);
      STMW_myregs(2, 1);
      STMW_myregs(4, 2);
      STMW_myregs(8, 3);
      STMW_myregs(16, 4);
      STMW_myregs(32, 5);
      STMW_myregs(64, 6);
      STMW_myregs(128, 7);
      STMW_myregs(256, 8);
      STMW_myregs(512, 9);
      STMW_myregs(1024, 10);
      STMW_myregs(2048, 11);
      STMW_myregs(4096, 12);
      STMW_myregs(8192, 13);
      STMW_myregs(16384, 14);
      if(opcode & 32768) {
        CPUWriteMemory(address, myregs[15].I+4);
 
           
        myregs[base].I = temp;
      }
       
    }
  break;
  CASE_16(0x940)
    {
      // STMDB Rn, {Rlist}^
 
      int base = (opcode & 0x000F0000) >> 16;
      u32 temp = myregs[base].I - 
        4 * (cpuBitsSet[opcode & 255] + cpuBitsSet[(opcode >> 8) & 255]);
      u32 address = temp & 0xFFFFFFFC;
      int offset = 0;

      STM_myregs(1, 0);
      STM_myregs(2, 1);
      STM_myregs(4, 2);
      STM_myregs(8, 3);
      STM_myregs(16, 4);
      STM_myregs(32, 5);
      STM_myregs(64, 6);
      STM_myregs(128, 7);

      if(armMode == 0x11) {
        STM_myregs(256, R8_FIQ);
        STM_myregs(512, R9_FIQ);
        STM_myregs(1024, R10_FIQ);
        STM_myregs(2048, R11_FIQ);
        STM_myregs(4096, R12_FIQ);
      } else {
        STM_myregs(256, 8);
        STM_myregs(512, 9);
        STM_myregs(1024, 10);
        STM_myregs(2048, 11);
        STM_myregs(4096, 12);
      }

      if(armMode != 0x10 && armMode != 0x1f) {
        STM_myregs(8192, R13_USR);
        STM_myregs(16384, R14_USR);
      } else {
        STM_myregs(8192, 13);
        STM_myregs(16384, 14);
      }

      if(opcode & 32768) {
        CPUWriteMemory(address, myregs[15].I+4);
 
      }
       
    }
    break;
  CASE_16(0x960)
    {
      // STMDB Rn!, {Rlist}^
 
      int base = (opcode & 0x000F0000) >> 16;
      u32 temp = myregs[base].I - 
        4 * (cpuBitsSet[opcode & 255] + cpuBitsSet[(opcode >> 8) & 255]);
      u32 address = temp & 0xFFFFFFFC;
      int offset = 0;

      STMW_myregs(1, 0);
      STMW_myregs(2, 1);
      STMW_myregs(4, 2);
      STMW_myregs(8, 3);
      STMW_myregs(16, 4);
      STMW_myregs(32, 5);
      STMW_myregs(64, 6);
      STMW_myregs(128, 7);

      if(armMode == 0x11) {
        STMW_myregs(256, R8_FIQ);
        STMW_myregs(512, R9_FIQ);
        STMW_myregs(1024, R10_FIQ);
        STMW_myregs(2048, R11_FIQ);
        STMW_myregs(4096, R12_FIQ);
      } else {
        STMW_myregs(256, 8);
        STMW_myregs(512, 9);
        STMW_myregs(1024, 10);
        STMW_myregs(2048, 11);
        STMW_myregs(4096, 12);
      }

      if(armMode != 0x10 && armMode != 0x1f) {
        STMW_myregs(8192, R13_USR);
        STMW_myregs(16384, R14_USR);
      } else {
        STMW_myregs(8192, 13);
        STMW_myregs(16384, 14);
      }

      if(opcode & 32768) {
        CPUWriteMemory(address, myregs[15].I+4);
 
        myregs[base].I = temp;
      }
       
    }
    break;

  CASE_16(0x980)
    {
      // STMIB Rn, {Rlist}
 
      int base = (opcode & 0x000F0000) >> 16;
      u32 address = (myregs[base].I+4) & 0xFFFFFFFC;
      int offset = 0;
      STM_myregs(1, 0);
      STM_myregs(2, 1);
      STM_myregs(4, 2);
      STM_myregs(8, 3);
      STM_myregs(16, 4);
      STM_myregs(32, 5);
      STM_myregs(64, 6);
      STM_myregs(128, 7);
      STM_myregs(256, 8);
      STM_myregs(512, 9);
      STM_myregs(1024, 10);
      STM_myregs(2048, 11);
      STM_myregs(4096, 12);
      STM_myregs(8192, 13);
      STM_myregs(16384, 14);
      if(opcode & 32768) {
        CPUWriteMemory(address, myregs[15].I+4);
 
      }
       
    }
    break;
  CASE_16(0x9a0)
    {
      // STMIB Rn!, {Rlist}
 
      int base = (opcode & 0x000F0000) >> 16;
      u32 address = (myregs[base].I+4) & 0xFFFFFFFC;
      int offset = 0;
      u32 temp = myregs[base].I + 4*(cpuBitsSet[opcode & 0xFF] +
                                  cpuBitsSet[(opcode >> 8) & 255]);
      STMW_myregs(1, 0);
      STMW_myregs(2, 1);
      STMW_myregs(4, 2);
      STMW_myregs(8, 3);
      STMW_myregs(16, 4);
      STMW_myregs(32, 5);
      STMW_myregs(64, 6);
      STMW_myregs(128, 7);
      STMW_myregs(256, 8);
      STMW_myregs(512, 9);
      STMW_myregs(1024, 10);
      STMW_myregs(2048, 11);
      STMW_myregs(4096, 12);
      STMW_myregs(8192, 13);
      STMW_myregs(16384, 14);
      if(opcode & 32768) {
        CPUWriteMemory(address, myregs[15].I+4);
        if(!offset) {
          myregs[base].I = temp;
           
        } 
           
      }
       
    }
    break;
  CASE_16(0x9c0)
    {
      // STMIB Rn, {Rlist}^
 
      int base = (opcode & 0x000F0000) >> 16;
      u32 address = (myregs[base].I+4) & 0xFFFFFFFC;
      int offset = 0;
      STM_myregs(1, 0);
      STM_myregs(2, 1);
      STM_myregs(4, 2);
      STM_myregs(8, 3);
      STM_myregs(16, 4);
      STM_myregs(32, 5);
      STM_myregs(64, 6);
      STM_myregs(128, 7);
      if(armMode == 0x11) {
        STM_myregs(256, R8_FIQ);
        STM_myregs(512, R9_FIQ);
        STM_myregs(1024, R10_FIQ);
        STM_myregs(2048, R11_FIQ);
        STM_myregs(4096, R12_FIQ);
      } else {
        STM_myregs(256, 8);
        STM_myregs(512, 9);
        STM_myregs(1024, 10);
        STM_myregs(2048, 11);
        STM_myregs(4096, 12);      
      }
      if(armMode != 0x10 && armMode != 0x1f) {
        STM_myregs(8192, R13_USR);
        STM_myregs(16384, R14_USR);
      } else {
        STM_myregs(8192, 13);
        STM_myregs(16384, 14);
      }
      if(opcode & 32768) {
        CPUWriteMemory(address, myregs[15].I+4);
 
      }
       
    }
    break;
  CASE_16(0x9e0)
    {
      // STMIB Rn!, {Rlist}^
 
      int base = (opcode & 0x000F0000) >> 16;
      u32 address = (myregs[base].I+4) & 0xFFFFFFFC;
      int offset = 0;
      u32 temp = myregs[base].I + 4*(cpuBitsSet[opcode & 0xFF] +
                                  cpuBitsSet[(opcode >> 8) & 255]);    
      STMW_myregs(1, 0);
      STMW_myregs(2, 1);
      STMW_myregs(4, 2);
      STMW_myregs(8, 3);
      STMW_myregs(16, 4);
      STMW_myregs(32, 5);
      STMW_myregs(64, 6);
      STMW_myregs(128, 7);
      if(armMode == 0x11) {
        STMW_myregs(256, R8_FIQ);
        STMW_myregs(512, R9_FIQ);
        STMW_myregs(1024, R10_FIQ);
        STMW_myregs(2048, R11_FIQ);
        STMW_myregs(4096, R12_FIQ);
      } else {
        STMW_myregs(256, 8);
        STMW_myregs(512, 9);
        STMW_myregs(1024, 10);
        STMW_myregs(2048, 11);
        STMW_myregs(4096, 12);      
      }
      if(armMode != 0x10 && armMode != 0x1f) {
        STMW_myregs(8192, R13_USR);
        STMW_myregs(16384, R14_USR);
      } else {
        STMW_myregs(8192, 13);
        STMW_myregs(16384, 14);
      }
      if(opcode & 32768) {
        CPUWriteMemory(address, myregs[15].I+4);
        if(!offset) {
          myregs[base].I = temp;
           
        } 
           
      }
       
    }
    break;
    
#define LDM_myregs(val,num) \
  if(opcode & (val)) {\
    myregs[(num)].I = CPUReadMemory(address);\
    if(offset){}\
    else {\
      offset = 1;\
    }\
    address += 4;\
  }

  CASE_16(0x810)
    {
      // LDMDA Rn, {Rlist}
 
      int base = (opcode & 0x000F0000) >> 16;
      u32 temp = myregs[base].I - 
        4 * (cpuBitsSet[opcode & 255] + cpuBitsSet[(opcode >> 8) & 255]);
      u32 address = (temp + 4) & 0xFFFFFFFC;
       
      int offset = 0;
      LDM_myregs(1, 0);
      LDM_myregs(2, 1);
      LDM_myregs(4, 2);
      LDM_myregs(8, 3);
      LDM_myregs(16, 4);
      LDM_myregs(32, 5);
      LDM_myregs(64, 6);
      LDM_myregs(128, 7);
      LDM_myregs(256, 8);
      LDM_myregs(512, 9);
      LDM_myregs(1024, 10);
      LDM_myregs(2048, 11);
      LDM_myregs(4096, 12);
      LDM_myregs(8192, 13);
      LDM_myregs(16384, 14);
      if(opcode & 32768) {
        myregs[15].I = CPUReadMemory(address);
        armNextPC = myregs[15].I;
        myregs[15].I += 4;
        //ARM_PREFETCH;
 
      }
       
    }
    break;
  CASE_16(0x830)
    {
      // LDMDA Rn!, {Rlist}
 
      int base = (opcode & 0x000F0000) >> 16;
      u32 temp = myregs[base].I - 
        4 * (cpuBitsSet[opcode & 255] + cpuBitsSet[(opcode >> 8) & 255]);
      u32 address = (temp + 4) & 0xFFFFFFFC;
       
      int offset = 0;
      LDM_myregs(1, 0);
      LDM_myregs(2, 1);
      LDM_myregs(4, 2);
      LDM_myregs(8, 3);
      LDM_myregs(16, 4);
      LDM_myregs(32, 5);
      LDM_myregs(64, 6);
      LDM_myregs(128, 7);
      LDM_myregs(256, 8);
      LDM_myregs(512, 9);
      LDM_myregs(1024, 10);
      LDM_myregs(2048, 11);
      LDM_myregs(4096, 12);
      LDM_myregs(8192, 13);
      LDM_myregs(16384, 14);
      if(opcode & 32768) {
        myregs[15].I = CPUReadMemory(address);

        armNextPC = myregs[15].I;
        myregs[15].I += 4;
        //ARM_PREFETCH;
      }
       
      if(!(opcode & (1 << base)))
        myregs[base].I = temp;
    }
    break;
  CASE_16(0x850)
    {
      // LDMDA Rn, {Rlist}^
 
      int base = (opcode & 0x000F0000) >> 16;
      u32 temp = myregs[base].I - 
        4 * (cpuBitsSet[opcode & 255] + cpuBitsSet[(opcode >> 8) & 255]);
      u32 address = (temp + 4) & 0xFFFFFFFC;
       
      int offset = 0;
      if(opcode & 0x8000) {
        LDM_myregs(1, 0);
        LDM_myregs(2, 1);
        LDM_myregs(4, 2);
        LDM_myregs(8, 3);
        LDM_myregs(16, 4);
        LDM_myregs(32, 5);
        LDM_myregs(64, 6);
        LDM_myregs(128, 7);
        LDM_myregs(256, 8);
        LDM_myregs(512, 9);
        LDM_myregs(1024, 10);
        LDM_myregs(2048, 11);
        LDM_myregs(4096, 12);
        LDM_myregs(8192, 13);
        LDM_myregs(16384, 14);

        myregs[15].I = CPUReadMemory(address);

        //CPUSwitchMode(myregs[17].I & 0x1f, false);
        if(armState) {
          armNextPC = myregs[15].I & 0xFFFFFFFC;
          myregs[15].I = armNextPC + 4; 
        } else {
          armNextPC = myregs[15].I & 0xFFFFFFFE;
          myregs[15].I = armNextPC + 2;
          //THUMB_PREFETCH;
        }
      } else {
        LDM_myregs(1, 0);
        LDM_myregs(2, 1);
        LDM_myregs(4, 2);
        LDM_myregs(8, 3);
        LDM_myregs(16, 4);
        LDM_myregs(32, 5);
        LDM_myregs(64, 6);
        LDM_myregs(128, 7);

        if(armMode == 0x11) {
          LDM_myregs(256, R8_FIQ);
          LDM_myregs(512, R9_FIQ);
          LDM_myregs(1024, R10_FIQ);
          LDM_myregs(2048, R11_FIQ);
          LDM_myregs(4096, R12_FIQ);
        } else {
          LDM_myregs(256, 8);
          LDM_myregs(512, 9);
          LDM_myregs(1024, 10);
          LDM_myregs(2048, 11);
          LDM_myregs(4096, 12);
        }

        if(armMode != 0x10 && armMode != 0x1f) {
          LDM_myregs(8192, R13_USR);
          LDM_myregs(16384, R14_USR);
        } else {
          LDM_myregs(8192, 13);
          LDM_myregs(16384, 14);
        }
 
      }
         
    }
    break;
  CASE_16(0x870)
    {
      // LDMDA Rn!, {Rlist}^
 
      int base = (opcode & 0x000F0000) >> 16;
      u32 temp = myregs[base].I - 
        4 * (cpuBitsSet[opcode & 255] + cpuBitsSet[(opcode >> 8) & 255]);
      u32 address = (temp + 4) & 0xFFFFFFFC;
       
      int offset = 0;
      if(opcode & 0x8000) {
        LDM_myregs(1, 0);
        LDM_myregs(2, 1);
        LDM_myregs(4, 2);
        LDM_myregs(8, 3);
        LDM_myregs(16, 4);
        LDM_myregs(32, 5);
        LDM_myregs(64, 6);
        LDM_myregs(128, 7);
        LDM_myregs(256, 8);
        LDM_myregs(512, 9);
        LDM_myregs(1024, 10);
        LDM_myregs(2048, 11);
        LDM_myregs(4096, 12);
        LDM_myregs(8192, 13);
        LDM_myregs(16384, 14);

        myregs[15].I = CPUReadMemory(address);

        if(!(opcode & (1 << base)))    
          myregs[base].I = temp;

        //CPUSwitchMode(myregs[17].I & 0x1f, false);
        if(armState) {
          armNextPC = myregs[15].I & 0xFFFFFFFC;
          myregs[15].I = armNextPC + 4; 
        } else {
          armNextPC = myregs[15].I & 0xFFFFFFFE;
          myregs[15].I = armNextPC + 2;
          //THUMB_PREFETCH;
        }
      } else {
        LDM_myregs(1, 0);
        LDM_myregs(2, 1);
        LDM_myregs(4, 2);
        LDM_myregs(8, 3);
        LDM_myregs(16, 4);
        LDM_myregs(32, 5);
        LDM_myregs(64, 6);
        LDM_myregs(128, 7);

        if(armMode == 0x11) {
          LDM_myregs(256, R8_FIQ);
          LDM_myregs(512, R9_FIQ);
          LDM_myregs(1024, R10_FIQ);
          LDM_myregs(2048, R11_FIQ);
          LDM_myregs(4096, R12_FIQ);
        } else {
          LDM_myregs(256, 8);
          LDM_myregs(512, 9);
          LDM_myregs(1024, 10);
          LDM_myregs(2048, 11);
          LDM_myregs(4096, 12);
        }

        if(armMode != 0x10 && armMode != 0x1f) {
          LDM_myregs(8192, R13_USR);
          LDM_myregs(16384, R14_USR);
        } else {
          LDM_myregs(8192, 13);
          LDM_myregs(16384, 14);
        }

        if(!(opcode & (1 << base)))    
          myregs[base].I = temp;
      }
       
    }
    break;
    
  CASE_16(0x890)
    {
      // LDMIA Rn, {Rlist}
 
      int base = (opcode & 0x000F0000) >> 16;
      u32 address = myregs[base].I & 0xFFFFFFFC;
       
      int offset = 0;
      LDM_myregs(1, 0);
      LDM_myregs(2, 1);
      LDM_myregs(4, 2);
      LDM_myregs(8, 3);
      LDM_myregs(16, 4);
      LDM_myregs(32, 5);
      LDM_myregs(64, 6);
      LDM_myregs(128, 7);
      LDM_myregs(256, 8);
      LDM_myregs(512, 9);
      LDM_myregs(1024, 10);
      LDM_myregs(2048, 11);
      LDM_myregs(4096, 12);
      LDM_myregs(8192, 13);
      LDM_myregs(16384, 14);
      if(opcode & 32768) {
        myregs[15].I = CPUReadMemory(address);

        armNextPC = myregs[15].I;
        myregs[15].I += 4;
        //ARM_PREFETCH;
 
      }
       
    }
  break;
  CASE_16(0x8b0)
    {
      // LDMIA Rn!, {Rlist}
 
      int base = (opcode & 0x000F0000) >> 16;
      u32 temp = myregs[base].I + 
        4 * (cpuBitsSet[opcode & 255] + cpuBitsSet[(opcode >> 8) & 255]);
      u32 address = myregs[base].I & 0xFFFFFFFC;
       
      int offset = 0;
      LDM_myregs(1, 0);
      LDM_myregs(2, 1);
      LDM_myregs(4, 2);
      LDM_myregs(8, 3);
      LDM_myregs(16, 4);
      LDM_myregs(32, 5);
      LDM_myregs(64, 6);
      LDM_myregs(128, 7);
      LDM_myregs(256, 8);
      LDM_myregs(512, 9);
      LDM_myregs(1024, 10);
      LDM_myregs(2048, 11);
      LDM_myregs(4096, 12);
      LDM_myregs(8192, 13);
      LDM_myregs(16384, 14);
      if(opcode & 32768) {
        myregs[15].I = CPUReadMemory(address);

        armNextPC = myregs[15].I;
        myregs[15].I += 4;
        //ARM_PREFETCH;
      }
       
      if(!(opcode & (1 << base)))
        myregs[base].I = temp;
    }
    break;
  CASE_16(0x8d0)
    {
      // LDMIA Rn, {Rlist}^
 
      int base = (opcode & 0x000F0000) >> 16;
      u32 address = myregs[base].I & 0xFFFFFFFC;
       
      int offset = 0;
      if(opcode & 0x8000) {
        LDM_myregs(1, 0);
        LDM_myregs(2, 1);
        LDM_myregs(4, 2);
        LDM_myregs(8, 3);
        LDM_myregs(16, 4);
        LDM_myregs(32, 5);
        LDM_myregs(64, 6);
        LDM_myregs(128, 7);
        LDM_myregs(256, 8);
        LDM_myregs(512, 9);
        LDM_myregs(1024, 10);
        LDM_myregs(2048, 11);
        LDM_myregs(4096, 12);
        LDM_myregs(8192, 13);
        LDM_myregs(16384, 14);

        myregs[15].I = CPUReadMemory(address);

        //CPUSwitchMode(myregs[17].I & 0x1f, false);
        if(armState) {
          armNextPC = myregs[15].I & 0xFFFFFFFC;
          myregs[15].I = armNextPC + 4; 
        } else {
          armNextPC = myregs[15].I & 0xFFFFFFFE;
          myregs[15].I = armNextPC + 2;
          //THUMB_PREFETCH;
        }
      } else {
        LDM_myregs(1, 0);
        LDM_myregs(2, 1);
        LDM_myregs(4, 2);
        LDM_myregs(8, 3);
        LDM_myregs(16, 4);
        LDM_myregs(32, 5);
        LDM_myregs(64, 6);
        LDM_myregs(128, 7);

        if(armMode == 0x11) {
          LDM_myregs(256, R8_FIQ);
          LDM_myregs(512, R9_FIQ);
          LDM_myregs(1024, R10_FIQ);
          LDM_myregs(2048, R11_FIQ);
          LDM_myregs(4096, R12_FIQ);
        } else {
          LDM_myregs(256, 8);
          LDM_myregs(512, 9);
          LDM_myregs(1024, 10);
          LDM_myregs(2048, 11);
          LDM_myregs(4096, 12);
        }

        if(armMode != 0x10 && armMode != 0x1f) {
          LDM_myregs(8192, R13_USR);
          LDM_myregs(16384, R14_USR);
        } else {
          LDM_myregs(8192, 13);
          LDM_myregs(16384, 14);
        }
      }
       
    }
  break;
  CASE_16(0x8f0)
    {
      // LDMIA Rn!, {Rlist}^
 
      int base = (opcode & 0x000F0000) >> 16;
      u32 temp = myregs[base].I +
        4 * (cpuBitsSet[opcode & 255] + cpuBitsSet[(opcode >> 8) & 255]);
      u32 address = myregs[base].I & 0xFFFFFFFC;
       
      int offset = 0;
      if(opcode & 0x8000) {
        LDM_myregs(1, 0);
        LDM_myregs(2, 1);
        LDM_myregs(4, 2);
        LDM_myregs(8, 3);
        LDM_myregs(16, 4);
        LDM_myregs(32, 5);
        LDM_myregs(64, 6);
        LDM_myregs(128, 7);
        LDM_myregs(256, 8);
        LDM_myregs(512, 9);
        LDM_myregs(1024, 10);
        LDM_myregs(2048, 11);
        LDM_myregs(4096, 12);
        LDM_myregs(8192, 13);
        LDM_myregs(16384, 14);

        myregs[15].I = CPUReadMemory(address);

        if(!(opcode & (1 << base)))    
          myregs[base].I = temp;

        //CPUSwitchMode(myregs[17].I & 0x1f, false);
        if(armState) {
          armNextPC = myregs[15].I & 0xFFFFFFFC;
          myregs[15].I = armNextPC + 4; 
        } else {
          armNextPC = myregs[15].I & 0xFFFFFFFE;
          myregs[15].I = armNextPC + 2;
          //THUMB_PREFETCH;
        }
      } else {
        LDM_myregs(1, 0);
        LDM_myregs(2, 1);
        LDM_myregs(4, 2);
        LDM_myregs(8, 3);
        LDM_myregs(16, 4);
        LDM_myregs(32, 5);
        LDM_myregs(64, 6);
        LDM_myregs(128, 7);

        if(armMode == 0x11) {
          LDM_myregs(256, R8_FIQ);
          LDM_myregs(512, R9_FIQ);
          LDM_myregs(1024, R10_FIQ);
          LDM_myregs(2048, R11_FIQ);
          LDM_myregs(4096, R12_FIQ);
        } else {
          LDM_myregs(256, 8);
          LDM_myregs(512, 9);
          LDM_myregs(1024, 10);
          LDM_myregs(2048, 11);
          LDM_myregs(4096, 12);
        }

        if(armMode != 0x10 && armMode != 0x1f) {
          LDM_myregs(8192, R13_USR);
          LDM_myregs(16384, R14_USR);
        } else {
          LDM_myregs(8192, 13);
          LDM_myregs(16384, 14);
        }

        if(!(opcode & (1 << base)))    
          myregs[base].I = temp;
 
      }
       
    }
    break;
    
  CASE_16(0x910)
    {
      // LDMDB Rn, {Rlist}
 
      int base = (opcode & 0x000F0000) >> 16;
      u32 temp = myregs[base].I - 
        4 * (cpuBitsSet[opcode & 255] + cpuBitsSet[(opcode >> 8) & 255]);
      u32 address = temp & 0xFFFFFFFC;
       
      int offset = 0;
      LDM_myregs(1, 0);
      LDM_myregs(2, 1);
      LDM_myregs(4, 2);
      LDM_myregs(8, 3);
      LDM_myregs(16, 4);
      LDM_myregs(32, 5);
      LDM_myregs(64, 6);
      LDM_myregs(128, 7);
      LDM_myregs(256, 8);
      LDM_myregs(512, 9);
      LDM_myregs(1024, 10);
      LDM_myregs(2048, 11);
      LDM_myregs(4096, 12);
      LDM_myregs(8192, 13);
      LDM_myregs(16384, 14);
      if(opcode & 32768) {
        myregs[15].I = CPUReadMemory(address);

        armNextPC = myregs[15].I;
        myregs[15].I += 4;
        //ARM_PREFETCH;
 
      }
       
    }
    break;
  CASE_16(0x930)
    {
      // LDMDB Rn!, {Rlist}
 
      int base = (opcode & 0x000F0000) >> 16;
      u32 temp = myregs[base].I - 
        4 * (cpuBitsSet[opcode & 255] + cpuBitsSet[(opcode >> 8) & 255]);
      u32 address = temp & 0xFFFFFFFC;
       
      int offset = 0;
      LDM_myregs(1, 0);
      LDM_myregs(2, 1);
      LDM_myregs(4, 2);
      LDM_myregs(8, 3);
      LDM_myregs(16, 4);
      LDM_myregs(32, 5);
      LDM_myregs(64, 6);
      LDM_myregs(128, 7);
      LDM_myregs(256, 8);
      LDM_myregs(512, 9);
      LDM_myregs(1024, 10);
      LDM_myregs(2048, 11);
      LDM_myregs(4096, 12);
      LDM_myregs(8192, 13);
      LDM_myregs(16384, 14);
      if(opcode & 32768) {
        myregs[15].I = CPUReadMemory(address);

        armNextPC = myregs[15].I;
        myregs[15].I += 4;
        //ARM_PREFETCH;
      }
       
      if(!(opcode & (1 << base)))
        myregs[base].I = temp;
    }
    break;
  CASE_16(0x950)
    {
      // LDMDB Rn, {Rlist}^
 
      int base = (opcode & 0x000F0000) >> 16;
      u32 temp = myregs[base].I - 
        4 * (cpuBitsSet[opcode & 255] + cpuBitsSet[(opcode >> 8) & 255]);
      u32 address = temp & 0xFFFFFFFC;
       
      int offset = 0;
      if(opcode & 0x8000) {
        LDM_myregs(1, 0);
        LDM_myregs(2, 1);
        LDM_myregs(4, 2);
        LDM_myregs(8, 3);
        LDM_myregs(16, 4);
        LDM_myregs(32, 5);
        LDM_myregs(64, 6);
        LDM_myregs(128, 7);
        LDM_myregs(256, 8);
        LDM_myregs(512, 9);
        LDM_myregs(1024, 10);
        LDM_myregs(2048, 11);
        LDM_myregs(4096, 12);
        LDM_myregs(8192, 13);
        LDM_myregs(16384, 14);

        myregs[15].I = CPUReadMemory(address);

        //CPUSwitchMode(myregs[17].I & 0x1f, false);
        if(armState) {
          armNextPC = myregs[15].I & 0xFFFFFFFC;
          myregs[15].I = armNextPC + 4; 
        } else {
          armNextPC = myregs[15].I & 0xFFFFFFFE;
          myregs[15].I = armNextPC + 2;
          //THUMB_PREFETCH;
        }
      } else {
        LDM_myregs(1, 0);
        LDM_myregs(2, 1);
        LDM_myregs(4, 2);
        LDM_myregs(8, 3);
        LDM_myregs(16, 4);
        LDM_myregs(32, 5);
        LDM_myregs(64, 6);
        LDM_myregs(128, 7);

        if(armMode == 0x11) {
          LDM_myregs(256, R8_FIQ);
          LDM_myregs(512, R9_FIQ);
          LDM_myregs(1024, R10_FIQ);
          LDM_myregs(2048, R11_FIQ);
          LDM_myregs(4096, R12_FIQ);
        } else {
          LDM_myregs(256, 8);
          LDM_myregs(512, 9);
          LDM_myregs(1024, 10);
          LDM_myregs(2048, 11);
          LDM_myregs(4096, 12);
        }

        if(armMode != 0x10 && armMode != 0x1f) {
          LDM_myregs(8192, R13_USR);
          LDM_myregs(16384, R14_USR);
        } else {
          LDM_myregs(8192, 13);
          LDM_myregs(16384, 14);
        }
      }
       
    }
    break;
  CASE_16(0x970)
    {
      // LDMDB Rn!, {Rlist}^
 
      int base = (opcode & 0x000F0000) >> 16;
      u32 temp = myregs[base].I - 
        4 * (cpuBitsSet[opcode & 255] + cpuBitsSet[(opcode >> 8) & 255]);
      u32 address = temp & 0xFFFFFFFC;
       
      int offset = 0;
      if(opcode & 0x8000) {
        LDM_myregs(1, 0);
        LDM_myregs(2, 1);
        LDM_myregs(4, 2);
        LDM_myregs(8, 3);
        LDM_myregs(16, 4);
        LDM_myregs(32, 5);
        LDM_myregs(64, 6);
        LDM_myregs(128, 7);
        LDM_myregs(256, 8);
        LDM_myregs(512, 9);
        LDM_myregs(1024, 10);
        LDM_myregs(2048, 11);
        LDM_myregs(4096, 12);
        LDM_myregs(8192, 13);
        LDM_myregs(16384, 14);

        myregs[15].I = CPUReadMemory(address);

        if(!(opcode & (1 << base)))    
          myregs[base].I = temp;

        //CPUSwitchMode(myregs[17].I & 0x1f, false);
        if(armState) {
          armNextPC = myregs[15].I & 0xFFFFFFFC;
          myregs[15].I = armNextPC + 4; 
        } else {
          armNextPC = myregs[15].I & 0xFFFFFFFE;
          myregs[15].I = armNextPC + 2;
          //THUMB_PREFETCH;
        }
      } else {
        LDM_myregs(1, 0);
        LDM_myregs(2, 1);
        LDM_myregs(4, 2);
        LDM_myregs(8, 3);
        LDM_myregs(16, 4);
        LDM_myregs(32, 5);
        LDM_myregs(64, 6);
        LDM_myregs(128, 7);

        if(armMode == 0x11) {
          LDM_myregs(256, R8_FIQ);
          LDM_myregs(512, R9_FIQ);
          LDM_myregs(1024, R10_FIQ);
          LDM_myregs(2048, R11_FIQ);
          LDM_myregs(4096, R12_FIQ);
        } else {
          LDM_myregs(256, 8);
          LDM_myregs(512, 9);
          LDM_myregs(1024, 10);
          LDM_myregs(2048, 11);
          LDM_myregs(4096, 12);
        }

        if(armMode != 0x10 && armMode != 0x1f) {
          LDM_myregs(8192, R13_USR);
          LDM_myregs(16384, R14_USR);
        } else {
          LDM_myregs(8192, 13);
          LDM_myregs(16384, 14);
        }

        if(!(opcode & (1 << base)))    
          myregs[base].I = temp;
      }
    }
    break;
    
  CASE_16(0x990)
    {
      // LDMIB Rn, {Rlist}
      int base = (opcode & 0x000F0000) >> 16;
      u32 address = (myregs[base].I+4) & 0xFFFFFFFC;
      int offset = 0;
      LDM_myregs(1, 0);
      LDM_myregs(2, 1);
      LDM_myregs(4, 2);
      LDM_myregs(8, 3);
      LDM_myregs(16, 4);
      LDM_myregs(32, 5);
      LDM_myregs(64, 6);
      LDM_myregs(128, 7);
      LDM_myregs(256, 8);
      LDM_myregs(512, 9);
      LDM_myregs(1024, 10);
      LDM_myregs(2048, 11);
      LDM_myregs(4096, 12);
      LDM_myregs(8192, 13);
      LDM_myregs(16384, 14);
      if(opcode & 32768) {
        myregs[15].I = CPUReadMemory(address);

        armNextPC = myregs[15].I;
        myregs[15].I += 4;
        //ARM_PREFETCH;
      }
    }
    break;
  CASE_16(0x9b0)
    {
      // LDMIB Rn!, {Rlist}
      int base = (opcode & 0x000F0000) >> 16;
      u32 temp = myregs[base].I + 
        4 * (cpuBitsSet[opcode & 255] + cpuBitsSet[(opcode >> 8) & 255]);
      u32 address = (myregs[base].I+4) & 0xFFFFFFFC;
      int offset = 0;
      LDM_myregs(1, 0);
      LDM_myregs(2, 1);
      LDM_myregs(4, 2);
      LDM_myregs(8, 3);
      LDM_myregs(16, 4);
      LDM_myregs(32, 5);
      LDM_myregs(64, 6);
      LDM_myregs(128, 7);
      LDM_myregs(256, 8);
      LDM_myregs(512, 9);
      LDM_myregs(1024, 10);
      LDM_myregs(2048, 11);
      LDM_myregs(4096, 12);
      LDM_myregs(8192, 13);
      LDM_myregs(16384, 14);
      if(opcode & 32768) {
        myregs[15].I = CPUReadMemory(address);

        armNextPC = myregs[15].I;
        myregs[15].I += 4;
        //ARM_PREFETCH;
      }
      if(!(opcode & (1 << base)))
        myregs[base].I = temp;
    }
    break;    
  CASE_16(0x9d0)
    {
      // LDMIB Rn, {Rlist}^
      int base = (opcode & 0x000F0000) >> 16;
      u32 address = (myregs[base].I+4) & 0xFFFFFFFC;
      int offset = 0;
      if(opcode & 0x8000) {
        LDM_myregs(1, 0);
        LDM_myregs(2, 1);
        LDM_myregs(4, 2);
        LDM_myregs(8, 3);
        LDM_myregs(16, 4);
        LDM_myregs(32, 5);
        LDM_myregs(64, 6);
        LDM_myregs(128, 7);
        LDM_myregs(256, 8);
        LDM_myregs(512, 9);
        LDM_myregs(1024, 10);
        LDM_myregs(2048, 11);
        LDM_myregs(4096, 12);
        LDM_myregs(8192, 13);
        LDM_myregs(16384, 14);

        myregs[15].I = CPUReadMemory(address);

        //CPUSwitchMode(myregs[17].I & 0x1f, false);
        if(armState) {
          armNextPC = myregs[15].I & 0xFFFFFFFC;
          myregs[15].I = armNextPC + 4; 
        } else {
          armNextPC = myregs[15].I & 0xFFFFFFFE;
          myregs[15].I = armNextPC + 2;
          //THUMB_PREFETCH;
        }
      } else {
        LDM_myregs(1, 0);
        LDM_myregs(2, 1);
        LDM_myregs(4, 2);
        LDM_myregs(8, 3);
        LDM_myregs(16, 4);
        LDM_myregs(32, 5);
        LDM_myregs(64, 6);
        LDM_myregs(128, 7);

        if(armMode == 0x11) {
          LDM_myregs(256, R8_FIQ);
          LDM_myregs(512, R9_FIQ);
          LDM_myregs(1024, R10_FIQ);
          LDM_myregs(2048, R11_FIQ);
          LDM_myregs(4096, R12_FIQ);
        } else {
          LDM_myregs(256, 8);
          LDM_myregs(512, 9);
          LDM_myregs(1024, 10);
          LDM_myregs(2048, 11);
          LDM_myregs(4096, 12);
        }

        if(armMode != 0x10 && armMode != 0x1f) {
          LDM_myregs(8192, R13_USR);
          LDM_myregs(16384, R14_USR);
        } else {
          LDM_myregs(8192, 13);
          LDM_myregs(16384, 14);
        }
      }
    }
    break;
  CASE_16(0x9f0)
    {
      // LDMIB Rn!, {Rlist}^
      int base = (opcode & 0x000F0000) >> 16;
      u32 temp = myregs[base].I +
        4 * (cpuBitsSet[opcode & 255] + cpuBitsSet[(opcode >> 8) & 255]);
      u32 address = (myregs[base].I+4) & 0xFFFFFFFC;
      int offset = 0;
      if(opcode & 0x8000) {
        LDM_myregs(1, 0);
        LDM_myregs(2, 1);
        LDM_myregs(4, 2);
        LDM_myregs(8, 3);
        LDM_myregs(16, 4);
        LDM_myregs(32, 5);
        LDM_myregs(64, 6);
        LDM_myregs(128, 7);
        LDM_myregs(256, 8);
        LDM_myregs(512, 9);
        LDM_myregs(1024, 10);
        LDM_myregs(2048, 11);
        LDM_myregs(4096, 12);
        LDM_myregs(8192, 13);
        LDM_myregs(16384, 14);

        myregs[15].I = CPUReadMemory(address);

        if(!(opcode & (1 << base)))    
          myregs[base].I = temp;

        //CPUSwitchMode(myregs[17].I & 0x1f, false);
        if(armState) {
          armNextPC = myregs[15].I & 0xFFFFFFFC;
          myregs[15].I = armNextPC + 4; 
        } else {
          armNextPC = myregs[15].I & 0xFFFFFFFE;
          myregs[15].I = armNextPC + 2;
          //THUMB_PREFETCH;
        }
      } else {
        LDM_myregs(1, 0);
        LDM_myregs(2, 1);
        LDM_myregs(4, 2);
        LDM_myregs(8, 3);
        LDM_myregs(16, 4);
        LDM_myregs(32, 5);
        LDM_myregs(64, 6);
        LDM_myregs(128, 7);

        if(armMode == 0x11) {
          LDM_myregs(256, R8_FIQ);
          LDM_myregs(512, R9_FIQ);
          LDM_myregs(1024, R10_FIQ);
          LDM_myregs(2048, R11_FIQ);
          LDM_myregs(4096, R12_FIQ);
        } else {
          LDM_myregs(256, 8);
          LDM_myregs(512, 9);
          LDM_myregs(1024, 10);
          LDM_myregs(2048, 11);
          LDM_myregs(4096, 12);
        }

        if(armMode != 0x10 && armMode != 0x1f) {
          LDM_myregs(8192, R13_USR);
          LDM_myregs(16384, R14_USR);
        } else {
          LDM_myregs(8192, 13);
          LDM_myregs(16384, 14);
        }

        if(!(opcode & (1 << base)))    
          myregs[base].I = temp;
      }
    }
    break;    
  CASE_256(0xa00)
    {
      // B <offset>
      int offset = opcode & 0x00FFFFFF;
      if(offset & 0x00800000) {
        offset |= 0xFF000000;
      }
      offset <<= 2;
      myregs[15].I += offset;
      armNextPC = myregs[15].I;
      myregs[15].I += 4;
      //ARM_PREFETCH;
    }
    break;
  CASE_256(0xb00)
    {
      // BL <offset>
      int offset = opcode & 0x00FFFFFF;
      if(offset & 0x00800000) {
        offset |= 0xFF000000;
      }
      offset <<= 2;
      myregs[14].I = myregs[15].I - 4;
      myregs[15].I += offset;
      armNextPC = myregs[15].I;
      myregs[15].I += 4;
      //ARM_PREFETCH;
    }
    break;
  CASE_256(0xf00)
    // SWI <comment>

    break;
#ifdef GP_SUPPORT
  case 0xe11:
  case 0xe13:
  case 0xe15:
  case 0xe17:
  case 0xe19:
  case 0xe1b:
  case 0xe1d:
  case 0xe1f:
    // MRC
    break;
  case 0xe01:
  case 0xe03:
  case 0xe05:
  case 0xe07:
  case 0xe09:
  case 0xe0b:
  case 0xe0d:
  case 0xe0f:
    // MRC
    break;    
#endif
  default:
#ifdef DEV_VERSION
      Log("Undefined ARM instruction %08x at %08x\n", opcode,
          armNextPC-4);
#endif
    break;
    // END
  }
}

}

void emuInstrTHUMB(u16 opcode, s32 *R)
{

reg_pair* myregs = (reg_pair*)R;


switch(opcode >> 8) {
 case 0x00:
 case 0x01:
 case 0x02:
 case 0x03:
 case 0x04:
 case 0x05:
 case 0x06:
 case 0x07:
   {
     // LSL Rd, Rm, #Imm 5
     int dest = opcode & 0x07;
     int source = (opcode >> 3) & 0x07;
     int shift = (opcode >> 6) & 0x1f;
     u32 value;
     
     if(shift) {
       LSL_RD_RM_I5;
     } else {
       value = myregs[source].I;
     }
     myregs[dest].I = value;
     // C_FLAG set above
     N_FLAG = (value & 0x80000000 ? true : false);
     Z_FLAG = (value ? false : true);
   }
   break;
 case 0x08:
 case 0x09:
 case 0x0a:
 case 0x0b:
 case 0x0c:
 case 0x0d:
 case 0x0e:
 case 0x0f:
   {
     // LSR Rd, Rm, #Imm 5
     int dest = opcode & 0x07;
     int source = (opcode >> 3) & 0x07;
     int shift = (opcode >> 6) & 0x1f;
     u32 value;
     
     if(shift) {
       LSR_RD_RM_I5;
     } else {
       C_FLAG = myregs[source].I & 0x80000000 ? true : false;
       value = 0;
     }
     myregs[dest].I = value;
     // C_FLAG set above
     N_FLAG = (value & 0x80000000 ? true : false);
     Z_FLAG = (value ? false : true);
   }
   break;
 case 0x10:
 case 0x11:
 case 0x12:
 case 0x13:
 case 0x14:
 case 0x15:
 case 0x16:
 case 0x17:
   {     
     // ASR Rd, Rm, #Imm 5
     int dest = opcode & 0x07;
     int source = (opcode >> 3) & 0x07;
     int shift = (opcode >> 6) & 0x1f;
     u32 value;
     
     if(shift) {
       ASR_RD_RM_I5;
     } else {
       if(myregs[source].I & 0x80000000) {
         value = 0xFFFFFFFF;
         C_FLAG = true;
       } else {
         value = 0;
         C_FLAG = false;
       }
     }
     myregs[dest].I = value;
     // C_FLAG set above
     N_FLAG = (value & 0x80000000 ? true : false);
     Z_FLAG = (value ? false :true);
   }
   break;
 case 0x18:
 case 0x19:
   {
     // ADD Rd, Rs, Rn
     int dest = opcode & 0x07;
     int source = (opcode >> 3) & 0x07;
     u32 value = myregs[(opcode>>6)& 0x07].I;
     ADD_RD_RS_RN;
   }
   break;
 case 0x1a:
 case 0x1b:
   {
     // SUB Rd, Rs, Rn
     int dest = opcode & 0x07;
     int source = (opcode >> 3) & 0x07;
     u32 value = myregs[(opcode>>6)& 0x07].I;
     SUB_RD_RS_RN;
   }
   break;
 case 0x1c:
 case 0x1d:
   {
     // ADD Rd, Rs, #Offset3
     int dest = opcode & 0x07;
     int source = (opcode >> 3) & 0x07;
     u32 value = (opcode >> 6) & 7;
     ADD_RD_RS_O3;
   }
   break;
 case 0x1e:
 case 0x1f:
   {
     // SUB Rd, Rs, #Offset3
     int dest = opcode & 0x07;
     int source = (opcode >> 3) & 0x07;
     u32 value = (opcode >> 6) & 7;
     SUB_RD_RS_O3;
   }
   break;
 case 0x20:
 case 0x21:
 case 0x22:
 case 0x23:
 case 0x24:
 case 0x25:
 case 0x26:
 case 0x27:
     {
   u8 myregsist = (opcode >> 8) & 7;
   // MOV R0~R7, #Offset8
   myregs[myregsist].I = opcode & 255;
   N_FLAG = false;
   Z_FLAG = (myregs[myregsist].I ? false : true);
     }
   break;
case 0x28:
   // CMP R0, #Offset8
   CMP_RN_O8(0);
   break;
 case 0x29:
   // CMP R1, #Offset8
   CMP_RN_O8(1);
   break;
 case 0x2a:
   // CMP R2, #Offset8
   CMP_RN_O8(2);
   break;
 case 0x2b:
   // CMP R3, #Offset8
   CMP_RN_O8(3);
   break;
 case 0x2c:
   // CMP R4, #Offset8
   CMP_RN_O8(4);
   break;
 case 0x2d:
   // CMP R5, #Offset8
   CMP_RN_O8(5);
   break;
 case 0x2e:
   // CMP R6, #Offset8
   CMP_RN_O8(6);
   break;
 case 0x2f:
   // CMP R7, #Offset8
   CMP_RN_O8(7);
   break;
 case 0x30:
   // ADD R0,#Offset8
   ADD_RN_O8(0);
   break;   
 case 0x31:
   // ADD R1,#Offset8
   ADD_RN_O8(1);
   break;   
 case 0x32:
   // ADD R2,#Offset8
   ADD_RN_O8(2);
   break;   
 case 0x33:
   // ADD R3,#Offset8
   ADD_RN_O8(3);
   break;   
 case 0x34:
   // ADD R4,#Offset8
   ADD_RN_O8(4);
   break;   
 case 0x35:
   // ADD R5,#Offset8
   ADD_RN_O8(5);
   break;   
 case 0x36:
   // ADD R6,#Offset8
   ADD_RN_O8(6);
   break;   
 case 0x37:
   // ADD R7,#Offset8
   ADD_RN_O8(7);
   break;
 case 0x38:
   // SUB R0,#Offset8
   SUB_RN_O8(0);
   break;
 case 0x39:
   // SUB R1,#Offset8
   SUB_RN_O8(1);
   break;
 case 0x3a:
   // SUB R2,#Offset8
   SUB_RN_O8(2);
   break;
 case 0x3b:
   // SUB R3,#Offset8
   SUB_RN_O8(3);
   break;
 case 0x3c:
   // SUB R4,#Offset8
   SUB_RN_O8(4);
   break;
 case 0x3d:
   // SUB R5,#Offset8
   SUB_RN_O8(5);
   break;
 case 0x3e:
   // SUB R6,#Offset8
   SUB_RN_O8(6);
   break;
 case 0x3f:
   // SUB R7,#Offset8
   SUB_RN_O8(7);
   break;

 case 0x40:
   switch((opcode >> 6) & 3) {
   case 0x00:
     {
       // AND Rd, Rs
       int dest = opcode & 7;
       myregs[dest].I &= myregs[(opcode >> 3)&7].I;
       N_FLAG = myregs[dest].I & 0x80000000 ? true : false;
       Z_FLAG = myregs[dest].I ? false : true;
#ifdef BKPT_SUPPORT     
#define THUMB_CONSOLE_OUTPUT(a,b) \
  if((opcode == 0x4000) && (myregs[0].I == 0xC0DED00D)) {\
    extern void (*dbgOutput)(char *, u32);\
    dbgOutput((a), (b));\
  }
#else
#define THUMB_CONSOLE_OUTPUT(a,b)
#endif
       THUMB_CONSOLE_OUTPUT(NULL, myregs[2].I);
     }
     break;
   case 0x01:
     // EOR Rd, Rs
     {
       int dest = opcode & 7;
       myregs[dest].I ^= myregs[(opcode >> 3)&7].I;
       N_FLAG = myregs[dest].I & 0x80000000 ? true : false;
       Z_FLAG = myregs[dest].I ? false : true;
     }
     break;
   case 0x02:
     // LSL Rd, Rs
     {
       int dest = opcode & 7;
       u32 value = myregs[(opcode >> 3)&7].B.B0;
       if(value) {
         if(value == 32) {
           value = 0;
           C_FLAG = (myregs[dest].I & 1 ? true : false);
         } else if(value < 32) {
           LSL_RD_RS;
         } else {
           value = 0;
           C_FLAG = false;
         }
         myregs[dest].I = value;        
       }
       N_FLAG = myregs[dest].I & 0x80000000 ? true : false;
       Z_FLAG = myregs[dest].I ? false : true;
     }
     break;
   case 0x03:
     {
       // LSR Rd, Rs
       int dest = opcode & 7;
       u32 value = myregs[(opcode >> 3)&7].B.B0;
       if(value) {
         if(value == 32) {
           value = 0;
           C_FLAG = (myregs[dest].I & 0x80000000 ? true : false);
         } else if(value < 32) {
           LSR_RD_RS;
         } else {
           value = 0;
           C_FLAG = false;
         }
         myregs[dest].I = value;        
       }
       N_FLAG = myregs[dest].I & 0x80000000 ? true : false;
       Z_FLAG = myregs[dest].I ? false : true;
     }
     break;
   }
   break;
 case 0x41:
   switch((opcode >> 6) & 3) {
   case 0x00:
     {
       // ASR Rd, Rs
       int dest = opcode & 7;
       u32 value = myregs[(opcode >> 3)&7].B.B0;
       // ASR
       if(value) {
         if(value < 32) {
           ASR_RD_RS;
           myregs[dest].I = value;        
         } else {
           if(myregs[dest].I & 0x80000000){
             myregs[dest].I = 0xFFFFFFFF;
             C_FLAG = true;
           } else {
             myregs[dest].I = 0x00000000;
             C_FLAG = false;
           }
         }
       }
       N_FLAG = myregs[dest].I & 0x80000000 ? true : false;
       Z_FLAG = myregs[dest].I ? false : true;
     }
     break;
   case 0x01:
     {
       // ADC Rd, Rs
       int dest = opcode & 0x07;
       u32 value = myregs[(opcode >> 3)&7].I;
       // ADC
       ADC_RD_RS;
     }
     break;
   case 0x02:
     {
       // SBC Rd, Rs
       int dest = opcode & 0x07;
       u32 value = myregs[(opcode >> 3)&7].I;
       
       // SBC
       SBC_RD_RS;
     }
     break;
   case 0x03:
     // ROR Rd, Rs
     {
       int dest = opcode & 7;
       u32 value = myregs[(opcode >> 3)&7].B.B0;
       
       if(value) {
         value = value & 0x1f;
         if(value == 0) {
           C_FLAG = (myregs[dest].I & 0x80000000 ? true : false);
         } else {
           ROR_RD_RS;
           myregs[dest].I = value;
         }
       }
       N_FLAG = myregs[dest].I & 0x80000000 ? true : false;
       Z_FLAG = myregs[dest].I ? false : true;
     }
     break;
   }
   break;
 case 0x42:
   switch((opcode >> 6) & 3) {
   case 0x00:
     {
       // TST Rd, Rs
       u32 value = myregs[opcode & 7].I & myregs[(opcode >> 3) & 7].I;
       N_FLAG = value & 0x80000000 ? true : false;
       Z_FLAG = value ? false : true;
     }
     break;
   case 0x01:
     {
       // NEG Rd, Rs
       int dest = opcode & 7;
       int source = (opcode >> 3) & 7;
       NEG_RD_RS;
     }
     break;
   case 0x02:
     {
       // CMP Rd, Rs
       int dest = opcode & 7;
       u32 value = myregs[(opcode >> 3)&7].I;
       CMP_RD_RS;
     }
     break;
   case 0x03:
     {
       // CMN Rd, Rs
       int dest = opcode & 7;
       u32 value = myregs[(opcode >> 3)&7].I;
       // CMN
       CMN_RD_RS;
     }
     break;
   }
   break;
 case 0x43:
   switch((opcode >> 6) & 3) {
   case 0x00:
     {
       // ORR Rd, Rs       
       int dest = opcode & 7;
       myregs[dest].I |= myregs[(opcode >> 3) & 7].I;
       Z_FLAG = myregs[dest].I ? false : true;
       N_FLAG = myregs[dest].I & 0x80000000 ? true : false;
     }
     break;
   case 0x01:
     {
       // MUL Rd, Rs
       int dest = opcode & 7;
       u32 rm = myregs[dest].I;
       myregs[dest].I = myregs[(opcode >> 3) & 7].I * rm;
       if (((s32)rm) < 0)
         rm = ~rm;
       Z_FLAG = myregs[dest].I ? false : true;
       N_FLAG = myregs[dest].I & 0x80000000 ? true : false;
     }
     break;
   case 0x02:
     {
       // BIC Rd, Rs
       int dest = opcode & 7;
       myregs[dest].I &= (~myregs[(opcode >> 3) & 7].I);
       Z_FLAG = myregs[dest].I ? false : true;
       N_FLAG = myregs[dest].I & 0x80000000 ? true : false;
     }
     break;
   case 0x03:
     {
       // MVN Rd, Rs
       int dest = opcode & 7;
       myregs[dest].I = ~myregs[(opcode >> 3) & 7].I;
       Z_FLAG = myregs[dest].I ? false : true;
       N_FLAG = myregs[dest].I & 0x80000000 ? true : false;
     }
     break;
   }
   break;
 case 0x44:
   {
     int dest = opcode & 7;
     int base = (opcode >> 3) & 7;
     switch((opcode >> 6)& 3) {
     default:
       goto unknown_thumb;
     case 1:
       // ADD Rd, Hs
       myregs[dest].I += myregs[base+8].I;
       break;
     case 2:
       // ADD Hd, Rs
       myregs[dest+8].I += myregs[base].I;
       if(dest == 7) {
         myregs[15].I &= 0xFFFFFFFE;
         armNextPC = myregs[15].I;
         myregs[15].I += 2;
         ////THUMB_PREFETCH;
       }       
       break;
     case 3:
       // ADD Hd, Hs
       myregs[dest+8].I += myregs[base+8].I;
       if(dest == 7) {
         myregs[15].I &= 0xFFFFFFFE;
         armNextPC = myregs[15].I;
         myregs[15].I += 2;
         ////THUMB_PREFETCH;  
       }
       break;
     }
   }
   break;
 case 0x45:
   {
     int dest = opcode & 7;
     int base = (opcode >> 3) & 7;
     u32 value;
     switch((opcode >> 6) & 3) {
     case 0:
       // CMP Rd, Hs
       value = myregs[base].I;
       CMP_RD_RS;
       break;
     case 1:
       // CMP Rd, Hs
       value = myregs[base+8].I;
       CMP_RD_RS;
       break;
     case 2:
       // CMP Hd, Rs
       value = myregs[base].I;
       dest += 8;
       CMP_RD_RS;
       break;
     case 3:
       // CMP Hd, Hs
       value = myregs[base+8].I;
       dest += 8;
       CMP_RD_RS;
       break;
     }
   }
   break;
 case 0x46:
   {
     int dest = opcode & 7;
     int base = (opcode >> 3) & 7;
     switch((opcode >> 6) & 3) {
     case 0:
       // this form should not be used...
       // MOV Rd, Rs
       myregs[dest].I = myregs[base].I;
       break;
     case 1:
       // MOV Rd, Hs
       myregs[dest].I = myregs[base+8].I;
       break;
     case 2:
       // MOV Hd, Rs
       myregs[dest+8].I = myregs[base].I;
       if(dest == 7) {
#ifdef BKPT_SUPPORT
	     UPDATE_OLD_myregs
#endif

         myregs[15].I &= 0xFFFFFFFE;
         armNextPC = myregs[15].I;
         myregs[15].I += 2;
         ////THUMB_PREFETCH;
       }
       break;
     case 3:
       // MOV Hd, Hs
       myregs[dest+8].I = myregs[base+8].I;
       if(dest == 7) {

#ifdef BKPT_SUPPORT
	     UPDATE_OLD_myregs
#endif

         myregs[15].I &= 0xFFFFFFFE;
         armNextPC = myregs[15].I;
         myregs[15].I += 2;
         ////THUMB_PREFETCH;
       }   
       break;
     }
   }
   break;
 case 0x47:
   {
     int base = (opcode >> 3) & 7;
     switch((opcode >>6) & 3) {
     case 0:
       // BX Rs
#ifdef BKPT_SUPPORT
		 UPDATE_OLD_myregs
#endif
       myregs[15].I = (myregs[base].I) & 0xFFFFFFFE;
       if(myregs[base].I & 1) {
         armState = false;
         armNextPC = myregs[15].I;
         myregs[15].I += 2;
         ////THUMB_PREFETCH;
       } else {
         armState = true;
         myregs[15].I &= 0xFFFFFFFC;
         armNextPC = myregs[15].I;
         myregs[15].I += 4;
         ////ARM_PREFETCH;
       }
       break;
     case 1:
       // BX Hs

#ifdef BKPT_SUPPORT
		 UPDATE_OLD_myregs
#endif

       myregs[15].I = (myregs[8+base].I) & 0xFFFFFFFE;
       if(myregs[8+base].I & 1) {
         armState = false;
         armNextPC = myregs[15].I;
         myregs[15].I += 2;
         ////THUMB_PREFETCH;
       } else {
         armState = true;
         myregs[15].I &= 0xFFFFFFFC;       
         armNextPC = myregs[15].I;
         myregs[15].I += 4;
         //////ARM_PREFETCH;
       }
       break;
     default:
       goto unknown_thumb;
     }
   }
   break;
 case 0x48:
 case 0x49:
 case 0x4a:
 case 0x4b:
 case 0x4c:
 case 0x4d:
 case 0x4e:
 case 0x4f:
   // LDR R0~R7,[PC, #Imm]
   {
   u8 myregsist = (opcode >> 8) & 7;
     u32 address = (myregs[15].I & 0xFFFFFFFC) + ((opcode & 0xFF) << 2);
     myregs[myregsist].I = CPUReadMemoryQuick(address);
   }
   break;
 case 0x50:
 case 0x51:
   // STR Rd, [Rs, Rn]
   {
     u32 address = myregs[(opcode>>3)&7].I + myregs[(opcode>>6)&7].I;
     CPUWriteMemory(address,
                    myregs[opcode & 7].I);
   }
   break;
 case 0x52:
 case 0x53:
   // STRH Rd, [Rs, Rn]
   {
     u32 address = myregs[(opcode>>3)&7].I + myregs[(opcode>>6)&7].I;
     CPUWriteHalfWord(address,
                      myregs[opcode&7].W.W0);
   }
   break;
 case 0x54:
 case 0x55:
   // STRB Rd, [Rs, Rn]
   {
     u32 address = myregs[(opcode>>3)&7].I + myregs[(opcode >>6)&7].I;
     CPUWriteByte(address,
                  myregs[opcode & 7].B.B0);
   }
   break;
 case 0x56:
 case 0x57:
   // LDSB Rd, [Rs, Rn]
   {
     u32 address = myregs[(opcode>>3)&7].I + myregs[(opcode>>6)&7].I;
     myregs[opcode&7].I = (s8)CPUReadByte(address);
   }
   break;
 case 0x58:
 case 0x59:
   // LDR Rd, [Rs, Rn]
   {
     u32 address = myregs[(opcode>>3)&7].I + myregs[(opcode>>6)&7].I;
     myregs[opcode&7].I = CPUReadMemory(address);
   }
   break;
 case 0x5a:
 case 0x5b:
   // LDRH Rd, [Rs, Rn]
   {
     u32 address = myregs[(opcode>>3)&7].I + myregs[(opcode>>6)&7].I;
     myregs[opcode&7].I = CPUReadHalfWord(address);
   }
   break;
 case 0x5c:
 case 0x5d:
   // LDRB Rd, [Rs, Rn]
   {
     u32 address = myregs[(opcode>>3)&7].I + myregs[(opcode>>6)&7].I;
     myregs[opcode&7].I = CPUReadByte(address);
   }
   break;
 case 0x5e:
 case 0x5f:
   // LDSH Rd, [Rs, Rn]
   {
     u32 address = myregs[(opcode>>3)&7].I + myregs[(opcode>>6)&7].I;
     myregs[opcode&7].I = (s16)CPUReadHalfWordSigned(address);
   }
   break;
 case 0x60:
 case 0x61:
 case 0x62:
 case 0x63:
 case 0x64:
 case 0x65:
 case 0x66:
 case 0x67:
   // STR Rd, [Rs, #Imm]
   {
     u32 address = myregs[(opcode>>3)&7].I + (((opcode>>6)&31)<<2);
     CPUWriteMemory(address,
                    myregs[opcode&7].I);
   }
   break;
 case 0x68:
 case 0x69:
 case 0x6a:
 case 0x6b:
 case 0x6c:
 case 0x6d:
 case 0x6e:
 case 0x6f:
   // LDR Rd, [Rs, #Imm]
   {
     u32 address = myregs[(opcode>>3)&7].I + (((opcode>>6)&31)<<2);
     myregs[opcode&7].I = CPUReadMemory(address);
   }
   break;
 case 0x70:
 case 0x71:
 case 0x72:
 case 0x73:
 case 0x74:
 case 0x75:
 case 0x76:
 case 0x77:
   // STRB Rd, [Rs, #Imm]
   {
     u32 address = myregs[(opcode>>3)&7].I + (((opcode>>6)&31));
     CPUWriteByte(address,
                  myregs[opcode&7].B.B0);
   }
   break;
 case 0x78:
 case 0x79:
 case 0x7a:
 case 0x7b:
 case 0x7c:
 case 0x7d:
 case 0x7e:
 case 0x7f:
   // LDRB Rd, [Rs, #Imm]
   {
     u32 address = myregs[(opcode>>3)&7].I + (((opcode>>6)&31));
     myregs[opcode&7].I = CPUReadByte(address);
   }
   break;
 case 0x80:
 case 0x81:
 case 0x82:
 case 0x83:
 case 0x84:
 case 0x85:
 case 0x86:
 case 0x87:
   // STRH Rd, [Rs, #Imm]
   {
     u32 address = myregs[(opcode>>3)&7].I + (((opcode>>6)&31)<<1);
     CPUWriteHalfWord(address,
                      myregs[opcode&7].W.W0);
   }
   break;   
 case 0x88:
 case 0x89:
 case 0x8a:
 case 0x8b:
 case 0x8c:
 case 0x8d:
 case 0x8e:
 case 0x8f:
   // LDRH Rd, [Rs, #Imm]
   {
     u32 address = myregs[(opcode>>3)&7].I + (((opcode>>6)&31)<<1);
     myregs[opcode&7].I = CPUReadHalfWord(address);
   }
   break;
 case 0x90:
 case 0x91:
 case 0x92:
 case 0x93:
 case 0x94:
 case 0x95:
 case 0x96:
 case 0x97:
   // STR R0~R7, [SP, #Imm]
   {
   u8 myregsist = (opcode >> 8) & 7;
     u32 address = myregs[13].I + ((opcode&255)<<2);
     CPUWriteMemory(address, myregs[myregsist].I);
   }
   break;      
 case 0x98:
 case 0x99:
 case 0x9a:
 case 0x9b:
 case 0x9c:
 case 0x9d:
 case 0x9e:
 case 0x9f:
   // LDR R0~R7, [SP, #Imm]
   {
   u8 myregsist = (opcode >> 8) & 7;
     u32 address = myregs[13].I + ((opcode&255)<<2);   
     myregs[myregsist].I = CPUReadMemoryQuick(address);
   }
   break;
 case 0xa0:
 case 0xa1:
 case 0xa2:
 case 0xa3:
 case 0xa4:
 case 0xa5:
 case 0xa6:
 case 0xa7:
     {
   // ADD R0~R7, PC, Imm
   u8 myregsist = (opcode >> 8) & 7;
   myregs[myregsist].I = (myregs[15].I & 0xFFFFFFFC) + ((opcode&255)<<2);
     }
   break;   
 case 0xa8:
 case 0xa9:
 case 0xaa:
 case 0xab:
 case 0xac:
 case 0xad:
 case 0xae:
 case 0xaf:
     {
   // ADD R0~R7, SP, Imm
   u8 myregsist = (opcode >> 8) & 7;
   myregs[myregsist].I = myregs[13].I + ((opcode&255)<<2);
     }
   break;     
 case 0xb0:
   {
     // ADD SP, Imm
     int offset = (opcode & 127) << 2;
     if(opcode & 0x80)
       offset = -offset;
     myregs[13].I += offset;
   }
   break;
#define PUSH_myregs(val, r) \
  if(opcode & (val)) {\
    CPUWriteMemory(address, myregs[(r)].I);\
    offset = 1;\
    address += 4;\
  }
 case 0xb4:
   // PUSH {Rlist}
   {
     int offset = 0;
     u32 temp = myregs[13].I - 4 * cpuBitsSet[opcode & 0xff];
     u32 address = temp & 0xFFFFFFFC;
     PUSH_myregs(1, 0);
     PUSH_myregs(2, 1);
     PUSH_myregs(4, 2);
     PUSH_myregs(8, 3);
     PUSH_myregs(16, 4);
     PUSH_myregs(32, 5);
     PUSH_myregs(64, 6);
     PUSH_myregs(128, 7);
     myregs[13].I = temp;
   }
   break;
 case 0xb5:
   // PUSH {Rlist, LR}
   {
     int offset = 0;
     u32 temp = myregs[13].I - 4 - 4 * cpuBitsSet[opcode & 0xff];
     u32 address = temp & 0xFFFFFFFC;
     PUSH_myregs(1, 0);
     PUSH_myregs(2, 1);
     PUSH_myregs(4, 2);
     PUSH_myregs(8, 3);
     PUSH_myregs(16, 4);
     PUSH_myregs(32, 5);
     PUSH_myregs(64, 6);
     PUSH_myregs(128, 7);
     PUSH_myregs(256, 14);
     myregs[13].I = temp;
   }
   break;
#define POP_myregs(val, r) \
  if(opcode & (val)) {\
    myregs[(r)].I = CPUReadMemory(address);\
    offset = 1;\
    address += 4;\
  }
 case 0xbc:
   // POP {Rlist}
   {
     int offset = 0;
     u32 address = myregs[13].I & 0xFFFFFFFC;
     u32 temp = myregs[13].I + 4*cpuBitsSet[opcode & 0xFF];
     POP_myregs(1, 0);
     POP_myregs(2, 1);
     POP_myregs(4, 2);
     POP_myregs(8, 3);
     POP_myregs(16, 4);
     POP_myregs(32, 5);
     POP_myregs(64, 6);
     POP_myregs(128, 7);
     myregs[13].I = temp;
   }
   break;   
 case 0xbd:
   // POP {Rlist, PC}
   {
     int offset = 0;
     u32 address = myregs[13].I & 0xFFFFFFFC;
     u32 temp = myregs[13].I + 4 + 4*cpuBitsSet[opcode & 0xFF];
     POP_myregs(1, 0);
     POP_myregs(2, 1);
     POP_myregs(4, 2);
     POP_myregs(8, 3);
     POP_myregs(16, 4);
     POP_myregs(32, 5);
     POP_myregs(64, 6);
     POP_myregs(128, 7);
     myregs[15].I = (CPUReadMemory(address) & 0xFFFFFFFE);
     armNextPC = myregs[15].I;
     myregs[15].I += 2;
     myregs[13].I = temp;
     ////THUMB_PREFETCH;
   }
   break;      
#define THUMB_STM_myregs(val,r,b) \
  if(opcode & (val)) {\
    CPUWriteMemory(address, myregs[(r)].I);\
    if(!offset) {\
      myregs[(b)].I = temp;\
    }\
    offset = 1;\
    address += 4;\
  }
 case 0xc0:
 case 0xc1:
 case 0xc2:
 case 0xc3:
 case 0xc4:
 case 0xc5:
 case 0xc6:
 case 0xc7:
   {
     // STM R0~7!, {Rlist}
     u8 myregsist = (opcode >> 8) & 7;
     u32 address = myregs[myregsist].I & 0xFFFFFFFC;
     u32 temp = myregs[myregsist].I + 4*cpuBitsSet[opcode & 0xff];
     int offset = 0;
     // store
     THUMB_STM_myregs(1, 0, myregsist);
     THUMB_STM_myregs(2, 1, myregsist);
     THUMB_STM_myregs(4, 2, myregsist);
     THUMB_STM_myregs(8, 3, myregsist);
     THUMB_STM_myregs(16, 4, myregsist);
     THUMB_STM_myregs(32, 5, myregsist);
     THUMB_STM_myregs(64, 6, myregsist);
     THUMB_STM_myregs(128, 7, myregsist);
   }
   break;   
#define THUMB_LDM_myregs(val,r) \
  if(opcode & (val)) {\
    myregs[(r)].I = CPUReadMemory(address);\
    offset = 1;\
    address += 4;\
  }
 case 0xc8:
 case 0xc9:
 case 0xca:
 case 0xcb:
 case 0xcc:
 case 0xcd:
 case 0xce:
 case 0xcf:
   {
     // LDM R0~R7!, {Rlist}
     u8 myregsist = (opcode >> 8) & 7;
	//iprintf("%x\n",myregs[myregsist].I);
	myregs[myregsist].I -= 4;
     u32 address = myregs[myregsist].I & 0xFFFFFFFC;
     u32 temp = myregs[myregsist].I + 4*cpuBitsSet[opcode & 0xFF]; //ichfly - 4 by me
     int offset = 0;
     // load
     THUMB_LDM_myregs(1, 0);
     THUMB_LDM_myregs(2, 1);
     THUMB_LDM_myregs(4, 2);
     THUMB_LDM_myregs(8, 3);
     THUMB_LDM_myregs(16, 4);
     THUMB_LDM_myregs(32, 5);
     THUMB_LDM_myregs(64, 6);
     THUMB_LDM_myregs(128, 7);
     if(!(opcode & (1<<myregsist)))
       myregs[myregsist].I = temp;
	   //iprintf("%x\n",myregs[myregsist].I);
   }
   break;
 case 0xd0:
   // BEQ offset
#ifdef BKPT_SUPPORT
		 UPDATE_OLD_myregs
#endif
   if(Z_FLAG) {
     myregs[15].I += ((s8)(opcode & 0xFF)) << 1;
     armNextPC = myregs[15].I;
     myregs[15].I += 2;
     ////THUMB_PREFETCH;
   }
   break;      
 case 0xd1:
   // BNE offset
   if(!Z_FLAG) {
#ifdef BKPT_SUPPORT
		 UPDATE_OLD_myregs
#endif
     myregs[15].I += ((s8)(opcode & 0xFF)) << 1;       
     armNextPC = myregs[15].I;
     myregs[15].I += 2;
     ////THUMB_PREFETCH;

   }
   break;   
 case 0xd2:
   // BCS offset
   if(C_FLAG) {
#ifdef BKPT_SUPPORT
		 UPDATE_OLD_myregs
#endif
     myregs[15].I += ((s8)(opcode & 0xFF)) << 1;       
     armNextPC = myregs[15].I;
     myregs[15].I += 2;
     ////THUMB_PREFETCH;

   }
   break;   
 case 0xd3:
   // BCC offset
   if(!C_FLAG) {
#ifdef BKPT_SUPPORT
		 UPDATE_OLD_myregs
#endif
     myregs[15].I += ((s8)(opcode & 0xFF)) << 1;       
     armNextPC = myregs[15].I;
     myregs[15].I += 2;
     ////THUMB_PREFETCH;
   }
   break;   
 case 0xd4:
   // BMI offset
   if(N_FLAG) {
#ifdef BKPT_SUPPORT
		 UPDATE_OLD_myregs
#endif
     myregs[15].I += ((s8)(opcode & 0xFF)) << 1;       
     armNextPC = myregs[15].I;
     myregs[15].I += 2;
     ////THUMB_PREFETCH;
   }
   break;   
 case 0xd5:
   // BPL offset
   if(!N_FLAG) {
#ifdef BKPT_SUPPORT
		 UPDATE_OLD_myregs
#endif
     myregs[15].I += ((s8)(opcode & 0xFF)) << 1;       
     armNextPC = myregs[15].I;
     myregs[15].I += 2;
     ////THUMB_PREFETCH;
   }
   break;   
 case 0xd6:
   // BVS offset
   if(V_FLAG) {
#ifdef BKPT_SUPPORT
		 UPDATE_OLD_myregs
#endif
     myregs[15].I += ((s8)(opcode & 0xFF)) << 1;       
     armNextPC = myregs[15].I;
     myregs[15].I += 2;
     ////THUMB_PREFETCH;
   }
   break;   
 case 0xd7:
   // BVC offset
   if(!V_FLAG) {
#ifdef BKPT_SUPPORT
		 UPDATE_OLD_myregs
#endif
     myregs[15].I += ((s8)(opcode & 0xFF)) << 1;       
     armNextPC = myregs[15].I;
     myregs[15].I += 2;
     ////THUMB_PREFETCH;
   }
   break;   
 case 0xd8:
   // BHI offset
   if(C_FLAG && !Z_FLAG) {
#ifdef BKPT_SUPPORT
		 UPDATE_OLD_myregs
#endif
     myregs[15].I += ((s8)(opcode & 0xFF)) << 1;       
     armNextPC = myregs[15].I;
     myregs[15].I += 2;
     ////THUMB_PREFETCH;
   }
   break;   
 case 0xd9:
   // BLS offset
   if(!C_FLAG || Z_FLAG) {
#ifdef BKPT_SUPPORT
		 UPDATE_OLD_myregs
#endif
     myregs[15].I += ((s8)(opcode & 0xFF)) << 1;       
     armNextPC = myregs[15].I;
     myregs[15].I += 2;
     ////THUMB_PREFETCH;
   }
   break;   
 case 0xda:
   // BGE offset
   if(N_FLAG == V_FLAG) {
#ifdef BKPT_SUPPORT
		 UPDATE_OLD_myregs
#endif
     myregs[15].I += ((s8)(opcode & 0xFF)) << 1;       
     armNextPC = myregs[15].I;
     myregs[15].I += 2;
     ////THUMB_PREFETCH;
   }
   break;   
 case 0xdb:
   // BLT offset
   if(N_FLAG != V_FLAG) {
#ifdef BKPT_SUPPORT
		 UPDATE_OLD_myregs
#endif
     myregs[15].I += ((s8)(opcode & 0xFF)) << 1;       
     armNextPC = myregs[15].I;
     myregs[15].I += 2;
     ////THUMB_PREFETCH;
   }
   break;   
 case 0xdc:
   // BGT offset
   if(!Z_FLAG && (N_FLAG == V_FLAG)) {
#ifdef BKPT_SUPPORT
		 UPDATE_OLD_myregs
#endif
     myregs[15].I += ((s8)(opcode & 0xFF)) << 1;       
     armNextPC = myregs[15].I;
     myregs[15].I += 2;
     ////THUMB_PREFETCH;
   }
   break;   
 case 0xdd:
   // BLE offset
   if(Z_FLAG || (N_FLAG != V_FLAG)) {
#ifdef BKPT_SUPPORT
		 UPDATE_OLD_myregs
#endif
     myregs[15].I += ((s8)(opcode & 0xFF)) << 1;       
     armNextPC = myregs[15].I;
     myregs[15].I += 2;
     ////THUMB_PREFETCH;
   }
   break;
 case 0xe0:
 case 0xe1:
 case 0xe2:
 case 0xe3:
 case 0xe4:
 case 0xe5:
 case 0xe6:
 case 0xe7:
   {
     // B offset
     int offset = (opcode & 0x3FF) << 1;
     if(opcode & 0x0400)
       offset |= 0xFFFFF800;
     myregs[15].I += offset;
     armNextPC = myregs[15].I;
     myregs[15].I += 2;
     ////THUMB_PREFETCH;
   }
   break;
 case 0xf0:
 case 0xf1:
 case 0xf2:
 case 0xf3:
   {
     // BLL #offset
     int offset = (opcode & 0x7FF);
     myregs[14].I = myregs[15].I + (offset << 12);
   }
   break;      
 case 0xf4:
 case 0xf5:
 case 0xf6:
 case 0xf7:
   {
     // BLL #offset
     int offset = (opcode & 0x7FF);
     myregs[14].I = myregs[15].I + ((offset << 12) | 0xFF800000);
   }
   break;   
 case 0xf8:
 case 0xf9:
 case 0xfa:
 case 0xfb:
 case 0xfc:
 case 0xfd:
 case 0xfe:
 case 0xff:
   {
     // BLH #offset
     int offset = (opcode & 0x7FF);
     u32 temp = myregs[15].I-2;
     myregs[15].I = (myregs[14].I + (offset<<1))&0xFFFFFFFE;
     armNextPC = myregs[15].I;
     myregs[15].I += 2;
     myregs[14].I = temp|1;
     ////THUMB_PREFETCH;
   }
   break;
#ifdef BKPT_SUPPORT
 case 0xbe:
   // BKPT #comment
   extern void (*dbgSignal)(int,int);
   myregs[15].I -= 2;
   armNextPC -= 2;   
   dbgSignal(5, opcode & 255);
   return;
#endif
 case 0xb1:
 case 0xb2:
 case 0xb3:
 case 0xb6:
 case 0xb7:
 case 0xb8:
 case 0xb9:
 case 0xba:
 case 0xbb:
#ifndef BKPT_SUPPORT
 case 0xbe:
#endif
 case 0xbf:
 case 0xde:
 default:
 unknown_thumb:
#ifdef DEV_VERSION
     Log("Undefined THUMB instruction %04x at %08x\n", opcode, armNextPC-2);
#endif
   break;
}
}

