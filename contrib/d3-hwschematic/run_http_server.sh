#!/bin/bash
##

# Absolute path to this script, e.g. /home/user/bin/foo.sh
SCRIPT=`readlink -f "$0"`
# Absolute path this script is in, thus /home/user/bin
SCRIPTDIR=`dirname "$SCRIPT"`
SCRIPTNAME=`basename "$SCRIPT"`

function print_help {
    echo "Usage: $SCRIPTNAME [-h] -c <core name> -v <core variant> <input file>"
    echo "runs a http server providing d3-hwschematic environemnt"
    echo "  -p <port number>  TCP port number to bind to"
    exit 0
}


PORT_NUMBER=8080
while getopts 'p:h' c
do
  case $c in
    p) PORT_NUMBER=$OPTARG ;;
    h) print_help ;;
  esac
done
shift $((OPTIND-1))

cd $SCRIPTDIR
# make sure server gets killed
trap "kill 0" EXIT
# start server
python3 -m http.server ${PORT_NUMBER} &
sleep 1
xdg-open http://127.0.0.1:8080/
# wait for server
wait
