#!/bin/bash

# This script is called at startup to build default captions using the repeater callsign locator and video source names.

# These captions are stored in the /home/pi/tmp folder so are volatile.

CALLSIGN=$(get_config_var callsign $CONFIGFILE)
LOCATOR=$(get_config_var locator $CONFIGFILE)
BACKIMAGE=$(get_config_var backimage $CONFIGFILE)
INPUT1NAME=$(get_config_var input1name $CONFIGFILE)
INPUT2NAME=$(get_config_var input2name $CONFIGFILE)
INPUT3NAME=$(get_config_var input3name $CONFIGFILE)
INPUT4NAME=$(get_config_var input4name $CONFIGFILE)
INPUT5NAME=$(get_config_var input5name $CONFIGFILE)
INPUT6NAME=$(get_config_var input6name $CONFIGFILE)
INPUT7NAME=$(get_config_var input7name $CONFIGFILE)

# Build the Main Ident Slide
rm /home/pi/tmp/ident.jpg >/dev/null 2>/dev/null
rm /home/pi/tmp/caption.png >/dev/null 2>/dev/null
LINE1=$CALLSIGN
LINE2=$LOCATOR
convert -size 1280x600 xc:transparent -fill white \
  -gravity North -pointsize 40 -annotate 0 "ATV Repeater" \
  -gravity Center -pointsize 120 -annotate 0 "$LINE1\n\n$LINE2" \
  /home/pi/tmp/caption.png
convert "$BACKIMAGE" /home/pi/tmp/caption.png \
  -geometry +0+60 -composite /home/pi/tmp/ident.jpg

# Build the K Slide
rm /home/pi/tmp/k.jpg >/dev/null 2>/dev/null
rm /home/pi/tmp/caption.png >/dev/null 2>/dev/null
convert -size 1280x600 xc:transparent -fill white \
  -gravity North -pointsize 40 -annotate 0 "ATV Repeater $CALLSIGN" \
  -gravity Center -pointsize 180 -annotate 0 "K" \
  -gravity South -pointsize 40 -annotate 0 "Ready for next transmission" \
  /home/pi/tmp/caption.png
convert "$BACKIMAGE" /home/pi/tmp/caption.png \
  -geometry +0+60 -composite /home/pi/tmp/k.jpg


# Build the Input 1 Slide
rm /home/pi/tmp/input1.jpg >/dev/null 2>/dev/null
rm /home/pi/tmp/caption.png >/dev/null 2>/dev/null
convert -size 1280x600 xc:transparent -fill white \
  -gravity North -pointsize 40 -annotate 0 "ATV Repeater $CALLSIGN" \
  -gravity Center -pointsize 60 -annotate 0 "Signal detected on $INPUT1NAME" \
  /home/pi/tmp/caption.png
convert "$BACKIMAGE" /home/pi/tmp/caption.png \
  -geometry +0+60 -composite /home/pi/tmp/input1.jpg

# Build the Input 2 Slide
rm /home/pi/tmp/input2.jpg >/dev/null 2>/dev/null
rm /home/pi/tmp/caption.png >/dev/null 2>/dev/null
convert -size 1280x600 xc:transparent -fill white \
  -gravity North -pointsize 40 -annotate 0 "ATV Repeater $CALLSIGN" \
  -gravity Center -pointsize 60 -annotate 0 "Signal detected on $INPUT2NAME" \
  /home/pi/tmp/caption.png
convert "$BACKIMAGE" /home/pi/tmp/caption.png \
  -geometry +0+60 -composite /home/pi/tmp/input2.jpg

# Build the Input 3 Slide
rm /home/pi/tmp/input3.jpg >/dev/null 2>/dev/null
rm /home/pi/tmp/caption.png >/dev/null 2>/dev/null
convert -size 1280x600 xc:transparent -fill white \
  -gravity North -pointsize 40 -annotate 0 "ATV Repeater $CALLSIGN" \
  -gravity Center -pointsize 60 -annotate 0 "Signal detected on $INPUT3NAME" \
  /home/pi/tmp/caption.png
convert "$BACKIMAGE" /home/pi/tmp/caption.png \
  -geometry +0+60 -composite /home/pi/tmp/input3.jpg

# Build the Input 4 Slide
rm /home/pi/tmp/input4.jpg >/dev/null 2>/dev/null
rm /home/pi/tmp/caption.png >/dev/null 2>/dev/null
convert -size 1280x600 xc:transparent -fill white \
  -gravity North -pointsize 40 -annotate 0 "ATV Repeater $CALLSIGN" \
  -gravity Center -pointsize 60 -annotate 0 "Signal detected on $INPUT4NAME" \
  /home/pi/tmp/caption.png
convert "$BACKIMAGE" /home/pi/tmp/caption.png \
  -geometry +0+60 -composite /home/pi/tmp/input4.jpg

# Build the Input 5 Slide
rm /home/pi/tmp/input5.jpg >/dev/null 2>/dev/null
rm /home/pi/tmp/caption.png >/dev/null 2>/dev/null
convert -size 1280x600 xc:transparent -fill white \
  -gravity North -pointsize 40 -annotate 0 "ATV Repeater $CALLSIGN" \
  -gravity Center -pointsize 60 -annotate 0 "Signal detected on $INPUT5NAME" \
  /home/pi/tmp/caption.png
convert "$BACKIMAGE" /home/pi/tmp/caption.png \
  -geometry +0+60 -composite /home/pi/tmp/input5.jpg

# Build the Input 6 Slide
rm /home/pi/tmp/input6.jpg >/dev/null 2>/dev/null
rm /home/pi/tmp/caption.png >/dev/null 2>/dev/null
convert -size 1280x600 xc:transparent -fill white \
  -gravity North -pointsize 40 -annotate 0 "ATV Repeater $CALLSIGN" \
  -gravity Center -pointsize 60 -annotate 0 "Signal detected on $INPUT6NAME" \
  /home/pi/tmp/caption.png
convert "$BACKIMAGE" /home/pi/tmp/caption.png \
  -geometry +0+60 -composite /home/pi/tmp/input6.jpg

# Build the Input 7 Slide
rm /home/pi/tmp/input7.jpg >/dev/null 2>/dev/null
rm /home/pi/tmp/caption.png >/dev/null 2>/dev/null
convert -size 1280x600 xc:transparent -fill white \
  -gravity North -pointsize 40 -annotate 0 "ATV Repeater $CALLSIGN" \
  -gravity Center -pointsize 60 -annotate 0 "Signal detected on $INPUT7NAME" \
  /home/pi/tmp/caption.png
convert "$BACKIMAGE" /home/pi/tmp/caption.png \
  -geometry +0+60 -composite /home/pi/tmp/input7.jpg

# Build Test Card F Wide with the callsign for the Carousel
rm /home/pi/tmp/tcfw.jpg >/dev/null 2>/dev/null
rm /home/pi/tmp/caption.png >/dev/null 2>/dev/null
convert -size 1280x100 xc:transparent -fill white \
  -gravity Center -pointsize 70 -annotate 0 "$CALLSIGN" \
  /home/pi/tmp/caption.png
convert /home/pi/atv-rptr/media/tcfw.jpg /home/pi/tmp/caption.png \
  -geometry +0+597 -composite /home/pi/tmp/tcfw.jpg

# Build the "Stopped" Slide
rm /home/pi/tmp/stopped.jpg >/dev/null 2>/dev/null
rm /home/pi/tmp/caption.png >/dev/null 2>/dev/null
convert -size 1280x720 xc:black -fill white \
  -gravity North -pointsize 40 -annotate 0 "ATV Repeater $CALLSIGN" \
  -gravity Center -pointsize 40 -annotate 0 "Controller Software Not Running" \
  -gravity South -pointsize 40 -annotate 0 "Restart Controller software by SSH Control or Reboot" \
  /home/pi/tmp/stopped.png

# Build the default Ident CW Audio file and convert to 32000 rate to prevent glitches
IDENTCWAUDIO=$(get_config_var identcwaudio $CONFIGFILE)
if [[ "$IDENTCWAUDIO" == "on" ]]; then
  IDENTCWSPEED=$(get_config_var identcwspeed $CONFIGFILE)
  IDENTCWPITCH=$(get_config_var identcwpitch $CONFIGFILE)
  cd /home/pi/tmp
  echo " $CALLSIGN" > callsign.txt
  /home/pi/atv-rptr/bin/txt2morse -f "$IDENTCWPITCH" -r "$IDENTCWSPEED" -o identtemp.wav callsign.txt
  sox identtemp.wav -r 32000 ident.wav
fi

# Build the default "K" CW Audio file and convert to 32000 rate to prevent glitches
KCWAUDIO=$(get_config_var kcwaudio $CONFIGFILE)
if [[ "$KCWAUDIO" == "on" ]]; then
  KCWSPEED=$(get_config_var kcwspeed $CONFIGFILE)
  KCWPITCH=$(get_config_var kcwpitch $CONFIGFILE)
  cd /home/pi/tmp
  echo " K" > k.txt
  /home/pi/atv-rptr/bin/txt2morse -f "$KCWPITCH" -r "$KCWSPEED" -o ktemp.wav k.txt
  sox ktemp.wav -r 32000 k.wav
fi


