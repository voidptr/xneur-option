#!/bin/sh

test_serv="ya.ru"
packets_count=10
interface="iwi0"

usage()
{
        echo "Usage: `basename $0` gateway"
        exit 1
}

if [ $# -ne 1 ]; then
	usage
fi

ipm='[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}'
mycomp="192.168.${1}"
mycomp="`echo ${mycomp} | grep -Eo ^${ipm}$`"

if [ -z "${mycomp}" ]; then
	usage
fi

/etc/rc.d/wpa_supplicant status ${interface} > /dev/null 2>&1
ret=$?

if [ $ret -eq 0 ]; then
	echo -n "Stopping WPA for ${interface}... "

	/etc/rc.d/wpa_supplicant stop ${interface} > /dev/null 2>&1
	ret=$?

	if [ $ret -eq 0 ]; then
		echo "done"
	else
		echo "failed"
		exit 1
	fi
fi

echo -n "Starting WPA for ${interface}... "

/etc/rc.d/wpa_supplicant start ${interface} > /dev/null 2>&1
ret=$?

if [ $ret -eq 0 ]; then
	echo "done"
else
	echo "failed"
	exit 1
fi

sleep 1

client=`ps aux | grep dhclient | grep -v grep | wc -l`
if [ $client -eq 0 ]; then
	echo -n "Starting dhclient... "

	dhclient ${interface} > /dev/null 2>&1
	ret=$?
	sleep 1

	if [ $ret -eq 0 ]; then
                echo "done"
        else
                echo "failed"
		exit 1
        fi
fi

client=`ps aux | grep dhclient | grep -v grep | wc -l`
if [ $client -ne 0 ]; then
	echo -n "Killing dhclient... "

	killall dhclient > /dev/null 2>&1
	ret=$?
	sleep 1

	if [ $ret -eq 0 ]; then
		echo "done"
	else
		echo "failed"
		exit 1
	fi
else
	echo "dhclient starting failed"
	exit 1
fi

echo -n "Setting default route to ${mycomp}... "

rm /etc/resolv.conf && echo "nameserver ${mycomp}" > /etc/resolv.conf
route flush > /dev/null
route delete 0.0.0.0 > /dev/null 2>&1
route add 192.168.0.0 192.168.124.1 255.255.0.0 > /dev/null 2>&1
route add 0.0.0.0 $mycomp 0.0.0.0 > /dev/null 2>&1
route add 83.142.161.241 192.168.124.1 > /dev/null 2>&1

echo "done"
echo -n "Testing connection... "

loss=`ping -c ${packets_count} -i 0 ${test_serv} | grep -Eo [0-9]\{1,3\}% | grep -Eo [0-9]+`

if [ -z "${loss}" ]; then
	loss=100
fi

eval "loss=$((${loss} * ${packets_count} / 100))"

if [ $loss -eq 0 ]; then
	echo "connection working"
elif [ $loss -ne $packets_count ]; then
	echo "${loss} of ${packets_count} packet loss"
else
	echo "all packets loss"
fi

ip="`ifconfig ${interface} | grep -Eo 'inet [^ ]+? ' | grep -Eo ${ipm}`"
echo "Your ip is: ${ip}"
