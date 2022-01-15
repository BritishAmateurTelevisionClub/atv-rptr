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

void *InputStatusListener(void * arg)
{
  int i;
  int inputValues1[8];
  int inputValues2[8];
  int lastStableValue[8];
  bool change1Detected = false;
  bool validChangeDetected = false;

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

    if ((change1Detected == true) && (validChangeDetected == true))
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

//int main(void)
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
        if ((strcmp(buf, "00") == 0) || ((atoi(buf) >= 1) && (atoi(buf) <= 99)))
        {
          UDP_Command(atoi(buf));
        }

        //if (strcmp(buf, "00") == 0)
        //{
        //  inputStatusChange = true;
        //  StatusScreenOveride = false;
        //}

        //if (strcmp(buf, "01") == 0)
        //{
        //  inputStatusChange = true;
        //  StatusScreenOveride = true;
        //}
		
		//now reply the client with the same data
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
}

//pi@raspberrypi:~ $ ncat -v 127.0.0.1 8888 -u
//Ncat: Version 7.70 ( https://nmap.org/ncat )
//Ncat: Connected to 127.0.0.1:8888.
//hello



