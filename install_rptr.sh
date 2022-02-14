#!/bin/bash

# BATC ATV Repeater Install by davecrump on 20220101

BuildLogMsg() {
  if [[ "$1" == "0" ]]; then
    echo $(date -u) "Build Success " "$2" | sudo tee -a /var/log/rptr/initial_build_log.txt  > /dev/null
  else
    echo $(date -u) "Build Fail    " "$2" | sudo tee -a /var/log/rptr/initial_build_log.txt  > /dev/null
  fi
}

# Create the directory and first entry for the build and update logs
sudo mkdir /var/log/rptr
echo $(date -u) "New Build started" | sudo tee -a /var/log/rptr/initial_build_log.txt  > /dev/null

# Check current user
whoami | grep -q pi
if [ $? != 0 ]; then
  echo "Install must be performed as user pi"
  BuildLogMsg "0" "Exiting, not user pi"

  exit
fi

# Check Correct Raspios Version (Buster Legacy)
lsb_release -a | grep -q buster
if [ $? != 0 ]; then
  echo
  echo "The Repeater Controller requires the Raspios Buster Lite (Legacy) operating system"
  echo "You may have used bullseye, which is the latest, but not suitable for this software"
  echo 
  echo "Press any key to exit"
  read -n 1
  printf "\n"
  if [[ "$REPLY" = "d" || "$REPLY" = "D" ]]; then  # Allow to proceed for development
    echo "Continuing build......"
    BuildLogMsg "0" "Warning, NOT BUSTER OS"
  else
    BuildLogMsg "0" "Exiting, NOT BUSTER OS"
    exit
  fi
fi

# Check which source needs to be loaded
GIT_SRC="BritishAmateurTelevisionClub"
GIT_SRC_FILE=".atv-rptr_gitsrc"

if [ "$1" == "-d" ]; then
  GIT_SRC="davecrump";
  echo
  echo "-------------------------------------------------------------------"
  echo "----- Installing development version of the BATC ATV Repeater -----"
  echo "-------------------------------------------------------------------"
  BuildLogMsg "0" "Installing Dev Version"
elif [ "$1" == "-u" -a ! -z "$2" ]; then
  GIT_SRC="$2"
  echo
  echo "WARNING: Installing ${GIT_SRC} development version, press enter to continue or 'q' to quit."
  read -n1 -r -s key;
  if [[ $key == q ]]; then
    exit 1;
  fi
  echo "ok!";
else
  echo
  echo "-----------------------------------------------------------------------"
  echo "----- Installing BATC Production version of the BATC ATV Repeater -----"
  echo "-----------------------------------------------------------------------"
  BuildLogMsg "0" "Installing Production Version"
fi

# Update the package manager
echo
echo "------------------------------------"
echo "----- Updating Package Manager -----"
echo "------------------------------------"
sudo dpkg --configure -a
SUCCESS=$?; BuildLogMsg $SUCCESS "dpkg configure"
sudo apt-get update --allow-releaseinfo-change
SUCCESS=$?; BuildLogMsg $SUCCESS "apt-get update"

# Uninstall the apt-listchanges package to allow silent install of ca certificates (201704030)
# http://unix.stackexchange.com/questions/124468/how-do-i-resolve-an-apparent-hanging-update-process
sudo apt-get -y remove apt-listchanges
SUCCESS=$?; BuildLogMsg $SUCCESS "remove apt-listchanges"

# Upgrade the distribution
echo
echo "-----------------------------------"
echo "----- Performing dist-upgrade -----"
echo "-----------------------------------"
sudo apt-get -y dist-upgrade
SUCCESS=$?; BuildLogMsg $SUCCESS "dist-upgrade"

# Install the packages that we need
echo
echo "-------------------------------"
echo "----- Installing Packages -----"
echo "-------------------------------"

sudo apt-get -y install git
SUCCESS=$?; BuildLogMsg $SUCCESS "git install"
sudo apt-get -y install cmake 
SUCCESS=$?; BuildLogMsg $SUCCESS "cmake install"
sudo apt-get -y install fbi
SUCCESS=$?; BuildLogMsg $SUCCESS "fbi install"
sudo apt-get -y install libjpeg-dev
SUCCESS=$?; BuildLogMsg $SUCCESS "libjpeg-dev install"
sudo apt-get -y install pigpio
SUCCESS=$?; BuildLogMsg $SUCCESS "pigpio install"
sudo apt-get -y install imagemagick
SUCCESS=$?; BuildLogMsg $SUCCESS "imagemagick install"
sudo apt-get -y install ncat
SUCCESS=$?; BuildLogMsg $SUCCESS "ncat install"
sudo apt-get -y install lirc
SUCCESS=$?; BuildLogMsg $SUCCESS "lirc install"
sudo apt-get -y install ir-keytable
SUCCESS=$?; BuildLogMsg $SUCCESS "ir-keytable install"
sudo apt-get -y install multimon-ng
SUCCESS=$?; BuildLogMsg $SUCCESS "multimon-ng install"
sudo apt-get -y install sox
SUCCESS=$?; BuildLogMsg $SUCCESS "sox install"
sudo apt-get -y install vlc
SUCCESS=$?; BuildLogMsg $SUCCESS "vlc install"
sudo apt-get install -y i2c-tools
SUCCESS=$?; BuildLogMsg $SUCCESS "i2c-tools"

cd /home/pi

# Set auto login to command line.
sudo raspi-config nonint do_boot_behaviour B2
SUCCESS=$?; BuildLogMsg $SUCCESS "raspi-config auto-login"

# set the framebuffer to 32 bit depth by disabling dtoverlay=vc4-fkms-v3d
#echo
#echo "------------------------------------"
#echo "---- Amending /boot/config.txt -----"
#echo "------------------------------------"

sudo sed -i "/^dtoverlay=vc4-fkms-v3d/c\#dtoverlay=vc4-fkms-v3d" /boot/config.txt
SUCCESS=$?; BuildLogMsg $SUCCESS "Disabled dtoverlay=vc4-fkms-v3d"

# Turn overscan off for full-screen captions
sudo sed -i "/^#disable_overscan=1/c\disable_overscan=1" /boot/config.txt
SUCCESS=$?; BuildLogMsg $SUCCESS "Disabled overscan"

# Enable the IR Output GPIO
sudo sed -i "/^#dtoverlay=gpio-ir-tx,gpio_pin=18/c\dtoverlay=gpio-ir-tx,gpio_pin=18" /boot/config.txt
SUCCESS=$?; BuildLogMsg $SUCCESS "Enabled IR GPIO"

# Enable i2c on physical pins 8 and 10
sudo sed -i "/^#dtparam=i2c_arm=on/c\dtparam=i2c_arm=on" /boot/config.txt
sudo sed -i "/^dtparam=i2c_arm=on/a \dtoverlay=i2c-gpio,i2c_gpio_sda=14,i2c_gpio_scl=15" /boot/config.txt
SUCCESS=$?; BuildLogMsg $SUCCESS "Enabled i2c on pins 8 and 10"

# Reduce the dhcp client timeout to speed off-network startup
echo
echo "-------------------------------------------"
echo "---- Reducing the dhcp client timeout -----"
echo "-------------------------------------------"
sudo bash -c 'echo -e "\n# Shorten dhcpcd timeout from 30 to 5 secs" >> /etc/dhcpcd.conf'
sudo bash -c 'echo -e "timeout 5\n" >> /etc/dhcpcd.conf'
SUCCESS=$?; BuildLogMsg $SUCCESS "Shortened DHCP Timeout"

cd /home/pi/

# Amend /etc/fstab to create a tmpfs drive at ~/tmp for multiple images (201708150)
sudo sed -i '4itmpfs           /home/pi/tmp    tmpfs   defaults,noatime,nosuid,size=10m  0  0' /etc/fstab
SUCCESS=$?; BuildLogMsg $SUCCESS "Created tempfs"

# Enable the pigpio daemon (which will start on reboot)
sudo systemctl enable pigpiod
SUCCESS=$?; BuildLogMsg $SUCCESS "Enabled pigpiod daemon"

# Download the previously selected version of the BATC ATV Repeater Software
echo
echo "-----------------------------------------"
echo "----- Downloading Repeater Software -----"
echo "-----------------------------------------"

cd /home/pi
wget https://github.com/${GIT_SRC}/atv-rptr/archive/refs/heads/main.zip -O main.zip
SUCCESS=$?; BuildLogMsg $SUCCESS "Downloaded Repeater Software"

# Unzip the repeater software and copy to the Pi
unzip -o main.zip
mv atv-rptr-main atv-rptr
rm main.zip
cd /home/pi


# Compile atv-rptr software
echo
echo "------------------------------"
echo "----- Compiling atv-rptr -----"
echo "------------------------------"

mkdir atv-rptr/bin
cd /home/pi/atv-rptr/src/rptr
touch main.c
make
SUCCESS=$?; BuildLogMsg $SUCCESS "Compiled Repeater Software"
sudo make install
cd /home/pi

# Compile the txt2morse software
echo
echo "------------------------------"
echo "----- Compiling txt2morse ----"
echo "------------------------------"

cd /home/pi/atv-rptr/src/txt2morse
touch txt2morse.c
make
SUCCESS=$?; BuildLogMsg $SUCCESS "Compiled txt2morse Software"
cp /home/pi/atv-rptr/src/txt2morse/build/txt2morse /home/pi/atv-rptr/bin/txt2morse
cd /home/pi

# Download, compile and install the executable for hardware shutdown button
#echo
#echo "------------------------------------------------------------"
#echo "----- Installing the hardware shutdown Button software -----"
#echo "------------------------------------------------------------"

#git clone https://github.com/philcrump/pi-sdn /home/pi/pi-sdn-build

# Install new version that sets swapoff
#cp -f /home/pi/rpidatv/src/pi-sdn/main.c /home/pi/pi-sdn-build/main.c
#cd /home/pi/pi-sdn-build
#make
#mv pi-sdn /home/pi/
#cd /home/pi


echo
echo "--------------------------------------"
echo "----- Configure the Menu Aliases -----"
echo "--------------------------------------"

# Install the menu aliases
echo "alias menu='/home/pi/atv-rptr/scripts/menu.sh menu'" >> /home/pi/.bash_aliases
echo "alias urptr='/home/pi/atv-rptr/utils/update_rptr.sh'" >> /home/pi/.bash_aliases
echo "alias rptr='/home/pi/atv-rptr/utils/run_rptr.sh'" >> /home/pi/.bash_aliases
echo "alias stop='/home/pi/atv-rptr/utils/stop.sh'" >> /home/pi/.bash_aliases

echo if test -z \"\$SSH_CLIENT\" >> ~/.bashrc 
echo then >> ~/.bashrc
echo "source /home/pi/atv-rptr/scripts/startup.sh" >> ~/.bashrc
echo fi >> ~/.bashrc

#Configure the boot parameters

if !(grep disable_splash /boot/config.txt) then
  sudo sh -c "echo disable_splash=1 >> /boot/config.txt"
fi
if !(grep global_cursor_default /boot/cmdline.txt) then
  sudo sed -i '1s,$, vt.global_cursor_default=0,' /boot/cmdline.txt
fi

# Copy the log file rotation configuration
sudo cp /home/pi/atv-rptr/utils/templates/rptr /etc/logrotate.d/rptr

# Create a destination for Custom Media that is not over-written during updates
mkdir /home/pi/custom_media

# Record Version Number
cp /home/pi/atv-rptr/latest_version.txt /home/pi/atv-rptr/config/installed_version.txt
cd /home/pi

# Log Completed Build Details
INSTALLEDVERSION=$(head -c 9 /home/pi/atv-rptr/config/installed_version.txt)
SUCCESS="0"; BuildLogMsg $SUCCESS "Completed Install of Version "$INSTALLEDVERSION""
echo $(date -u) "Initial Install of Version "$INSTALLEDVERSION"" | sudo tee -a /var/log/rptr/update_log.txt  > /dev/null

# Save git source used
echo "${GIT_SRC}" > /home/pi/atv-rptr/config/${GIT_SRC_FILE}

echo
echo "SD Card Serial:"
cat /sys/block/mmcblk0/device/cid

# Reboot
echo
echo "--------------------------------"
echo "----- Complete.  Rebooting -----"
echo "--------------------------------"
sleep 1

sudo reboot now
exit


