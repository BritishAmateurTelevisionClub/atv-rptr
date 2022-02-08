/* txt2morse.c -- Convert ASCII text to Morse code audio 
 * 
 * By Andrew A. Cashner
 * 
 * Versions
 * 2.0 -- 2016-11-29 -- Add command-line options for setting frequency,
 *                      rate, and output file name
 * 1.0 -- 2016-09 -- First release on GitHub
 * 
 * The program reads in text from a file,
 * converts the text to morse code,
 * and outputs the result as an audio file, .wav by default.
 * The program uses Douglas Thain's sound library,
 * http://www.nd.edu/~dthain/course/cse20211/fall2013/wavfile
 * 
 * Compile with -lm flag to include math library
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <getopt.h>
#include <string.h>

#include <math.h>
#include <time.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#include "wavfile.h"
#include "wavfile.c"

#define MAX_FILENAME 80
#define MAX_CHARS    55
#define MAX_CHAR_SEQ 8
#define MAX_ASCII    127
#define MAX_SIGNAL_LENGTH WAVFILE_SAMPLES_PER_SECOND

/* CONSTANTS */
const double default_frequency = 800.0;
const int default_rate_wpm = 12; 

/* FUNCTION PROTOTYPES */
void write_tone(FILE *outfile, short waveform[],
		double frequency, int duration);

void write_silence(FILE *outfile, short waveform[], int duration);

void write_morse_char(FILE *outfile, short waveform[],
		      double frequency, double unit_duration,
		      int signal_code[]);

/* LOOKUP TABLES */
const char *message[] = {
     /* MSG_HELP */
     "\ntxt2morse -- Convert ASCII text to Morse-code audio\n\n"
      "Usage: txt2morse [OPTION]... FILE...\n\n"
      "Options:\n"
      " -f, --frequency Set Morse audio frequency in Hz (default: 800)\n"
      " -r, --rate      Set Morse rate in words per minute (default: 12)\n"
      " -o, --output    Set name of output file\n\n"
      "By default, the output filename is taken from the input file,\n"
      "with .wav extension substituted.\n",

     /* MSG_VERSION */
     "txt2morse version 2.0 by Andrew A. Cashner (public domain)",

     /* MSG_ERROR */
     "An unknown error occurred."
};
const enum { MSG_HELP, MSG_VERSION, MSG_ERROR } msg_code;

const enum { DOT, DASH, CHAR_SPC, WORD_SPC, ENDCODE } sign_type;


int main(int argc, char *argv[]) 
{
     /* Setup for reading command line arguments */
     int c;
     int option_index = 0;
     static struct option long_options[] = {
	  {"help",        no_argument,       0, 'h'},
	  {"version",     no_argument,       0, 'v'},
	  {"frequency",   required_argument, 0, 'f'},
	  {"rate",        required_argument, 0, 'r'},
	  {"output-file", required_argument, 0, 'o'},
	  {0, 0, 0, 0}
     };

     /* Set default values */
     double frequency = default_frequency;
     int rate_wpm = default_rate_wpm;
     double unit_duration = WAVFILE_SAMPLES_PER_SECOND / default_rate_wpm;

     FILE *infile, *outfile;
     char infile_name[MAX_FILENAME], outfile_name[MAX_FILENAME];
     bool outfile_specified = false;

     /* Array of numeric values for each tone's waveform */
     short waveform[MAX_SIGNAL_LENGTH];

     /* Lookup table of morse signal codes */
     int morse_table[MAX_CHARS][MAX_CHAR_SEQ + 1] = {
	  {'A', DOT, DASH, ENDCODE },
	  {'B', DASH, DOT, DOT, DOT, ENDCODE },
	  {'C', DASH, DOT, DASH, DOT, ENDCODE },
	  {'D', DASH, DOT, DOT, ENDCODE },
	  {'E', DOT, ENDCODE },
	  {'F', DOT, DOT, DASH, DOT, ENDCODE },
	  {'G', DASH, DASH, DOT, ENDCODE },
	  {'H', DOT, DOT, DOT, DOT, ENDCODE },
	  {'I', DOT, DOT, ENDCODE },
	  {'J', DOT, DASH, DASH, DASH, ENDCODE },
	  {'K', DASH, DOT, DASH, ENDCODE },
	  {'L', DOT, DASH, DOT, DOT, ENDCODE },
	  {'M', DASH, DASH, ENDCODE },
	  {'N', DASH, DOT, ENDCODE },
	  {'O', DASH, DASH, DASH, ENDCODE },
	  {'P', DOT, DASH, DASH, DOT, ENDCODE },
	  {'Q', DASH, DASH, DOT, DASH, ENDCODE },
	  {'R', DOT, DASH, DOT, ENDCODE },
	  {'S', DOT, DOT, DOT, ENDCODE },
	  {'T', DASH, ENDCODE },
	  {'U', DOT, DOT, DASH, ENDCODE },
	  {'V', DOT, DOT, DOT, DASH, ENDCODE },
	  {'W', DOT, DASH, DASH, ENDCODE },
	  {'X', DASH, DOT, DOT, DASH, ENDCODE },
	  {'Y', DASH, DOT, DASH, DASH, ENDCODE },
	  {'Z', DASH, DASH, DOT, DOT, ENDCODE },
	  {'0', DASH, DASH, DASH, DASH, DASH, ENDCODE },
	  {'1', DOT, DASH, DASH, DASH, DASH, ENDCODE },
	  {'2', DOT, DOT, DASH, DASH, DASH, ENDCODE },
	  {'3', DOT, DOT, DOT, DASH, DASH, ENDCODE },
	  {'4', DOT, DOT, DOT, DOT, DASH, ENDCODE },
	  {'5', DOT, DOT, DOT, DOT, DOT, ENDCODE },
	  {'6', DASH, DOT, DOT, DOT, DOT, ENDCODE },
	  {'7', DASH, DASH, DOT, DOT, DOT, ENDCODE },
	  {'8', DASH, DASH, DASH, DOT, DOT, ENDCODE },
	  {'9', DASH, DASH, DASH, DASH, DOT, ENDCODE },
	  {'.', DOT, DASH, DOT, DASH, DOT, DASH, ENDCODE },
	  {',', DASH, DASH, DOT, DOT, DASH, DASH, ENDCODE },
	  {'?', DOT, DOT, DASH, DASH, DOT, DOT, ENDCODE },
	  {'\'', DOT, DASH, DASH, DASH, DASH, DOT, ENDCODE },
	  {'!', DASH, DOT, DASH, DOT, DASH, DASH, ENDCODE },
	  {'/', DASH, DOT, DOT, DASH, DOT, ENDCODE },
	  {'(', DASH, DOT, DASH, DASH, DOT, ENDCODE },
	  {')', DASH, DOT, DASH, DASH, DOT, DASH, ENDCODE }, 
	  {'&', DOT, DASH, DOT, DOT, DOT, ENDCODE },
	  {':', DASH, DASH, DASH, DOT, DOT, DOT, ENDCODE },
	  {';', DASH, DOT, DASH, DOT, DASH, DOT, ENDCODE },
	  {'=', DASH, DOT, DOT, DOT, DASH, ENDCODE },
	  {'+', DOT, DASH, DOT, DASH, DOT, ENDCODE },
	  {'-', DASH, DOT, DOT, DOT, DOT, DASH, ENDCODE },
	  {'_', DOT, DOT, DASH, DASH, DOT, DASH, ENDCODE },
	  {'\"', DOT, DASH, DOT, DOT, DASH, DOT, ENDCODE },
	  {'$', DOT, DOT, DOT, DASH, DOT, DOT, DASH, ENDCODE },
	  {'@', DOT, DASH, DASH, DOT, DASH, DOT, ENDCODE },
	  {' ', WORD_SPC, ENDCODE }
     };
     /* Lookup table indexed to ASCII codes */
     int ascii_table[MAX_ASCII];
     int morse_table_index;
     int *signal_code;

     int i, ascii_char;
     int lower_upper_ascii_difference = 'a' - 'A';


     /* Process options:
      * - Optionally set frequency and rate
      * - Open files for input and output
      */

     while ((c = getopt_long(argc, argv, "hvf:r:o:",
			     long_options, &option_index)) != -1) {
	  switch (c) {
	  case 'h':
	       printf("%s\n", message[MSG_HELP]);
	       exit(0);
	  case 'v':
	       printf("%s\n", message[MSG_VERSION]);
	       exit(0);
	  case 'f':
	       if (sscanf(optarg, "%lf", &frequency) != 1) {
		    fprintf(stderr, "Bad frequency %s\n", optarg);
		    exit(EXIT_FAILURE);
	       } else break;
	  case 'r':
	       if (sscanf(optarg, "%d", &rate_wpm) != 1) {
		    fprintf(stderr, "Bad rate %s\n", optarg);
		    exit(EXIT_FAILURE);
	       } else {
		    unit_duration =
			 WAVFILE_SAMPLES_PER_SECOND / rate_wpm;
		    /* TODO -- substitute real calculation here */
		    break;
	       }
	  case 'o':
	       strcpy(outfile_name, optarg);
	       outfile_specified = true;
	       break;
	  case '?':
	       exit(EXIT_FAILURE);
	  default:
	       abort();
	  }
     }

     if (optind < argc) {
	  if (argc - optind > 1) {
	       fprintf(stderr, "Too many arguments.\n\n %s\n",
		       message[MSG_HELP]);
	       exit(EXIT_FAILURE);
	  } else {
	       strcpy(infile_name, argv[optind]);
	  }
     } else {
	  fprintf(stderr, "No input file specified.\n\n %s\n",
		  message[MSG_HELP]);
	  exit(EXIT_FAILURE);
     }

     /* Open input file */
     infile = fopen(infile_name, "r");
     if (infile == NULL) {
	  fprintf(stderr,
		  "Could not open file %s for reading.\n", infile_name);
	  exit(EXIT_FAILURE);
     }

     /* Open output file: Use input file name with .wav extension 
      * unless output filename was specified 
      */
     if (outfile_specified == false) {
	  strcpy(outfile_name, infile_name);
	  i = strlen(outfile_name);
	  if (outfile_name[i - 4] == '.') {
	       strcpy(&outfile_name[i - 3], "wav");
	  } else {
	       fprintf(stderr,
		       "Bad format for file name %s\n", infile_name);
	       exit(EXIT_FAILURE);
	  }
     }
     outfile = wavfile_open(outfile_name);
     if (outfile == NULL) {
	  fprintf(stderr, "Could not open file %s for writing.\n", outfile_name);
	  exit(EXIT_FAILURE);
     }

     /* Make lookup table to access morse codes through ASCII values */
     /* First make empty ASCII entries point to space character, which
	is last value of morse_table */
     for (i = 0; i <= MAX_ASCII; ++i) {
	  ascii_char = morse_table[MAX_CHARS][0];
     }
     for (i = 0; i < MAX_CHARS; ++i) {
	  ascii_char = morse_table[i][0];
	  ascii_table[ascii_char] = i;
     }
  
     /* Read in characters, look up series of dots and dashes in sign
	table, output appropriate format for each dot, dash, or space. */
     while ((ascii_char = fgetc(infile)) != EOF) {
	  /* Ensure valid input */
	  if (ascii_char > MAX_ASCII) {
	       break;
	  }
	  /* Ignore newlines, no processing needed */
	  else if (ascii_char == '\n') {
	       continue;
	  }
	  /* Convert lowercase to uppercase */
	  else if (ascii_char >= 'a' && ascii_char <= 'z') {
	       ascii_char -= lower_upper_ascii_difference; 
	  }
    
	  /* Get morse output patterns for each component character from
	     lookup table, so 'A' -> DOT, DASH -> ". ---" */
	  morse_table_index = ascii_table[ascii_char];
	  signal_code = &morse_table[morse_table_index][1];
	  write_morse_char(outfile, waveform, frequency,
			   unit_duration, signal_code);
     }

     fclose(infile);
     wavfile_close(outfile);
     return (0);
}


void write_tone(FILE *outfile, short waveform[],
		double frequency, int duration)
{
     int i;
     double timepoint;
     int volume = 32000;
     for (i = 0; i < duration; ++i) {
	  timepoint = (double) i / WAVFILE_SAMPLES_PER_SECOND;
	  waveform[i] = volume * sin(frequency * timepoint * 2 * M_PI);
     }
     wavfile_write(outfile, waveform, duration);
     return;
}
void write_silence(FILE *outfile, short waveform[], int duration)
{
     int i;
     for (i = 0; i < duration; ++i) {
	  waveform[i] = 0;
     }
     wavfile_write(outfile, waveform, duration);
     return;
}
void write_morse_char(FILE *outfile, short waveform[],
		      double frequency, double unit_duration,
		      int signal_code[])
{
     int i;
     for (i = 0; signal_code[i] != ENDCODE; ++i) {
	  if (signal_code[i] == WORD_SPC) {
	       /* Write word space */
	       write_silence(outfile, waveform, 6 * unit_duration);
	       break;
	  } else if (signal_code[i] == DOT) {
	       /* Write dot */
	       write_tone(outfile, waveform, frequency, unit_duration);
	  } else {
	       /* Write dash */
	       write_tone(outfile, waveform, frequency, 3 * unit_duration);
	  }
	  /* Write inter-signal space */
	  write_silence(outfile, waveform, unit_duration);
     }
     /* Write inter-character space */
     write_silence(outfile, waveform, 2 * unit_duration);
     return;
}
  
