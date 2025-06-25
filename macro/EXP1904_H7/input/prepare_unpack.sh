#!/bin/bash

INPUTLMDDIR=$(pwd)
LOG_ERRORS=""

usage()
{
    printf "Usage: $0 -f FILENAME [-e] [-h]\n\n\
This script provides a path to an LMD file, which contains raw data from an experiment and a path to an \n\
XML file with description of the memory fields in the LMD file. The paths are written to \n\
the /opt/accdaq/run.sh bash script, which is used to perform further unpacking of the LMD to a ROOT file \n\
with a structure taken from the XML file. $0 script is to be run from the \"input\" directory of \n\
the project root directory. The /opt/accdaq/run.sh script is to be run from the /opt/accdaq directory.\n\n\
OPTIONS:\n\n\
\t-f FILENAME\t- name of the lmd file to unpack. Attention! The file should be located in directory;\n\
\t\t\t \"input\" of the project root directory. FILENAME shall not contain path;\n\
\t-e\t\t- when provided the errors of unpacking are logged;\n\
\t-h\t\t- show usage and exit.\n\n"; 
    exit 1;
}

while getopts ":f:eh" o; do
    case "${o}" in
        f)
            INPUTFILENAME=${OPTARG}
            ;;
        e)
            LOG_ERRORS="log_errors"
            ;;
        h)
            usage
            ;;
    esac
done

if [ -z "${INPUTFILENAME}" ]
then
    usage
fi

sed -i "14s|.*|INPUTLMDDIR=$INPUTLMDDIR|" /opt/accdaq/run.sh
sed -i "22s|.*|OUTPUTROOTDIR=$INPUTLMDDIR|" /opt/accdaq/run.sh
sed -i "27s|.*|SETUPFILE=$INPUTLMDDIR/../parameters/setupEXP1904_final.xml|" /opt/accdaq/run.sh
sed -i "32s|.*|INPUTFILENAME=$INPUTFILENAME|" /opt/accdaq/run.sh
if [ ! -z "$LOG_ERRORS" -a "$LOG_ERRORS" != " " -a "$LOG_ERRORS" == "log_errors" ]
then
    sed -i "37s|.*|TEXTERRFILE=${INPUTLMDDIR}/${INPUTFILENAME}.err.txt|" /opt/accdaq/run.sh
else
    sed -i "37s|.*|TEXTERRFILE=/dev/null|" /opt/accdaq/run.sh
fi
sed -i "40s|.*|TEXTOUTFILE=${INPUTLMDDIR}/${INPUTFILENAME}.out.txt|" /opt/accdaq/run.sh
