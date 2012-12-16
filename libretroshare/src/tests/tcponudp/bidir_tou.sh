#/bin/sh 
# Script to Test the udp_server code.

EXEC=./bidir_tou
DATAFILE=../../lib/libretroshare.a  

TMPOUTPUT=tmpoutput$$
EXPECTEDPERIOD=10

$EXEC 127.0.0.1 4401 127.0.0.1 4032 < $DATAFILE > $TMPOUTPUT 

if diff -s $DATAFILE $TMPOUTPUT
then
	echo "SUCCESS"
else
	echo "FAILURE to accurately transfer DATA"
fi

rm $TMPOUTPUT

