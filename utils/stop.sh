#/bin/bash

# Stop all the repeater functions and exit to an ssh prompt.
# PTT behaviour to be defined

pkill run-audio.sh  >/dev/null 2>/dev/null

pkill dtmf_listener.sh >/dev/null 2>/dev/null

sudo killall arecord >/dev/null 2>/dev/null

sudo killall -9 fbi >/dev/null 2>/dev/null

sudo killall rptr >/dev/null 2>/dev/null

exit

