#!/bin/bash

# Compile and run the repeater software

# set -x

CONFIGFILE="/home/pi/atv-rptr/config/repeater_config.txt"

############ Function to Read from Config File ###############

get_config_var() {
lua - "$1" "$2" <<EOF
local key=assert(arg[1])
local fn=assert(arg[2])
local file=assert(io.open(fn))
for line in file:lines() do
local val = line:match("^#?%s*"..key.."=(.*)$")
if (val ~= nil) then
print(val)
break
end
end
EOF
}

pkill run-audio.sh  >/dev/null 2>/dev/null
pkill dtmf_listener.sh >/dev/null 2>/dev/null
sudo killall arecord >/dev/null 2>/dev/null
sudo killall -9 fbi >/dev/null 2>/dev/null
sudo killall rptr >/dev/null 2>/dev/null

cd /home/pi
reset

# Check that the config file is in unix format, convert if not
dos2unix < "$CONFIGFILE" | cmp - "$CONFIGFILE" >/dev/null 2>/dev/null
if [ $? != 0 ]; then
  dos2unix "$CONFIGFILE" >/dev/null 2>/dev/null
fi

# Put up the Start-up Splash Screen, which will be killed by the repeater process
sudo fbi -T 1 -noverbose -a /home/pi/atv-rptr/media/starting_up.jpg >/dev/null 2>/dev/null

# Source the script to build the default captions
printf "Building the Captions\n"

source /home/pi/atv-rptr/scripts/build_captions.sh


# Start a low level pink noise to maintain audio channel active

AUDIO_KEEP_ALIVE=$(get_config_var audiokeepalive $CONFIGFILE)
if [[ "$AUDIO_KEEP_ALIVE" == "yes" ]];
then
  /home/pi/atv-rptr/scripts/run-audio.sh &
fi

# Start the DTMF Listener if required
DTMF_CONTROL=$(get_config_var dtmfcontrol $CONFIGFILE)
if [[ "$DTMF_CONTROL" == "on" ]];
then
  ps cax | grep 'multimon-ng' > /dev/null
  if [ $? -ne 0 ]; then
    echo "DTMF Process is not running.  Starting the DTMF Listener"
    (/home/pi/atv-rptr/scripts/dtmf_listener.sh /dev/null 2>/dev/null) &
  fi
fi

# Start the Repeater Controller
/home/pi/atv-rptr/bin/rptr
exit


