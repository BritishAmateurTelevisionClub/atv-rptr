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


// Threads
pthread_t thinputactivemonitor;
pthread_t thidenttimer;
pthread_t thkcarousel;
pthread_t thsocketmonitor;

// Mutex's
pthread_mutex_t fbi_lock;

// Function Prototypes

void GetConfigParam(char *PathConfigFile, char *Param, char *Value);
void SetConfigParam(char *PathConfigFile, char *Param, char *Value);
void strcpyn(char *outstring, char *instring, int n);
void trimTrailing(char * str);
void log_rptr_error(char *errorstring);
void CheckConfigFile();
void read_config_file();
void setUpGPIO();
void update_status_screen();
void fbiThenKill(char *PathImageFile);
uint16_t IdentLength();
void *Show_Ident(void * arg);
void *Show_K_Carousel(void * arg);
void Seti2cAudioSwitch(int bitv[8]);
void sendIRcode(char *inputCode);
void Select_HDMI_Switch(int selection);
int Switchto(int new_output);
void DisplayK();
int priorityDecision();
void repeaterEngine();

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
  strcpy(Value, "");
  char error_message[255];

  file_test = file_exist(PathConfigFile);  // Log error if file does not exist
  if (file_test == 1)
  {
    snprintf(error_message, 255, "Config File %s not found", PathConfigFile);
    log_rptr_error(error_message);
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

  trimTrailing(Value);

  if (strlen(Value) == 0)  // Log error if parameter undefined
  {  
    snprintf(error_message, 255, "Parameter %s Undefined In Config File", Param);
    log_rptr_error(error_message);
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
  char error_message[255];

  if (debug_level == 2)
  {
    printf("Set Config called %s %s %s\n", PathConfigFile , ParamWithEquals, Value);
  }

  if (strlen(Value) == 0)  // Don't write empty values
  {
    snprintf(error_message, 255, "Attempt To Set Empty Config Value to parameter %s", Param);
    log_rptr_error(error_message);
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
 * @brief Trims trailing spaces in strings
 *
 * @param *str
 *
 * @return void
*******************************************************************************/
void trimTrailing(char * str)
{
  int index = -1;
  int i = 0;

  // Find last index of non-white space character

  while(str[i] != '\0')
  {
    if (str[i] != ' ' && str[i] != '\t' && str[i] != '\n')
    {
      index= i;
    }
     i++;
  }

  /* Mark next character to last non-white space character as NULL */
  str[index + 1] = '\0';
}

/***************************************************************************//**
 * @brief safely writes up to 220 characters of errorstring to the error log with a timestamp
 *
 * @param *errorstring
 *
 * @return void
*******************************************************************************/

void log_rptr_error(char *errorstring)
{
  time_t t; 
  struct tm tm;
  char timestamped_errorstring[255];
  char echo_command[511];

  t = time(NULL);
  tm = *gmtime(&t);

  snprintf(timestamped_errorstring, 255, "%d-%02d-%02d %02d:%02d:%02d ", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
  strcat (timestamped_errorstring, errorstring);
  // printf("\n%s\n\n", timestamped_errorstring);
  //snprintf(echo_command, 511, "echo \"%s\" >> /home/pi/atv-rptr/logs/error_log.txt", timestamped_errorstring);
  snprintf(echo_command, 511, "echo \"%s\" | sudo tee -a /var/log/rptr/error_log.txt  > /dev/null", timestamped_errorstring);
  //printf("\n%s\n\n", echo_command);
  system(echo_command);
}

/***************************************************************************//**
 * @brief Checks to see if new entries exist in the Config file
 *        and adds them if required
 *
 * @param None
 *
 * @return none
 * 
*******************************************************************************/

void CheckConfigFile()
{
  char shell_command[255];
  FILE *fp;
  int r;

  sprintf(shell_command, "grep -q 'atemip=' %s", PATH_CONFIG);
  fp = popen(shell_command, "r");
  r = pclose(fp);

  if (WEXITSTATUS(r) != 0)
  {
    printf("ATEM config values not detected\n");
    printf("Adding 14 parameters to config file\n");

    sprintf(shell_command, "echo >> %s", PATH_CONFIG);
    system(shell_command);
    sprintf(shell_command, "echo \\# ATEM Switching instructions >> %s", PATH_CONFIG);
    system(shell_command);
    sprintf(shell_command, "echo atemip=192.168.2.148 >> %s", PATH_CONFIG);
    system(shell_command);
    sprintf(shell_command, "echo atemoutput=0 >> %s", PATH_CONFIG);
    system(shell_command);
    sprintf(shell_command, "echo atemprogram=1 >> %s", PATH_CONFIG);
    system(shell_command);
    sprintf(shell_command, "echo atempreview=2 >> %s", PATH_CONFIG);
    system(shell_command);
    sprintf(shell_command, "echo output0atem=1 >> %s", PATH_CONFIG);
    system(shell_command);
    sprintf(shell_command, "echo output1atem=2 >> %s", PATH_CONFIG);
    system(shell_command);
    sprintf(shell_command, "echo output2atem=3 >> %s", PATH_CONFIG);
    system(shell_command);
    sprintf(shell_command, "echo output3atem=4 >> %s", PATH_CONFIG);
    system(shell_command);
    sprintf(shell_command, "echo output4atem=5 >> %s", PATH_CONFIG);
    system(shell_command);
    sprintf(shell_command, "echo output5atem=6 >> %s", PATH_CONFIG);
    system(shell_command);
    sprintf(shell_command, "echo output6atem=7 >> %s", PATH_CONFIG);
    system(shell_command);
    sprintf(shell_command, "echo output7atem=8 >> %s", PATH_CONFIG);
    system(shell_command);
    sprintf(shell_command, "echo outputmultiatem=10 >> %s", PATH_CONFIG);
    system(shell_command);
  }
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

  // Video output (for sizing config screen)
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

  // Ident Audio Out port (may need better decision making here later)
  GetConfigParam(PATH_CONFIG,"audioout", audioout);
  audiooutcard = 0;  // default hdmi
  if (strcmp(audioout, "jack") == 0)
  {
    audiooutcard = 1;
  }
  if (strcmp(audioout, "usb") == 0)
  {
    audiooutcard = 2;
  }

  // Audio Keep Alive
  strcpy(Value, "no");
  GetConfigParam(PATH_CONFIG, "audiokeepalive", Value);
  if (strcmp(Value, "yes") == 0)
  {
    audiokeepalive = true;

    // Set level
    GetConfigParam(PATH_CONFIG, "audiokeepalivelevel", Value);
    audiokeepalivelevel = atoi(Value);
    if ((audiokeepalivelevel < 0) || (audiokeepalivelevel > 100))
    {
      audiokeepalivelevel = 85;
    }
  }

  // Transmit Enabled?
  strcpy(Value, "no");
  GetConfigParam(PATH_CONFIG, "transmitenabled", Value);
  if (strcmp(Value, "yes") == 0)
  {
    transmitenabled = true;
  }

  // Beacon Mode?
  strcpy(Value, "no");
  GetConfigParam(PATH_CONFIG, "beaconmode", Value);
  if (strcmp(Value, "yes") == 0)
  {
    beaconmode = true;
  }

  // Rack Mains Control?
  strcpy(Value, "no");
  GetConfigParam(PATH_CONFIG, "rackmainscontrol", Value);
  if (strcmp(Value, "yes") == 0)
  {
    rackmainscontrol = true;
  }

  // Rack mains control parameters
  if ( rackmainscontrol == true)
  {
    // Front panel Rack Shutdown Button Pin
    strcpy(Value, "");
    GetConfigParam(PATH_CONFIG, "racksdbuttongpiopin", Value);
    rxsdbuttonGPIO = PinToBroadcom(atoi(Value));

    // Rack safe shutdown output pin
    strcpy(Value, "");
    GetConfigParam(PATH_CONFIG, "racksdsignalgpiopin", Value);
    rxsdsignalGPIO = PinToBroadcom(atoi(Value));

    // Rack mains Control output Pin
    strcpy(Value, "");
    GetConfigParam(PATH_CONFIG, "rackmainspwrgpiopin", Value);
    rxmainspwrGPIO = PinToBroadcom(atoi(Value));

    // Turn receivers off to save power?
    strcpy(Value, "no");
    GetConfigParam(PATH_CONFIG, "rackpowersave", Value);
    if (strcmp(Value, "yes") == 0)
    {
      rxpowersave = true;
    }

    // If required read in the power on and off times
    if (rxpowersave == true)
    {
      strcpy(Value, "0");
      GetConfigParam(PATH_CONFIG, "rackpoweron1", Value);
      rxpoweron1 = atoi(Value);
      if ((rxpoweron1 < 0) || (rxpoweron1 > 2359))
      {
        rxpoweron1 = 0;
        system("echo rackpoweron1_in_config_file_out_of_limits >> /home/pi/atv-rptr/logs/error_log.txt");
      }

      strcpy(Value, "0");
      GetConfigParam(PATH_CONFIG, "rackpoweroff1", Value);
      rxpoweroff1 = atoi(Value);
      if ((rxpoweroff1 < 0) || (rxpoweroff1 > 2359))
      {
        rxpoweroff1 = 0;
        system("echo rackpoweroff1_in_config_file_out_of_limits >> /home/pi/atv-rptr/logs/error_log.txt");
      }

      strcpy(Value, "0");
      GetConfigParam(PATH_CONFIG, "rackpoweron2", Value);
      rxpoweron2 = atoi(Value);
      if ((rxpoweron2 < 0) || (rxpoweron2 > 2359))
      {
        rxpoweron2 = 0;
        system("echo rackpoweron2_in_config_file_out_of_limits >> /home/pi/atv-rptr/logs/error_log.txt");
      }

      strcpy(Value, "0");
      GetConfigParam(PATH_CONFIG, "rackpoweroff2", Value);
      rxpoweroff2 = atoi(Value);
      if ((rxpoweroff2 < 0) || (rxpoweroff2 > 2359))
      {
        rxpoweroff2 = 0;
        system("echo rackpoweroff2_in_config_file_out_of_limits >> /home/pi/atv-rptr/logs/error_log.txt");
      }
    }
  }

  // Continuous TX or power-saving?
  strcpy(Value, "no");
  GetConfigParam(PATH_CONFIG, "transmitwhennotinuse", Value);
  if (strcmp(Value, "yes") == 0)
  {
    transmitwhennotinuse = true;
  }

  // Operate 24/7?
  strcpy(Value, "no");
  GetConfigParam(PATH_CONFIG, "24houroperation", Value);
  if (strcmp(Value, "yes") == 0)
  {
    hour24operation = true;
  }

  // Save Power in the second half hour during active hours?
  strcpy(Value, "no");
  GetConfigParam(PATH_CONFIG, "halfhourpowersave", Value);
  if (strcmp(Value, "yes") == 0)
  {
    halfhourpowersave = true;
  }

  // Operating times
  strcpy(Value, "0");
  GetConfigParam(PATH_CONFIG, "operatingtimestart", Value);
  operatingtimestart = atoi(Value);
  if ((operatingtimestart < 0) || (operatingtimestart > 2359))
  {
    operatingtimestart = 0;
    system("echo operatingtimestart_in_config_file_out_of_limits >> /home/pi/atv-rptr/logs/error_log.txt");
  }

  strcpy(Value, "0");
  GetConfigParam(PATH_CONFIG, "operatingtimefinish", Value);
  operatingtimefinish = atoi(Value);
  if ((operatingtimefinish < 0) || (operatingtimefinish > 2359))
  {
    operatingtimefinish = 0;
    system("echo operatingtimefinish_in_config_file_out_of_limits >> /home/pi/atv-rptr/logs/error_log.txt");
  }

  // Repeat during quiet hours?
  strcpy(Value, "no");
  GetConfigParam(PATH_CONFIG, "repeatduringquiethours", Value);
  if (strcmp(Value, "yes") == 0)
  {
    repeatduringquiethours = true;
  }

  // Transmit idents during quiet hours?
  strcpy(Value, "no");
  GetConfigParam(PATH_CONFIG, "identduringquiethours", Value);
  if (strcmp(Value, "yes") == 0)
  {
    identduringquiethours = true;
  }

  // PTT command GPIO Pin
  strcpy(Value, "");
  GetConfigParam(PATH_CONFIG, "pttgpiopin", Value);
  pttGPIO = PinToBroadcom(atoi(Value));

  // Front Panel Shutdown Button Enabled?
  strcpy(Value, "no");
  GetConfigParam(PATH_CONFIG, "fpshutdown", Value);
  if (strcmp(Value, "yes") == 0)
  {
    fpsdenabled = true;

    // FP Shutdown GPIO Pin
    strcpy(Value, "");
    GetConfigParam(PATH_CONFIG, "fpsdgpiopin", Value);
    fpsdGPIO = PinToBroadcom(atoi(Value));
  }

  // Number of inputs
  strcpy(Value, "");
  GetConfigParam(PATH_CONFIG, "availableinputs", Value);
  availableinputs = atoi(Value);
  if ((availableinputs < 1) || (availableinputs > 7))
  {
    availableinputs = 7;
  }

  // DTMF Config

  // DTMF Control Enabled?
  GetConfigParam(PATH_CONFIG, "dtmfcontrol", Value);
  if (strcmp(Value, "on") == 0)
  {
    dtmf_enabled = true;
  }

  // DTMF Command timeout in seconds 
  strcpy(Value, "");
  GetConfigParam(PATH_CONFIG, "dtmfactiontimeout", Value);
  dtmfactiontimeout = atoi(Value);

  // DTMF Reset Code (Puts repeater back to default operation)
  GetConfigParam(PATH_CONFIG, "dtmfreset", dtmfresetcode);

  // DTMF Status View Code (Puts repeater into status view)
  GetConfigParam(PATH_CONFIG, "dtmfstatusview", dtmfstatusviewcode);

  // DTMF Quad View Code (Puts repeater into quad view)
  GetConfigParam(PATH_CONFIG, "dtmfquadview", dtmfquadviewcode);

  // DTMF Talkback Audio enable Code
  GetConfigParam(PATH_CONFIG, "dtmftalkbackaudioenable", dtmftalkbackaudioenablecode);

  // DTMF Talkback Audio disable Code
  GetConfigParam(PATH_CONFIG, "dtmftalkbackaudiodisable", dtmftalkbackaudiodisablecode);

  // DTMF Keeper TX off Code (Turns repeater off (AND modifies config file)
  GetConfigParam(PATH_CONFIG, "dtmfkeepertxoff", dtmfkeepertxoffcode);

  // DTMF Keeper TX on Code (Turns repeater on (AND modifies config file)
  GetConfigParam(PATH_CONFIG, "dtmfkeepertxon", dtmfkeepertxoncode);

  // DTMF Keeper Reboot Code (Puts repeater into quad view)
  GetConfigParam(PATH_CONFIG, "dtmfkeeperreboot", dtmfkeeperrebootcode);

  // DTMF Input Select codes
  for (i = 0; i <= availableinputs; i++)
  {
    strcpy(Value, "");
    snprintf(Param, 127, "dtmfselectinput%d", i);
    GetConfigParam(PATH_CONFIG, Param, Value);
    dtmfselectinput[i] = atoi(Value);
  }

  // DTMF Control of Accessory Outputs

  // Number of outputs (0 - 10)
  strcpy(Value, "");
  GetConfigParam(PATH_CONFIG, "dtmfoutputs", Value);
  dtmfoutputs = atoi(Value);

  if (dtmfoutputs > 10)
  {
    dtmfoutputs = 0;
    system("echo dtmfoutputs_in_config_file_out_of_limits >> /home/pi/atv-rptr/logs/error_log.txt");
  }
  else
  {
    if (dtmfoutputs > 0)
    {
      for(i = 1; i <= dtmfoutputs; i++)
      {
        // Accessory Output GPIO pin
        strcpy(Value, "");
        snprintf(Param, 127, "dtmfgpioout%dpin", i);
        GetConfigParam(PATH_CONFIG, Param, Value);
        dtmfoutputGPIO[i] = PinToBroadcom(atoi(Value));

        // Accessory Output GPIO pin label
        strcpy(Value, "");
        snprintf(Param, 127, "dtmfgpioout%dlabel", i);
        GetConfigParam(PATH_CONFIG, Param, dtmfgpiooutlabel[i]);

        // Accessory Output GPIO on code DTMF
        strcpy(Value, "");
        snprintf(Param, 127, "dtmfgpioout%don", i);
        GetConfigParam(PATH_CONFIG, Param, dtmfgpiooutoncode[i]);

        // Accessory Output GPIO off code DTMF
        strcpy(Value, "");
        snprintf(Param, 127, "dtmfgpioout%doff", i);
        GetConfigParam(PATH_CONFIG, Param, dtmfgpiooutoffcode[i]);
      }
    }
  }

  // Monitoring of Accessory Outputs

  // Number of inputs (0 - 10)
  strcpy(Value, "");
  GetConfigParam(PATH_CONFIG, "dtmfinputs", Value);
  dtmfinputs = atoi(Value);

  if (dtmfinputs > 10)
  {
    dtmfinputs = 0;
    system("echo dtmfinputs_in_config_file_out_of_limits >> /home/pi/atv-rptr/logs/error_log.txt");
  }
  else
  {
    if (dtmfinputs > 0)
    {
      for(i = 1; i <= dtmfinputs; i++)
      {
        // Accessory Input GPIO pin
        strcpy(Value, "");
        snprintf(Param, 127, "dtmfgpioin%dpin", i);
        GetConfigParam(PATH_CONFIG, Param, Value);
        dtmfinputGPIO[i] = PinToBroadcom(atoi(Value));

        // Accessory Input GPIO pin label
        strcpy(Value, "");
        snprintf(Param, 127, "dtmfgpioin%dlabel", i);
        GetConfigParam(PATH_CONFIG, Param, dtmfgpioinlabel[i]);
      }
    }
  }

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
    strcpy(Value, "");
    GetConfigParam(PATH_CONFIG, "identcwlevel", Value);
    identcwlevel = atoi(Value);
    if ((identcwlevel < 0) || (identcwlevel > 100))
    {
      identcwlevel = 100;
    }
    GetConfigParam(PATH_CONFIG, "identcwfile", identcwfile);
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
    strcpy(Value, "");
    GetConfigParam(PATH_CONFIG, "kcwlevel", Value);
    kcwlevel = atoi(Value);
    if ((kcwlevel < 0) || (kcwlevel > 100))
    {
      kcwlevel = 100;
    }
    GetConfigParam(PATH_CONFIG, "kcwfile", kcwfile);
  }

  // Announce bleep config
  GetConfigParam(PATH_CONFIG, "announcebleep", Value);
  if (strcmp(Value, "on") == 0)
  {
    announcebleep = true;
    strcpy(Value, "");
    GetConfigParam(PATH_CONFIG, "announcebleeplevel", Value);
    announcebleeplevel = atoi(Value);
    if ((announcebleeplevel < 0) || (announcebleeplevel > 100))
    {
      announcebleeplevel = 100;
    }
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
    for (i = 1; i <= 9; i++)
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

  // Audio Switch Configuration
  GetConfigParam(PATH_CONFIG, "audioswitch", audioswitch);
  if (strcmp(audioswitch, "i2c") == 0)
  {
    strcpy(Value, "");
    GetConfigParam(PATH_CONFIG, "audioi2caddress", Value);
    audioi2caddress = atoi(Value);
    if ((audioi2caddress < 0) || (audioi2caddress > 7))
    {
      audioi2caddress = 7;
    }
  }

  // Talkback audio
  GetConfigParam(PATH_CONFIG, "talkbackaudio", Value);
  if (strcmp(Value, "off") == 0)
  {
    talkbackaudio = false;
  }
  else
  {
    talkbackaudio = true;
  }
  strcpy(Value, "");
  GetConfigParam(PATH_CONFIG, "talkbackaudioi2cbit", Value);
  talkbackaudioi2cbit = atoi(Value);
  if ((talkbackaudioi2cbit < 0) || (talkbackaudioi2cbit > 7))
  {
    talkbackaudioi2cbit = 7;
  }

  // HDMI Switch Configuration

  // IR or GPIO Switched
  GetConfigParam(PATH_CONFIG, "outputswitchcontrol", outputswitchcontrol);
  if (strcmp(outputswitchcontrol, "ir") == 0)
  {
    // Reset Code for HDMI Switch
    GetConfigParam(PATH_CONFIG, "outputhdmiresetcode", outputhdmiresetcode);

    // Daisy Chain input Code for primary HDMI Switch
    GetConfigParam(PATH_CONFIG, "output2ndhdmicode", output2ndhdmicode);

    // Quad View Code for primary HDMI Switch
    GetConfigParam(PATH_CONFIG, "outputhdmiquadcode", outputhdmiquadcode);
  }

  // Input and Output Configuration


  // html switched
  if (strcmp(outputswitchcontrol, "html") == 0)
  {
    // Base url and port for server (without trailing slash)
    GetConfigParam(PATH_CONFIG, "networkctrlurl", networkctrlurl);

    // quad output command
    GetConfigParam(PATH_CONFIG, "outputquadnetcommand", outputquadnetcommand);

    for(i = 0 ; i <= availableinputs ; i++)
    {
      // html output switch commands
      strcpy(Value, "");
      snprintf(Param, 127, "output%dnetcommand", i);
      GetConfigParam(PATH_CONFIG, Param, Value);
      strcpy(outputnetcommand[i], Value);
    }
  }

  // Show GPIO in addition to IR or html?
  GetConfigParam(PATH_CONFIG, "showoutputongpio", Value);
  if (strcmp(Value, "yes") == 0)
  {
    showoutputongpio = true;
  }
  else
  {
    showoutputongpio = false;
  }

  // Behavior on input conflict
  GetConfigParam(PATH_CONFIG, "activeinputhold", Value);
  if (strcmp(Value, "no") == 0)
  {
    activeinputhold = false;
  }

  // Show Quad For Multiple Inputs?
  GetConfigParam(PATH_CONFIG, "showquadformultipleinputs", Value);
  if (strcmp(Value, "yes") == 0)
  {
    showquadformultipleinputs = true;
  }

  // Cascaded HDMI Switches?
  GetConfigParam(PATH_CONFIG, "cascadedswitches", Value);
  if (strcmp(Value, "yes") == 0)
  {
    cascadedswitches = true;
  }

  // ATEM IP address
  strcpy(Value, "");
  GetConfigParam(PATH_CONFIG, "atemip", Value);
  strcpy(atemip, Value);

  // ATEM Video Output
  strcpy(Value, "");
  GetConfigParam(PATH_CONFIG, "atemoutput", Value);
  strcpy(atemoutput, Value);

  // ATEM Program Output
  strcpy(Value, "");
  GetConfigParam(PATH_CONFIG, "atemprogram", Value);
  strcpy(atemprogram, Value);

  // ATEM Preview Output
  strcpy(Value, "");
  GetConfigParam(PATH_CONFIG, "atempreview", Value);
  strcpy(atempreview, Value);

  // ATEM Multiview input
  strcpy(Value, "");
  GetConfigParam(PATH_CONFIG, "outputmultiatem", Value);
  strcpy(outputmultiatem, Value);

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

    // Audio Switch i2c bit
    strcpy(Value, "");
    snprintf(Param, 127, "output%daudioi2cbit", i);
    GetConfigParam(PATH_CONFIG, Param, Value);
    outputaudioi2cbit[i] = atoi(Value);
    if ((outputaudioi2cbit[i] < 0) || (outputaudioi2cbit[i] > 7))
    {
      outputaudioi2cbit[i] = 0;
    }

    // ATEM Switch configuration
    strcpy(Value, "");
    snprintf(Param, 127, "output%datem", i);
    GetConfigParam(PATH_CONFIG, Param, outputatem[i]);

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
  int pin;
  bool pin_in_use = false;
  bool pin_conflict = false;
  char pinfunction_pri[63];
  char pinfunction_sec[63];
  strcpy(pinfunction_sec, "");

  // Check for GPIO Conflicts:
  for (pin = 1; pin <= 40; pin++)
  {
    pin_in_use = false;
    pin_conflict = false;
    strcpy(pinfunction_pri, "not used");
    strcpy(pinfunction_sec, "");

    switch (pin)
    {
      case 1:
        pin_in_use = true;
        strcpy(pinfunction_pri, "3.3 volts out");
        break;
      case 2:
        pin_in_use = true;
        strcpy(pinfunction_pri, "5 volts in");
        break;
      case 4:
        pin_in_use = true;
        strcpy(pinfunction_pri, "5 volts in");
        break;
      case 6:
        pin_in_use = true;
        strcpy(pinfunction_pri, "Ground");
        break;
      case 8:
        pin_in_use = true;
        strcpy(pinfunction_pri, "i2c sda if used");
        break;
      case 9:
        pin_in_use = true;
        strcpy(pinfunction_pri, "Ground");
        break;
      case 10:
        pin_in_use = true;
        strcpy(pinfunction_pri, "i2c scl if used");
        break;
      case 12:
        pin_in_use = true;
        strcpy(pinfunction_pri, "IR Sender Output");
        break;
      case 13:
        pin_in_use = true;
        strcpy(pinfunction_pri, "IR Bank Select");
        break;
      case 14:
        pin_in_use = true;
        strcpy(pinfunction_pri, "Ground");
        break;
      case 17:
        pin_in_use = true;
        strcpy(pinfunction_pri, "3.3 volts out");
        break;
      case 20:
        pin_in_use = true;
        strcpy(pinfunction_pri, "Ground");
        break;
      case 25:
        pin_in_use = true;
        strcpy(pinfunction_pri, "Ground");
        break;
      case 27:
        pin_in_use = true;
        strcpy(pinfunction_pri, "Reserved for RPi Hat EEPROM");
        break;
      case 28:
        pin_in_use = true;
        strcpy(pinfunction_pri, "Reserved for RPi Hat EEPROM");
        break;
      case 30:
        pin_in_use = true;
        strcpy(pinfunction_pri, "Ground");
        break;
      case 34:
        pin_in_use = true;
        strcpy(pinfunction_pri, "Ground");
        break;
      case 39:
        pin_in_use = true;
        strcpy(pinfunction_pri, "Ground");
        break;
    }

    // Input Active signal inputs
    for (i = 1; i <= availableinputs ; i++)
    {
      if (pin_in_use == false)
      {
        if (inputactiveGPIO[i] == PinToBroadcom(pin))
        {
          pin_in_use = true;
          snprintf(pinfunction_pri, 62, "Input %d Active Signal", i);
        }
      }
      else       // pin already in use
      {
        if (inputactiveGPIO[i] == PinToBroadcom(pin))
        {
          pin_conflict = true;
          snprintf(pinfunction_sec, 62, "Input %d Active Signal", i);
        }
      }
    }

    // Output select Outputs
    if ((showoutputongpio == true) || (strcmp(outputswitchcontrol, "gpio") == 0))
    {
      for (i = 0; i <= availableinputs ; i++)
      {
        if (pin_in_use == false)
        {
          if (outputGPIO[i] == PinToBroadcom(pin))
          {
            pin_in_use = true;
            snprintf(pinfunction_pri, 62, "Output %d Select Signal", i);
          }
        }
        else       // pin already in use
        {
          if (outputGPIO[i] == PinToBroadcom(pin))
          {
            pin_conflict = true;
            snprintf(pinfunction_sec, 62, "Output %d Select Signal", i);
          }
        }
      }
    }

    // PTT GPIO
    if (pin_in_use == false)
    {
      if (pttGPIO == PinToBroadcom(pin))
      {
        pin_in_use = true;
        snprintf(pinfunction_pri, 62, "PTT Activate Pin");
      }
    }
    else       // pin already in use
    {
      if (pttGPIO == PinToBroadcom(pin))
      {
        pin_conflict = true;
        snprintf(pinfunction_sec, 62, "PTT Activate Pin");
      }
    }

    // Shutdown button GPIO
    if (fpsdenabled == true)
    {
      if (pin_in_use == false)
      {
        if (fpsdGPIO == PinToBroadcom(pin))
        {
          pin_in_use = true;
          snprintf(pinfunction_pri, 62, "Controller Shutdown Button");
        }
      }
      else       // pin already in use
      {
        if (fpsdGPIO == PinToBroadcom(pin))
        {
          pin_conflict = true;
          snprintf(pinfunction_sec, 62, "Controller Shutdown Button");
        }
      }
    }

    // Rack Shutdown button GPIO
    if (rackmainscontrol == true)
    {
      if (pin_in_use == false)
      {
        if (rxsdbuttonGPIO == PinToBroadcom(pin))
        {
          pin_in_use = true;
          snprintf(pinfunction_pri, 62, "Rack Shutdown Button");
        }
      }
      else       // pin already in use
      {
        if (rxsdbuttonGPIO == PinToBroadcom(pin))
        {
          pin_conflict = true;
          snprintf(pinfunction_sec, 62, "Rack Shutdown Button");
        }
      }
    }

    // Rack Safe Shutdown Signal GPIO
    if (rackmainscontrol == true)
    {
      if (pin_in_use == false)
      {
        if (rxsdsignalGPIO == PinToBroadcom(pin))
        {
          pin_in_use = true;
          snprintf(pinfunction_pri, 62, "Rack safe Shutdown Signal");
        }
      }
      else       // pin already in use
      {
        if (rxsdsignalGPIO == PinToBroadcom(pin))
        {
          pin_conflict = true;
          snprintf(pinfunction_sec, 62, "Rack safe Shutdown Signal");
        }
      }
    }

    // Rack Mains Shutdown Control Signal GPIO
    if (rackmainscontrol == true)
    {
      if (pin_in_use == false)
      {
        if (rxmainspwrGPIO == PinToBroadcom(pin))
        {
          pin_in_use = true;
          snprintf(pinfunction_pri, 62, "Rack Mains Shutdown Control Signal");
        }
      }
      else       // pin already in use
      {
        if (rxmainspwrGPIO == PinToBroadcom(pin))
        {
          pin_conflict = true;
          snprintf(pinfunction_sec, 62, "Rack Mains Shutdown Control Signal");
        }
      }
    }

    // Accessory output GPIOs
    if (dtmfoutputs > 0)
    {
      for (i = 1; i <= dtmfoutputs ; i++)
      {
        if (pin_in_use == false)
        {
          if (dtmfoutputGPIO[i] == PinToBroadcom(pin))
          {
            pin_in_use = true;
            snprintf(pinfunction_pri, 62, "DTMF Logic Output %d", i);
          }
        }
        else       // pin already in use
        {
          if (dtmfoutputGPIO[i] == PinToBroadcom(pin))
          {
            pin_conflict = true;
            snprintf(pinfunction_sec, 62, "DTMF Logic Output %d", i);
          }
        }
      }
    }

    // Accessory Input GPIOs
    if (dtmfinputs > 0)
    {
      for (i = 1; i <= dtmfinputs ; i++)
      {
        if (pin_in_use == false)
        {
          if (dtmfinputGPIO[i] == PinToBroadcom(pin))
          {
            pin_in_use = true;
            snprintf(pinfunction_pri, 62, "Accessory input %d", i);
          }
        }
        else       // pin already in use
        {
          if (dtmfinputGPIO[i] == PinToBroadcom(pin))
          {
            pin_conflict = true;
            snprintf(pinfunction_sec, 62, "Accessory input %d", i);
          }
        }
      }
    }

    if (strlen(pinfunction_sec) > 0)
    {
      printf("GPIO Pin %d allocated to %s.      Conflicting function: %s\n", pin, pinfunction_pri, pinfunction_sec);
    }
    else
    {
      printf("GPIO Pin %d allocated to %s.\n", pin, pinfunction_pri);
    }

    if (pin_conflict == true)
    {
      printf("#################################\n");
      printf("#                               #\n");
      printf("#  WARNING - GPIO PIN CONFLICT  #\n");
      printf("#                               #\n");
      printf("#################################\n\n");
      printf("GPIO Pin %d allocated to %s and %s\n\n", pin, pinfunction_pri, pinfunction_sec);
      strcpy(pinfunction_sec, "");
    }
  }


  // Set all the "Input Active" GPIOs to read-only
  for (i = 1; i <= availableinputs ; i++)
  {
    set_mode(localGPIO, inputactiveGPIO[i], 0);
  }

  // Set the PTT GPIO as an output and low
  set_mode(localGPIO, pttGPIO, 1);
  gpio_write(localGPIO, pttGPIO, 0);

  // Set shutdown button GPIO as an input with pull-up
  if (fpsdenabled == true)
  {
    set_mode(localGPIO, fpsdGPIO, 0);
    set_pull_up_down(localGPIO, fpsdGPIO, PI_PUD_UP);

    // read it to check that it is not low.  If it is, disable it now
    if (gpio_read(localGPIO, fpsdGPIO) != 1)
    {
      fpsdenabled = false;
    }
  }

  // Set receiver shutdown button GPIO as an input with pull-up
  // and mains power and shutdown signals to off
  if (rxpowersave == true)
  {
    set_mode(localGPIO, rxsdbuttonGPIO, 0);
    set_pull_up_down(localGPIO, rxsdbuttonGPIO, PI_PUD_UP);

    // read it to check that it is not low.  If it is, disable it now
    if (gpio_read(localGPIO, rxsdbuttonGPIO) != 1)
    {
      rxpowersave = false;
    }

    set_mode(localGPIO, rxsdsignalGPIO, 1);
    gpio_write(localGPIO, rxsdsignalGPIO, 1);

    set_mode(localGPIO, rxmainspwrGPIO, 1);
    gpio_write(localGPIO, rxmainspwrGPIO, 0);
  }

  // If GPIO-switched video switching, set the outputs and set to low
  if ((showoutputongpio == true) || (strcmp(outputswitchcontrol, "gpio") == 0))
  {
    for (i = 0; i <= availableinputs ; i++)
    {
      set_mode(localGPIO, outputGPIO[i], 1);
      gpio_write(localGPIO, outputGPIO[i], 0);
    }
  }

  // Set Accessory output GPIO as outputs and low
  if (dtmfoutputs > 0)
  {
    for (i = 1; i <= dtmfoutputs ; i++)
    {
      set_mode(localGPIO, dtmfoutputGPIO[i], 1);
      gpio_write(localGPIO, dtmfoutputGPIO[i], 0);
    }
  }

  // Set Accessory input GPIO as inputs
  if (dtmfinputs > 0)
  {
    for (i = 1; i <= dtmfinputs ; i++)
    {
      set_mode(localGPIO, dtmfinputGPIO[i], 0);
    }
  }

  // Set Daisy Chain IR Output GPIO as an output and initialise low
  set_mode(localGPIO, daisychainirselectgpio, 1);
  gpio_write(localGPIO, daisychainirselectgpio, 0);
  

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
  int level;

  time_t t; 
  struct tm tm;

  t = time(NULL);
  tm = *gmtime(&t);


  setBackColour(0,0,0);
  clearScreen();
  setForeColour(255, 255, 255);
  const font_t *font_ptr = &font_dejavu_sans_30;

  char ipaddress[17] = "Not connected";
  GetIPAddr(ipaddress);

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

 
  for(i = 0 ; i <= availableinputs ; i++)
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

  snprintf(display_text, 31, "Local IP %s", ipaddress);
  Text2(screen_width * 22 / 32, screen_height - (6 * line_height), display_text, font_ptr);

  snprintf(display_text, 31, "CPU Temp %.1f C", GetCPUTemp());
  Text2(screen_width * 22 / 32, screen_height - (7 * line_height), display_text, font_ptr);

  snprintf(display_text, 31, "UTC Time: %02d:%02d", tm.tm_hour, tm.tm_min);
  Text2(screen_width * 22 / 32, screen_height - (8 * line_height), display_text, font_ptr);

  if (dtmfinputs > 0)
  {
    for (i = 1; i <= dtmfinputs; i++)
    {
      level = gpio_read(localGPIO, dtmfinputGPIO[i]);
      if (level == 0)
      {
        snprintf(display_text, 63, "%s: OFF", dtmfgpioinlabel[i]);
      }
      if (level == 1)
      {
        snprintf(display_text, 63, "%s: ON ", dtmfgpioinlabel[i]);
      }
      Text2(screen_width * 22 / 32, screen_height - ((8 + i) * line_height), display_text, font_ptr);
    }
  }

  // Show current config
  snprintf(display_text, 127, "Current Status: %s", StatusForConfigDisplay);
  Text2(screen_width / 32, screen_height - (15 * line_height), display_text, font_ptr);

  //time_t t; 
  //struct tm tm;

  //t = time(NULL);
  //tm = *gmtime(&t);
  //printf("now: %d-%02d-%02d %02d:%02d:%02d\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
}


void fbiThenKill(char *PathImageFile)
{
  char SystemCommand[127];

  snprintf(SystemCommand, 127, "sudo fbi -T 1 -noverbose -a %s >/dev/null 2>/dev/null", PathImageFile);
  system(SystemCommand);
  usleep(400000);  // Increased from 0.2 s to improve reliability

  strcpy(SystemCommand, "sudo killall -9 fbi >/dev/null 2>/dev/null");
  system(SystemCommand);
  usleep(200000);
}


uint16_t IdentLength()  // Returns the length of the ident audio file in ms
{
  FILE *fp;
  char length[15];
  uint16_t lengthms = 0;

  /* Open the command for reading. */
  fp = popen("soxi -D /home/pi/tmp/ident.wav", "r");
  if (fp == NULL) {
    printf("Failed to run command\n" );
    exit(1);
  }

  /* Read the output a line at a time - output it. */
  while (fgets(length, 14, fp) != NULL)
  {
    printf("%s", length);
  }

  /* close */
  pclose(fp);

  lengthms = (uint16_t)(1000.0 * atof(length));

  printf("Length = %d ms\n", lengthms);
  return lengthms;
}


void *Show_Ident(void * arg)
{
  uint64_t last_ident;
  uint64_t ident_required;
  uint64_t ident_finish;
  uint64_t quiet_hours_check;
  uint32_t identduration;
  char identlevelcommand[127];
  char identplaycommand[127];
  int newi2caudiostatus[8];
  bool i2caudioswitchforident = false;
  int i;

  time_t t; 
  struct tm tm;
  int previous_minute = 0;

  // Check the length of the Ident
  if (identcwduration >= 1000 * identmediaduration)
  {
    identduration = identcwduration;
  }
  else
  {
    identduration = 1000 * identmediaduration;
  }

  last_ident = monotonic_ms();
  ident_required = last_ident  + identinterval * 1000;
  ident_finish = last_ident + (identinterval * 1000) + identduration + 2000;  // Add 2000 ms for approx process time
  printf("Starting Ident Thread.  Ident Interval = %d\n", identinterval);
  quiet_hours_check = monotonic_ms() + 1000;
  int refresh_status_second_count = 0;

  while (run_repeater == true)
  {
    //printf("Ident decision. monotonic = %llu; ident required = %llu ident_finish %llu\n", monotonic_ms(), ident_required, ident_finish);

    // check once per second for quiet hours switching
    if (monotonic_ms() > quiet_hours_check)
    {
      PTTEvent(7);
      quiet_hours_check = quiet_hours_check + 1000;

      // Periodic Status Screen Refresh
      if (StatusScreenOveride == true)
      {
        if (refresh_status_second_count > 10)
        {
          update_status_screen();
          refresh_status_second_count = 0;
        }
        refresh_status_second_count = refresh_status_second_count + 1;
      }

      // Write time to log once per minute
      t = time(NULL);
      tm = *gmtime(&t);
      if (tm.tm_min != previous_minute)
      {
        printf("Log Timestamp: %d-%02d-%02d %02d:%02d:%02d UTC\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
        previous_minute = tm.tm_min;
      }
    }

    if (monotonic_ms() > ident_required)
    {
      printf("Commencing Ident\n");
      ident_active = true;
      last_ident = ident_required;
      ident_required = last_ident + identinterval * 1000;

      // Check that ident video or caption is to be displayed
      if ((strcmp(identmediafile, "none") != 0) && (strcmp(identmediatype, "none") != 0))
      {
        // kill vlc?

        // Wait for fbi to be available
        pthread_mutex_lock(&fbi_lock);

        fbiThenKill(identmediafile);

        // Release fbi
        pthread_mutex_unlock(&fbi_lock);

        Select_HDMI_Switch(0);
      }

      // Play the audio file if required
      if (identcwaudio == true)
      {
        printf("Starting Ident Audio\n");
        if (audiooutcard == 1)                    // RPi Audio Jack
        {
          snprintf(identlevelcommand, 127, "amixer -c 1  -- sset Headphone Playback Volume %d%% >/dev/null 2>/dev/null", identcwlevel);
          system (identlevelcommand);
          usleep(200000); 
          snprintf(identplaycommand, 127, "aplay -D plughw:CARD=Headphones,DEV=0 %s >/dev/null 2>/dev/null &", identcwfile);
          system (identplaycommand);
        }
        else if (audiooutcard == 2)               // USB Audio Dongle
        {
          snprintf(identlevelcommand, 127, "amixer -c 2  -- sset Speaker Playback Volume %d%% >/dev/null 2>/dev/null", identcwlevel);
          system (identlevelcommand);
          usleep(200000); 
          snprintf(identplaycommand, 127, "aplay -D plughw:CARD=Device,DEV=0 %s >/dev/null 2>/dev/null &", identcwfile);
          system (identplaycommand);
        }
        else                                     // HDMI and default
        {
          snprintf(identlevelcommand, 127, "amixer -c 0  -- sset HDMI Playback Volume %d%% >/dev/null 2>/dev/null", identcwlevel);
          system (identlevelcommand);
          usleep(200000); 
          snprintf(identplaycommand, 127, "aplay -D plughw:CARD=b1,DEV=0 %s >/dev/null 2>/dev/null &", identcwfile);
          system (identplaycommand);
        }

        if ((currenti2caudiostatus[0] == 0) && (strcmp(audioswitch, "i2c") == 0))  // i2c audio req for ident
        {
          i2caudioswitchforident = true;
          newi2caudiostatus[0] = 1;
          for (i = 1; i <= 7; i++)
          {
            newi2caudiostatus[i] = currenti2caudiostatus[i];
          }
          Seti2cAudioSwitch(newi2caudiostatus);
        }
      }

      // Raise PTT if required
      PTTEvent(5);

      // Now set the exact ident finish time
      ident_finish = monotonic_ms() + identduration;  // Sets the exact ident finish time
    }

    if (monotonic_ms() > ident_finish)
    {
      printf("Stopping Ident Audio\n");
      ident_finish = ident_required + identduration + 2000;  // Set approx ident finish time in the future

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

        // Cut PTT if required
        PTTEvent(6);

        // Carousel will refresh on next image

        // switch to the current screen
        Select_HDMI_Switch(inputAfterIdent);        
      }

      // Reduce playback volume back to level for keep-alive noise if required
      if (audiooutcard == 0)                    // HDMI Audio
      {
        snprintf(identlevelcommand, 127, "amixer -c 0  -- sset HDMI Playback Volume %d%% >/dev/null 2>/dev/null", audiokeepalivelevel);
        system (identlevelcommand);
      }

      // Turn off i2c audio for ident if required
      if (i2caudioswitchforident == true)
      {
        newi2caudiostatus[0] = 0;
        Seti2cAudioSwitch(newi2caudiostatus);
      }

      // input may have dropped during ident so make sure carousel runs
      if (inputAfterIdent == 0)  
      {
        run_carousel = true;
        if (carousel_active == false)
        {
          DisplayK();
        }
      }
    }
    usleep(100000);
  }
  return NULL;
}


void *Show_K_Carousel(void * arg)
{
  uint64_t media_start;
  run_carousel = true;
  int i;
  int next_i;
  int carouselSource;
  bool pastendoffirstcarousel = false;
  char cwlevelcommand[127];
  char cwplaycommand[127];

  printf("Entering the KCarousel thread\n");
  strcpy(StatusForConfigDisplay, "Displaying the K");
  carousel_active = true;
  // Display the K initially
  if ((ident_active == false) && (StatusScreenOveride == false))
  {
    printf("Displaying the K\n");

    pthread_mutex_lock(&fbi_lock);

    fbiThenKill(kmediafile);

    // Release fbi
    pthread_mutex_unlock(&fbi_lock);
   
    // Play the audio file if required
    if (kcwaudio == true)
    {
      if (audiooutcard == 1)                    // RPi Audio Jack
      {
        snprintf(cwlevelcommand, 127, "amixer -c 1  -- sset Headphone Playback Volume %d%% >/dev/null 2>/dev/null", kcwlevel);
        system (cwlevelcommand);
        usleep(200000); 
        snprintf(cwlevelcommand, 127, "aplay -D plughw:CARD=Headphones,DEV=0 %s >/dev/null 2>/dev/null &", kcwfile);
        system (cwlevelcommand);
      }
      else if (audiooutcard == 2)               // USB Audio Dongle
      {
        snprintf(cwlevelcommand, 127, "amixer -c 2  -- sset Speaker Playback Volume %d%% >/dev/null 2>/dev/null", kcwlevel);
        system (cwlevelcommand);
        usleep(200000); 
        snprintf(cwlevelcommand, 127, "aplay -D plughw:CARD=Device,DEV=0 %s >/dev/null 2>/dev/null &", kcwfile);
        system (cwlevelcommand);
      }
      else                                     // HDMI and default
      {
        snprintf(cwlevelcommand, 127, "amixer -c 0  -- sset HDMI Playback Volume %d%% >/dev/null 2>/dev/null", kcwlevel);
        system (cwlevelcommand);
        usleep(200000); 
        snprintf(cwplaycommand, 127, "aplay -D plughw:CARD=b1,DEV=0 %s >/dev/null 2>/dev/null &", kcwfile);
        system (cwplaycommand);

        // Reduce playback volume for keep-alive noise
        snprintf(cwplaycommand, 127, "amixer -c 0  -- sset HDMI Playback Volume %d%% >/dev/null 2>/dev/null", audiokeepalivelevel);
        system (cwplaycommand);
      }
    }
  }

  // Now wait kmediaduration seconds
  media_start = monotonic_ms();
  //printf("Monotonic: %llu, K Media Start %llu, end at %llu\n", monotonic_ms(), media_start, media_start + kmediaduration * 1000);
  while (monotonic_ms() < media_start + kmediaduration * 1000)
  {
    usleep(10000);
    if ((inputactive[1] == 1) ||
        (inputactive[2] == 1) ||
        (inputactive[3] == 1) ||
        (inputactive[4] == 1) ||
        (inputactive[5] == 1) ||
        (inputactive[6] == 1) ||
        (inputactive[7] == 1))            // An Input is active
    {
      media_start = monotonic_ms() - kmediaduration * 1000;
      printf("Exiting K display duration as active input detected\n");
    }
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

  pastendoffirstcarousel = false;
  strcpy(StatusForConfigDisplay, "Displaying the Carousel");

  while ((run_carousel == true) && (output_overide == false))
  {
    for (i = 1; i <= carouselscenes; i++)
    {
      media_start = monotonic_ms();

      // Display the Carousel scene
      if ((StatusScreenOveride != true) && (ident_active != true) && (run_carousel == true))
      {
        if (strcmp(carouselmediatype[i], "jpg") == 0)          // Scene is an image
        {  
          pthread_mutex_lock(&fbi_lock);

          fbiThenKill(carouselfile[i]);

          // Release fbi
          pthread_mutex_unlock(&fbi_lock);
          printf("Carousel Scene %d displayed: Image\n", i);
        }

        if (strcmp(carouselmediatype[i], "source") == 0)       // Scene is a source
        {
          carouselSource = atoi(carouselfile[i]);
          if ((carouselSource < 1) || (carouselSource > availableinputs))
          {
            carouselSource = 0;
          }
          if (output_overide == false)
          {
            Select_HDMI_Switch(carouselSource);
            inputAfterIdent = carouselSource;
            printf("Carousel Scene %d displayed: Source %d\n", i, carouselSource);
          }
        }

        if ((strcmp(carouselmediatype[i], "status") == 0) && (output_overide == false))      // Scene is the Status page
        {
          carouselSource = 0;
          // Status page
          update_status_screen();

          // Select_HDMI_Switch(0);
          inputAfterIdent = 0;
          printf("Carousel Scene %d displayed: Status Page\n", i);
        }
      }

      // Now wait (carouselmediaduration[i] * 1000) seconds

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

      if ((strcmp(carouselmediatype[i], "source") == 0) && (output_overide == false))      // Scene was a source so reset switch
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

    // Drop the PTT if required at the end of the first carousel
    if (pastendoffirstcarousel == false)
    {
      PTTEvent(3);
      pastendoffirstcarousel = true;
    }
  }
  carousel_active = false;
  strcpy(StatusForConfigDisplay, "Exiting the Carousel");
  printf("Exiting the KCarousel thread\n");

  return NULL;
}

void Seti2cAudioSwitch(int bitv[8])
{
  int i2cdevnumber = 22;
  int hexvalue = 0;
  char hexstring[31];
  char i2cstring[127];
  int i;

  hexvalue = (bitv[0]) + (2 * bitv[1]) + (4 * bitv[2]) + (8 * bitv[3]) + (16 * bitv[4]) + (32 * bitv[5]) + (64 * bitv[6]) + (128 * bitv[7]);
  if (hexvalue <= 15)
  {
    snprintf(hexstring, 30, "0x0%x", hexvalue);
  }
  else
  {
    snprintf(hexstring, 30, "0x%x", hexvalue);
  }

  // Set i2c Switch to all outputs
  snprintf(i2cstring, 120, "i2cset -y %d 0x2%d 0x00 0x00", i2cdevnumber, audioi2caddress);
   printf("%s\n", i2cstring);
  system(i2cstring);

  // Set the output levels
  snprintf(i2cstring, 120, "i2cset -y %d 0x2%d 0x0A %s", i2cdevnumber, audioi2caddress, hexstring);
   printf("%s\n", i2cstring);
  system(i2cstring);

  // Store the current status
  for (i = 0; i <= 7; i++)
  {
    currenti2caudiostatus[i] = bitv[i];
  }
}


void sendIRcode(char *inputCode)
{
  char IRCommandStub[63];
  char SystemCommand[255];

  if (inputCode[0] == '2')      // Daisy-chained switches
  {
    // select the appropriate input on the upstream (first, quad) switch
    // so switch to the upstream IR sender
    gpio_write(localGPIO, daisychainirselectgpio, 1);
    usleep(100000);     // Let switch settle

    snprintf(IRCommandStub, 30, "%s", inputCode + 1);     // start at the 2nd character
    printf("Upstream (quad) IRCommandStub = -%s-\n", IRCommandStub);
    snprintf(SystemCommand, 126, "ir-ctl -S %s -d /dev/lirc0", IRCommandStub);
    system(SystemCommand);

    // now switch to the downstream (second, not quad) switch
    gpio_write(localGPIO, daisychainirselectgpio, 0);
    usleep(100000);     // Let switch settle

    // Select daisy chain input on downstream switch
    printf("Downstream IRCommandStub = -%s-\n", output2ndhdmicode);
    snprintf(SystemCommand, 126, "ir-ctl -S %s -d /dev/lirc0", output2ndhdmicode);
    system(SystemCommand);
  }
  else                                  // Not daisy-chained switches
  {
    snprintf(SystemCommand, 126, "ir-ctl -S %s -d /dev/lirc0", inputCode);
    printf("Downstream or only IRCommandStub = -%s-\n", inputCode);
    system(SystemCommand);
  }
}


void Select_HDMI_Switch(int selection)        // selection is between -1 (quad), 0 and availableinputs
{
  int i;
  int thisGPIOlevel;
  char SystemCommand[255];
//  char IRCommandStub[63];
  int bitv[8];
  bool commas = false;
  char IRcode[63];

  if ((selection < -1)  || (selection > availableinputs))
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

  if (strcmp(outputswitchcontrol, "ir") == 0)             // IR controlled HDMI switch
  {
    // First look to see if multiple key-presses (separated by commas) might be required
    for (i = 1; i < strlen(outputcode[selection]); i++)
    {
      if (outputcode[selection][i] == ',')
      {
        commas = true;
      }
    }

    if (commas == false)
    {
      if ((selection >= 0)  && (selection <= availableinputs))  // Normal input (not quad)
      {
        sendIRcode(outputcode[selection]);
      }
      else if (selection == -1)                          // Quad View requested
      {
        sendIRcode(outputhdmiquadcode);
      }
    }
    else // Handle Command string that includes commas
    {
      if ((selection >= 0)  && (selection <= availableinputs))  // Normal input (not quad)
      {
        printf("IR Command with commas:-%s-\n", outputcode[selection]);

        // Parse the IR Command
        i = 0;  // character of input string

        strncpy(IRcode, "", sizeof(IRcode));

        while (i < strlen(outputcode[selection]))
        {
          if (outputcode[selection][i] != ',')   // not a comma
          {
            IRcode[strlen(IRcode)] = outputcode[selection][i];
            IRcode[strlen(IRcode) + 1] = '\0';

            // send the code if we've reached the end of the string
            if (i == strlen(outputcode[selection]) - 1)
            {
              printf("Sending code at end %s\n", IRcode);
              sendIRcode(IRcode);
            }
          }
          else  // it's a comma
          {
          if (strlen(IRcode) > 2)
            {
              // send the IR code
              printf("Sending code %s\n", IRcode);
              sendIRcode(IRcode);

              // null the code for next time
              strncpy(IRcode, "", sizeof(IRcode));
            } 
            printf("Pausing 200 ms\n");
            usleep (200000);
          }
          i = i + 1;
        }
      }
      else if (selection == -1)                          // Quad View requested
      {
        printf("IR Command with commas:-%s-\n", outputhdmiquadcode);

        // Parse the IR Command
        i = 0;  // character of input string

        strncpy(IRcode, "", sizeof(IRcode));

        while (i < strlen(outputhdmiquadcode))
        {
          if (outputhdmiquadcode[i] != ',')   // not a comma
          {
            IRcode[strlen(IRcode)] = outputhdmiquadcode[i];
            IRcode[strlen(IRcode) + 1] = '\0';

            // send the code if we've reached the end of the string
            if (i == strlen(outputhdmiquadcode) - 1)
            {
              printf("Sending code at end %s\n", IRcode);
              sendIRcode(IRcode);
            }
          }
          else  // it's a comma
          {
          if (strlen(IRcode) > 2)
            {
              // send the IR code
              printf("Sending code %s\n", IRcode);
              sendIRcode(IRcode);

              // null the code for next time
              strncpy(IRcode, "", sizeof(IRcode));
            } 
            printf("Pausing 200 ms\n");
            usleep (200000);
          }
          i = i + 1;
        }
      }
    }
  }

  if (strcmp(outputswitchcontrol, "html") == 0)                 // Network controlled HDMI switch
  {
    if ((selection >= 0)  && (selection <= availableinputs))    // Normal input
    {
      snprintf(SystemCommand, 254, "curl %s%s &", networkctrlurl, outputnetcommand[selection]);
      printf("Network Command: %s\n", SystemCommand);
      system(SystemCommand);
    }
    else                                                         // Quad View requested
    {
      snprintf(SystemCommand, 254, "curl %s%s &", networkctrlurl, outputquadnetcommand);
      printf("Network Command: %s\n", SystemCommand);
      system(SystemCommand);
    }
  }

  if (strcmp(outputswitchcontrol, "atem") == 0)                 // Network controlled ATEM
  {
    if ((selection >= 0)  && (selection <= availableinputs))    // Normal input
    {
      snprintf(SystemCommand, 254, "/home/pi/atv-rptr/scripts/atem_switch.sh %s %s %s", atemip, atemoutput, outputatem[selection]);
      printf("Network Command: %s\n", SystemCommand);
      system(SystemCommand);
      snprintf(SystemCommand, 254, "/home/pi/atv-rptr/scripts/atem_switch.sh %s %s %s", atemip, atemprogram, outputatem[selection]);
      printf("Network Command: %s\n", SystemCommand);
      system(SystemCommand);
    }
    else                                                         // Quad View requested - use MultiView set preview to 1, and program to 2
    {
      snprintf(SystemCommand, 254, "/home/pi/atv-rptr/scripts/atem_switch.sh %s %s %s", atemip, atempreview, outputatem[1]);
      printf("Network Command: %s\n", SystemCommand);
      system(SystemCommand);
      snprintf(SystemCommand, 254, "/home/pi/atv-rptr/scripts/atem_switch.sh %s %s %s", atemip, atemprogram, outputatem[2]);
      printf("Network Command: %s\n", SystemCommand);
      system(SystemCommand);
      snprintf(SystemCommand, 254, "/home/pi/atv-rptr/scripts/atem_switch.sh %s %s %s", atemip, atemoutput, "10");
      printf("Network Command: %s\n", SystemCommand);
      system(SystemCommand);
      snprintf(SystemCommand, 254, "/home/pi/atv-rptr/scripts/atem_switch.sh %s %s %s", atemip, atemprogram, "10");
      printf("Network Command: %s\n", SystemCommand);
      system(SystemCommand);
    }
  }

  // Audio switching code
  if (strcmp(audioswitch, "i2c") == 0)
  {
    bitv[0] = 0;
    bitv[1] = 0;
    bitv[2] = 0;
    bitv[3] = 0;
    bitv[4] = 0;
    bitv[5] = 0;
    bitv[6] = 0;
    bitv[7] = 0;

    if (talkbackaudio == true)
    {
      bitv[talkbackaudioi2cbit] = 1;
    }

    if ((kcwaudio == true) || (identcwaudio == true))  // Controller audio always on?
    {
      bitv[outputaudioi2cbit[0]] = 1;
    }

    if (selection == -1)                          // Quad View requested
    {
      bitv[outputaudioi2cbit[1]] = inputactive[1];  // Only select active audios
      bitv[outputaudioi2cbit[2]] = inputactive[2];
      bitv[outputaudioi2cbit[3]] = inputactive[3];
      bitv[outputaudioi2cbit[4]] = inputactive[4];
    }

    if ((selection >= 0)  && (selection <= availableinputs))
    {
      bitv[outputaudioi2cbit[selection]] = 1;
    }

    Seti2cAudioSwitch(bitv);
  }
}


int Switchto(int new_output)
{
  uint64_t announce_start;
  int i;
  char bleepcommand[255];

  // kill VLC
  // fbi cue image
  printf("Entered Switchto for output %d\n", new_output);

  if ((new_output >= 1) && (new_output <= 7) && (announcemediaduration[new_output] > 0) 
   && (outputwasmultiinputquad == false))    // Announce Media required
  {
    // Wait for fbi to be available
    pthread_mutex_lock(&fbi_lock);

    fbiThenKill(announcemediafile[new_output]);

    // Release fbi
    pthread_mutex_unlock(&fbi_lock);

    Select_HDMI_Switch(0);  // Switch to controller image

    announce_start = monotonic_ms();

    // Set PTT on if appropriate
    PTTEvent(4);

    // Play Announce Bleep
    if (announcebleep == true)
    {
      if (audiooutcard == 1)                    // RPi Audio Jack
      {
        snprintf(bleepcommand, 127, "amixer -c 1  -- sset Headphone Playback Volume %d%% >/dev/null 2>/dev/null", announcebleeplevel);
        system (bleepcommand);
        usleep(200000); 
        snprintf(bleepcommand, 127, "aplay -D plughw:CARD=Headphones,DEV=0 /home/pi/tmp/announce.wav >/dev/null 2>/dev/null &");
        system (bleepcommand);
      }
      else if (audiooutcard == 2)               // USB Audio Dongle
      {
        snprintf(bleepcommand, 127, "amixer -c 2  -- sset Speaker Playback Volume %d%% >/dev/null 2>/dev/null", announcebleeplevel);
        system (bleepcommand);
        usleep(200000); 
        snprintf(bleepcommand, 127, "aplay -D plughw:CARD=Device,DEV=0 /home/pi/tmp/announce.wav >/dev/null 2>/dev/null &");
        system (bleepcommand);
      }
      else                                     // HDMI and default
      {
        snprintf(bleepcommand, 127, "amixer -c 0  -- sset HDMI Playback Volume %d%% >/dev/null 2>/dev/null", announcebleeplevel);
        system (bleepcommand);
        usleep(200000); 
        snprintf(bleepcommand, 127, "aplay -D plughw:CARD=b1,DEV=0 /home/pi/tmp/announce.wav >/dev/null 2>/dev/null &");
        system (bleepcommand);

        // Reduce playback volume for keep-alive noise
        snprintf(bleepcommand, 127, "amixer -c 0  -- sset HDMI Playback Volume %d%% >/dev/null 2>/dev/null", audiokeepalivelevel);
        system (bleepcommand);
      }
    }

    while (monotonic_ms() <= announce_start + announcemediaduration[new_output] * 1000)
    {
      usleep(10000);
      for (i = 1; i <= 7; i++)
      {
        if (inputActiveInitialState[i] != inputactive[i])
        {
          return(1);
        }
      }
    }
  }

  if ((new_output == -1) && (showquadformultipleinputs == true))  // Set flag so that announce is not displayed on dropping out of quad
  {
    outputwasmultiinputquad = true;
  }
  else
  {
    outputwasmultiinputquad = false;
  }

  Select_HDMI_Switch(new_output);  // switch to new_output if no change in input switch lines

  // Wait for fbi to be available
  pthread_mutex_lock(&fbi_lock);

  fbiThenKill(kmediafile);

  // Release fbi
  pthread_mutex_unlock(&fbi_lock);

  // Final check for an input change
  for (i = 1; i <= availableinputs; i++)
  {
    if (inputActiveInitialState[i] != inputactive[i])
    {
      return(1);
    }
  }
  return(0);
}

void DisplayK()
{
  printf("Entered DisplayK\n");
  strcpy(StatusForConfigDisplay, "Displaying the K");

  printf("Creating the Carousel Thread\n");
  pthread_create (&thkcarousel, NULL, &Show_K_Carousel, NULL);
}

int priorityDecision()
{
  int priority_test;
  int i;
  int decision_result = -2;  // -2 is no active inputs, -1 is quad, 0 is controller, 1 is first rptr input etc
  static int previous_decision_result;
  int active_input_count = 0;
  int quadtestinputs = 4;

  printf("previous decision result is %d, entering decision process\n", previous_decision_result);

  // Record initial state to monitor for changes
  printf("Setting inputActiveInitialState ");
  for (i = 1; i <= availableinputs; i++)
  {
    inputActiveInitialState[i] = inputactive[i];
    printf("%d: %d, ", i, inputActiveInitialState[i]);
  }
  printf("\n");

  // Check if quad is required for multiple active inputs
  if (showquadformultipleinputs == true)
  {
    if (availableinputs < quadtestinputs)  // so less than 4 inputs into the quad
    {
      quadtestinputs  = availableinputs;
    }

    for (i = 1; i <= quadtestinputs; i++)  
    {
      if ((inputprioritylevel[i] <= 8) && (inputactive[i] == 1))
      {
        active_input_count++;
      }
    }
  }

  if (active_input_count > 1)                            // Quad to be displayed
  {
    printf("Selecting Quad for Multiple Inputs\n");
    decision_result = -1;
  }
  else                                                   // Not the quad                                   
  {
    for (priority_test = 1; priority_test <= 8; priority_test++)
    {
      for (i = 1; i <= availableinputs; i++)
      {
        if ((inputprioritylevel[i] == priority_test) && (decision_result == -2) && (inputactive[i] == 1))  // so take first result
        {
          if (priority_test == 1)            // Always switch to the lowest numbered active priority 1 input
          {
            printf("Priority 1: ----- priority_test = %d,i = %d, inputactive[i] = %d\n", priority_test, i, inputactive[i]);
            decision_result = i;
          }
          else                               // priority 2 - 8
          {
            printf("Priority %d: ----- priority_test = %d,i = %d, inputactive[i] = %d\n", i, priority_test, i, inputactive[i]);
            if ((activeinputhold == true) && (inputactive[previous_decision_result] == 1) && (previous_decision_result != 0))  // Use previous selection
            {
              decision_result = previous_decision_result;
              printf("USING PREVIOUS RESULT\n");
            }
            else
            {
              decision_result = i;
              printf("USING HIGHEST PRIORITY RESULT\n");
            }
          }
        }
      }
    }
  }
  if (decision_result >= -1 )
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
  uint64_t output_overide_timer;
  bool previous_status_screenoveride = false;
  int inputChangeDuringAnnounce = 0;

  // Turn on the PTT if required
  PTTEvent(1);

  // Show the status screen if required
  strcpy(StatusForConfigDisplay, "Started Repeater Engine");
  if (StatusScreenOveride == true)
  {
    update_status_screen();
  }

  while (run_repeater == true)
  {
    if (output_overide == true)
    {
      strcpy(StatusForConfigDisplay, "Output Override Mode selected");

      // Set a timer and select the input on first entry;
      if (in_output_overide_mode == false)
      {
        output_overide_timer = monotonic_ms() + 1000 * dtmfactiontimeout;
        in_output_overide_mode = true;
        Select_HDMI_Switch(output_overide_source);    // Display Controller
        inputselected = output_overide_source;        // 
        inputAfterIdent = output_overide_source;      // global for return from ident
      }

      // Now check for exit conditions
      if ((dtmfactiontimeout != 0) && (output_overide_timer <= monotonic_ms()))
      {
        strcpy(StatusForConfigDisplay, "Output Override Mode deselected");

        output_overide = false;
        in_output_overide_mode = false;

        Select_HDMI_Switch(0);    // Display Controller
        inputselected = 0;        // 
        inputAfterIdent = 0;      // global for return from ident
      }
    }
    else if (inputStatusChange == true)
    {
      strcpy(StatusForConfigDisplay, "Input Status Change Detected");

      // Normal operation
      current_output = inputselected;
      new_output = priorityDecision();

      printf("Input status change.  Current Output = %d, New Output = %d\n", current_output, new_output);

      if ((new_output != current_output) || (new_output == -1) ||
         ((previous_status_screenoveride = true) && (StatusScreenOveride == false)))
         // only change if there is a change or quad is selected
      {
        // Reset the exit from status screen trigger
        if ((previous_status_screenoveride = true) && (StatusScreenOveride == false))
        {
          previous_status_screenoveride = false;
        }

        if (new_output == -2)  // No Active Inputs
        {
          strcpy(StatusForConfigDisplay, "No Active Inputs");

          Select_HDMI_Switch(0);  // Display Controller
          inputselected = 0;      // Is this still required??
          inputAfterIdent = 0;      // global for return from ident
          if ((ident_active == false) && (StatusScreenOveride == false))
          {
            if ((inputactive[1] == 1) ||
                (inputactive[2] == 1) ||
                (inputactive[3] == 1) ||
                (inputactive[4] == 1) ||
                (inputactive[5] == 1) ||
                (inputactive[6] == 1) ||
                (inputactive[7] == 1))
            {
              printf("Although signal had dropped, exiting before K as new signal is up\n");;
              inputStatusChange = true;
            }
            else
            {
              DisplayK();  // and then go to carousel
            }
          }
        }
        else                   // An input is active
        {
          if (StatusScreenOveride == false)  // Not on status screen
          {
            strcpy(StatusForConfigDisplay, "Announcing New Input");

            inputChangeDuringAnnounce = Switchto(new_output);   // So announce the new input and switch to it, then show K on controller
            printf("inputChangeDuringAnnounce = %d \n", inputChangeDuringAnnounce);
          }
          if (inputChangeDuringAnnounce == 0)  // Input stayed on until after input was selected
          {
            strcpy(StatusForConfigDisplay, "Switching to New Input");
            inputselected = new_output; // Is this still required?
            inputAfterIdent = new_output; // global for return from ident
          }
        }
      }

      if (inputChangeDuringAnnounce != 0)  // So something happened during the input announcement
      {
        strcpy(StatusForConfigDisplay, "New Input dropped during Announce");

        inputStatusChange = true;
        current_output = 0;
        new_output = -2;
        inputChangeDuringAnnounce = 0;
      }
      else
      {
        inputStatusChange = false;
      }

      if (StatusScreenOveride == true)
      {
        update_status_screen();
        Select_HDMI_Switch(0);
      }
    }
    else                 // Error check
    {
      if(((inputactive[1] == 1) ||
          (inputactive[2] == 1) ||
          (inputactive[3] == 1) ||
          (inputactive[4] == 1) ||
          (inputactive[5] == 1) ||
          (inputactive[6] == 1) ||
          (inputactive[7] == 1)) &&
          (inputAfterIdent == 0))    // An Input is active, but input 0 is selected
      {
        printf("Caught condition of active input, but input 0 selected.  Correcting.\n");
        inputStatusChange = true;
      }
    }
    usleep (10000); // 10ms loop
  }
}


static void
terminate(int dummy)
{
  run_repeater = false;
  run_carousel = false;

  // Deselect PTT and close GPIO
  PTTEvent(0);
  usleep(100000);
  pigpio_stop(localGPIO);

  // Display caption
  fbiThenKill("/home/pi/tmp/stopped.png");
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
  char SystemCommand[127];

  // Catch sigaction and call terminate
  for (i = 0; i < 16; i++)
  {
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = terminate;
    sigaction(i, &sa, NULL);
  }

  printf("BATC Repeater Controller Starting Up\n");
  printf("Reading the Config File\n");

  CheckConfigFile();

  read_config_file();

  printf("BATC Repeater Controller for %s\n", callsign);
  printf("Initialising GPIO\n");

  // Initialise GPIO
  localGPIO = pigpio_start(0, 0); // Connect to local Pi.

  if (localGPIO < 0)
  {
    printf("Can't connect to pigpio daemon\n");
    exit(1);
  }

  setUpGPIO();
  printf("Initialising the Status Screen\n");

  // Check the ident length
  identcwduration = IdentLength();

  initScreen();

  // Start the ident timer in a thread if required
  if (identinterval > 1)
  {
    printf("Creating ident timer thread\n");
    pthread_create (&thidenttimer, NULL, &Show_Ident, NULL);
  }
  else
  {
    printf("Ident disabled in config file\n");
  }

  // Monitor the input status lines in a thread
  printf("Creating input status monitor thread\n");
  pthread_create (&thinputactivemonitor, NULL, &InputStatusListener, NULL);

  // Monitor the control socket in a thread
  printf("Creating udp socket monitor thread\n");
  pthread_create (&thsocketmonitor, NULL, &SocketListener, NULL);

  // If IR controlled, reset the HDMI Switch
  if (strcmp(outputswitchcontrol, "ir") == 0)             // ir controlled HDMI switch
  {
    snprintf(SystemCommand, 126, "ir-ctl -S %s -d /dev/lirc0", outputhdmiresetcode);
    system(SystemCommand);
  }

  printf("Starting the main repeater controller\n");
  repeaterEngine();         // This keeps the repeater running

  // Flow does not get to here

  return 0;
}
