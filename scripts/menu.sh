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

    #"5 Power Button" "Set behaviour on double press of power button" \
    #"6 Daily Reboot" "Enable 12-hourly reboot for Repeater Operation" \
    #"7 Stop Reboot" "Disable 12-hourly reboot for Repeater Operation" \
    #"8 Hardware Shutdown" "Enable or disable hardware shutdown function" \

      #5\ *) do_Power_Button ;;
      #6\ *) sudo crontab /home/pi/ryde-build/configs/rptrcron ;;
      #7\ *) sudo crontab /home/pi/ryde-build/configs/blankcron ;;
      #8\ *) do_SD_Button ;;

do_show_update_log()
{
  reset
  more /var/log/rptr/update_log.txt
  printf "\nPress any key to return to the menu\n"
  read -n 1
}


do_show_build_log()
{
  reset
  more /var/log/rptr/initial_build_log.txt
  printf "\nPress any key to return to the menu\n"
  read -n 1
}


do_show_log()
{
  reset
  more /var/log/rptr/error_log.txt
  printf "\nPress any key to return to the menu\n"
  read -n 1
}


do_info()
{
  more /home/pi/atv-rptr/config/repeater_config.txt
  printf "\nPress any key to return to the menu\n"
  read -n 1
}


do_cmd_line_repeater()
{
  reset
  cd /home/pi
  /home/pi/atv-rptr/utils/update_rptr.sh
  printf "\nPress any key to return to the menu\n"
  read -n 1
}

do_diagnostics()
{
  status=0
  while [ "$status" -eq 0 ] 
  do
    menuchoice=$(whiptail --title "Repeater Diagnostics Menu" --menu "Select Choice" 20 78 7 \
    "1 Config" "Show Repeater Configuration File" \
    "2 Log File" "Show Repeater Error Log File" \
    "3 Build Log" "Show Initial Build Log File" \
    "4 Update Log" "Show Latest Update Log File" \
    "5 Rptr Test" "Run Repeater with the ability to read errors" \
	"6 Main Menu" "Go back to the Main Menu" \
      3>&2 2>&1 1>&3)
    case "$menuchoice" in
      1\ *) do_info ;;
      2\ *) do_show_log ;;
      3\ *) do_show_build_log ;;
      4\ *) do_show_update_log ;;
      5\ *) do_cmd_line_repeater ;;
	  6\ *) status=1 ;;
    esac
  done
  status=0
}


do_Check_HDMI()
{
  reset
  tvservice -n
  tvservice -s
  cd /home/pi
  read -p "Press enter to continue"
}


do_Restore_Factory()
{
  cp /home/pi/home/pi/atv-rptr/config/repeater_config.txt.factory /home/pi/home/pi/atv-rptr/config/repeater_config
  # Wait here until user presses a key
  whiptail --title "Factory Setting Restored" --msgbox "Touch any key to continue." 8 78
}


do_Settings()
{
  status=0
  while [ "$status" -eq 0 ] 
  do
    menuchoice=$(whiptail --title "Advanced Settings Menu" --menu "Select Choice and press enter" 16 78 8 \
      "1 Restore Factory" "Reset all settings to default" \
      "2 Check HDMI" "List HDMI settings for fault-finding" \
	  "3 Main Menu" "Go back to the Main Menu" \
        3>&2 2>&1 1>&3)
      case "$menuchoice" in
        1\ *) do_Restore_Factory ;;
        2\ *) do_Check_HDMI ;;
	    3\ *) status=1 ;;
      esac
  done
  status=0
}


do_update()
{
  /home/pi/atv-rptr/scripts/check_for_update.sh
}


do_update_menu()
{
  INSTALLEDVERSION=$(head -c 9 /home/pi/atv-rptr/config/installed_version.txt)
  status=0
  while [ "$status" -eq 0 ] 
  do
    menuchoice=$(whiptail --title "Current Version is "$INSTALLEDVERSION"" --menu "Select Choice" 20 78 7 \
    "1 Update" "Update Repeater Software" \
	"2 Main Menu" "Go back to the Main Menu" \
      3>&2 2>&1 1>&3)
    case "$menuchoice" in
      1\ *) do_update ;;
	  2\ *) status=1 ;;
    esac
  done
  status=0
}


do_normal_repeat()
{
  echo "00" > /dev/udp/127.0.0.1/8888 #>/dev/null 2>/dev/null
}

do_status()
{
  echo "01" > /dev/udp/127.0.0.1/8888 #>/dev/null 2>/dev/null
}

do_quad()
{
  echo "04" > /dev/udp/127.0.0.1/8888 #>/dev/null 2>/dev/null
}

do_0()
{
  echo "10" > /dev/udp/127.0.0.1/8888 #>/dev/null 2>/dev/null
}

do_1()
{
  echo "11" > /dev/udp/127.0.0.1/8888 #>/dev/null 2>/dev/null
}

do_2()
{
  echo "12" > /dev/udp/127.0.0.1/8888 #>/dev/null 2>/dev/null
}

do_3()
{
  echo "13" > /dev/udp/127.0.0.1/8888 #>/dev/null 2>/dev/null
}

do_4()
{
  echo "14" > /dev/udp/127.0.0.1/8888 #>/dev/null 2>/dev/null
}

do_5()
{
  echo "15" > /dev/udp/127.0.0.1/8888 #>/dev/null 2>/dev/null
}

do_6()
{
  echo "16" > /dev/udp/127.0.0.1/8888 #>/dev/null 2>/dev/null
}

do_7()
{
  echo "17" > /dev/udp/127.0.0.1/8888 #>/dev/null 2>/dev/null
}

do_input_control()
{
  status="0"
  while [ "$status" -eq 0 ] 
  do
    INPUT1NAME=$(get_config_var input1name $CONFIGFILE)
    INPUT2NAME=$(get_config_var input2name $CONFIGFILE)
    INPUT3NAME=$(get_config_var input3name $CONFIGFILE)
    INPUT4NAME=$(get_config_var input4name $CONFIGFILE)
    INPUT5NAME=$(get_config_var input5name $CONFIGFILE)
    INPUT6NAME=$(get_config_var input6name $CONFIGFILE)
    INPUT7NAME=$(get_config_var input7name $CONFIGFILE)

    menuchoice=$(whiptail --title "Direct Input Control Menu" --menu "Select Choice and press enter" 20 78 12 \
      "1 Input 1" "$INPUT1NAME" \
      "2 Input 2" "$INPUT2NAME" \
      "3 Input 3" "$INPUT3NAME" \
      "4 Input 4" "$INPUT4NAME" \
      "5 Input 5" "$INPUT5NAME" \
      "6 Input 6" "$INPUT6NAME" \
      "7 Input 7" "$INPUT7NAME" \
      "8 Controller" "Show Controller Screen" \
      "9 Status" "Show Status Screen" \
      "10 Quad" "Show Quad View" \
      "11 Normal" "Normal Repeater Operation" \
      "12 Main menu" "Return to Main Menu" \
        3>&2 2>&1 1>&3)
      case "$menuchoice" in
        1\ *) do_1 ;;
        2\ *) do_2 ;;
        3\ *) do_3 ;;
        4\ *) do_4 ;;
        5\ *) do_5 ;;
        6\ *) do_6 ;;
        7\ *) do_7 ;;
        8\ *) do_0 ;;
        9\ *) do_status ;;
        10\ *) do_quad ;;
        11\ *) do_normal_repeat ;;
        12\ *) status="1" ;;
      esac
    done
  status="0"
}


do_callsign()
{
  CALL=$(get_config_var callsign $CONFIGFILE)
  CALL=$(whiptail --inputbox "Enter Callsign (no spaces)" 8 78 $CALL --title "Set Repeater Callsign" 3>&1 1>&2 2>&3)
  if [ $? -eq 0 ]; then
    set_config_var callsign "$CALL" $CONFIGFILE
  fi

  LOCATOR=$(get_config_var locator $CONFIGFILE)
  LOCATOR=$(whiptail --inputbox "Enter Locator (no spaces)" 8 78 $LOCATOR --title "Set Repeater Locator" 3>&1 1>&2 2>&3)
  if [ $? -eq 0 ]; then
    set_config_var locator "$LOCATOR" $CONFIGFILE
  fi
}

do_Active_Hold()
{
  ACTIVE_HOLD=$(get_config_var activeinputhold $CONFIGFILE)
  if [[ "$ACTIVE_HOLD" == "yes" ]];
  then
    Radio1=ON
    Radio2=OFF
  else
    Radio1=OFF
    Radio2=ON
  fi

  ACTIVE_HOLD=$(whiptail --title "Choose Whether to Hold an Active Input" --radiolist \
    "Highlight choice, select with space bar and then press enter" 20 78 6 \
    "yes" "Active Input retains Repeater for all except Priority 1 Inputs" $Radio1 \
    "no" "Active Input is dropped in favour of any higher priority Input" $Radio2 \
    3>&2 2>&1 1>&3)
  if [ $? -eq 0 ]; then
    set_config_var activeinputhold $ACTIVE_HOLD $CONFIGFILE
  fi
}


do_Input_Count()
{
  INPUT_COUNT=$(get_config_var availableinputs $CONFIGFILE)
  INPUT_COUNT=$(whiptail --inputbox "Enter the number of active inputs" 8 78 $INPUT_COUNT --title "Set Number of Inputs" 3>&1 1>&2 2>&3)
  if [ $? -eq 0 ]; then
    if [ $INPUT_COUNT -ge 1 ] && [ $INPUT_COUNT -le 7 ]; then
      set_config_var availableinputs "$INPUT_COUNT" $CONFIGFILE
    else
      whiptail --title "ERROR" --msgbox "Please enter an input count between 1 and 7.  Press enter to continue" 8 78
    fi
  fi
}


do_Priority1()
{
  INPUT1PRIORITY=$(get_config_var input1prioritylevel $CONFIGFILE)
  INPUT1NAME=$(get_config_var input1name $CONFIGFILE)
  INPUT1PRIORITY=$(whiptail --inputbox "$INPUT1NAME Priority:" 8 78 $INPUT1PRIORITY --title "Set Input 1 Priority (9 to disable)" 3>&1 1>&2 2>&3)
  if [ $? -eq 0 ]; then
    if [ $INPUT1PRIORITY -ge 1 ] && [ $INPUT1PRIORITY -le 9 ]; then
      set_config_var input1prioritylevel "$INPUT1PRIORITY" $CONFIGFILE
    else
      whiptail --title "ERROR" --msgbox "Please enter an input priority between 1 and 9.  Press enter to continue" 8 78
    fi
  fi
}


do_Priority2()
{
  INPUT2PRIORITY=$(get_config_var input2prioritylevel $CONFIGFILE)
  INPUT2NAME=$(get_config_var input2name $CONFIGFILE)
  INPUT2PRIORITY=$(whiptail --inputbox "$INPUT2NAME Priority:" 8 78 $INPUT2PRIORITY --title "Set Input 2 Priority (9 to disable)" 3>&1 1>&2 2>&3)
  if [ $? -eq 0 ]; then
    if [ $INPUT2PRIORITY -ge 1 ] && [ $INPUT2PRIORITY -le 9 ]; then
      set_config_var input2prioritylevel "$INPUT2PRIORITY" $CONFIGFILE
    else
      whiptail --title "ERROR" --msgbox "Please enter an input priority between 1 and 9.  Press enter to continue" 8 78
    fi
  fi
}


do_Priority3()
{
  INPUT3PRIORITY=$(get_config_var input3prioritylevel $CONFIGFILE)
  INPUT3NAME=$(get_config_var input3name $CONFIGFILE)
  INPUT3PRIORITY=$(whiptail --inputbox "$INPUT3NAME Priority:" 8 78 $INPUT3PRIORITY --title "Set Input 3 Priority (9 to disable)" 3>&1 1>&2 2>&3)
  if [ $? -eq 0 ]; then
    if [ $INPUT3PRIORITY -ge 1 ] && [ $INPUT3PRIORITY -le 9 ]; then
      set_config_var input3prioritylevel "$INPUT3PRIORITY" $CONFIGFILE
    else
      whiptail --title "ERROR" --msgbox "Please enter an input priority between 1 and 9.  Press enter to continue" 8 78
    fi
  fi
}


do_Priority4()
{
  INPUT4PRIORITY=$(get_config_var input4prioritylevel $CONFIGFILE)
  INPUT4NAME=$(get_config_var input4name $CONFIGFILE)
  INPUT4PRIORITY=$(whiptail --inputbox "$INPUT4NAME Priority:" 8 78 $INPUT4PRIORITY --title "Set Input 4 Priority (9 to disable)" 3>&1 1>&2 2>&3)
  if [ $? -eq 0 ]; then
    if [ $INPUT4PRIORITY -ge 1 ] && [ $INPUT4PRIORITY -le 9 ]; then
      set_config_var input4prioritylevel "$INPUT4PRIORITY" $CONFIGFILE
    else
      whiptail --title "ERROR" --msgbox "Please enter an input priority between 1 and 9.  Press enter to continue" 8 78
    fi
  fi
}


do_Priority5()
{
  INPUT5PRIORITY=$(get_config_var input5prioritylevel $CONFIGFILE)
  INPUT5NAME=$(get_config_var input5name $CONFIGFILE)
  INPUT5PRIORITY=$(whiptail --inputbox "$INPUT5NAME Priority:" 8 78 $INPUT5PRIORITY --title "Set Input 5 Priority (9 to disable)" 3>&1 1>&2 2>&3)
  if [ $? -eq 0 ]; then
    if [ $INPUT5PRIORITY -ge 1 ] && [ $INPUT5PRIORITY -le 9 ]; then
      set_config_var input5prioritylevel "$INPUT5PRIORITY" $CONFIGFILE
    else
      whiptail --title "ERROR" --msgbox "Please enter an input priority between 1 and 9.  Press enter to continue" 8 78
    fi
  fi
}


do_Priority6()
{
  INPUT6PRIORITY=$(get_config_var input6prioritylevel $CONFIGFILE)
  INPUT6NAME=$(get_config_var input6name $CONFIGFILE)
  INPUT6PRIORITY=$(whiptail --inputbox "$INPUT6NAME Priority:" 8 78 $INPUT6PRIORITY --title "Set Input 6 Priority (9 to disable)" 3>&1 1>&2 2>&3)
  if [ $? -eq 0 ]; then
    if [ $INPUT6PRIORITY -ge 1 ] && [ $INPUT6PRIORITY -le 9 ]; then
      set_config_var input6prioritylevel "$INPUT6PRIORITY" $CONFIGFILE
    else
      whiptail --title "ERROR" --msgbox "Please enter an input priority between 1 and 9.  Press enter to continue" 8 78
    fi
  fi
}


do_Priority7()
{
  INPUT7PRIORITY=$(get_config_var input7prioritylevel $CONFIGFILE)
  INPUT7NAME=$(get_config_var input7name $CONFIGFILE)
  INPUT7PRIORITY=$(whiptail --inputbox "$INPUT7NAME Priority:" 8 78 $INPUT7PRIORITY --title "Set Input 7 Priority (9 to disable)" 3>&1 1>&2 2>&3)
  if [ $? -eq 0 ]; then
    if [ $INPUT7PRIORITY -ge 1 ] && [ $INPUT7PRIORITY -le 9 ]; then
      set_config_var input7prioritylevel "$INPUT7PRIORITY" $CONFIGFILE
    else
      whiptail --title "ERROR" --msgbox "Please enter an input priority between 1 and 9.  Press enter to continue" 8 78
    fi
  fi
}


do_input_config()
{
  INPUT1NAME=$(get_config_var input1name $CONFIGFILE)
  INPUT2NAME=$(get_config_var input2name $CONFIGFILE)
  INPUT3NAME=$(get_config_var input3name $CONFIGFILE)
  INPUT4NAME=$(get_config_var input4name $CONFIGFILE)
  INPUT5NAME=$(get_config_var input5name $CONFIGFILE)
  INPUT6NAME=$(get_config_var input6name $CONFIGFILE)
  INPUT7NAME=$(get_config_var input7name $CONFIGFILE)

  INPUT1PRIORITY=$(get_config_var input1prioritylevel $CONFIGFILE)
  INPUT2PRIORITY=$(get_config_var input2prioritylevel $CONFIGFILE)
  INPUT3PRIORITY=$(get_config_var input3prioritylevel $CONFIGFILE)
  INPUT4PRIORITY=$(get_config_var input4prioritylevel $CONFIGFILE)
  INPUT5PRIORITY=$(get_config_var input5prioritylevel $CONFIGFILE)
  INPUT6PRIORITY=$(get_config_var input6prioritylevel $CONFIGFILE)
  INPUT7PRIORITY=$(get_config_var input7prioritylevel $CONFIGFILE)

  menuchoice=$(whiptail --title "Input Enable, Prioritise and Disable Menu" --menu "Select Choice and press enter" 20 78 12 \
    "1 Input 1" "$INPUT1NAME Current Priority $INPUT1PRIORITY" \
    "2 Input 2" "$INPUT2NAME Current Priority $INPUT2PRIORITY" \
    "3 Input 3" "$INPUT3NAME Current Priority $INPUT3PRIORITY" \
    "4 Input 4" "$INPUT4NAME Current Priority $INPUT4PRIORITY" \
    "5 Input 5" "$INPUT5NAME Current Priority $INPUT5PRIORITY" \
    "6 Input 6" "$INPUT6NAME Current Priority $INPUT6PRIORITY" \
    "7 Input 7" "$INPUT7NAME Current Priority $INPUT7PRIORITY" \
    "8 Input Count" "Enter number of inputs connected" \
    "9 Active Hold" "Enable/disable active input hold" \
    "10 Apply" "Apply changes and return to Main Menu" \
    "11 Main menu" "Return to Main Menu" \
      3>&2 2>&1 1>&3)
    case "$menuchoice" in
      1\ *) do_Priority1 ;;
      2\ *) do_Priority2 ;;
      3\ *) do_Priority3 ;;
      4\ *) do_Priority4 ;;
      5\ *) do_Priority5 ;;
      6\ *) do_Priority6 ;;
      7\ *) do_Priority7 ;;
      8\ *) do_Input_Count ;;
      9\ *) do_Active_Hold ;;
      10\ *) do_reload ;;
      11\ *) ;;
    esac
}


do_DTMF_Enable()
{
  DTMF_CONTROL=$(get_config_var dtmfcontrol $CONFIGFILE)
  if [[ "$DTMF_CONTROL" == "on" ]];
  then
    Radio1=ON
    Radio2=OFF
  else
    Radio1=OFF
    Radio2=ON
  fi

  DTMF_CONTROL=$(whiptail --title "DTMF Control Switching" --radiolist \
    "Highlight choice, select with space bar and then press enter" 20 78 6 \
    "on" "DTMF Control Enabled" $Radio1 \
    "off" "DTMF Control Disabled" $Radio2 \
    3>&2 2>&1 1>&3)
  if [ $? -eq 0 ]; then
    set_config_var dtmfcontrol $DTMF_CONTROL $CONFIGFILE
  fi
}


do_Quad_Multiple()
{
  QUAD_MULTIPLE=$(get_config_var showquadformultipleinputs $CONFIGFILE)
  if [[ "$QUAD_MULTIPLE" == "yes" ]];
  then
    Radio1=ON
    Radio2=OFF
  else
    Radio1=OFF
    Radio2=ON
  fi

  QUAD_MULTIPLE=$(whiptail --title "Quad Display on Multiple Inputs" --radiolist \
    "Highlight choice, select with space bar and then press enter" 20 78 6 \
    "yes" "Quad Display Enabled" $Radio1 \
    "no" "Single Input Display Only" $Radio2 \
    3>&2 2>&1 1>&3)
  if [ $? -eq 0 ]; then
    set_config_var showquadformultipleinputs $QUAD_MULTIPLE $CONFIGFILE
  fi
}


do_Half_Hour()
{
  HALF_HOUR=$(get_config_var halfhourpowersave $CONFIGFILE)
  if [[ "$HALF_HOUR" == "yes" ]];
  then
    Radio1=ON
    Radio2=OFF
  else
    Radio1=OFF
    Radio2=ON
  fi

  HALF_HOUR=$(whiptail --title "Power Save During Second Half Hour" --radiolist \
    "Highlight choice, select with space bar and then press enter" 20 78 6 \
    "yes" "Power Save Enabled" $Radio1 \
    "no" "Power Save Disabled" $Radio2 \
    3>&2 2>&1 1>&3)
  if [ $? -eq 0 ]; then
    set_config_var halfhourpowersave $HALF_HOUR $CONFIGFILE
  fi
}


do_QH_Ident()
{
  IDENT_DURING_QH=$(get_config_var identduringquiethours $CONFIGFILE)
  if [[ "$IDENT_DURING_QH" == "yes" ]];
  then
    Radio1=ON
    Radio2=OFF
  else
    Radio1=OFF
    Radio2=ON
  fi

  IDENT_DURING_QH=$(whiptail --title "Choose Whether to Transmit Ident During Quiet Hours" --radiolist \
    "Highlight choice, select with space bar and then press enter" 20 78 6 \
    "yes" "Transmit Ident during quiet hours" $Radio1 \
    "no" "Do not transmit ident during quiet hours (normal setting)" $Radio2 \
    3>&2 2>&1 1>&3)
  if [ $? -eq 0 ]; then
    set_config_var identduringquiethours $IDENT_DURING_QH $CONFIGFILE
  fi
}


do_QH_Repeat()
{
  REPEAT_DURING_QH=$(get_config_var repeatduringquiethours $CONFIGFILE)
  if [[ "$REPEAT_DURING_QH" == "yes" ]];
  then
    Radio1=ON
    Radio2=OFF
  else
    Radio1=OFF
    Radio2=ON
  fi

  REPEAT_DURING_QH=$(whiptail --title "Choose Whether to Repeat During Quiet Hours" --radiolist \
    "Highlight choice, select with space bar and then press enter" 20 78 6 \
    "yes" "Repeat during quiet hours" $Radio1 \
    "no" "Do not repeat during quiet hours (normal setting)" $Radio2 \
    3>&2 2>&1 1>&3)
  if [ $? -eq 0 ]; then
    set_config_var repeatduringquiethours $REPEAT_DURING_QH $CONFIGFILE
  fi
}

do_24_Ops()
{
  OPS_24=$(get_config_var 24houroperation $CONFIGFILE)
  if [[ "$OPS_24" == "yes" ]];
  then
    Radio1=ON
    Radio2=OFF
  else
    Radio1=OFF
    Radio2=ON
  fi

  OPS_24=$(whiptail --title "Choose Whether to Run 24/7" --radiolist \
    "Highlight choice, select with space bar and then press enter" 20 78 6 \
    "yes" "Continuous Operation Throughout the Day" $Radio1 \
    "no" "Only Operate Between the Start and Stop Times" $Radio2 \
    3>&2 2>&1 1>&3)
  if [ $? -eq 0 ]; then
    set_config_var 24houroperation $OPS_24 $CONFIGFILE
  fi
}


do_Start_Time()
{
  START_TIME=$(get_config_var operatingtimestart $CONFIGFILE)
  START_TIME=$(whiptail --inputbox "Enter Start time 0000 to 2359" 8 78 $START_TIME --title "Set Daily Start Time" 3>&1 1>&2 2>&3)
  if [ $? -eq 0 ]; then
    if [ $START_TIME -ge 0 ] && [ $START_TIME -lt 2400 ]; then
      set_config_var operatingtimestart "$START_TIME" $CONFIGFILE
    else
      whiptail --title "ERROR" --msgbox "Please enter a time between 0000 and 2359.  Press enter to continue" 8 78
    fi
  fi
}


do_Stop_Time()
{
  STOP_TIME=$(get_config_var operatingtimefinish $CONFIGFILE)
  STOP_TIME=$(whiptail --inputbox "Enter Finish time 0001 to 2359" 8 78 $STOP_TIME --title "Set Daily Stop Time" 3>&1 1>&2 2>&3)
  if [ $? -eq 0 ]; then
    if [ $STOP_TIME -ge 1 ] && [ $STOP_TIME -lt 2400 ]; then
      set_config_var operatingtimefinish "$STOP_TIME" $CONFIGFILE
    else
      whiptail --title "ERROR" --msgbox "Please enter a time betweenn 0001 and 2359.  Press enter to continue" 8 78
    fi
  fi
}


do_Hours()
{
  menuchoice=$(whiptail --title "Set Operating Hours" --menu "Select Choice and press enter" 20 78 12 \
    "1 24/7 or Timed" "Select Continuous or Timed Operating Hours" \
    "2 Start Time" "Set the daily start time" \
    "3 Stop Time" "Set the daily stop time" \
    "4 Exit" "Return to Behaviour Menu" \
      3>&2 2>&1 1>&3)
    case "$menuchoice" in
      1\ *) do_24_Ops ;;
      2\ *) do_Start_Time ;;
      3\ *) do_Stop_Time ;;
      4\ *) do_behaviour ;;
    esac
}


do_Power_Save()
{
  TX_WHEN_NOT_IN_USE=$(get_config_var transmitwhennotinuse $CONFIGFILE)
  if [[ "$TX_WHEN_NOT_IN_USE" == "yes" ]];
  then
    Radio1=ON
    Radio2=OFF
  else
    Radio1=OFF
    Radio2=ON
  fi

  TX_WHEN_NOT_IN_USE=$(whiptail --title "Choose Whether to Transmit When Not in Use" --radiolist \
    "Highlight choice, select with space bar and then press enter" 20 78 6 \
    "yes" "Transmit continuously during operating hours" $Radio1 \
    "no" "Only transmit when repeating or during Ident" $Radio2 \
    3>&2 2>&1 1>&3)
  if [ $? -eq 0 ]; then
    set_config_var transmitwhennotinuse $TX_WHEN_NOT_IN_USE $CONFIGFILE
  fi
}


do_Beacon()
{
  BEACON_MODE=$(get_config_var beaconmode $CONFIGFILE)
  if [[ "$BEACON_MODE" == "yes" ]];
  then
    Radio1=ON
    Radio2=OFF
  else
    Radio1=OFF
    Radio2=ON
  fi

  BEACON_MODE=$(whiptail --title "Choose Whether to Enable Beacon Mode" --radiolist \
    "Highlight choice, select with space bar and then press enter" 20 78 6 \
    "yes" "Transmit only Carousel and Ident" $Radio1 \
    "no" "Normal Repeater Operation" $Radio2 \
    3>&2 2>&1 1>&3)
  if [ $? -eq 0 ]; then
    set_config_var beaconmode $BEACON_MODE $CONFIGFILE
  fi
}


do_Transmit_Enable()
{
  TRANSMIT_ENABLED=$(get_config_var transmitenabled $CONFIGFILE)
  if [[ "$TRANSMIT_ENABLED" == "yes" ]];
  then
    Radio1=ON
    Radio2=OFF
  else
    Radio1=OFF
    Radio2=ON
  fi

  TRANSMIT_ENABLED=$(whiptail --title "Choose Transmit Enable Option" --radiolist \
    "Highlight choice, select with space bar and then press enter" 20 78 6 \
    "yes" "Transmitter Enabled" $Radio1 \
    "no" "Transmitter Disabled" $Radio2 \
    3>&2 2>&1 1>&3)
  if [ $? -eq 0 ]; then
    set_config_var transmitenabled $TRANSMIT_ENABLED $CONFIGFILE
  fi
}


do_Audio_Keepalive()
{
  AUDIO_KEEP_ALIVE=$(get_config_var audiokeepalive $CONFIGFILE)
  if [[ "$AUDIO_KEEP_ALIVE" == "yes" ]];
  then
    Radio1=ON
    Radio2=OFF
  else
    Radio1=OFF
    Radio2=ON
  fi

  AUDIO_KEEP_ALIVE=$(whiptail --title "Choose Audio Keep-alive Option" --radiolist \
    "Highlight choice, select with space bar and then press enter" 20 78 6 \
    "yes" "Continuous keep-alive (very low-level white noise)" $Radio1 \
    "no" "No audio keep-alive (intermittent audio data in HDMI Output)" $Radio2 \
    3>&2 2>&1 1>&3)
  if [ $? -eq 0 ]; then
    set_config_var audiokeepalive $AUDIO_KEEP_ALIVE $CONFIGFILE
  fi
}


do_reload()
{
  reset
  echo "Stopping the Existing Processes"

  pkill run-audio.sh  >/dev/null 2>/dev/null
  pkill dtmf_listener.sh >/dev/null 2>/dev/null
  sudo killall arecord >/dev/null 2>/dev/null
  sudo killall -9 fbi >/dev/null 2>/dev/null
  sudo killall rptr >/dev/null 2>/dev/null

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
      (/home/pi/atv-rptr/scripts/dtmf_listener.sh /dev/null 2>/dev/null) &
    fi
  fi

  echo
  echo "Restarting the repeater controller"

  (/home/pi/atv-rptr/bin/rptr >/dev/null 2>/dev/null) &
}


do_behaviour()
{
  menuchoice=$(whiptail --title "Repeater Configuration Menu" --menu "Select Choice and press enter" 20 78 14 \
    "1 Audio keepalive" "Enable/disable low-level audio noise" \
    "2 Transmit enable" "Enable/disable Transmitter" \
    "3 Beacon" "Switch in or out of Beacon Mode" \
    "4 Power save" "Continuous transmit or only when in use" \
    "5 Operating hours" "Set operating hours or 24/7 operation" \
    "6 Quiet hours repeat" "Enable repeater TX during quiet hours" \
    "7 Quiet hours ident" "Enable Ident TX during quiet hours" \
    "8 Half Hour" "Power save in second half of the hour" \
    "9 Quad" "Show Quad Display when Multiple Inputs Active" \
    "10 Enable DTMF" "Enable DTMF Control" \
    "11 Apply" "Apply changes and return to Main Menu" \
    "12 Main menu" "Return to Main Menu" \
      3>&2 2>&1 1>&3)
    case "$menuchoice" in
      1\ *) do_Audio_Keepalive ;;
      2\ *) do_Transmit_Enable ;;
      3\ *) do_Beacon ;;
      4\ *) do_Power_Save ;;
      5\ *) do_Hours ;;
      6\ *) do_QH_Repeat ;;
      7\ *) do_QH_Ident ;;
      8\ *) do_Half_Hour ;;
      9\ *) do_Quad_Multiple ;;
      10\ *) do_DTMF_Enable ;;
      11\ *) do_reload ;;
      12\ *) ;;
    esac
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
  menuchoice=$(whiptail --title "Repeater Reboot Menu" --menu "Select Choice and press enter" 16 78 4 \
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

while [ "$status" -eq 0 ] 
  do
    # Display main menu

    menuchoice=$(whiptail --title "BATC Repeater Controller Main Menu" --menu "Select Choice and press enter:" 20 78 13 \
	"0 Reload" "Reload Repeater and Restart with New Settings" \
    "1 Behaviour" "Transmission triggers and times" \
    "2 Callsign" "Customise Callsign and Locator" \
    "3 Inputs" "Enable, Prioritise or Disable Inputs" \
    "4 Control" "Direct Control of Input Selection" \
    "5 Update" "Check Software Version and Update" \
    "6 Settings" "Advanced Settings Menu" \
    "7 Diagnostics" "View Config and Logs" \
    "8 Reboot" "Reboot, exit to the Linux command prompt or ShutDown" \
 	3>&2 2>&1 1>&3)

    case "$menuchoice" in
	0\ *) do_reload ;;
    1\ *) do_behaviour ;;
	2\ *) do_callsign ;;
    3\ *) do_input_config ;;
    4\ *) do_input_control ;;
	5\ *) do_update_menu ;;
    6\ *) do_Settings ;;
    7\ *) do_diagnostics ;;
    8\ *) do_shutdown_menu ;;
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
