#!/bin/bash

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

############ Function to Read value with - from Config File ###############

get-config_var() {
lua - "$1" "$2" <<EOF
local key=assert(arg[1])
local fn=assert(arg[2])
local file=assert(io.open(fn))
for line in file:lines() do
local val = line:match("^#?%s*"..key.."=[+-]?(.*)$")
if (val ~= nil) then
print(val)
break
end
end
EOF
}

#########################################################

CONFIGFILE="/home/pi/atv-rptr/config/repeater_config.txt"


do_stop()
{
  reset
  echo "Stopping the Existing Processes"

  pkill run-audio.sh  >/dev/null 2>/dev/null
  pkill dtmf_listener.sh >/dev/null 2>/dev/null
  sudo killall arecord >/dev/null 2>/dev/null
  sudo killall -9 fbi >/dev/null 2>/dev/null
  sudo killall rptr >/dev/null 2>/dev/null
  sudo killall speaker-test >/dev/null 2>/dev/null
}


do_IR_low()
{
  pigs w 18 0
}

 
do_IR_high()
{
  pigs w 18 1
}

 
do_Ctrl_low()
{
  pigs w 27 0
}

 
do_Ctrl_high()
{
  pigs w 27 1
}


do_logic_exit()
{
  pigs w 18 0
  pigs w 27 0
  logic=1
}

 
do_logic()
{
  # IR uses a TX on GPIO pin 12 (BCM(18)
  # And a selector on GPIO pin 13 (BCM 27)

  logic=0
  while [ "$logic" -eq 0 ] 
  do
    menuchoice=$(whiptail --title "Logic Selection Menu" --menu "Select Choice" 16 78 6 \
      "1 IR Low" "Set the IR LED off"  \
      "2 IR High" "Set the IR LED on" \
      "3 Ctrl Low" "Set the control line low (Downstream/Bottom)" \
      "4 Ctrl High" "Set the control line high (Upstream/Top)" \
      "5 Exit" "Exit to Main Menu" \
        3>&2 2>&1 1>&3)

    case "$menuchoice" in
      1\ *) do_IR_low ;;
      2\ *) do_IR_high ;;
      3\ *) do_Ctrl_low ;;
      4\ *) do_Ctrl_high ;;
      5\ *) do_logic_exit ;;
    esac
  done
}


do_U1()
{
  pigs w 27 1
  sleep 0.1

  ir-ctl -S nec:0x17 -d /dev/lirc0

  sleep 0.1
  pigs w 27 0
}


do_U2()
{
  pigs w 27 1
  sleep 0.1

  ir-ctl -S nec:0x12 -d /dev/lirc0

  sleep 0.1
  pigs w 27 0
}


do_U3()
{
  pigs w 27 1
  sleep 0.1

  ir-ctl -S nec:0x59 -d /dev/lirc0

  sleep 0.1
  pigs w 27 0
}


do_U4()
{
  pigs w 27 1
  sleep 0.1

  ir-ctl -S nec:0x08 -d /dev/lirc0

  sleep 0.1
  pigs w 27 0
}


do_UQ()
{
  pigs w 27 1
  sleep 0.1

  ir-ctl -S nec:0x18 -d /dev/lirc0

  sleep 0.1
  pigs w 27 0
}


do_U_exit()
{
  upstream=1
}


do_upstream()
{
  upstream=0
  while [ "$upstream" -eq 0 ] 
  do
    menuchoice=$(whiptail --title "Upstream Selection Menu" --menu "Select Choice" 16 78 6 \
      "1 Input 1" "GB3HV 70cm Ryde (Top, IR sel high)"  \
      "2 Input 2" "GB3HV 23cm Ryde (Top, IR sel high)" \
      "3 Input 3" "GB3HV Stream RX (Top, IR sel high)" \
      "4 Input 4" "GB3HV Analogue (Top, IR sel high)" \
      "5 Quad" "GB3HV Quad RXs (Top, IR sel high)" \
      "6 Exit" "Exit to Main Menu" \
        3>&2 2>&1 1>&3)

    case "$menuchoice" in
      1\ *) do_U1 ;;
      2\ *) do_U2 ;;
      3\ *) do_U3 ;;
      4\ *) do_U4 ;;
      5\ *) do_UQ ;;
      6\ *) do_U_exit ;;
    esac
  done
}


do_D1()
{
  pigs w 27 0
  sleep 0.1

  ir-ctl -S nec:0x17 -d /dev/lirc0
}


do_D2()
{
  pigs w 27 0
  sleep 0.1

  ir-ctl -S nec:0x12 -d /dev/lirc0
}


do_D3()
{
  pigs w 27 0
  sleep 0.1

  ir-ctl -S nec:0x59 -d /dev/lirc0
}


do_D4()
{
  pigs w 27 0
  sleep 0.1

  ir-ctl -S nec:0x08 -d /dev/lirc0
}


do_DQ()
{
  pigs w 27 0
  sleep 0.1

  ir-ctl -S nec:0x18 -d /dev/lirc0
}


do_D_exit()
{
  downstream=1
}


do_downstream()
{
  downstream=0
  while [ "$downstream" -eq 0 ] 
  do
    menuchoice=$(whiptail --title "Downstream Selection Menu" --menu "Select Choice" 16 78 6 \
      "1 Input 1" "GB3HV Controller (Bottom, IR sel low)"  \
      "2 Input 2" "GB3HV RX Switch (Bottom, IR sel low)" \
      "3 Input 3" "GB3HV Desktop RPi (Bottom, IR sel low)" \
      "4 Input 4" "No connection (Bottom, IR sel low)" \
      "5 Quad" "GB3HV Controller Quad (Bottom, IR sel low)" \
      "6 Exit" "Exit to Main Menu" \
        3>&2 2>&1 1>&3)

    case "$menuchoice" in
      1\ *) do_D1 ;;
      2\ *) do_D2 ;;
      3\ *) do_D3 ;;
      4\ *) do_D4 ;;
      5\ *) do_DQ ;;
      6\ *) do_D_exit ;;
    esac
  done
}


do_B1()
{
  pigs w 27 1  # Select upstream
  sleep 0.1

  ir-ctl -S nec:0x17 -d /dev/lirc0

  sleep 0.1
  pigs w 27 0  # Select downstream

  ir-ctl -S nec:0x12 -d /dev/lirc0
}


do_B2()
{
  pigs w 27 1  # Select upstream
  sleep 0.1

  ir-ctl -S nec:0x12 -d /dev/lirc0

  sleep 0.1
  pigs w 27 0  # Select downstream

  ir-ctl -S nec:0x12 -d /dev/lirc0
}


do_B3()
{
  pigs w 27 1  # Select upstream
  sleep 0.1

  ir-ctl -S nec:0x59 -d /dev/lirc0

  sleep 0.1
  pigs w 27 0  # Select downstream

  ir-ctl -S nec:0x12 -d /dev/lirc0
}


do_B4()
{
  pigs w 27 1  # Select upstream
  sleep 0.1

  ir-ctl -S nec:0x08 -d /dev/lirc0

  sleep 0.1
  pigs w 27 0  # Select downstream

  ir-ctl -S nec:0x12 -d /dev/lirc0
}


do_BQ()
{
  pigs w 27 1  # Select upstream
  sleep 0.1

  ir-ctl -S nec:0x18 -d /dev/lirc0

  sleep 0.1
  pigs w 27 0  # Select downstream

  ir-ctl -S nec:0x12 -d /dev/lirc0
}


do_BC1()
{
  pigs w 27 0  # Select downstream

  ir-ctl -S nec:0x17 -d /dev/lirc0
}


do_BC3()
{
  pigs w 27 0  # Select downstream

  ir-ctl -S nec:0x59 -d /dev/lirc0
}


do_BCQ()
{
  pigs w 27 0  # Select downstream

  ir-ctl -S nec:0x12 -d /dev/lirc0
}


do_B_exit()
{
  both=1
}


do_both()
{
  both=0
  while [ "$both" -eq 0 ] 
  do
    menuchoice=$(whiptail --title "Combined Menu (Upstream first)" --menu "Select Choice" 16 78 6 \
      "1 RX Input 1" "GB3HV 70cm Ryde"  \
      "2 RX Input 2" "GB3HV 23cm Ryde" \
      "3 RX Input 3" "GB3HV Stream RX" \
      "4 RX Input 4" "GB3HV Analogue" \
      "5 RX Quad" "GB3HV Quad RXs" \
      "6 Cont Input 1" "GB3HV Controller"  \
      "7 Cont Input 3" "GB3HV Desktop RPi" \
      "8 Cont Quad" "GB3HV Controller Quad" \
      "9 Exit" "Exit to Main Menu" \
        3>&2 2>&1 1>&3)

    case "$menuchoice" in
      1\ *) do_B1 ;;
      2\ *) do_B2 ;;
      3\ *) do_B3 ;;
      4\ *) do_B4 ;;
      5\ *) do_BQ ;;
      6\ *) do_BC1 ;;
      7\ *) do_BC3 ;;
      8\ *) do_BCQ ;;
      9\ *) do_B_exit ;;
    esac
  done
}


do_restart()
{
  # Put up the Start-up Splash Screen, which will be killed by the repeater process
  sudo fbi -T 1 -noverbose -a /home/pi/atv-rptr/media/starting_up.jpg >/dev/null 2>/dev/null

  echo
  echo "Building the Captions....."

  # Source the script to build the default captions
  source /home/pi/atv-rptr/scripts/build_captions.sh

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
      (/home/pi/atv-rptr/scripts/dtmf_listener.sh >/dev/null 2>/dev/null) &
    fi
  fi

  echo
  echo "Restarting the repeater controller"

  (/home/pi/atv-rptr/bin/rptr >/dev/null 2>/dev/null) &
}


do_exit()
{
  status=1
}


#********************************************* MAIN MENU *********************************
#************************* Execution of IR Menu starts here *************************

status=0

while [ "$status" -eq 0 ] 
  do
    # Display main menu

    menuchoice=$(whiptail --title "BATC Repeater IR Test Menu" --menu "Select Choice and press enter:" 20 78 13 \
	"0 Stop" "Stop the repeater controller" \
    "1 Logic" "Direct Control of the IR control lines" \
    "2 Upstream" "Send Commands to Upstream switch" \
    "3 Downstream" "Send Commands to Downstream switch" \
    "4 Both" "Send commands to both" \
    "5 Restart" "Restart the repeater" \
    "6 Exit" "Exit this utility" \
 	3>&2 2>&1 1>&3)

    case "$menuchoice" in
      0\ *) do_stop ;;
      1\ *) do_logic ;;
      2\ *) do_upstream ;;
      3\ *) do_downstream ;;
      4\ *) do_both ;;
      5\ *) do_restart ;;
      6\ *) do_exit ;;
    esac
  done
  reset
exit
