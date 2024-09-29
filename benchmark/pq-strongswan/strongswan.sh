#!/bin/bash
# Aggiungere un flag che permette di eseguire i comandi direttamente all'interno del container in modo tale da utilizzare per esempio pki con primitive post quantum
# Aggiungere un flag per eseguire una bash all'interno del container, per esempio per vedere quelli che sono gli algoritmi supportati da swanctl

GREEN='\033[0;32m'
YELLOW='\033[0;33m'
RED='\033[0;31m'
DEFAULT='\033[0m'

declare -A connections=(
 ["alpha"]="aes128ctr-sha256-x22519"
 ["bravo"]="aes128ctr-sha256-kyber1"
 ["charlie"]="aes256ctr-sha512-kyber3"
 ["delta"]="aes128ctr-sha256-modp3072-ke1_kyber1"
 ["echo"]="aes128ctr-sha256-ecp256-ke1_hqc1"
)

# ENV Variable 
INITIATOR="192.168.0.3"
RESPONDER="192.168.0.2"
CONTAINER="carol"

usage() {
	echo -e "${RED}"
	echo -e '   (\"-  ╔═╗┌┬┐┬─┐┌─┐┌┐┌┌─┐┌─┐┬ ┬┌─┐┌┐┌'
	echo -e '\\\_\     ╚═╗ │ ├┬┘│ │││││ ┬└─┐│││├─┤│││'
	echo -e '<____)   ╚═╝ ┴ ┴└─└─┘┘└┘└─┘└─┘└┴┘┴ ┴┘└┘'
	echo -e "${DEFAULT}"
	echo "Usage: $(basename "$0") [-h] [-l] [-i CONNECTION_NAME] [-p COMMAND] [-n ATTEMPS]"
	echo "Options:"
	echo "  -h                 Show this help message"
	echo "  -l                 List available connections and details"
	echo "  -i CONNECTION_NAME Initiate the specified connections"
	echo "  -n ATTEMPS         Initiate the specified connections"
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
    if docker exec "$CONTAINER" swanctl --list-sas | grep "$CONN" > /dev/null; then
        docker exec "$CONTAINER" bash -c "$PRE_CMD" > /dev/null
    fi
    echo -e "${GREEN}Completed${DEFAULT}"
}

listening() {

		echo -en "Listening on interface ${YELLOW}$INTERFACE${DEFAULT} for new ${YELLOW}$CONN${DEFAULT} connection... \t"
		sudo timeout --preserve-status 6 tcpdump -tt -i $INTERFACE -w init -n "udp port 500"  > /dev/null 2>&1 &
		sudo timeout --preserve-status 6 tcpdump -tt -i $INTERFACE -w auth -n "udp port 4500 or (ip[6:2] & 0x1fff != 0)"  > /dev/null 2>&1 &
		sleep 1
		sudo docker "exec" $CONTAINER bash -c "$COMMAND" > /dev/null
		sleep 5
		echo -e "${GREEN}Completed${DEFAULT}"
}

main(){
	if [ "$#" -eq 0 ]; then
        	usage
        	exit 0
    	fi
	local switch=0
	local CMD=""
	local N=1

	while getopts ":hli:n:p:" opt; do
		case $opt in
			h) usage; exit 0;;
			l) switch=1;;
			n) N="$OPTARG";;
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

	tot_auth_time=0
	tot_init_time=0
	tot_inte_time=0

	if [ -n "$CONN" ]; then
		
		COMMAND="swanctl --initiate --ike $CONN"
		PRE_CMD="swanctl --terminate --ike $CONN"
		for((att=1; att<=$N; att++))
		do
			pre_command
			INTERFACE=$(sudo ./utils/dockerveth.sh 2> /dev/null | grep $CONTAINER | awk '{print $2}')
			listening
			echo -en "Genereting the results for establishing connection... \t\t\t"
			tcpdump -tt -r init -e -n 2> /dev/null > init.packet
			tcpdump -tt -r auth -e -n 2> /dev/null > intero.packet
			tcpdump -tt -r auth -e -n 2> /dev/null | awk '/ikev2_auth\[I\]/ || /ikev2_auth\[R\]/{flag=1} flag' > auth.packet
			# Generating INIT information
			init_time=$(time_difference "init")
			init=$(tcpdump -r init -e 2> /dev/null | length)
			init_i=$(echo $init | awk -F' ' '{print $1}')
			init_r=$(echo $init | awk -F' ' '{print $2}')
			# Generating AUTH information
			auth_time=$(time_difference "auth")
			auth_i=$(reassemble $(tcpdump -r auth -e -n "udp and src host $INITIATOR" 2>/dev/null | awk '/ikev2_auth\[I\]/ || /ikev2_auth\[R\]/{flag=1} flag' | length))
			auth_r=$(reassemble $(tcpdump -r auth -e -n "udp and src host $RESPONDER" 2>/dev/null | awk '/ikev2_auth\[I\]/ || /ikev2_auth\[R\]/{flag=1} flag' | length))
			# Checking presence of INTERMEDIATE
			if [ $(stat -c %s "intero.packet") -gt $(stat -c %s "auth.packet") ]; then
				inte_time=$(echo "$(time_difference "intero") - $auth_time" | bc)
				inte_i=$(echo "$(reassemble $(tcpdump -tt -r auth -e -n "udp and src host $INITIATOR" 2> /dev/null | length))-$auth_i" | bc) 
				inte_r=$(echo "$(reassemble $(tcpdump -tt -r auth -e -n "udp and src host $RESPONDER" 2> /dev/null | length))-$auth_r" | bc)
				tot_inte_time=$(echo "$inte_time + $tot_inte_time" | bc)
			else

				inte_time="0"
				inte_i="0"
				inte_r="0"
			fi
			echo -e "${GREEN}Completed${DEFAULT}"

			{
				printf 'Phase\tInitiator\tResponder\tTime\n';
				printf '%s\t%s\t%s\t%s\n' "INIT" "$init_i byte" "$init_r byte" "0$init_time s";
				printf '%s\t%s\t%s\t%s\n' "INTERMEDIATE" "$inte_i byte" "$inte_r byte" "0$inte_time s";
				printf '%s\t%s\t%s\t%s\n' "AUTH" "$auth_i byte" "$auth_r byte" "0$auth_time s";

			} | ./utils/table.sh 4 green
			tot_auth_time=$(echo "scale=6; $auth_time + $tot_auth_time" | bc)
			echo "Errore prima di questa istruzione"
			tot_init_time=$(echo "scale=6; $init_time + $tot_init_time" | bc)
		done

		if [ $N -gt 1 ]; then
			ave_auth_time=$(echo "scale=6; $tot_auth_time / $N" | bc)
			ave_init_time=$(echo "scale=6; $tot_init_time / $N" | bc)
			echo "Avarage INIT Time: 0$ave_init_time"
			if [ $tot_inte_time != "0" ]; then
				ave_inte_time=$(echo "scale=6; 0$tot_inte_time / $N" | bc)
				echo "Avarage INTERMEDIATE Time: 0$ave_inte_time"
			fi
			echo "Avarage AUTH Time: 0$ave_auth_time"
		fi

	fi
	#rm -rf auth init
	rm -rf *.packet
}


main "$@"
