// main.c
/*
This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

Written by Dave, G8GKQ
*/
//

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>
#include <getopt.h>
#include <linux/input.h>
#include <fcntl.h>
#include <dirent.h>
#include <ctype.h>
#include <math.h>
#include <sys/stat.h> 
#include <sys/types.h> 
#include <time.h>
#include <pigpiod_if2.h>

#include "main.h"
#include "font/font.h"
#include "Graphics.h"
#include "look-ups.h"
#include "listeners.h"
#include "timing.h"


#define PATH_CONFIG "/home/pi/atv-rptr/config/repeater_config.txt"

int debug_level = 0;   // set to 2 to see all config file reads

int localGPIO;                                     // Identifier for piGPIO

bool run_repeater = true;                          // Used to neatly exit threads



// Threads
pthread_t thinputactivemonitor;
pthread_t thidenttimer;
pthread_t thkcarousel;
pthread_t thsocketmonitor;

// Function Prototypes

void GetConfigParam(char *PathConfigFile, char *Param, char *Value);
void SetConfigParam(char *PathConfigFile, char *Param, char *Value);
void strcpyn(char *outstring, char *instring, int n);
//void runInputStatusMonitor();
void update_status_screen();
void Select_HDMI_Switch(int);

void read_config_file();

void setUpGPIO();


/***************************************************************************//**
 * @brief Looks up the value of a Param in PathConfigFile and sets value
 *        Used to look up the configuration from portsdown_config.txt
 *
 * @param PatchConfigFile (str) the name of the configuration text file
 * @param Param the string labeling the parameter
 * @param Value the looked-up value of the parameter
 *
 * @return void
*******************************************************************************/

void GetConfigParam(char *PathConfigFile, char *Param, char *Value)
{
  char * line = NULL;
  size_t len = 0;
  int read;
  char ParamWithEquals[255];
  strcpy(ParamWithEquals, Param);
  strcat(ParamWithEquals, "=");
  int file_test = 0;

  file_test = file_exist(PathConfigFile);  // Log error if file does not exist
  if (file_test == 1)
  {
    system("echo ConfigFileNotFoundinGetConfigParam >> /home/pi/atv-rptr/logs/error_log.txt");
    strcpy(Value, " ");
    return;
  }

  FILE *fp=fopen(PathConfigFile, "r");
  if(fp != 0)
  {
    while ((read = getline(&line, &len, fp)) != -1)
    {
      if(strncmp (line, ParamWithEquals, strlen(Param) + 1) == 0)
      {
        strcpy(Value, line+strlen(Param)+1);
        char *p;
        if((p=strchr(Value,'\n')) !=0 ) *p=0; //Remove \n
        break;
      }
    }
    if (debug_level == 2)
    {
      printf("Get Config reads %s for %s and returns %s\n", PathConfigFile, Param, Value);
    }
  }
  else
  {
    printf("Config file not found \n");
  }
  fclose(fp);

  if (strlen(Value) == 0)  // Log error if parameter undefined
  {  
    system("echo ParameterUndefinedInConfigFile >> /home/pi/atv-rptr/logs/error_log.txt");
  }
}

/***************************************************************************//**
 * @brief sets the value of Param in PathConfigFile from a program variable
 *        Used to store the configuration in portsdown_config.txt
 *
 * @param PatchConfigFile (str) the name of the configuration text file
 * @param Param the string labeling the parameter
 * @param Value the looked-up value of the parameter
 *
 * @return void
*******************************************************************************/

void SetConfigParam(char *PathConfigFile, char *Param, char *Value)
{
  char * line = NULL;
  size_t len = 0;
  int read;
  char Command[511];
  char BackupConfigName[240];
  char ParamWithEquals[255];

  if (debug_level == 2)
  {
    printf("Set Config called %s %s %s\n", PathConfigFile , ParamWithEquals, Value);
  }

  if (strlen(Value) == 0)  // Don't write empty values
  {
    system("echo AttemptToSetEmptyConfigValue >> /home/pi/atv-rptr/logs/error_log.txt");
    return;
  }

  strcpy(BackupConfigName, PathConfigFile);
  strcat(BackupConfigName, ".bak");
  FILE *fp=fopen(PathConfigFile, "r");
  FILE *fw=fopen(BackupConfigName, "w+");
  strcpy(ParamWithEquals, Param);
  strcat(ParamWithEquals, "=");

  if(fp!=0)
  {
    while ((read = getline(&line, &len, fp)) != -1)
    {
      if(strncmp (line, ParamWithEquals, strlen(Param) + 1) == 0)
      {
        fprintf(fw, "%s=%s\n", Param, Value);
      }
      else
      {
        fprintf(fw, line);
      }
    }
    fclose(fp);
    fclose(fw);
    snprintf(Command, 511, "cp %s %s", BackupConfigName, PathConfigFile);
    system(Command);
  }
  else
  {
    printf("Config file not found \n");
    fclose(fp);
    fclose(fw);
  }
}

/***************************************************************************//**
 * @brief safely copies n characters of instring to outstring without overflow
 *
 * @param *outstring
 * @param *instring
 * @param n int number of characters to copy.  Max value is the outstring array size -1
 *
 * @return void
*******************************************************************************/
void strcpyn(char *outstring, char *instring, int n)
{
  //printf("\ninstring= -%s-, instring length = %d, desired length = %d\n", instring, strlen(instring), strnlen(instring, n));
  
  n = strnlen(instring, n);
  int i;
  for (i = 0; i < n; i = i + 1)
  {
    //printf("i = %d input character = %c\n", i, instring[i]);
    outstring[i] = instring[i];
  }
  outstring[n] = '\0'; // Terminate the outstring
}



/***************************************************************************//**
 * @brief Reads all the parameters in the saved config file to the global variables
 *        and sets derived parameters
 *
 * @param nil
 *
 * @return 0
*******************************************************************************/

void read_config_file()
{
  int i;
  char Param[127];
  char Value[127];

  // Basic Configuration
  GetConfigParam(PATH_CONFIG,"callsign", callsign);
  GetConfigParam(PATH_CONFIG,"locator", locator);
  GetConfigParam(PATH_CONFIG,"vidout", vidout);

  if (strcmp(vidout, "hdmi720") == 0)
  {
    screen_width = 1280;
    screen_height = 720;
  }
  if (strcmp(vidout, "hdmi1080") == 0)
  {
    screen_width = 1920;
    screen_height = 1080;
  }
  if (strcmp(vidout, "pal") == 0)
  {
    screen_width = 720;
    screen_height = 576;
  }
  if (strcmp(vidout, "ntsc") == 0)
  {
    screen_width = 720;
    screen_height = 480;
  }

  // Boot and operating behaviour
  GetConfigParam(PATH_CONFIG, "onboot", onboot);
  GetConfigParam(PATH_CONFIG, "operatingtimestart", Value);
  GetConfigParam(PATH_CONFIG, "operatingtimefinish", Value);
  // Need to add a time conversion in here

  // PTT command GPIO Pin
  strcpy(Value, "");
  GetConfigParam(PATH_CONFIG, "pttgpiopin", Value);
  pttGPIO = PinToBroadcom(atoi(Value));

  // Carousel Audio Config
  GetConfigParam(PATH_CONFIG, "carouselusbaudiomode", carouselusbaudio);
  strcpy(Value, "");
  GetConfigParam(PATH_CONFIG, "carouselusbaudiogain", Value);
  carouselusbaudiogain = atoi(Value);

  // DTMF Config
  GetConfigParam(PATH_CONFIG, "dtmfcontrol", Value);
  if (strcmp(Value, "on") == 0)
  {
    dtmf_enabled = true;
  }
  strcpy(Value, "");
  GetConfigParam(PATH_CONFIG, "dtmfactiontimeout", Value);
  dtmfactiontimeout = atoi(Value);
  GetConfigParam(PATH_CONFIG, "dtmfreset", dtmfresetcode);
  GetConfigParam(PATH_CONFIG, "dtmfstatusview", dtmfstatusviewcode);
  strcpy(Value, "");
  GetConfigParam(PATH_CONFIG, "dtmfselectinput0", Value);
  dtmfselectinputbase = atoi(Value);

  // Repeater Ident Config
  strcpy(Value, "");
  GetConfigParam(PATH_CONFIG, "identinterval", Value);
  identinterval = atoi(Value);
  GetConfigParam(PATH_CONFIG, "identmediatype", identmediatype);
  strcpy(Value, "");
  GetConfigParam(PATH_CONFIG, "identmediaduration", Value);
  identmediaduration = atoi(Value);
  GetConfigParam(PATH_CONFIG, "identmediafile", identmediafile);
  GetConfigParam(PATH_CONFIG, "identcwaudio", Value);
  if (strcmp(Value, "on") == 0)
  {
    identcwaudio = true;
  }

  // K slide config
  GetConfigParam(PATH_CONFIG, "kmediatype", kmediatype);
  strcpy(Value, "");
  GetConfigParam(PATH_CONFIG, "kmediaduration", Value);
  kmediaduration = atoi(Value);
  GetConfigParam(PATH_CONFIG, "kmediafile", kmediafile);
  GetConfigParam(PATH_CONFIG, "kcwaudio", Value);
  if (strcmp(Value, "on") == 0)
  {
    kcwaudio = true;
  }

  // Carousel config

  // Number of carousel scenes
  strcpy(Value, "");
  GetConfigParam(PATH_CONFIG, "carouselscenes", Value);
  carouselscenes = atoi(Value);
  if ((carouselscenes >= 1) && (carouselscenes <= 9))
  {
    for (i = 1; i <= carouselscenes; i++)
    {

      // Carousel scene media type
      snprintf(Param, 127, "carousel0%dmediatype", i);
      GetConfigParam(PATH_CONFIG, Param, carouselmediatype[i]);

      // Carousel scene file name (or input number)
      snprintf(Param, 127, "carousel0%dfile", i);
      GetConfigParam(PATH_CONFIG, Param, carouselfile[i]);

      // Carousel scene duration
      strcpy(Value, "");
      snprintf(Param, 127, "carousel0%dmediaduration", i);
      GetConfigParam(PATH_CONFIG, Param, Value);
      carouselmediaduration[i] = atoi(Value);
    }
  }
  if ((carouselscenes >= 10) && (carouselscenes <= 99))
  {
    for (i = 10; i <= carouselscenes; i++)
    {

      // Carousel scene media type
      snprintf(Param, 127, "carousel%dmediatype", i);
      GetConfigParam(PATH_CONFIG, Param, carouselmediatype[i]);

      // Carousel scene file name (or input number)
      snprintf(Param, 127, "carousel%dfile", i);
      GetConfigParam(PATH_CONFIG, Param, carouselfile[i]);

      // Carousel scene duration
      strcpy(Value, "");
      snprintf(Param, 127, "carousel%dmediaduration", i);
      GetConfigParam(PATH_CONFIG, Param, Value);
      carouselmediaduration[i] = atoi(Value);
    }
  }

  // HDMI Switch Configuration

  // IR or GPIO Switched
  GetConfigParam(PATH_CONFIG, "outputswitchcontrol", outputswitchcontrol);
  if (strcmp(outputswitchcontrol, "ir") == 0)
  {
    // Lirc name for remote control key file
    GetConfigParam(PATH_CONFIG, "outputswitchirname", outputswitchirname);

    // Reset Code for HDMI Switch
    GetConfigParam(PATH_CONFIG, "outputresetcode", outputresetcode);
  }

  // Show GPIO in addition to IR?
  GetConfigParam(PATH_CONFIG, "showoutputongpio", Value);
  if (strcmp(Value, "yes") == 0)
  {
    showoutputongpio = true;
  }
  else
  {
    showoutputongpio = false;
  }

  // Input and Output Configuration

  // Number of inputs
  strcpy(Value, "");
  GetConfigParam(PATH_CONFIG, "availableinputs", Value);
  availableinputs = atoi(Value);
  if ((availableinputs < 1) || (availableinputs > 7))
  {
    availableinputs = 7;
  }

  // Behavior on input conflict
  GetConfigParam(PATH_CONFIG, "activeinputhold", Value);
  if (strcmp(Value, "off") == 0)
  {
    activeinputhold = false;
  }

  for(i = 0 ; i <= availableinputs ; i++)
  {
    // Input Name
    strcpy(Value, "");
    snprintf(Param, 127, "input%dname", i);
    GetConfigParam(PATH_CONFIG, Param, Value);
    strcpy(inputname[i], Value);

    // HDMI Switch IR Select Code
    strcpy(Value, "");
    snprintf(Param, 127, "output%dcode", i);
    GetConfigParam(PATH_CONFIG, Param, Value);
    strcpy(outputcode[i], Value);
    
    // HDMI Switch GPIO Pin
    strcpy(Value, "");
    snprintf(Param, 127, "output%dhdmiswitchpin", i);
    GetConfigParam(PATH_CONFIG, Param, Value);
    outputGPIO[i] = PinToBroadcom(atoi(Value));

    if (i >= 1)  // All inputs except the controller input
    {
      // Input priority level
      strcpy(Value, "");
      snprintf(Param, 127, "input%dprioritylevel", i);
      GetConfigParam(PATH_CONFIG, Param, Value);
      inputprioritylevel[i] = atoi(Value);

      // Input active GPIO pin
      strcpy(Value, "");
      snprintf(Param, 127, "input%dactivegpiopin", i);
      GetConfigParam(PATH_CONFIG, Param, Value);
      inputactiveGPIO[i] = PinToBroadcom(atoi(Value));

      // Announce media info
      snprintf(Param, 127, "input%dannouncemediatype", i);
      GetConfigParam(PATH_CONFIG, Param, announcemediatype[i]);
      snprintf(Param, 127, "input%dannouncemediafile", i);
      GetConfigParam(PATH_CONFIG, Param, announcemediafile[i]);
      strcpy(Value, "");
      snprintf(Param, 127, "input%dannouncemediaduration", i);
      GetConfigParam(PATH_CONFIG, Param, Value);
      announcemediaduration[i] = atoi(Value);
    }
  }
}


void setUpGPIO()
{
  int i;

  // Set all the "Input Active" GPIOs to read-only
  for (i = 1; i <= 7 ; i++)
  {
    set_mode(localGPIO, inputactiveGPIO[i], 0);
  }

  // Set the PTT GPIO as an output
  set_mode(localGPIO, pttGPIO, 1);

  // If GPIO-switched HDMI, set the outputs
  for (i = 0; i <= 7 ; i++)
  {
    set_mode(localGPIO, outputGPIO[i], 1);
  }

  // Example pigpio code:

  // set_mode(localGPIO, pttGPIO, 1); (0 read, 1 write)
  // level = gpio_read(localGPIO, GPIO);

  // gpio_write(localGPIO, GPIO, 0);
  // gpio_write(localGPIO, GPIO, 1);
}

void update_status_screen()
{
  int line_height;
  int i;
  //int x = 0;
  //int y = 0;
  char display_text[127];
  //char *temp_text[127];

  setBackColour(0,0,0);
  clearScreen();
  setForeColour(255, 255, 255);
  const font_t *font_ptr = &font_dejavu_sans_30;

  switch(screen_height)
  {
    case 1080:
      //*font_ptr = &font_dejavu_sans_32;
      line_height = 60;
      break;
    case 576:
      //*font_ptr = &font_dejavu_sans_20;
      line_height = 30;
      break;
    case 480:
      //*font_ptr = &font_dejavu_sans_18;
      line_height = 25;
      break;
    case 720:
    default:
      //*font_ptr = &font_dejavu_sans_30;
      line_height = 40;
    break;
  }

  // Header Lines

  snprintf(display_text, 127, "BATC ATV Repeater Status Screen");
  TextMid2(screen_width / 2, screen_height - (2 * line_height), display_text, font_ptr);

  snprintf(display_text, 127, "Callsign %s, Locator %s", callsign, locator);
  TextMid2(screen_width / 2, screen_height - (3 * line_height), display_text, font_ptr);

  // Column Headings

  Text2(screen_width / 16, screen_height - (11 * (line_height / 2)), "Input", font_ptr);
  Text2(screen_width * 10 / 32, screen_height - (11 * (line_height / 2)), "Status", font_ptr);
  Text2(screen_width * 13 / 32, screen_height - (11 * (line_height / 2)), "Pri", font_ptr);
  Text2(screen_width * 15 / 32, screen_height - (11 * (line_height / 2)), "Output", font_ptr);

 
  for(i = 0 ; i <= 7 ; i++)
  {
    snprintf(display_text, 2, "%d", i);
    Text2(screen_width / 32, screen_height - ((7 + i) * line_height), display_text, font_ptr);

    // Input name
    strcpy(display_text, inputname[i]);
    Text2(screen_width / 16, screen_height - ((7 + i) * line_height), display_text, font_ptr);

    // Active/inactive?
    strcpy(display_text, "");
    if (inputactive[i] == 1)
    {
      strcpy(display_text, "Active");
      Text2(screen_width * 10 / 32, screen_height - ((7 + i) * line_height), display_text, font_ptr);
    }

    // Priority
    if (i > 0)
    {
      snprintf(display_text, 2, "%d", inputprioritylevel[i]);
      Text2(screen_width * 13 / 32, screen_height - ((7 + i) * line_height), display_text, font_ptr);
    }

    // Output Selection Status
    strcpy(display_text, "");
    if ((i == 0) && ((StatusScreenOveride == true) || (inputselected == 0)))
    {
      strcpy(display_text, "Selected");
      Text2(screen_width * 15 / 32, screen_height - ((7 + i) * line_height), display_text, font_ptr);
    }
    if ((i >= 1) && (inputselected == i))
    {
      if (StatusScreenOveride == true)
      {
        strcpy(display_text, "(Selected)");
      }
      else
      {
        strcpy(display_text, "Selected");
      }
      Text2(screen_width * 15 / 32, screen_height - ((7 + i) * line_height), display_text, font_ptr);
    }
  }

  snprintf(display_text, 31, "SW Version %d", GetSWVers());
  Text2(screen_width * 22 / 32, screen_height - (5 * line_height), display_text, font_ptr);

  snprintf(display_text, 31, "CPU Temp %.1f C", GetCPUTemp());
  Text2(screen_width * 22 / 32, screen_height - (6 * line_height), display_text, font_ptr);

  //time_t t; 
  //struct tm tm;

  //t = time(NULL);
  //tm = *gmtime(&t);
  //printf("now: %d-%02d-%02d %02d:%02d:%02d\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);


}

void *Show_Ident(void * arg)
{
  uint64_t last_ident;
  uint64_t ident_required;
  uint64_t ident_finish;
  char SystemCommand[127];

  last_ident = monotonic_ms();
  ident_required = last_ident  + identinterval * 1000;
  ident_finish = last_ident + (identinterval + identmediaduration) * 1000;
  printf("Starting Ident Thread.  Ident Interval = %d\n", identinterval);

  while (run_repeater == true)
  {
    //printf("Ident decision. monotonic = %llu; ident required = %llu ident_finish %llu\n", monotonic_ms(), ident_required, ident_finish);

    if (monotonic_ms() > ident_required)
    {
      printf("Commencing Ident\n");
      ident_active = true;
      last_ident = ident_required;
      ident_required = last_ident + identinterval * 1000;

      // kill vlc?

      // Display the ident, and switch to it
      snprintf(SystemCommand, 127, "sudo fbi -T 1 -noverbose -a %s >/dev/null 2>/dev/null", identmediafile);
      system("sudo fbi -T 1 -noverbose -a /home/pi/tmp/ident.jpg >/dev/null 2>/dev/null");
      Select_HDMI_Switch(0);

      // kill fbi to prevent multiple instances 
      strcpy(SystemCommand, "(sleep 1; sudo killall -9 fbi >/dev/null 2>/dev/null) &");
      system(SystemCommand);
    }

    if (monotonic_ms() > ident_finish)
    {
      ident_finish = ident_required + identmediaduration * 1000;
      ident_active = false;

      // Switch to status display if required
      if (StatusScreenOveride == true)
      {
        printf("Finishing Ident, switching to status screen\n");
        update_status_screen();
      }
      else
      {
        printf("Finishing Ident, switching to current input %d\n", inputAfterIdent);

        // Carousel will refresh on next image

        // switch to the current screen
        Select_HDMI_Switch(inputAfterIdent);        
      }
    }
    usleep(100000);
  }
  return NULL;
}

void *Show_K_Carousel(void * arg)
{
  uint64_t media_start;
  bool run_carousel = true;
  char SystemCommand[127];
  int i;
  int next_i;
  int carouselSource;

  printf("Entering the KCarousel thread\n");

  // Display the K initially
  if ((ident_active == false) && (StatusScreenOveride == false))
  {
    printf("Displaying the K\n");
    snprintf(SystemCommand, 127, "sudo fbi -T 1 -noverbose -a %s >/dev/null 2>/dev/null", kmediafile);
    system(SystemCommand);
  }

  // Now wait kmediaduration seconds
  media_start = monotonic_ms();
  //printf("Monotonic: %llu, K Media Start %llu, end at %llu\n", monotonic_ms(), media_start, media_start + kmediaduration * 1000);
  while (monotonic_ms() < media_start + kmediaduration * 1000)
  {
    usleep(10000);
  }

  printf("Finished displaying the K\n");


  if ((inputactive[1] == 1) ||
      (inputactive[2] == 1) ||
      (inputactive[3] == 1) ||
      (inputactive[4] == 1) ||
      (inputactive[5] == 1) ||
      (inputactive[6] == 1) ||
      (inputactive[7] == 1))
  {
    run_carousel = false;
    if (firstCarousel == true)    // Prompt check of inputs
    {
      inputStatusChange = true;
    }
  }
  firstCarousel = false;
     
  while (run_carousel == true)
  {
    for (i = 1; i <= carouselscenes; i++)
    {
      media_start = monotonic_ms();

      // Display the Carousel scene
      if ((StatusScreenOveride != true) && (ident_active != true) && (run_carousel == true))
      {
        if (strcmp(carouselmediatype[i], "jpg") == 0)          // Scene is an image
        {  
          snprintf(SystemCommand, 127, "sudo fbi -T 1 -noverbose -a %s >/dev/null 2>/dev/null", carouselfile[i]);
          system(SystemCommand);
        }

        if (strcmp(carouselmediatype[i], "source") == 0)       // Scene is a source
        {
          carouselSource = atoi(carouselfile[i]);
          if ((carouselSource < 1) || (carouselSource > 7))
          {
            carouselSource = 0;
          }
          Select_HDMI_Switch(carouselSource);
          inputAfterIdent = carouselSource;
        }
        printf("Carousel Scene %d displayed\n", i);
      }

      // Now wait kmediaduration seconds

      while ((run_carousel == true) && (monotonic_ms() < media_start + (carouselmediaduration[i] * 1000)))
      {
        //printf("monotonic = %llu, media_start + carouselmediaduration = %llu\n", monotonic_ms() , media_start + (carouselmediaduration[i] * 1000));
        {
          if ((inputactive[1] == 1) ||
              (inputactive[2] == 1) ||
              (inputactive[3] == 1) ||
              (inputactive[4] == 1) ||
              (inputactive[5] == 1) ||
              (inputactive[6] == 1) ||
              (inputactive[7] == 1))
          {
            run_carousel = false;
          }
          usleep(10000);
        }
      }

      strcpy(SystemCommand, "(sleep 1; sudo killall -9 fbi >/dev/null 2>/dev/null) &");
      system(SystemCommand);

      if (strcmp(carouselmediatype[i], "jpg") == 0)          // Scene was an image so kill fbi
      {  
        snprintf(SystemCommand, 127, "sudo killall -9 fbi >/dev/null 2>/dev/null");
        system(SystemCommand);
      }

      if (strcmp(carouselmediatype[i], "source") == 0)       // Scene was a source so reset switch
      {
        if (i == carouselscenes)  // last scene in carousel
        {
          next_i = 1;
        }
        else
        {
          next_i = i + 1;
        }
        if (strcmp(carouselmediatype[next_i], "source") != 0)  // next scene is not a source
        {
          Select_HDMI_Switch(0);
          inputAfterIdent = 0;
        }
      }
    }
  }
  return NULL;
}


void Select_HDMI_Switch(int selection)        // selection is between 0 and availableinputs
{
  int i;
  int thisGPIOlevel;
  char SystemCommand[127];

  if ((selection < 0)  || (selection > availableinputs))
  {
    printf("ERROR Select_HDMI_Switch called with switch = %d\n", selection);
    selection = 0;
  }
  printf("HDMI Switch input %d requested\n", selection);

  if ((strcmp(outputswitchcontrol, "gpio") == 0)  || (showoutputongpio == true))     // GPIO line controlled HDMI switch
  {
    for (i = 0; i <= availableinputs; i++)
    {
      if (selection == i)
      {
        thisGPIOlevel = 1;
      }
      else
      {
        thisGPIOlevel = 0;
      }

      gpio_write(localGPIO, outputGPIO[i], thisGPIOlevel);
    }
  }

  if (strcmp(outputswitchcontrol, "rs232") == 0)             // RS232 controlled HDMI switch
  {
    // RS2323 code here
  }

  if (strcmp(outputswitchcontrol, "ir") == 0)             // ir controlled HDMI switch
  {
    // IR code here
    snprintf(SystemCommand, 126, "irsend SEND_ONCE %s %s", outputswitchirname, outputcode[selection]);
        //system("irsend SEND_ONCE Portta_HDMI KEY_1");
    system(SystemCommand);
  }
}


void Switchto(int new_output)
{
  char SystemCommand[127];
  // kill VLC
  // fbi cue image
  printf("Entered Switchto for output %d\n", new_output);

  if ((new_output >= 1) && (new_output <= 7))    // fbi announcemedia image
  {
    snprintf(SystemCommand, 127, "sudo fbi -T 1 -noverbose -a %s >/dev/null 2>/dev/null", announcemediafile[new_output]);
    system(SystemCommand);
    Select_HDMI_Switch(0);  // Switch to controller image
    usleep (announcemediaduration[new_output] *1000000);    // delay cue image seconds
  }

  Select_HDMI_Switch(new_output);  // switch to new_output

  // fork 1 sec delay, kill fbi and then fbi K image
  snprintf(SystemCommand, 127, "sudo fbi -T 1 -noverbose -a %s >/dev/null 2>/dev/null", kmediafile);
  system(SystemCommand);
}

void DisplayK()
{
  char SystemCommand[127];
  printf("Entered DisplayK\n");

  snprintf(SystemCommand, 127, "sudo fbi -T 1 -noverbose -a %s >/dev/null 2>/dev/null", kmediafile);
  system(SystemCommand);
  printf("Creating the Carousel Thread\n");
  pthread_create (&thkcarousel, NULL, &Show_K_Carousel, NULL);
}

int priorityDecision()
{
  int priority_test;
  int i;
  int decision_result = -1;
  static int previous_decision_result;

  printf("previous decision result is %d, entering decision process\n", previous_decision_result);

  for (priority_test = 1; priority_test <= 8; priority_test++)
  {
    for (i = 1; i <= availableinputs; i++)
    {
      if ((inputprioritylevel[i] == priority_test) && (decision_result == -1) && (inputactive[i] == 1))  // so take first result
      {
        if (priority_test == 1)            // Always switch to the lowest numbered active priority 1 input
        {
          printf("priority_test = %d,i = %d, inputactive[i] = %d\n", priority_test, i, inputactive[i]);
          decision_result = i;
        }
        else                               // priority 2 - 8
        {
          if ((activeinputhold == true) && (inputactive[previous_decision_result] == 1))  // Use previous selection
          {
            decision_result = i;
          }
          if (activeinputhold == false)
          {
            decision_result = i;
          }
        }
      }
    }
  }
  if (decision_result >= 1 )
  {
    previous_decision_result = decision_result;
  }
  else
  {
    previous_decision_result = 0;
  }
  printf("After process, decision was %d\n", decision_result);

  return decision_result;
}

void repeaterEngine()
{
  int current_output = 0;
  int new_output = -1;
  inputStatusChange = true;   // To prompt selection of any input live on start-up
  firstCarousel = true;

  // Show the status screen if required
  if (StatusScreenOveride == true)
  {
    update_status_screen();
  }

  while (run_repeater == true)
  {
    if (inputStatusChange == true)
    {
      current_output = inputselected;
      new_output = priorityDecision();

      printf("Input status change.  Current Output = %d, New Output = %d\n", current_output, new_output);

      if (new_output != current_output)  // only change if there is a change
      {
        if (new_output == -1)  // No Active Inputs
        {
          Select_HDMI_Switch(0);  // Display Controller
          inputselected = 0;      // Is this still required??
          inputAfterIdent = 0;      // global for return from ident
          if ((ident_active == false) && (StatusScreenOveride == false))
          {
            DisplayK();  // and then go to carousel
          }
        }
        else
        {
          if (StatusScreenOveride == true)
          {
            update_status_screen();
            Select_HDMI_Switch(0);
          }
          else
          {
            Switchto(new_output);
          }
          inputselected = new_output; // Is this still required?
          inputAfterIdent = new_output; // global for return from ident
          //pthread_join(thkcarousel, NULL);
        }
      }
      inputStatusChange = false;

      if (StatusScreenOveride == true)
      {
        update_status_screen();
      }
    }
    usleep (1000); // 1ms loop
  }
}



static void
terminate(int dummy)
{
  run_repeater = false;
  pigpio_stop(localGPIO); // Disconnect pigpio from local Pi. 

  char Commnd[255];

  printf("Terminate\n");

  sprintf(Commnd,"stty echo");
  system(Commnd);
  sprintf(Commnd,"reset");
  system(Commnd);
  exit(1);
}


int main(int argc, char *argv[])
{

  int i;

  // Catch sigaction and call terminate
  for (i = 0; i < 16; i++)
  {
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = terminate;
    sigaction(i, &sa, NULL);
  }

  // Initialise GPIO
  localGPIO = pigpio_start(0, 0); // Connect to local Pi.

  if (localGPIO < 0)
  {
    printf("Can't connect to pigpio daemon\n");
    exit(1);
  }

  setUpGPIO();

  read_config_file();

  printf("BATC Repeater Controller for %s\n", callsign);

  initScreen();

  // Start the ident timer in a thread
  printf("Creating ident timer thread\n");
  pthread_create (&thidenttimer, NULL, &Show_Ident, NULL);


  // Monitor the input status lines in a thread
  printf("Creating input status monitor thread\n");
  pthread_create (&thinputactivemonitor, NULL, &InputStatusListener, NULL);

  // Monitor the control socket in a thread
  printf("Creating udp socket monitor thread\n");
  pthread_create (&thsocketmonitor, NULL, &SocketListener, NULL);

  //StatusScreenOveride = true;

  printf("Starting the main repeater controller\n");
  repeaterEngine();         // This keeps the repeater running

  // Code does not get to here

  return 0;
}
