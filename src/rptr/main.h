#ifndef __RPTR_H__
#define __RPTR_H__

#define NEON_ALIGNMENT (4*4*2) // 128b

// Config File Parameters
char callsign[31];                                 // Free text, up to 31 characters
char locator[31];                                  // Free text, up to 31 characters
char vidout[31];                                   // hdmi720, hdmi1080, pal, ntsc
char onboot[31];                                   // repeat beacon txoff nil status
char inputname[8][31];                             // Input names
char outputhdmiresetcode[31];                      // HDMI switch reset code issued on start-up
char outputswitchcontrol[31];                      // gpio or ir (Maybe rs232 later)
char output2ndhdmicode[31];                        // The code for the daisy chain input on the primary hdmi switch
char outputcode[8][31];                            // RS232 or ir code for each HDMI switch selection
char carouselusbaudio[31];                         // Options are off, both, left right (mono from USB dongle)
char dtmfresetcode[31];                            // Stored as a string because it begins with a zero
char dtmfstatusviewcode[31];                       // Stored as a string because it begins with a zero
char identmediatype[31];                           // jpg?                      
char identmediafile[63];                           // full path                      
char kmediatype[31];                               // jpg?                      
char kmediafile[63];                               // full path                      
char carouselmediatype[100][31];                   // jpg, mp4 or source
char carouselfile[100][63];                        // full path
char announcemediatype[8][31];                     // Announce for each input
char announcemediafile[8][63];                     // Announce for each input

// Derived Config File Parameters
int availableinputs = 7;                           // How many connected inputs? 1 - 7
int outputGPIO[8];                                 // HDMI Switch GPIO Broadcom numbers as an int
int inputactiveGPIO[8];                            // Input active GPIO Broadcom numbers as an int
int inputprioritylevel[8];                         // 0 highest, 9 disabled
int pttGPIO;                                       // PTT GPIO Boadcom number
int carouselusbaudiogain;                          // 0 to 100
bool dtmf_enabled = false;                         // 
int dtmfactiontimeout = 600;                       // seconds.  Default 600.  0 = no timeout
int dtmfselectinput[8];                            // Like *10# displays input 0
int identinterval = 900;                           // Interval between start of idents.
int identmediaduration = 5;                        // seconds
bool identcwaudio = false;                         //
int kmediaduration = 3;                            // seconds
bool kcwaudio = false;                             //
int carouselscenes = 3;                            // Max 99
int carouselmediaduration[100];                    // seconds
int announcemediaduration[8];                      // seconds for each input
bool activeinputhold = true;                       // lower priority inputs do not get replaced by higher priority (except pri 1)
bool showoutputongpio = false;                     // Toggle gpio lines in addition to IR

// Current Status parameters
int inputactive[8] = {1, 0, 0, 0, 0, 0, 0, 0};     // 0 if inactive, 1 if active
int inputselected = 0;                             // 0 - 7 for selected input (not valid for status screen)
bool StatusScreenOveride = false;                  // True displays status screen
bool inputStatusChange = true;                     // Signals change of input status
int outputcontrol = 0;                             // master parameter to change output switch
bool ident_active = false;                         // Used to make sure that ident is not over-riden
bool carousel_active = false;                      // Used to restart carousel after ident
int inputAfterIdent;                               // Used to reselect correct output after ident
bool firstCarousel;                                // Used to indicate that inputs should be re- checked after running the 1st K 
bool output_overide = false;                       // Set by console menu or dtmf to show a specific input source
int output_overide_source = 0;                     // Set by console menu or dtmf to show a specific input source
bool in_output_overide_mode = false;

// Display parameters
int screen_width;               // These are defined in the config file
int screen_height;



#endif /* __RPTR__ */