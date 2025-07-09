#!/bin/bash

# This script remotely shuts down the desktop RPi

CMDFILE="/home/pi/dektop_shutdown.txt"
DTPW=password

# Write the assembled Desktop command to a temp file (remove # for action)

#/bin/cat <<EOM >$CMDFILE
#sshpass -p "$DTPW" ssh -t -o StrictHostKeyChecking=no pi@Desktop.local 'bash -s' <<'ENDSSH' 
#sudo shutdown now
#ENDSSH
#EOM

# Run the Command

#source "$CMDFILE"

exit
