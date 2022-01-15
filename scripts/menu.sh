#!/bin/bash

# Repeater Menu Application

############ Function to Write to Repeater Config File ###############

set_config_var() {
lua - "$1" "$2" "$3" <<EOF > "$3.bak"
local key=assert(arg[1])
local value=assert(arg[2])
local fn=assert(arg[3])
local file=assert(io.open(fn))
local made_change=false
for line in file:lines() do
if line:match("^#?%s*"..key.."=.*$") then
line=key.."="..value
made_change=true
end
print(line)
end
if not made_change then
print(key.."="..value)
end
EOF
mv "$3.bak" "$3"
}



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


do_normal_repeat()
{
  echo "00" > /dev/udp/127.0.0.1/8888 >/dev/null 2>/dev/null
}

do_status()
{
  echo "01" > /dev/udp/127.0.0.1/8888 >/dev/null 2>/dev/null
}

do_0()
{
  echo "10" > /dev/udp/127.0.0.1/8888 >/dev/null 2>/dev/null
}

do_1()
{
  echo "11" > /dev/udp/127.0.0.1/8888 >/dev/null 2>/dev/null
}

do_2()
{
  echo "12" > /dev/udp/127.0.0.1/8888 >/dev/null 2>/dev/null
}

do_3()
{
  echo "13" > /dev/udp/127.0.0.1/8888 >/dev/null 2>/dev/null
}

do_4()
{
  echo "14" > /dev/udp/127.0.0.1/8888 >/dev/null 2>/dev/null
}

do_5()
{
  echo "15" > /dev/udp/127.0.0.1/8888 >/dev/null 2>/dev/null
}

do_6()
{
  echo "16" > /dev/udp/127.0.0.1/8888 >/dev/null 2>/dev/null
}

do_6()
{
  echo "17" > /dev/udp/127.0.0.1/8888 >/dev/null 2>/dev/null
}


do_update()
{
  /home/pi/atv-rptr/scripts/check_for_update.sh
}


do_info()
{
  /home/pi/atv-rptr/scripts/display_info.sh
}


do_cmd_line_repeater()
{
  reset
  cd /home/pi
  /home/pi/atv-rptr/utils/urptr.sh
  printf "\nPress any key to return to the menu\n"
  read -n 1
}

do_utils()
{
  status=0
  while [ "$status" -eq 0 ] 
  do
    menuchoice=$(whiptail --title "Select Ryde Utility" --menu "Select Choice" 20 78 7 \
    "4 Info" "Display System Information" \
    "6 Rptr Test" "Run Repeater with the ability to read errors" \
	"7 Main Menu" "Go back to the Main Menu" \
      3>&2 2>&1 1>&3)
    case "$menuchoice" in
      4\ *) do_info ;;
      6\ *) do_cmd_line_ryde ;;
	  7\ *) status=1 ;;
    esac
  done
  status=0
}


do_Restore_Factory()
{
  cp /home/pi//home/pi/atv-rptr/configs/factory_config.yaml /home/pi/atv-rptr/configs/config.yaml
  # Wait here until user presses a key
  whiptail --title "Factory Setting Restored" --msgbox "Touch any key to continue." 8 78
}


do_SD_Button()
{
  Radio1=OFF
  Radio2=OFF

  # Check current status
  # If /home/pi/.pi-sdn exists then it is loaded

  if test -f "/home/pi/.pi-sdn"; then
    Radio1=ON
  else
    Radio2=ON
  fi
  SD_BUTTON=$(whiptail --title "Select Hardware Shutdown Function" --radiolist \
    "Highlight choice, select with space bar and then press enter" 20 78 5 \
    "SHUTDOWN" "RPi Immediately Shuts Down" $Radio1 \
    "DO NOTHING" "Nothing happens" $Radio2 \
    3>&2 2>&1 1>&3)
  if [ $? -eq 0 ]; then
    if [ "$SD_BUTTON" == "SHUTDOWN" ]; then
      cp /home/pi/ryde-build/text.pi-sdn /home/pi/.pi-sdn  ## Load it at logon
      /home/pi/.pi-sdn                                     ## Load it now
    else
      rm /home/pi/.pi-sdn >/dev/null 2>/dev/null           ## Stop it being loaded at log-on
      sudo pkill -x pi-sdn                                 ## kill the current process
    fi
  fi
}

do_Check_HDMI()
{
  reset
  tvservice -n
  tvservice -s
  cd /home/pi
  read -p "Press enter to continue"
}


do_Settings()
{
  menuchoice=$(whiptail --title "Advanced Settings Menu" --menu "Select Choice and press enter" 16 78 8 \
    "2 Restore Factory" "Reset all settings to default" \
    "3 Check HDMI" "List HDMI settings for fault-finding" \
    "5 Power Button" "Set behaviour on double press of power button" \
    "6 Daily Reboot" "Enable 12-hourly reboot for Repeater Operation" \
    "7 Stop Reboot" "Disable 12-hourly reboot for Repeater Operation" \
    "8 Hardware Shutdown" "Enable or disable hardware shutdown function" \
      3>&2 2>&1 1>&3)
    case "$menuchoice" in
      2\ *) do_Restore_Factory ;;
      3\ *) do_Check_HDMI ;;
      5\ *) do_Power_Button ;;
      6\ *) sudo crontab /home/pi/ryde-build/configs/rptrcron ;;
      7\ *) sudo crontab /home/pi/ryde-build/configs/blankcron ;;
      8\ *) do_SD_Button ;;
    esac
}



do_Reboot()
{
  sudo reboot now
}


do_Nothing()
{
  :
}


do_Norm_HDMI()
{
  # change #enable_tvout=1 or enable_tvout=1 to #enable_tvout=1

  # first check if "#enable_tvout=1" is in /boot/config.txt
  grep -q "#enable_tvout=1" /boot/config.txt
  if [ $? -ne 0 ]; then  #  "#enable_tvout=1" is not there, so check if "enable_tvout=1" is there
    grep -q "enable_tvout=1" /boot/config.txt
    if [ $? -eq 0 ]; then  # "enable_tvout=1" is there, so replace it with "#enable_tvout=1"
      sudo sed -i '/enable_tvout=1/c\#enable_tvout=1' /boot/config.txt  >/dev/null 2>/dev/null
    else                   # not there, so append the commented statement
      sudo bash -c 'echo " " >> /boot/config.txt '
      sudo bash -c 'echo "# Uncomment to enable Comp Vid output" >> /boot/config.txt '
      sudo bash -c 'echo "#enable_tvout=1" >> /boot/config.txt '
      sudo bash -c 'echo " " >> /boot/config.txt '
    fi
  fi

  # change #hdmi_safe=1 or hdmi_safe=1 to #hdmi_safe=1

  # first check if "#hdmi_safe=1" is in /boot/config.txt
  grep -q "#hdmi_safe=1" /boot/config.txt
  if [ $? -ne 0 ]; then  #  "#hdmi_safe=1" is not there, so check if "hdmi_safe=1" is there
    grep -q "hdmi_safe=1" /boot/config.txt
    if [ $? -eq 0 ]; then  # "hdmi_safe=1" is there, so replace it with "#hdmi_safe=1"
      sudo sed -i '/hdmi_safe=1/c\#hdmi_safe=1' /boot/config.txt  >/dev/null 2>/dev/null
    else                   # not there, so append the commented statement
      sudo bash -c 'echo " " >> /boot/config.txt '
      sudo bash -c 'echo "# uncomment if you get no picture on HDMI for a default safe mode" >> /boot/config.txt '
      sudo bash -c 'echo "#hdmi_safe=1" >> /boot/config.txt '
      sudo bash -c 'echo " " >> /boot/config.txt '
    fi
  fi
}


do_Safe_HDMI()
{
  # change #enable_tvout=1 or enable_tvout=1 to #enable_tvout=1

  # first check if "#enable_tvout=1" is in /boot/config.txt
  grep -q "#enable_tvout=1" /boot/config.txt
  if [ $? -ne 0 ]; then  #  "#enable_tvout=1" is not there, so check if "enable_tvout=1" is there
    grep -q "enable_tvout=1" /boot/config.txt
    if [ $? -eq 0 ]; then  # "enable_tvout=1" is there, so replace it with "#enable_tvout=1"
      sudo sed -i '/enable_tvout=1/c\#enable_tvout=1' /boot/config.txt  >/dev/null 2>/dev/null
    else                   # not there, so append the commented statement
      sudo bash -c 'echo " " >> /boot/config.txt '
      sudo bash -c 'echo "# Uncomment to enable Comp Vid output" >> /boot/config.txt '
      sudo bash -c 'echo "#enable_tvout=1" >> /boot/config.txt '
      sudo bash -c 'echo " " >> /boot/config.txt '
    fi
  fi

  # change #hdmi_safe=1 or hdmi_safe=1 to hdmi_safe=1

  # first check if "#hdmi_safe=1" is in /boot/config.txt
  grep -q "#hdmi_safe=1" /boot/config.txt
  if [ $? -eq 0 ]; then  #  "#hdmi_safe=1" is there, so change it to "hdmi_safe=1"
    sudo sed -i '/#hdmi_safe=1/c\hdmi_safe=1' /boot/config.txt  >/dev/null 2>/dev/null
  else                   # "#hdmi_safe=1" is not there so check "hdmi_safe=1" is there
    grep -q "hdmi_safe=1" /boot/config.txt
    if [ $? -ne 0 ]; then  # "hdmi_safe=1" is not there, so add it (else do nothing)
      sudo bash -c 'echo " " >> /boot/config.txt '
      sudo bash -c 'echo "# uncomment if you get no picture on HDMI for a default safe mode" >> /boot/config.txt '
      sudo bash -c 'echo "hdmi_safe=1" >> /boot/config.txt '
      sudo bash -c 'echo " " >> /boot/config.txt '
    fi
  fi
}

do_Comp_Vid_Aspect_4_3()
{
  grep -q "sdtv_aspect=1" /boot/config.txt
  if [ $? -ne 0 ]; then  #  "sdtv_aspect=1" is not there so change it
    sudo sed -i 's/sdtv_aspect=3/sdtv_aspect=1/' /boot/config.txt
  fi
}

do_Comp_Vid_Aspect_16_9()
{
  grep -q "sdtv_aspect=3" /boot/config.txt
  if [ $? -ne 0 ]; then  #  "sdtv_aspect=3" is not there so change it
    sudo sed -i 's/sdtv_aspect=1/sdtv_aspect=3/' /boot/config.txt
  fi
}


do_Comp_Vid_Aspect()
{
  Radio1=OFF
  Radio2=OFF
  Radio3=OFF

  grep -q "sdtv_aspect=1" /boot/config.txt
  if [ $? -eq 0 ]; then  #  "sdtv_aspect=1" 4:3
    Radio1=ON
  else
    grep -q "sdtv_aspect=3" /boot/config.txt
    if [ $? -eq 0 ]; then  #  "sdtv_aspect=3" 16:9
      Radio2=ON
    else
      Radio3=ON
    fi
  fi

  NEW_VA=$(whiptail --title "Choose the Composite Video Aspect Ratio" --radiolist \
    "Highlight choice, select with space bar and then press enter" 20 78 5 \
    "4:3" "4:3 for older displays" $Radio1 \
    "16:9" "16:9 for widescreen displays" $Radio2 \
    "Not Set" "Defaults to 4:3" $Radio3 \
    3>&2 2>&1 1>&3)
  if [ $? -eq 0 ]; then
    case "$NEW_VA" in
      "4:3")
        do_Comp_Vid_Aspect_4_3
      ;;
      "16:9")
        do_Comp_Vid_Aspect_16_9
      ;;
    esac
  fi
}


do_Comp_Vid_PAL()
{
  # change #hdmi_safe=1 or hdmi_safe=1 to #hdmi_safe=1

  # first check if "#hdmi_safe=1" is in /boot/config.txt
  grep -q "#hdmi_safe=1" /boot/config.txt
  if [ $? -ne 0 ]; then  #  "#hdmi_safe=1" is not there, so check if "hdmi_safe=1" is there
    grep -q "hdmi_safe=1" /boot/config.txt
    if [ $? -eq 0 ]; then  # "hdmi_safe=1" is there, so replace it with "#hdmi_safe=1"
      sudo sed -i '/hdmi_safe=1/c\#hdmi_safe=1' /boot/config.txt  >/dev/null 2>/dev/null
    else                   # not there, so append the commented statement
      sudo bash -c 'echo " " >> /boot/config.txt '
      sudo bash -c 'echo "# uncomment if you get no picture on HDMI for a default safe mode" >> /boot/config.txt '
      sudo bash -c 'echo "#hdmi_safe=1" >> /boot/config.txt '
      sudo bash -c 'echo " " >> /boot/config.txt '
    fi
  fi

  #change #sdtv_mode=1/2 or sdtv_mode=1/2 to sdtv_mode=2

  # first check if "#sdtv_mode=2" is in /boot/config.txt
  grep -q "#sdtv_mode=2" /boot/config.txt
  if [ $? -eq 0 ]; then  #  "#sdtv_mode=2" is there, so change it to "sdtv_mode=2"
    sudo sed -i '/#sdtv_mode=2/c\sdtv_mode=2' /boot/config.txt  >/dev/null 2>/dev/null
  else                   # "#sdtv_mode=2" is not there so check if "#sdtv_mode=1" is there
    grep -q "#sdtv_mode=1" /boot/config.txt
    if [ $? -eq 0 ]; then  #  "#sdtv_mode=1" is there, so change it to "sdtv_mode=2"
      sudo sed -i '/#sdtv_mode=1/c\sdtv_mode=2' /boot/config.txt  >/dev/null 2>/dev/null
    else                   # neither "#sdtv_mode=2" nor "#sdtv_mode=1" are there
      grep -q "sdtv_mode=1" /boot/config.txt  # so check if "sdtv_mode=1" is there
      if [ $? -eq 0 ]; then  #  "sdtv_mode=1" is there, so change it to "sdtv_mode=2"
        sudo sed -i '/sdtv_mode=1/c\sdtv_mode=2' /boot/config.txt  >/dev/null 2>/dev/null
      else       # check if "sdtv_mode=2" is there and add it if not
        grep -q "sdtv_mode=2" /boot/config.txt  
        if [ $? -ne 0 ]; then  # "sdtv_mode=2" is not there, so add it at the end (else do nothing)
          sudo bash -c 'echo " " >> /boot/config.txt '
          sudo bash -c 'echo "# uncomment for composite PAL" >> /boot/config.txt '
          sudo bash -c 'echo "sdtv_mode=2" >> /boot/config.txt '
          sudo bash -c 'echo " " >> /boot/config.txt '
        fi
      fi
    fi
  fi

  # change #enable_tvout=1 or enable_tvout=1 to enable_tvout=1 (Add if not present)

  # first check if "#enable_tvout=1" is in /boot/config.txt
  grep -q "#enable_tvout=1" /boot/config.txt
  if [ $? -eq 0 ]; then  # "#enable_tvout=1" is there, so replace it with "enable_tvout=1"
    sudo sed -i '/#enable_tvout=1/c\enable_tvout=1' /boot/config.txt  >/dev/null 2>/dev/null
  else                   # "#enable_tvout=1" is not there, so check for "enable_tvout=1"
    grep -q "enable_tvout=1" /boot/config.txt
    if [ $? -ne 0 ]; then  #  "enable_tvout=1" is not there, so add it at the end (else do nothing)
      sudo bash -c 'echo " " >> /boot/config.txt '
      sudo bash -c 'echo "# Uncomment to enable Comp Vid output" >> /boot/config.txt '
      sudo bash -c 'echo "enable_tvout=1" >> /boot/config.txt '
      sudo bash -c 'echo " " >> /boot/config.txt '
    fi
  fi

  # Ask about aspect ratio
  do_Comp_Vid_Aspect
}


do_Comp_Vid_NTSC()
{
  # change #hdmi_safe=1 or hdmi_safe=1 to #hdmi_safe=1

  # first check if "#hdmi_safe=1" is in /boot/config.txt
  grep -q "#hdmi_safe=1" /boot/config.txt
  if [ $? -ne 0 ]; then  #  "#hdmi_safe=1" is not there, so check if "hdmi_safe=1" is there
    grep -q "hdmi_safe=1" /boot/config.txt
    if [ $? -eq 0 ]; then  # "hdmi_safe=1" is there, so replace it with "#hdmi_safe=1"
      sudo sed -i '/hdmi_safe=1/c\#hdmi_safe=1' /boot/config.txt  >/dev/null 2>/dev/null
    else                   # not there, so append the commented statement
      sudo bash -c 'echo " " >> /boot/config.txt '
      sudo bash -c 'echo "# uncomment if you get no picture on HDMI for a default safe mode" >> /boot/config.txt '
      sudo bash -c 'echo "#hdmi_safe=1" >> /boot/config.txt '
      sudo bash -c 'echo " " >> /boot/config.txt '
    fi
  fi

  #change #sdtv_mode=1/2 or sdtv_mode=1/2 to sdtv_mode=1

  # first check if "#sdtv_mode=1" is in /boot/config.txt
  grep -q "#sdtv_mode=1" /boot/config.txt
  if [ $? -eq 0 ]; then  #  "#sdtv_mode=1" is there, so change it to "sdtv_mode=1"
    sudo sed -i '/#sdtv_mode=1/c\sdtv_mode=1' /boot/config.txt  >/dev/null 2>/dev/null
  else                   # "#sdtv_mode=1" is not there so check if "#sdtv_mode=2" is there
    grep -q "#sdtv_mode=2" /boot/config.txt
    if [ $? -eq 0 ]; then  #  "#sdtv_mode=2" is there, so change it to "sdtv_mode=1"
      sudo sed -i '/#sdtv_mode=2/c\sdtv_mode=1' /boot/config.txt  >/dev/null 2>/dev/null
    else                   # neither "#sdtv_mode=1" nor "#sdtv_mode=2" are there
      grep -q "sdtv_mode=2" /boot/config.txt  # so check if "sdtv_mode=2" is there
      if [ $? -eq 0 ]; then  #  "sdtv_mode=2" is there, so change it to "sdtv_mode=1"
        sudo sed -i '/sdtv_mode=2/c\sdtv_mode=1' /boot/config.txt  >/dev/null 2>/dev/null
      else       # check if "sdtv_mode=1" is there and add it if not
        grep -q "sdtv_mode=1" /boot/config.txt  
        if [ $? -ne 0 ]; then  # "sdtv_mode=1" is not there, so add it at the end (else do nothing)
          sudo bash -c 'echo " " >> /boot/config.txt '
          sudo bash -c 'echo "# uncomment for composite PAL" >> /boot/config.txt '
          sudo bash -c 'echo "sdtv_mode=1" >> /boot/config.txt '
          sudo bash -c 'echo " " >> /boot/config.txt '
        fi
      fi
    fi
  fi

  # change #enable_tvout=1 or enable_tvout=1 to enable_tvout=1 (Add if not present)

  # first check if "#enable_tvout=1" is in /boot/config.txt
  grep -q "#enable_tvout=1" /boot/config.txt
  if [ $? -eq 0 ]; then  # "#enable_tvout=1" is there, so replace it with "enable_tvout=1"
    sudo sed -i '/#enable_tvout=1/c\enable_tvout=1' /boot/config.txt  >/dev/null 2>/dev/null
  else                   # "#enable_tvout=1" is not there, so check for "enable_tvout=1"
    grep -q "enable_tvout=1" /boot/config.txt
    if [ $? -ne 0 ]; then  #  "enable_tvout=1" is not there, so add it at the end (else do nothing)
      sudo bash -c 'echo " " >> /boot/config.txt '
      sudo bash -c 'echo "# Uncomment to enable Comp Vid output" >> /boot/config.txt '
      sudo bash -c 'echo "enable_tvout=1" >> /boot/config.txt '
      sudo bash -c 'echo " " >> /boot/config.txt '
    fi
  fi

  # Ask about aspect ratio
  do_Comp_Vid_Aspect
}

do_Audio_Jack()
{
  Radio1=OFF
  Radio2=OFF
  Radio3=OFF
  Radio4=OFF

  grep -q "^#        vlcArgs += '--gain 4 --alsa-audio-device hw:CARD=Headphones,DEV=0 '" \
    /home/pi/ryde/rydeplayer/player.py
  if [ $? -eq 0 ]; then  #  Line commented, so HDMI currently selected
    Radio1=ON
  else
    grep -q "^        vlcArgs += '--gain 4 --alsa-audio-device hw:CARD=Headphones,DEV=0 '" \
      /home/pi/ryde/rydeplayer/player.py
    if [ $? -eq 0 ]; then  #  RPi Jack currently selected
      Radio2=ON
    else
      grep -q "^        vlcArgs += '--gain 4 --alsa-audio-device hw:CARD=Device,DEV=0 '" \
        /home/pi/ryde/rydeplayer/player.py
      if [ $? -eq 0 ]; then  #  USB Dongle currently selected
        Radio3=ON
      else
        Radio4=ON
      fi
    fi
  fi

  AUDIO_JACK=$(whiptail --title "Choose the Audio Output Port" --radiolist \
    "Highlight choice, select with space bar and then press enter" 20 78 6 \
    "HDMI" "Audio on HDMI" $Radio1 \
    "RPi Jack" "Audio on the RPi 3.5 mm Jack" $Radio2 \
    "USB" "Audio on a White USB Dongle" $Radio3 \
    "Not Set" "Defaults to same as video" $Radio4 \
    3>&2 2>&1 1>&3)
  if [ $? -eq 0 ]; then
    case "$AUDIO_JACK" in
      "HDMI")
        sed -i "/--alsa-audio-device/c\#        vlcArgs += '--gain 4 --alsa-audio-device hw:CARD=Headphones,DEV=0 '" \
          /home/pi/ryde/rydeplayer/player.py
      ;;
      "RPi Jack")
        sed -i "/--alsa-audio-device/c\        vlcArgs += '--gain 4 --alsa-audio-device hw:CARD=Headphones,DEV=0 '" \
          /home/pi/ryde/rydeplayer/player.py
      ;;
      "USB")
        sed -i "/--alsa-audio-device/c\        vlcArgs += '--gain 4 --alsa-audio-device hw:CARD=Device,DEV=0 '" \
          /home/pi/ryde/rydeplayer/player.py
      ;;
    esac
  fi
  REBOOT_REQUIRED=no
}


do_video_change()
{
  REBOOT_REQUIRED=yes
  menuchoice=$(whiptail --title "Video Output Menu" --menu "Select Choice" 16 78 6 \
    "1 Normal HDMI" "Recommended Mode"  \
    "2 HDMI Safe Mode" "Use for HDMI Troubleshooting" \
    "3 PAL Composite Video" "Use the RPi Video Output Jack" \
    "4 NTSC Composite Video" "Use the RPi Video Output Jack" \
    "5 Audio Output" "Change Audio Output Destination" \
      3>&2 2>&1 1>&3)
    case "$menuchoice" in
        1\ *) do_Norm_HDMI ;;
        2\ *) do_Safe_HDMI ;;
        3\ *) do_Comp_Vid_PAL ;;
        4\ *) do_Comp_Vid_NTSC ;;
        5\ *) do_Audio_Jack ;;
    esac

  if [ "$REBOOT_REQUIRED" == "yes" ]; then
    menuchoice=$(whiptail --title "Reboot Now?" --menu "Reboot to Apply Changes?" 16 78 10 \
      "1 Yes" "Immediate Reboot"  \
      "2 No" "Apply changes at next Reboot" \
        3>&2 2>&1 1>&3)
      case "$menuchoice" in
        1\ *) do_Reboot ;;
        2\ *) do_Nothing ;;
      esac
  fi
}


do_Shutdown2()
{
  sudo shutdown now
}

do_Shutdown()
{
  menuchoice=$(whiptail --title "REALLY SHUTDOWN? SITE VISIT??" --menu "Select Choice and press enter" 16 78 4 \
    "1 Back" "Return to Main Menu" \
    "2 Reboot now" "Immediate Reboot" \
    "3 Shutdown now" "Immediate Shutdown" \
      3>&2 2>&1 1>&3)
    case "$menuchoice" in
      1\ *) ;;
      2\ *) do_Reboot ;;
      3\ *) do_Shutdown2 ;;
    esac
}




do_Exit()
{
  exit
}


do_shutdown_menu()
{
  menuchoice=$(whiptail --title "Shutdown Menu" --menu "Select Choice and press enter" 16 78 4 \
    "1 Exit to Linux" "Exit Menu to Command Prompt" \
    "2 Reboot now" "Immediate Reboot" \
    "3 Shutdown now" "Immediate Shutdown.  Really?"  \
      3>&2 2>&1 1>&3)
    case "$menuchoice" in
      1\ *) do_Exit ;;
      2\ *) do_Reboot ;;
      3\ *) do_Shutdown ;;
    esac
}


do_stop()
{
  sudo killall python3 >/dev/null 2>/dev/null
  
  sleep 0.3
  if pgrep -x "python3" >/dev/null 2>/dev/null
  then
    sudo killall -9 python3 >/dev/null 2>/dev/null
  fi
}


do_receive()
{
  /home/pi/ryde-build/rx.sh &

  # Wait here receiving until user presses a key
  whiptail --title "Receiving" --msgbox "Touch any key to stop receiving" 8 78
  do_stop
}


#********************************************* MAIN MENU *********************************
#************************* Execution of Console Menu starts here *************************

status=0

# Loop round main menu
#while [ "$status" -eq 0 ] 
#  do
    # Display main menu

#    menuchoice=$(whiptail --title "BATC Ryde Receiver Main Menu" --menu "Select Choice and press enter:" 20 78 13 \
#	"0 Receive" "Start the Ryde Receiver" \
#    "1 Stop" "Stop the Ryde Receiver" \
#    "2 Start-up" "Set the start-up Preset" \
#    "3 Bands" "Set the band details such as LNB Offset" \
#    "4 Presets" "Set the details for each preset" \
#	"5 Video" "Select the Video and Audio Output Mode" \
#	"6 Remote" "Select the Remote Control Type" \
#	"7 IR Check" "View the IR Codes From a new Remote" \
#    "8 Settings" "Advanced Settings" \
#	"9 Utils" "Ryde Utilities and Stream Viewer" \
#	"10 Update" "Check for Software Update" \
#	"11 DVB-T RX" "Menu-driven DVB-T RX using Knucker Tuner"\
#    "12 Shutdown" "Shutdown, Reboot or exit to the Linux command prompt" \
# 	3>&2 2>&1 1>&3)

#        case "$menuchoice" in
#	    0\ *) do_receive   ;;
#        1\ *) do_stop   ;;
#        2\ *) do_Set_Defaults ;;
#        3\ *) do_Set_Bands ;;
#        4\ *) do_Set_Presets ;;
#	    5\ *) do_video_change ;;
#   	    6\ *) do_Set_RC_Type ;;
#   	    7\ *) do_Check_RC_Codes ;;
#	    8\ *) do_Settings ;;
#        9\ *) do_utils ;;
#	    10\ *) do_update ;;
#	    11\ *) do_dvbt ;;
#        12\ *) do_shutdown_menu ;;
#            *)

        # Display exit message if user jumps out of menu
#        whiptail --title "Exiting to Linux Prompt" --msgbox "To return to the menu system, type menu" 8 78

        # Set status to exit
#        status=1

        # Sleep while user reads message, then exit
        #sleep 1
#      exit ;;
#    esac
#  done
#exit


while [ "$status" -eq 0 ] 
  do
    # Display main menu

    menuchoice=$(whiptail --title "BATC Repeater Controller Main Menu" --menu "Select Choice and press enter:" 20 78 13 \
	"0 Repeat" "Select Normal Repeater Operation Mode" \
    "1 Status" "Enter Status Display Mode" \
    "2 Input 0" "Show Input 0 = HDMI Port 1 (Controller)" \
    "3 Input 1" "Show Input 1 = HDMI Port 2" \
    "4 Input 2" "Show Input 2 = HDMI Port 3" \
    "5 Input 3" "Show Input 3 = HDMI Port 4" \
    "6 Input 4" "Show Input 4 = HDMI Port 5" \
    "7 Input 5" "Show Input 5 = HDMI Port 6" \
    "8 Input 6" "Show Input 6 = HDMI Port 7" \
    "9 Input 7" "Show Input 7 = HDMI Port 8" \
    "12 Shutdown" "Shutdown, Reboot or exit to the Linux command prompt" \
 	3>&2 2>&1 1>&3)

        case "$menuchoice" in
	    0\ *) do_normal_repeat ;;
        1\ *) do_status ;;
        12\ *) do_shutdown_menu ;;
            *)

        # Display exit message if user jumps out of menu
        whiptail --title "Exiting to Linux Prompt" --msgbox "To return to the menu system, type menu" 8 78

        # Set status to exit
        status=1

        # Sleep while user reads message, then exit
        sleep 1
      exit ;;
    esac
  done
exit
