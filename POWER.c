#include "fxlib.h"

// instant syscalls (fx-9860)
#define SCA 0xD201D002
#define SCB 0x422B0009
#define SCE 0x80010070
typedef void(*sc_vpuc)(unsigned char*);
typedef int(*sc_iv)(void);
typedef int(*sc_i2pui)( unsigned int*, unsigned int* );
typedef int(*sc_i4pui)( unsigned int*, unsigned int*, unsigned int*, unsigned int* );
typedef int(*sc_i2puc2pus)( unsigned char*, unsigned char*, unsigned short*, unsigned short* );
typedef int(*sc_iipci)( int, unsigned char*, int );
const unsigned int sc003B[] = { SCA, SCB, SCE, 0x003B };
#define RTC_GetTicks (*(sc_iv)sc003B)
const unsigned int sc0D65[] = { SCA, SCB, SCE, 0x0D65 };
#define InputDateMonth (*(sc_i2pui)sc0D65)
const unsigned int sc0D67[] = { SCA, SCB, SCE, 0x0D67 };
#define InputDateYear (*(sc_i2pui)sc0D67)
const unsigned int sc0D64[] = { SCA, SCB, SCE, 0x0D64 };
#define InputDate (*(sc_i4pui)sc0D64)
const unsigned int sc0015[] = { SCA, SCB, SCE, 0x0015 };
#define GlibGetOSVersionInfo (*(sc_i2puc2pus)sc0015)
const unsigned int sc06C4[] = { SCA, SCB, SCE, 0x06C4 };
#define KBD_PRGM_GetKey (*(sc_vpuc)sc06C4)
const unsigned int sc0160[] = { SCA, SCB, SCE, 0x0160 };
#define LongToAsc (*(sc_iipci)sc0160)
// instant syscalls

// for fx9860 only!
#define PORT_E_DR 0xA4000128

//
#define OS202GII2 0x02020000
//
char IsSlim(){
   if ( ( OSVersionAsInt() & 0xFFFF0000 ) == OS202GII2 ) return 0;
   else return !( *(char*)PORT_E_DR & 0x08 );
}
//
char IsSD(){
   return ( *(char*)PORT_E_DR & 0x01 );
}

//
int IsEmulator(){
   return !( *(int*)0x8000FFD0 || *(int*)0x8000FFD4 );
}

#define OS102 0x01020000
#define OS103 0x01030000
#define OS104 0x01040000
#define OS105 0x01050000
#define OS1051 0x01051000
#define OS110 0x01100000
#define OS111 0x01110000
#define OS200 0x02000300
#define OS200slim 0x02000100
#define OS200GII 0x02000200

//
int OSVersionAsInt( void ){
unsigned char mainversion;
unsigned char minorversion;
unsigned short release;
unsigned short build;
  GlibGetOSVersionInfo( &mainversion, &minorversion, &release, &build );
  return ( ( mainversion << 24 ) & 0xFF000000 ) | ( ( minorversion << 16 ) & 0x00FF0000 ) | ( release & 0x0000FFFF );
}

//
int GetMainBatteryVoltagePointer(){
unsigned int ea;
unsigned int j;
    ea = *(unsigned int*)0x8001007C;
    ea = ea + 0x049C*4;
    ea = *(unsigned int*)( ea );
    while ( *(unsigned short*)( ea ) != 0xE464 ){
      ea = ea + 2;
    };
    ea = ea + 2;
    j = *(unsigned char*)( ea + 1 );
    j = 4*j;
    j = ( ea + j + 4 ) & 0xFFFFFFFC;
    return *(unsigned int*)( j );
}

int GetBatteryStatus( int battery, int*firstlevel, int*secondlevel ){
int (*iGBS)( int ) = 0;  // declare an int function pointer
int*battconfig = (int*)0x80000334;
   *firstlevel = 0x17F;
   *secondlevel = 0x16D;

   iGBS = (int(*)(int))GetMainBatteryVoltagePointer();
 
     switch ( OSVersionAsInt() ){
       case OS200 :
         if ( *battconfig==0x43444844 ){
           *firstlevel = 0x1A5;
           *secondlevel = 0x191;
         };
         break;
      case OS200GII :
	//case OS202GII2 :
         break;
      case OS200slim :
         *firstlevel = 0x2AA;
         *secondlevel = 0x288;
         break;
     };
   if (IsSlim()){
      *firstlevel = 0x2AA;
      *secondlevel = 0x288;
   }   
   if (iGBS!=0) return (*iGBS)( battery );
   else return 0;
}

// the warning level lies around 2/3 full scale
int MainBatteryPercentage( void ){
int firstlevel, secondlevel;
int i;
   if ( IsEmulator() ){
      i = 100;
      firstlevel = 70;
      secondlevel = 65;
   } else i = GetBatteryStatus( 1, &firstlevel, &secondlevel );
      
   if (firstlevel > 0 ) return ( 200*i )/(firstlevel*3);
   else return 0;
}

int AddIn_main(int isAppli, unsigned short OptionNum)
{
// battery = 1 = main (do not use other values)
// firstlevel: warning level
// secondlevel: shutdown level

int fl = 33, sl = 66, m = 1, batt = 0;

char c[4] = {0};

locate(1,1);

batt = GetBatteryStatus(m, &fl, &sl);

Bdisp_DrawLineVRAM(0,0,batt,0);
Bdisp_PutDisp_DD();
GetKey(&fl);
}
#pragma section _BR_Size
unsigned long BR_Size;
#pragma section
#pragma section _TOP
int InitializeSystem(int isAppli, unsigned short OptionNum)
{return INIT_ADDIN_APPLICATION(isAppli, OptionNum);}
#pragma section

