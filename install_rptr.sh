#!/bin/bash

# BATC ATV Repeater Install by davecrump on 20220101

# Check current user
whoami | grep -q pi
if [ $? != 0 ]; then
  echo "Install must be performed as user pi"
  exit
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
fi

# Update the package manager
echo
echo "------------------------------------"
echo "----- Updating Package Manager -----"
echo "------------------------------------"
sudo dpkg --configure -a
sudo apt-get update --allow-releaseinfo-change

# Uninstall the apt-listchanges package to allow silent install of ca certificates (201704030)
# http://unix.stackexchange.com/questions/124468/how-do-i-resolve-an-apparent-hanging-update-process
sudo apt-get -y remove apt-listchanges

# Upgrade the distribution
echo
echo "-----------------------------------"
echo "----- Performing dist-upgrade -----"
echo "-----------------------------------"
sudo apt-get -y dist-upgrade

# Install the packages that we need
echo
echo "-------------------------------"
echo "----- Installing Packages -----"
echo "-------------------------------"

sudo apt-get -y install git
sudo apt-get -y install cmake 
sudo apt-get -y install fbi
sudo apt-get -y install libjpeg-dev
sudo apt-get -y install pigpio
sudo apt-get -y install imagemagick
sudo apt-get -y install ncat
sudo apt-get -y install lirc
sudo apt-get -y install ir-keytable

cd /home/pi

# Set auto login to command line.
sudo raspi-config nonint do_boot_behaviour B2

# set the framebuffer to 32 bit depth by disabling dtoverlay=vc4-fkms-v3d
#echo
#echo "----------------------------------------------"
#echo "---- Setting Framebuffer to 32 bit depth -----"
#echo "----------------------------------------------"

sudo sed -i "/^dtoverlay=vc4-fkms-v3d/c\#dtoverlay=vc4-fkms-v3d" /boot/config.txt

# Turn overscan off for full-screen captions
sudo sed -i "/^#disable_overscan=1/c\disable_overscan=1" /boot/config.txt

# Enable the IR Output GPIO
sudo sed -i "/^#dtoverlay=gpio-ir-tx,gpio_pin=18/c\dtoverlay=gpio-ir-tx,gpio_pin=18" /boot/config.txt

# Reduce the dhcp client timeout to speed off-network startup
echo
echo "-------------------------------------------"
echo "---- Reducing the dhcp client timeout -----"
echo "-------------------------------------------"
sudo bash -c 'echo -e "\n# Shorten dhcpcd timeout from 30 to 5 secs" >> /etc/dhcpcd.conf'
sudo bash -c 'echo -e "timeout 5\n" >> /etc/dhcpcd.conf'
cd /home/pi/

# Amend /etc/fstab to create a tmpfs drive at ~/tmp for multiple images (201708150)
sudo sed -i '4itmpfs           /home/pi/tmp    tmpfs   defaults,noatime,nosuid,size=10m  0  0' /etc/fstab

# Enable the pigpio daemon (which will start on reboot)
sudo systemctl enable pigpiod

# Download the previously selected version of the BATC ATV Repeater Software
echo
echo "-----------------------------------------"
echo "----- Downloading Repeater Software -----"
echo "------------------------------------------"
wget https://github.com/${GIT_SRC}/atv-rptr/archive/master.zip

# Unzip the repeater software and copy to the Pi
unzip -o master.zip
mv atv-rptr-master atv-rptr
rm master.zip
cd /home/pi


# Compile atv-rptr sofftware
echo
echo "------------------------------"
echo "----- Compiling atv-rptr -----"
echo "------------------------------"
cd /home/pi/atv-rptr/src/ctrl
#make
#sudo make install


# Download, compile and install the executable for hardware shutdown button
echo
echo "------------------------------------------------------------"
echo "----- Installing the hardware shutdown Button software -----"
echo "------------------------------------------------------------"

git clone https://github.com/philcrump/pi-sdn /home/pi/pi-sdn-build

# Install new version that sets swapoff
cp -f /home/pi/rpidatv/src/pi-sdn/main.c /home/pi/pi-sdn-build/main.c
cd /home/pi/pi-sdn-build
make
mv pi-sdn /home/pi/
cd /home/pi


echo
echo "--------------------------------------"
echo "----- Configure the Menu Aliases -----"
echo "--------------------------------------"

# Install the menu aliases
echo "alias menu='/home/pi/atv-rptr/scripts/menu.sh menu'" >> /home/pi/.bash_aliases
echo "alias urptr='/home/pi/atv-rptr/utils/update_rptr.sh'" >> /home/pi/.bash_aliases
echo "alias rptr='/home/pi/atv-rptr/utils/run_rptr.sh'" >> /home/pi/.bash_aliases
echo "alias stop='/home/pi/atv-rptr/utils/stop.sh'" >> /home/pi/.bash_aliases
echo "alias restart='/home/pi/atv-rptr/utils/restart.sh'" >> /home/pi/.bash_aliases

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

# Record Version Number
cd /home/pi/atv-rptr/config/
cp latest_version.txt installed_version.txt
cd /home/pi

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


