#!/bin/bash



var="N";

while [ $var != "S" ]; do
   var=$(pidof -q supermercato  && echo "N" || echo "S")
done


	if [ $# != 1 ]; then
		echo "Use: bash analisi.sh super.txt"
		exit 1;
	fi
	
	if [ ! -e "$1" ]; then
    	echo "control logfile" ;
    	exit 1;
	fi

(cat $1 | grep "cliente" | sort) > ./testfile/client.txt


CLI=./testfile/'client.txt'
echo -e "\n \n\t\t\t\t*** CLIENTS STAT ***\t\t\t "
echo " _______________________________________________________________________________________"
echo -e "| CLIENT ID\t| BOUGHT_PRODUCTS |     TOTAL_TIME\t |  TAIL_TIME\t| VISITED_TAIL  |"

	while read -r line;do
	ID_C=$(echo $line | cut -d '|' -f1 |cut -d ' ' -f3)
	PR_C=$(echo $line | cut -d '|' -f2 |cut -d ' ' -f5)
	TT=$(echo $line | cut -d '|' -f3 |cut -d ' ' -f6)
	TD=$(echo $line | cut -d '|' -f4 |cut -d ' ' -f7)
	VT=$(echo $line | cut -d '|' -f5 |cut -d ' ' -f6)
	echo -e "|     $ID_C \t|\t$PR_C \t  | \t $TT s \t |  $TD s \t|       $VT  \t|"
done <$CLI
echo "|_______________________________________________________________________________________|"




(cat $1 | grep "cassa" | sort) > ./testfile/cashier.txt


DESK=./testfile/'cashier.txt'
echo -e "\n\t\t\t\t*** CASHIERS STAT ***\t\t\t "
echo " _______________________________________________________________________________________"
echo -e "| CASSA ID\t|  TOTAL_PRODUCTS | TOTAL_CLIENTS |  TIME_OPEN\t| SERVICE_TIME  | #CLOSE|"

	while read -r line;do
	ID=$(echo $line | cut -d '|' -f1 |cut -d ' ' -f3)
	PR=$(echo $line | cut -d '|' -f2 |cut -d ' ' -f5)
	CL=$(echo $line | cut -d '|' -f3 |cut -d ' ' -f5)
	TO=$(echo $line | cut -d '|' -f4 |cut -d ' ' -f6)
	ST=$(echo $line | cut -d '|' -f5 |cut -d ' ' -f6)
	NC=$(echo $line | cut -d '|' -f6 |cut -d ' ' -f4)
	echo -e "|     $ID \t|\t$PR \t  | \t $CL \t  |  $TO s \t|    $ST  \t|  $NC  \t|"
    done <$DESK
echo "|_______________________________________________________________________________________|"


(cat $1 | grep "direttore" | sort) > ./testfile/direttore.txt

DIR=./testfile/'direttore.txt'

echo -e "\n\t\t\t\t*** DIRECTOR STAT ***\t\t\t "

	while read -r line;do
		C=$(echo $line  |cut -d ' ' -f8)
		P=$(echo $line |cut -d ' ' -f12)
	echo -e "CLIENT $C  HAVE BOUGHT  $P PRODUCTS  | SACKED BY DIRECTOR "
	done <$DIR

echo "test terminato"

exit 0;
