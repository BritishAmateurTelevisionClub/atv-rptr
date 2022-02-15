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

#include<arpa/inet.h>
#include<sys/socket.h>

#include "listeners.h"
#include "look-ups.h"

#define PATH_CONFIG "/home/pi/atv-rptr/config/repeater_config.txt"

int PTTEvent(int);


void *InputStatusListener(void * arg)
{
  int i;
  int inputValues1[8];
  int inputValues2[8];
  int lastStableValue[8];
  bool change1Detected = false;
  bool validChangeDetected = false;
  bool sdbutton1stpress = false;
  uint64_t first_press_time;

  while (run_repeater == true)
  {
    change1Detected = false;

    for (i = 1; i <= availableinputs; i++)
    {
      inputValues1[i] = gpio_read(localGPIO, inputactiveGPIO[i]);
      if (inputValues1[i] != lastStableValue[i])
      {
        change1Detected = true;
      }
    }

    validChangeDetected = true;

    if (change1Detected == true)
    {
      if (inputStatusChange == false)
      {
        printf("Switch change detected, checking for debounce\n");
      }
      else
      {
        printf("Status change already true, Switch change detected, checking for debounce\n");
      }

      usleep(100000);            // Wait 100ms for switch bounce
      for (i = 1; i <= availableinputs; i++)
      {
        inputValues2[i] = gpio_read(localGPIO, inputactiveGPIO[i]);
        if (inputValues2[i] != inputValues1[i])  // So not a valid change
        {
          validChangeDetected = false;
        }
      }
    }

    if ((change1Detected == true) && (validChangeDetected == true) && (beaconmode == false))
    {
      printf("Input Status - ");
      for (i = 1; i <= availableinputs; i++)
      {
        if ((lastStableValue[i] != inputValues2[i]) && (inputprioritylevel[i] < 9))  // Input not disabled
        {
          inputStatusChange = true;
        }

        lastStableValue[i] = inputValues2[i];
        inputactive[i] = inputValues2[i];
        printf("%d: %d, ", i, inputactive[i]);
      }
      printf("\n");
    }

    // Check the front panel shutdown button
    if (fpsdenabled == true)
    {
      if (gpio_read(localGPIO, fpsdGPIO) == 0)
      {
        if (sdbutton1stpress == false)     // Put up status screen
        {
          inputStatusChange = true;
          StatusScreenOveride = true;
          output_overide = false;
          in_output_overide_mode = false;
          strcpy(StatusForConfigDisplay, "Shutdown Button pressed");
          first_press_time = monotonic_ms();
          sdbutton1stpress = true;
        }
      }
      if (sdbutton1stpress == true)     // Status screen displayed awaiting shutdown
      {
        if (first_press_time + 3000 < monotonic_ms())  // 3 seconds elapsed
        {
          if (gpio_read(localGPIO, fpsdGPIO) == 0)     // button still low
          {
            system("sudo shutdown now");
          }
          else                                         // button high so go back to normal
          {
            inputStatusChange = true;
            StatusScreenOveride = false;
            output_overide = false;
            in_output_overide_mode = false;
            sdbutton1stpress = false;
          }
        }
      }
    }

    usleep(100000);  // Check buttons at 10 Hz
  }
  return NULL;
}


/*
	Simple udp server
*/
#define BUFLEN 512	//Max length of buffer
#define PORT 8888	//The port on which to listen for incoming data

void die(char *s)
{
	perror(s);
	exit(1);
}


void *SocketListener(void * arg)
{
	struct sockaddr_in si_me, si_other;
	
	int s;
    //int i;
    socklen_t  slen;
    //int slen;
    int recv_len;
	char buf[BUFLEN];

    slen = sizeof(si_other);
	
    printf("Creating socket\n");
	//create a UDP socket
	if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
	{
		die("socket");
	}
	
	// zero out the structure
	memset((char *) &si_me, 0, sizeof(si_me));
	
	si_me.sin_family = AF_INET;
	si_me.sin_port = htons(PORT);
	si_me.sin_addr.s_addr = htonl(INADDR_ANY);

    printf("Binding socket\n");
	
	//bind socket to port
	if( bind(s , (struct sockaddr*)&si_me, sizeof(si_me) ) == -1)
	{
		die("bind");
	}
    printf("Listening -----------------\n");
	
	//keep listening for data
	while(1)
	{
		printf("Waiting for data...\n");
		fflush(stdout);
		
		//try to receive some data, this is a blocking call
		if ((recv_len = recvfrom(s, buf, BUFLEN, 0, (struct sockaddr *) &si_other, &slen)) == -1)
		{
			die("recvfrom()");
		}
		
		//print details of the client/peer and the data received
		printf("Received packet from %s:%d\n", inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port));
		printf("Data: %s\n" , buf);

        // Strip trailing cr
        if (strlen(buf) > 1)
        {
          buf[strlen(buf) - 1] = '\0';  // Strip trailing cr
        }

        // Check for valid command
        if ((strcmp(buf, "00") == 0) || ((atoi(buf) >= 1) && (atoi(buf) <= 99))
         || ((atoi(buf) >= 90000) && (atoi(buf) <= 99990)))
        {
          UDP_Command(atoi(buf));
        }

		//now reply the client with the same data (Not used)
		//if (sendto(s, buf, recv_len, 0, (struct sockaddr*) &si_other, slen) == -1)
		//{
		//	die("sendto()");
		//}
	}

	close(s);
	return NULL;
}

void UDP_Command(int command_code)
{
  int i;

  if (command_code == atoi(dtmfresetcode))
  {
    inputStatusChange = true;
    StatusScreenOveride = false;
    output_overide = false;
    in_output_overide_mode = false;
    printf("Reset Code received\n");
    return;
  }

  if (command_code == atoi(dtmfstatusviewcode))
  {
    inputStatusChange = true;
    StatusScreenOveride = true;
    output_overide = false;
    in_output_overide_mode = false;
    printf("Status Code received\n");
    return;
  }

  if (command_code == atoi(dtmfquadviewcode))
  {
    in_output_overide_mode = false;  // This will reset the timer
    inputStatusChange = true;
    StatusScreenOveride = false;
    output_overide = true;
    output_overide_source = -1;  // Quad View code
    return;
  }

  if (command_code == atoi(dtmftalkbackaudioenablecode))
  {
    inputStatusChange = true;
    talkbackaudio = true;
    return;
  }

  if (command_code == atoi(dtmftalkbackaudiodisablecode))
  {
    inputStatusChange = true;
    talkbackaudio = false;
    return;
  }

  for (i = 0; i <= 7; i++)
  {
    if (command_code == dtmfselectinput[i])
    {
      in_output_overide_mode = false;  // This will reset the timer
      inputStatusChange = true;
      StatusScreenOveride = false;
      output_overide = true;
      output_overide_source = i;
      printf("Overide Code %d received\n", command_code);
      return;
    }
  }

  if (dtmfoutputs > 0)
  {
    for (i = 1; i <= dtmfoutputs; i++)
    {
      if (command_code == atoi(dtmfgpiooutoncode[i]))
      {
        gpio_write(localGPIO, dtmfoutputGPIO[i], 1);
      }
      if (command_code == atoi(dtmfgpiooutoffcode[i]))
      {
        gpio_write(localGPIO, dtmfoutputGPIO[i], 0);
      }
    }
  }

  if (command_code == atoi(dtmfkeepertxoffcode))
  {
    transmitenabled = false;
    PTTEvent(0);
    SetConfigParam(PATH_CONFIG, "transmitenabled", "no");
    return;
  }

  if (command_code == atoi(dtmfkeepertxoncode))
  {
    transmitenabled = true;
    PTTEvent(2);
    SetConfigParam(PATH_CONFIG, "transmitenabled", "yes");
    return;
  }

  if (command_code == atoi(dtmfkeeperrebootcode))
  {
    system("sudo reboot now");
    return;
  }
}

//pi@raspberrypi:~ $ ncat -v 127.0.0.1 8888 -u
//Ncat: Version 7.70 ( https://nmap.org/ncat )
//Ncat: Connected to 127.0.0.1:8888.
//hello


int PTTEvent(int EventType)
{
  // PTT Event Type List
  // 0 Unconditional off
  // 1 Unconditional on
  // 2 Initial Start-up
  // 3 End of first run of carousel
  // 4 Start of Input Announce
  // 5 Start of Ident
  // 6 End of Ident
  // 7 Check for quiet hours and second half hour (called once per second)

  bool quiet_hours = false;
  bool secondhalfhour = false;
  int utc24time = 0;
  static bool previous_quiet_hours;
  static bool previous_secondhalfhour;
  time_t t; 
  struct tm tm;

  t = time(NULL);
  tm = *gmtime(&t);
  utc24time = tm.tm_hour * 100 + tm.tm_min;

  //printf("24 hour time = %d\n", utc24time);

  if (EventType == 0)
  {
    gpio_write(localGPIO, pttGPIO, 0);
    return(0);
  }
  if (EventType == 1)
  {
    gpio_write(localGPIO, pttGPIO, 1);
    return(1);
  }

  // Now work out if we are in quiet hours
  if (hour24operation == true)
  {
    quiet_hours = false;
  }
  else
  {
    if (operatingtimestart < operatingtimefinish)  // operating hours does not cross midnight UTC
    {
      if ((utc24time < operatingtimestart) || (utc24time > operatingtimefinish))  // in quiet hours
      {
        quiet_hours = true;
      }
      else
      {
        quiet_hours = false;
      }
    }
    else                                          // operating hours start before midnight and end after
    {
      if ((utc24time < operatingtimestart) && (utc24time > operatingtimefinish))  // in quiet hours
      {
        quiet_hours = true;
      }
      else
      {
        quiet_hours = false;
      }
    }  
  }

  // Now work out if we are in the second half hour with Power Save
  if ((halfhourpowersave == true) && (quiet_hours == false) && (tm.tm_min > 29) && (tm.tm_min < 60))
  {
    secondhalfhour = true;
  }

  if (transmitenabled == true)
  {
    switch(EventType)
    {
      case 2:                               // Initial Start-up
        if (quiet_hours == true)            // Don't transmit on start-up in quiet hours
        {
          gpio_write(localGPIO, pttGPIO, 0);
          return(0);
        }
        else      // transmit, even if only until the end of the first sequence
        {
          gpio_write(localGPIO, pttGPIO, 1);
          return(1);
        }
        break;
      case 3:                                 // End of first run of carousel
                                              // So drop carrier if in quiet hours
                                              // or transmitwhennotinuse = false
                                              // or secondhalfhour = true
         if ((quiet_hours == true)
          || (transmitwhennotinuse == false) || (secondhalfhour == true))
         {
           gpio_write(localGPIO, pttGPIO, 0);
           return(0);
         }
         break;
      case 4:                                  // Start of Input Announce
                                               // So raise carrier if 
                                               // secondhalfhour = true or
                                               // (transmitwhennotinuse = false or
                                               // repeatduringquiethours = true and quiet_hours == true)
        if ((secondhalfhour == true) || (transmitwhennotinuse == false)
         || ((quiet_hours == true) && (repeatduringquiethours == true)))
        {
          gpio_write(localGPIO, pttGPIO, 1);
          return(1);
        }
        break;
      case 5:                                  // Start of Ident
                                               // So raise carrier if 
                                               // secondhalfhour = true or
                                               // transmitwhennotinuse = false or
                                               // identduringquiethours = true and quiet_hours == true
        if ((secondhalfhour == true) || (transmitwhennotinuse == false)
         || ((quiet_hours == true) && (identduringquiethours == true)))
        {
          gpio_write(localGPIO, pttGPIO, 1);
          return(1);
        }
        break;
      case 6:                                  // End of Ident
                                               // So drop carrier if 
                                               // secondhalfhour = true and (inputAfterIdent == 0)
                                               // or
                                               // quiet_hours == true and (inputAfterIdent == 0)
                                               // or
                                               // transmitwhennotinuse = false and (inputAfterIdent == 0)
        if (((secondhalfhour == true) && (inputAfterIdent == 0))
         || ((quiet_hours == true) && (inputAfterIdent == 0))
         || ((transmitwhennotinuse == false) && (inputAfterIdent == 0)))
        {
          gpio_write(localGPIO, pttGPIO, 0);
          return(0);
        }
        break;
      case 7:                                  // Periodic check for quiet hours and second half hour
                                               // First check if there has been any change
        if ((quiet_hours == previous_quiet_hours) && (secondhalfhour == previous_secondhalfhour))
        {
          return(2);
        }
        else if (quiet_hours != previous_quiet_hours) // Quiet hours has Changed
        {
                                               // So raise carrier if 
                                               // quiet_hours == false and transmitwhennotinuse = false
          if ((quiet_hours == false) && (transmitwhennotinuse == false))
          {
            gpio_write(localGPIO, pttGPIO, 1);
            previous_quiet_hours = quiet_hours;
            return(1);
          }
                                               // Drop carrier if 
                                               // quiet_hours == true and (inputAfterIdent == 0)
          if ((quiet_hours == true) && (inputAfterIdent == 0))
          {
            gpio_write(localGPIO, pttGPIO, 0);
            previous_quiet_hours = quiet_hours;
            return(0);
          }
        }
        else if (secondhalfhour != previous_secondhalfhour) // Half hour has changed
        {
                                               // So raise carrier if 
                                               // secondhalfhour = false and not in quiet hours and 
                                               // transmitwhennotinuse = false
          if ((secondhalfhour == false) && (quiet_hours == false) && (transmitwhennotinuse == false))
          {
            gpio_write(localGPIO, pttGPIO, 1);
            previous_secondhalfhour = secondhalfhour;
            return(1);
          }
                                               // Drop carrier if 
                                               // secondhalfhour == true and (inputAfterIdent == 0)
          if ((secondhalfhour == true) && (inputAfterIdent == 0))
          {
            gpio_write(localGPIO, pttGPIO, 0);
            previous_secondhalfhour = secondhalfhour;
            return(0);
          }
        }
        break;
      default:                               // PTT Off
        gpio_write(localGPIO, pttGPIO, 0);
        return(0);
      break;
    }
  }
  return(0);
}


