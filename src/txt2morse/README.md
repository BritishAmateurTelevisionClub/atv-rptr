# `txt2morse` -- Convert text to Morse code audio

This is `txt2morse`, by Andrew A. Cashner

This C program converts a plain text file to a WAV file of Morse code.
You can specify the frequency and the speed of the Morse code.
Defaults are 800 Hz frequency and 12 words per minute.

## Change Log

* 2017-01-04 -- Version 2.0
    - Command-line options for filename, frequency, rate; improved program structure 
* 2016-09-01 -- Version 1.0

## Usage Examples

* `txt2morse file.txt`
    - Creates file `file.wav` with Morse code for `file.txt`
* `txt2morse -o outfile.wav infile.txt`
    - Creates file `outfile.wav` with Morse code for `infile.txt`
* `txt2morse -f 1200 -r 6 file.txt`
    - Creates file `file.wav` with Morse code for `file.txt`
    - Uses a frequency of 1200 Hz and a rate of 6 words per minute
 
Afterward you may wish to convert the large WAV file to OGG or MP3
using a utility like `sox`.

## Acknowledgments and License

This software uses a small sound library by Douglas Thain.

The rest of this software is in the public domain.
You may do what you like with it.
I provide no warranty of any kind.
