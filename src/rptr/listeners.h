#ifndef __LISTENERS_H__
#define __LISTENERS_H__

extern void SetConfigParam(char *PathConfigFile, char *Param, char *Value);
extern uint64_t monotonic_ms(void);

void *InputStatusListener(void * arg);
void *SocketListener(void * arg);
void UDP_Command(int);
int PTTEvent(int);

extern bool run_repeater;
extern int localGPIO;
extern int inputactive[8];
extern int inputactiveGPIO[8];
extern int inputprioritylevel[8];
extern bool inputStatusChange;
extern int availableinputs;
extern bool StatusScreenOveride;
extern bool output_overide;
extern bool in_output_overide_mode;
extern int output_overide_source;
extern char dtmfresetcode[31];
extern char dtmfstatusviewcode[31];
extern char dtmfquadviewcode[31];
extern int dtmfselectinput[8];
extern int dtmfoutputs;
extern char dtmfgpiooutoncode[10][31];
extern char dtmfgpiooutoffcode[10][31];
extern int dtmfoutputGPIO[10];                            // DTMF Output GPIO Broadcom numbers as an int
extern char dtmfkeepertxoffcode[31];                      // 5-figure string
extern char dtmfkeepertxoncode[31];                       // 5-figure string
extern char dtmfkeeperrebootcode[31];                     // 5-figure string
extern int localGPIO;                                     // Identifier for piGPIO
extern int pttGPIO;
extern bool hour24operation;
extern bool halfhourpowersave;
extern bool transmitenabled;
extern bool transmitwhennotinuse;
extern bool identduringquiethours;
extern bool repeatduringquiethours;
extern int operatingtimestart;
extern int operatingtimefinish;
extern int inputAfterIdent;
extern bool beaconmode;
extern char dtmftalkbackaudioenablecode[31];              // Stored as a string because it begins with a zero
extern char dtmftalkbackaudiodisablecode[31];             // Stored as a string because it begins with a zero
extern bool talkbackaudio;
extern char StatusForConfigDisplay[100];
extern int fpsdGPIO;
extern bool fpsdenabled;
extern bool rackmainscontrol;
extern bool initial_start;
extern int rxsdbuttonGPIO;
extern int rxmainspwrGPIO;
extern int rxsdsignalGPIO;
extern bool manual_receiver_switch_state;
extern bool manual_receiver_overide;
extern bool rxpowersave;
extern int rxpoweron1;
extern int rxpoweroff1;
extern int rxpoweron2;
extern int rxpoweroff2;


#endif /* __LISTENERS_H__ */