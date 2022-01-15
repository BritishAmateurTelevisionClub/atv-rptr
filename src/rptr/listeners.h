#ifndef __LISTENERS_H__
#define __LISTENERS_H__

void *InputStatusListener(void * arg);
void *SocketListener(void * arg);
void UDP_Command(int);

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
extern int dtmfselectinput[8];

#endif /* __LISTENERS_H__ */