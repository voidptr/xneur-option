#/usr/local/bin/bash

cnt=100
time1=`date +%s`

for ((i = 0; i < cnt; i++)); do
	php -f send_string.php &
done

time2=`date +%s`

let "elapsed = time2 - time1"
echo $elapsed
