#ifndef __RPTR_H__
#define __RPTR_H__

#define NEON_ALIGNMENT (4*4*2) // 128b

// Config File Parameters
char callsign[31];                                 // Free text, up to 31 characters
char locator[31];                                  // Free text, up to 31 characters
char vidout[31];                                   // hdmi720, hdmi1080, pal, ntsc
char audioout[31];                                 // for ident: hdmi, jack or usb
char dtmfgpiooutlabel[10][31];                     // Label for Accessory Output GPIOs
char dtmfgpiooutoncode[10][31];                    // Stored as a string
char dtmfgpiooutoffcode[10][31];                   // Stored as a string
char dtmfgpioinlabel[10][31];                      // Label for Accessory Input GPIOs
char audioswitch[31];                              // hdmi or i2c
char inputname[8][31];                             // Input names
char outputhdmiresetcode[31];                      // HDMI switch reset code issued on start-up
char outputswitchcontrol[31];                      // gpio or ir (Maybe rs232 later)
char output2ndhdmicode[31];                        // The code for the daisy chain input on the primary hdmi switch
char outputhdmiquadcode[31];                       // The code for Quad View on the primary hdmi switch
char networkctrlurl[63];                           // url for Bitfocus Companion Server or similar
char outputquadnetcommand[63];                     // command to be added to url above
char outputnetcommand[8][63];                      // command to be added to base url
char outputcode[8][31];                            // RS232 or ir code for each HDMI switch selection
char carouselusbaudio[31];                         // Options are off, both, left right (mono from USB dongle)
char dtmfresetcode[31];                            // Stored as a string because it begins with a zero
char dtmfstatusviewcode[31];                       // Stored as a string because it begins with a zero
char dtmfquadviewcode[31];                         // Stored as a string because it begins with a zero
char dtmftalkbackaudioenablecode[31];              // Stored as a string because it begins with a zero
char dtmftalkbackaudiodisablecode[31];             // Stored as a string because it begins with a zero
char dtmfkeepertxoffcode[31];                      // 5-figure string
char dtmfkeepertxoncode[31];                       // 5-figure string
char dtmfkeeperrebootcode[31];                     // 5-figure string
char identmediatype[31];                           // jpg?                      
char identmediafile[63];                           // full path                      
char identcwfile[63];                              // full path                      
char kmediatype[31];                               // jpg?                      
char kmediafile[63];                               // full path                      
char kcwfile[63];                                  // full path                      
char carouselmediatype[100][31];                   // jpg, mp4 or source
char carouselfile[100][63];                        // full path
char announcemediatype[8][31];                     // Announce for each input
char announcemediafile[8][63];                     // Announce for each input

// Derived Config File Parameters
int audiooutcard;                                  // normally 0 hdmi, 1, jack, 2 usb
bool audiokeepalive = false;                       // Is low level audio noise running?
int audiokeepalivelevel = 85;                      // Percentage level for keepalive
bool transmitenabled = false;                      // Is PTT enabled?
bool beaconmode = false;                           // Run in Beacon Mode?
bool rackmainscontrol = false;                     // Allocate GPIOs for rack mains control and use them?           
int rxsdbuttonGPIO;                                // Front panel RX shutdown button GPIO Broadcom
int rxsdsignalGPIO;                                // RX shutdown trigger GPIO Broadcom
int rxmainspwrGPIO;                                // RX shutdown mains power switch GPIO Broadcom
bool rxpowersave = false;                          // Turn receivers off to save power?
int rxpoweron1 = 0;                                // int 0 to 2359
int rxpoweroff1 = 0;                               // int 0 to 2359
int rxpoweron2 = 0;                                // int 0 to 2359
int rxpoweroff2 = 0;                               // int 0 to 2359
bool transmitwhennotinuse = false;                 // Transmit even with no input?
bool hour24operation = false;                      // Operate 24/7?
bool halfhourpowersave = false;                    // Save power during second half hour in active hours?
int operatingtimestart = 0;                        // int 0 to 2359
int operatingtimefinish = 0;                       // int 0 to 2359
bool repeatduringquiethours = false;               // Allow repeater to operate during quiet hours?
bool identduringquiethours = false;                // Transmit idents during quiet hours?
int dtmfoutputs = 0;                               // Quantity of DTMF-controlled Accessory GPIO Outputs
int dtmfoutputGPIO[10];                            // DTMF Output GPIO Broadcom numbers as an int
int dtmfinputs = 0;                                // Quantity of Accessory GPIO Inputs (not DTMF-controlled)
int dtmfinputGPIO[10];                             // Accessory input GPIO Broadcom numbers as an int
int audioi2caddress = 7;                           // 0 to 7
bool talkbackaudio = true;                         //
int talkbackaudioi2cbit = 7;                       // 0 to 7
int availableinputs = 7;                           // How many connected inputs? 1 - 7
int outputGPIO[8];                                 // HDMI Switch GPIO Broadcom numbers as an int
int inputactiveGPIO[8];                            // Input active GPIO Broadcom numbers as an int
int inputprioritylevel[8];                         // 0 highest, 9 disabled
int pttGPIO;                                       // PTT GPIO Boadcom number
int fpsdGPIO = 7;                                  // Shutdown button GPIO Boadcom number
bool fpsdenabled = false;                          //
int carouselusbaudiogain;                          // 0 to 100
bool dtmf_enabled = false;                         // 
int dtmfactiontimeout = 600;                       // seconds.  Default 600.  0 = no timeout
int dtmfselectinput[8];                            // Like *10# displays input 0
int identinterval = 900;                           // Interval between start of idents.
int identmediaduration = 5;                        // seconds
bool identcwaudio = false;                         //
int identcwlevel = 100;                            // percentage
int kmediaduration = 3;                            // seconds
bool kcwaudio = false;                             //
int kcwlevel = 100;                                // percentage
bool announcebleep = false;                        //
int announcebleeplevel = 100;                      // percentage
int carouselscenes = 3;                            // Max 99
int carouselmediaduration[100];                    // seconds
int announcemediaduration[8];                      // seconds for each input
bool activeinputhold = true;                       // lower priority inputs do not get replaced by higher priority (except pri 1)
bool showquadformultipleinputs = false;            // Switch to quad view for multiple inputs
bool cascadedswitches = false;                     // Using 2 switches?
bool showoutputongpio = false;                     // Toggle gpio lines in addition to IR
int outputaudioi2cbit[8];                          // range 0 - 7, value 0 to 7
int daisychainirselectgpio = 27;                   // Second IR select Broadcom number

// Current Status parameters
int inputactive[8] = {1, 0, 0, 0, 0, 0, 0, 0};     // 0 if inactive, 1 if active
int inputActiveInitialState[8];                    // Set when a decision is made
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
bool run_repeater = true;                          // Used to neatly exit threads
bool run_carousel;                                 // Used to neatly exit threads
bool outputwasmultiinputquad = false;              // Used to prevent Announce when quad has been displayed
bool manual_receiver_switch_state = true;          // The last manual-commanded switch state. true = on
bool manual_receiver_overide = false;              // Set to indicate that the button has been used
bool initial_start = true;                         // Set to indicate that status screen should be displayed if rack off

// Display parameters
int screen_width;                                  // These are defined in the config file
int screen_height;                                 // but only used for text sizing
char StatusForConfigDisplay[100];                  // Status to be shown on config display

// Audio Parameters

int currenti2caudiostatus[8];                      // used to enable intelligent switching of ident

int localGPIO;                                     // Identifier for piGPIO

#endif /* __RPTR_H__ */
