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

/***************************************************************************//**
 * @brief Checks if a file exists
 *
 * @param nil
 *
 * @return 0 if exists, 1 if not
*******************************************************************************/

int file_exist (char *filename)
{
  if (access(filename, R_OK) == 0) 
  {
    // file exists
    return 0;
  }
  else
  {
    // file doesn't exist
    return 1;
  }
}



/***************************************************************************//**
 * @brief Looks up the CPU Temp
 *
 * @param CPUTemp (str) CPU Temp to be passed as a string
 *
 * @return void
*******************************************************************************/

float GetCPUTemp()
{
  FILE *fp;
  char CPUTemp[127];
  float TempmC = -10.0;

  /* Open the command for reading. */
  fp = popen("cat /sys/class/thermal/thermal_zone0/temp", "r");
  if (fp == NULL) {
    printf("Failed to run command\n" );
    exit(1);
  }

  /* Read the output a line at a time - output it. */
  while (fgets(CPUTemp, 20, fp) != NULL)
  {
    TempmC = atof(CPUTemp);
    //printf("%s", CPUTemp);
  }

  pclose(fp);

  if ((TempmC > 0) && (TempmC < 100000))
  {
    return (TempmC / 1000.0);
  }
  return 0.0;  
}

/***************************************************************************//**
 * @brief Looks up the current installed Software Version
 *
 * @param nil
 *
 * @return Software Version as an Integer
*******************************************************************************/

int GetSWVers()
{
  FILE *fp;
  char SVersion[255];
  int Version = 0;

  /* Open the command for reading. */
  fp = popen("cat /home/pi/atv-rptr/config/installed_version.txt", "r");
  if (fp == NULL) {
    printf("Failed to run command\n" );
    exit(1);
  }

  /* Read the output a line at a time - output it. */
  while (fgets(SVersion, 16, fp) != NULL)
  {
    //printf("%s", SVersion);
    Version = atoi(SVersion);
  }

  /* close */
  pclose(fp);
  return Version;
}

/***************************************************************************//**
 * @brief Looks up the Broadcom number for a physical GPIO pin
 *
 * @param GPIO physical pin number
 *
 * @return Broadcom software GPIO number
*******************************************************************************/

int PinToBroadcom(int Pin)
{
  int Broadcom = 0;  // Illegal (reserved) default

  switch(Pin)
  {
    case 3:
      Broadcom = 2;
      break;
    case 5:
      Broadcom = 3;
      break;
    case 7:
      Broadcom = 4;
      break;
    case 8:
      Broadcom = 14;
      break;
    case 10:
      Broadcom = 15;
      break;
    case 11:
      Broadcom = 17;
      break;
    case 12:
      Broadcom = 18;
      break;
    case 13:
      Broadcom = 27;
      break;
    case 15:
      Broadcom = 22;
      break;
    case 16:
      Broadcom = 23;
      break;
    case 18:
      Broadcom = 24;
      break;
    case 19:
      Broadcom = 10;
      break;
    case 21:
      Broadcom = 9;
      break;
    case 22:
      Broadcom = 25;
      break;
    case 23:
      Broadcom = 11;
      break;
    case 24:
      Broadcom = 8;
      break;
    case 26:
      Broadcom = 7;
      break;
    case 27:
      Broadcom = 0; // Reserved
      break;
    case 28:
      Broadcom = 1; // Reserved
      break;
    case 29:
      Broadcom = 5;
      break;
    case 31:
      Broadcom = 6;
      break;
    case 32:
      Broadcom = 12;
      break;
    case 33:
      Broadcom = 13;
      break;
    case 35:
      Broadcom = 19;
      break;
    case 36:
      Broadcom = 16;
      break;
    case 37:
      Broadcom = 26;
      break;
    case 38:
      Broadcom = 20;
      break;
    case 40:
      Broadcom = 21;
      break;
    default:
      Broadcom = 0; // Reserved
      break;
  }
  return Broadcom;
}

