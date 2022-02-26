#!/bin/bash

# DTMF listener script.  Sends integers from complete codes (ie 12 from *12#)
# to 127.0.0.1:8888

############ Set Environment Variables ###############

CONFIGFILE="/home/pi/atv-rptr/config/repeater_config.txt"

############ Function to Read from Repeater Config File ###############

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

#########################################################################

# Check audio input device connected

arecord -l | grep -E "USB Audio Device|USB AUDIO|Head|Sound Device" >/dev/null 2>/dev/null
if [ $? != 0 ]; then
  echo "No compatible DTMF input device detected"
  exit
fi

# Check for audio card number
CARD="$(arecord -l \
      | grep -E "USB Audio Device|USB AUDIO|Head|Sound Device" \
      | head -c 6 | tail -c 1)"

# Set audio gain (0 - 100, default 62)
AUDIO_GAIN=$(get_config_var dtmfaudiogain $CONFIGFILE)
AUDIO_GAIN+="%"

amixer -c $CARD -- sset Mic Capture $AUDIO_GAIN #> /dev/null 2>&1

# Initialise
CODE="blocked"

while read l1 ;do 
  if [[ $l1 =~ DTMF: ]]; then                             # Output line includes DTMF 
    set -- $l1 
    #echo  "$l1"
    DTMF_CODE=${l1:6:1}                                   # Extract the DTMF Character
    #echo $DTMF_CODE

    if [[ "$DTMF_CODE" == "*" ]]; then                    # Character is * so reset and initialise
      #echo STAR
      CODE_TO_SEND=""
      CODE="building"

    elif [[ "$DTMF_CODE" == "#" ]]; then                  # Character is hash, so send assembled code string
      if [[ "$CODE" == "building" ]]; then
        #echo "Sending Code" "$CODE_TO_SEND"
        echo "$CODE_TO_SEND" > /dev/udp/127.0.0.1/8888
      fi
      CODE="blocked"

    else                                                  # Character is not star or hash so add to end of string
      if [[ "$CODE" == "building" ]]; then
        CODE_TO_SEND+=$DTMF_CODE
      fi
    fi
  fi
done < <( arecord -f S16_LE -r 48000 -c 1 -B 4800 -t raw -D plughw:"$CARD",0 \
        | sox -c 1 -t raw -r 48000 -b 16 -e signed-integer - --buffer 1024 -t raw -b 16  - rate 22050 \
        | multimon-ng -a DTMF -)



