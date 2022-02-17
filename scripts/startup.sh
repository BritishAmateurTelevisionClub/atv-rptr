#!/bin/bash

# set -x

# This script is sourced from .bashrc at boot and ssh session start
# to sort out driver issues and
# to select the user's selected start-up option.
# Dave Crump 20220102

############ Set Environment Variables ###############

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


######################### Start here #####################

# Check that the config file is in unix format, convert if not
dos2unix < "$CONFIGFILE" | cmp - "$CONFIGFILE" >/dev/null 2>/dev/null
if [ $? != 0 ]; then
  dos2unix "$CONFIGFILE" >/dev/null 2>/dev/null
fi

# Put up the Start-up Splash Screen, which will be killed by the repeater process
sudo fbi -T 1 -noverbose -a /home/pi/atv-rptr/media/starting_up.jpg >/dev/null 2>/dev/null

# Source the script to build the default captions
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
  /home/pi/atv-rptr/scripts/dtmf_listener.sh &
fi

# Start the Repeater Controller
/home/pi/atv-rptr/bin/rptr

