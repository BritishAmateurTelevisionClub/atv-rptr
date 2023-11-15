#! /usr/bin/expect

#usage ./atem_switch.sh {atem_mini_ip_address} {output_label} {video_input}

set timeout 5
set port 9990
set ip [lindex $argv 0]
set output_label [lindex $argv 1]
set video_input [lindex $argv 2]

spawn telnet $ip $port
expect "END PRELUDE:"
send "VIDEO OUTPUT ROUTING:\r$output_label $video_input\r\r"

spawn telnet $ip $port
expect "END PRELUDE:"
send "VIDEO OUTPUT ROUTING:\r$output_label $video_input\r\r"
