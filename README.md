# The BATC ATV Repeater Controller

**The BATC ATV Repeater Controller** is a digital amateur television repeater controller for the Raspberry Pi 4.  Full details can be found on the BATC Wiki: https://wiki.batc.org.uk/Repeater_Controller 

![Repeater Block Digram](/media/block.jpg)

This version is based on Raspberry Pi OS Lite (Legacy) and is only compatible with the Raspberry Pi 4.  

# Installation

The preferred installation method only needs a Windows PC connected to the same (internet-connected) network as your Raspberry Pi.  Do not connect a keyboard directly to your Raspberry Pi.

- First download the 2022-01-28 release of Raspberry Pi OS Lite (Legacy) on to your Windows PC from here 
https://downloads.raspberrypi.org/raspios_oldstable_lite_armhf/images/raspios_oldstable_lite_armhf-2022-01-28/2022-01-28-raspios-buster-armhf-lite.zip
- Unzip the image and then transfer it to a Micro-SD Card using Win32diskimager https://sourceforge.net/projects/win32diskimager/

- Before you remove the card from your Windows PC, look at the card with windows explorer; the volume should be labeled "boot".  Create a new empty file called ssh in the top-level (root) directory by right-clicking, selecting New, Text Document, and then change the name to ssh (not ssh.txt).  You should get a window warning about changing the filename extension.  Click OK.  If you do not get this warning, you have created a file called ssh.txt and you need to rename it ssh.  IMPORTANT NOTE: by default, Windows (all versions) hides the .txt extension on the ssh file.  To change this, in Windows Explorer, select File, Options, click the View tab, and then untick "Hide extensions for known file types". Then click OK.

- Connect an HDMI display to the Raspberry Pi (no keyboard or mouse), insert the card and power on.  The Rpi will reboot once and then be ready for the installation.

- Find the IP address of your Raspberry Pi using an IP Scanner (such as Advanced IP Scanner http://filehippo.com/download_advanced_ip_scanner/ for Windows, or Fing on an iPhone) to get the RPi's IP address.  You may be able to read the IP address from the HDMI screen. 

- From your windows PC use KiTTY (https://www.fosshub.com/KiTTY.html) to log in to the IP address that you noted earlier.  You will get a Security warning the first time you try; this is normal.

- Log in (user: pi, password: raspberry) then cut and paste the following code in, one line at a time:


```sh
wget https://raw.githubusercontent.com/BritishAmateurTelevisionClub/atv-rptr/main/install_rptr.sh
chmod +x install_rptr.sh
./install_rptr.sh
```

The initial build can take up to 15 minutes, however it does not need any user input, so go and make a cup of coffee and keep an eye on the screen.  When the build is finished the Pi will reboot.

- If your ISP is Virgin Media and you receive an error after entering the wget line: 'GnuTLS: A TLS fatal alert has been received.', it may be that your ISP is blocking access to GitHub.  If (only if) you get this error with Virgin Media, paste the following command in, and press return.
```sh
sudo sed -i 's/^#name_servers.*/name_servers=8.8.8.8/' /etc/resolvconf.conf
```
Then reboot, and try again.  The command asks your RPi to use Google's DNS, not your ISP's DNS.

- If your ISP is BT, you will need to make sure that "BT Web Protect" is disabled so that you are able to download the software.

- After the reboot, initial configuration is best done by direct editing of the /home/pi/atv-rptr/config/repeater_config.txt file.  Refer to the BATC Wiki for guidance.  Note that the repeater must be restarted to read configuration file changes.


# Advanced notes

To load the development version, cut and paste in the following lines:

```sh
wget https://raw.githubusercontent.com/davecrump/atv-rptr/main/install_rptr.sh
chmod +x install_rptr.sh
./install_rptr.sh -d
```

