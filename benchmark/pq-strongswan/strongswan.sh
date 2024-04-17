#!/bin/bash
# Aggiungere un flag che permette di eseguire i comandi direttamente all'interno del container in modo tale da utilizzare per esempio pki con primitive post quantum

GREEN='\033[0;32m'
YELLOW='\033[0;33m'
RED='\033[0;31m'
DEFAULT='\033[0m'

declare -A connections=(
 ["alpha"]="aes128ctr-sha256-x22519"
 ["bravo"]="aes128ctr-sha256-kyber1"
 ["charlie"]="aes256ctr-sha512-kyber3"
)

# ENV Variable 
INITIATOR="192.168.0.3"
RESPONDER="192.168.0.2"
CONTAINER="carol"

usage() {
    echo "Usage: $(basename "$0") [-h] [-l] [-i CONNECTION_NAME] [-p COMMAND]"
    echo "Options:"
    echo "  -h                 Show this help message"
    echo "  -l                 List available connections and details"
    echo "  -i CONNECTION_NAME Initiate the specified connections"
    echo "  -p, --pki COMMAND  Run the specified command with pki tool"
}

run_pki() {
	sudo docker run --rm -v certs:/certs -w /certs strongx509/pq-strongswan "pki"
}


list_connections(){
	echo "Available conntections"
	{
		printf 'Name\tChiper-Suite\n';
		for connection in "${!connections[@]}"; do
			printf '%s\t%s\n' "$connection" "${connections[$connection]}";
		done

	} | ./utils/table.sh 2 red
}


length() {
 awk -F'length ' '{print $2}' | awk -F':' '{print $1}'
}

reassemble(){
	local sum=0
	for item in $@; do sum=$((sum+item)); done
	echo $sum
}

time_difference() {
    local buffer="$1"
    local start=$(cat "$1.packet" | grep -Eo "^[0-9]+.[0-9]+" | head -n 1)
    local final=$(cat "$1.packet" | grep -Eo "^[0-9]+.[0-9]+" | tail -n 1)    
    # Calcola la differenza tra i tempi
    local difference=$(echo "$final - $start" | bc)
    echo $difference
}

pre_command() {
    echo -en "Checking if docker is running... \t\t\t\t\t"
    docker_info=$(docker info 2>/dev/null)
    if [ $? -eq 0 ]; then
    	echo -e "${GREEN}Running${DEFAULT}"
    else
    	echo -e "${RED}Please Start Docker Deamon${DEFUALT}"
	exit 1
    fi
    echo -en "Checking Strongswan contaniner... \t\t\t\t\t"
    flag=$(docker-compose ps --status running | grep -o carol | head -n 1)
    if [ ! -z $flag ]; then
	    echo -e "${GREEN}Running${DEFAULT}"
    else
	    echo -e "${YELLOW}Please Start${DEFAULT}"
	    exit 1
    fi
    # Terminating existing connection
    echo -en "Terminating existing connections... \t\t\t\t\t"
    if sudo docker exec "$CONTAINER" swanctl --list-sas | grep "$CONN" > /dev/null; then
        sudo docker exec "$CONTAINER" bash -c "$PRE_CMD" > /dev/null
    fi
    echo -e "${GREEN}Completed${DEFAULT}"
}

main(){
	if [ "$#" -eq 0 ]; then
        	usage
        	exit 0
    	fi
	local switch=0
	local CMD=""

	while getopts ":hli:p:" opt; do
		case $opt in
			h) usage; exit 0;;
			l) switch=1;;
			i) CONN="$OPTARG";;
			p) CMD="$OPTARG";;
		esac
	done
	if [ "$switch" -eq 1 ]; then
        	list_connections
        	exit 0
   	fi

	if [ -n "$CMD" ]; then
		echo "stai usando l'opzione pki"
		run_pki $cmd
		exit 0
	fi

	if [ -n "$CONN" ]; then

		OUTD="logs"
		TIMESTAMP=$(date +"%s")
		OUTF="./logs/$CONN.$TIMESTAMP.log"
		COMMAND="swanctl --initiate --ike $CONN"
		PRE_CMD="swanctl --terminate --ike $CONN"
		pre_command
		# Listening with tcpdump and starting connection
		#
		INTERFACE=$(sudo ./utils/dockerveth.sh 2> /dev/null | grep $CONTAINER | awk '{print $2}')
		echo -en "Listening on interface ${YELLOW}$INTERFACE${DEFAULT} for new ${YELLOW}$CONN${DEFAULT} connection... \t"
		sudo timeout --preserve-status 6 tcpdump -tt -i $INTERFACE -w init -n "udp port 500"  > /dev/null 2>&1 &
		sudo timeout --preserve-status 6 tcpdump -tt -i $INTERFACE -w auth -n "udp port 4500 or (ip[6:2] & 0x1fff != 0)"  > /dev/null 2>&1 &
		sleep 1
		sudo docker "exec" $CONTAINER bash -c "$COMMAND" > $OUTF
		sleep 5
		echo -e "${GREEN}Completed${DEFAULT}"

		echo -en "Genereting the results for establishing connection... \t\t\t"

		tcpdump -tt -r init -e -n 2> /dev/null > init.packet
		tcpdump -tt -r auth -e -n 2> /dev/null | awk '/#43/ || /fragmented IP/ {print}' > intermediate.packet
		tcpdump -tt -r auth -e -n 2> /dev/null | awk '/ikev2_auth\[I\]/ || /ikev2_auth\[R\]/{flag=1} flag' > auth.packet

		init_time=$(time_difference "init")
		init=$(tcpdump -r init -e 2> /dev/null | length)
		init_i=$(echo $init | awk -F' ' '{print $1}')
		init_r=$(echo $init | awk -F' ' '{print $2}')

		# Intermediate info
		if [ -s intermediate.packet ]; then
			inte_time=$(time_difference "intermediate")
			inte_i=$(reassemble $(tcpdump -r auth -e -n "udp and src host $INITIATOR" 2> /dev/null | awk '/#43/ || /fragmented IP/ {print}' | length))
			inte_r=$(reassemble $(tcpdump -r auth -e -n "udp and src host $RESPONDER" 2> /dev/null | awk '/#43/ || /fragmented IP/ {print}' | length))
		else
			inte_time="N/S"
			inte_i="N/S"
			inte_r="N/S"
		fi

		# Auth info
		auth_time=$(time_difference "auth")
		auth_i=$(reassemble $(tcpdump -r auth -e -n "udp and src host $INITIATOR" 2>/dev/null | awk '/ikev2_auth\[I\]/ || /ikev2_auth\[R\]/{flag=1} flag' | length))
		auth_r=$(reassemble $(tcpdump -r auth -e -n "udp and src host $RESPONDER" 2>/dev/null | awk '/ikev2_auth\[I\]/ || /ikev2_auth\[R\]/{flag=1} flag' | length))

		echo -e "${GREEN}Completed${DEFAULT}"
		{
			printf 'Phase\tInitiator\tResponder\tTime\n';
			printf '%s\t%s\t%s\t%s\n' "INIT" "$init_i byte" "$init_r byte" "0$init_time s";
			printf '%s\t%s\t%s\t%s\n' "INTERMEDIATE" "$inte_i byte" "$inte_r byte" "0$inte_time s";
			printf '%s\t%s\t%s\t%s\n' "AUTH" "$auth_i byte" "$auth_r byte" "0$auth_time s";

		} | ./utils/table.sh 4 green
		
		##log_dim=$(cat $OUTF | grep "^\[NET\]"  | grep -o -P "(?<=\().*?(?=\))" | awk -F' ' '{print $1}')
		#init_i_payload=$(echo $log_dim | awk -F' ' '{print $1}')
		#init_r_payload=$(echo $log_dim | awk -F' ' '{print $2}')
		#auth_i_payload=$(echo $log_dim | awk -F' ' '{print $3}')
		#auth_r_payload=$(echo $log_dim | awk -F' ' '{print $4}')
		#echo "$init_i_payload"
		#echo "$init_r_payload"
		#echo "$auth_i_payload"
		#echo "$auth_r_payload"

	fi
	rm -rf auth init
	rm -rf *.packet
}

echo -e "${RED}"
echo -e '   (\"-  ╔═╗┌┬┐┬─┐┌─┐┌┐┌┌─┐┌─┐┬ ┬┌─┐┌┐┌'
echo -e '\\\_\     ╚═╗ │ ├┬┘│ │││││ ┬└─┐│││├─┤│││'
echo -e '<____)   ╚═╝ ┴ ┴└─└─┘┘└┘└─┘└─┘└┴┘┴ ┴┘└┘'
echo -e "${DEFAULT}"

main "$@"

