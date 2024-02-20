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

# Check that audio input device is connected

arecord -l | grep -E "USB Audio Device|USB AUDIO|Head|Sound Device" >/dev/null 2>/dev/null
if [ $? != 0 ]; then
  echo "No compatible DTMF input device detected"
  exit
fi

# Check for the input audio card number
CARD="$(arecord -l \
      | grep -E "USB Audio Device|USB AUDIO|Head|Sound Device" \
      | head -c 6 | tail -c 1)"

# Set input audio gain (0 - 100, default 62) for DTMF and talkback
AUDIO_GAIN=$(get_config_var dtmfaudiogain $CONFIGFILE)
AUDIO_GAIN+="%"

amixer -c $CARD -- sset Mic Capture $AUDIO_GAIN >/dev/null 2>/dev/null

# Check if talkback audio required on hdmi
TALKBACK_AUDIO=$(get_config_var controllertalkbackaudio $CONFIGFILE)

# Delete any previous talkback audio fifo
sudo rm /home/pi/audio.raw >/dev/null 2>/dev/null

# Initialise
CODE="blocked"

if [ "$TALKBACK_AUDIO" != "on" ]; then

  ## DTMF without talkback audio

  while read l1 ;do 
    if [[ $l1 =~ DTMF: ]]; then                             # Output line includes DTMF 
      set -- $l1 
      #echo  "$l1"
      DTMF_CODE=${l1:6:1}                                   # Extract the DTMF Character
      echo "DTMF Code Received: "$DTMF_CODE

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

else      ## DTMF with talkback audio

  # Check audio output device details:
  AUDIO_OUT=$(get_config_var audioout $CONFIGFILE)
  if [ "$AUDIO_OUT" == "hdmi" ]; then
    OUTPUT_DEVICE="plughw:CARD=b1,DEV=0"
  fi
  if [ "$AUDIO_OUT" == "jack" ]; then
    OUTPUT_DEVICE="plughw:CARD=Headphones,DEV=0"
  fi
  if [ "$AUDIO_OUT" == "usb" ]; then
    OUTPUT_DEVICE="plughw:CARD=Device,DEV=0"
  fi

  # Create the audio fifo and start playing it
  mkfifo /home/pi/audio.raw
  aplay -f S16_LE -r 48000 -c 1 -t raw -D "$OUTPUT_DEVICE" /home/pi/audio.raw  & 

  # Listen for DTMF and tee off the audio to play it
  while read l1 ;do 
    if [[ $l1 =~ DTMF: ]]; then                             # Output line includes DTMF 
      set -- $l1 
      #echo  "$l1"
      DTMF_CODE=${l1:6:1}                                   # Extract the DTMF Character
      echo "DTMF Code Received: "$DTMF_CODE

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
          | (tee /home/pi/audio.raw) \
          | sox -c 1 -t raw -r 48000 -b 16 -e signed-integer - --buffer 1024 -t raw -b 16  - rate 22050 \
          | multimon-ng -a DTMF -)
fi



