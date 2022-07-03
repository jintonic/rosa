#!/bin/bash
# https://stackoverflow.com/a/23930212/1801749
read -r -d '' HELP << END
Submit indexing jobs to a Portable Batch System (PBS)
 
Usage:
/path/to/idx.sh /path/to/input/files/folder /path/to/output/files/folder
END

echo "parse arguments..."
if [ $# -lt 2 ]; then echo "$HELP"; exit; fi # missing arguments
test -d "$1" || { echo "$1 doesn't exist. Quit."; exit; }
test -d "$2" || { echo "$2 doesn't exist. Quit."; exit; }
if [ ${2:0:1} != '/' ]; then # relative path
  exp=$PWD/$2
else # absolute path
  exp=$2
fi

echo "check binary files in $1..."
njobs=`ls -1 $1/SIS*Raw_*_*.bin 2>/dev/null | wc -l`
if [ $njobs -le 0 ]; then echo "no binary file in $1. Quit."; exit; fi

echo "submit $njobs indexing jobs..."
for file in `ls -1 $1/SIS*Raw_*_*.bin`; do
  name=$(basename -- $file) # remove path from file name
  number=${name##*_}; number=${number%.bin} # get number from file name
  log=${name%.bin}.log; err=${name%.bin}.err; script=${name%.bin}.sh
  echo "#!/bin/sh" > $script
  echo "root -b -q $PWD/idx.C'(\"$file\",\"$exp\")'" >> $script
  # man qsub. -V: copy ENV to node; err & output must be separated at hcdata
  qsub -V -N idx$number -o $log -e $err $script
done

echo "check progress..."
while true; do
  qstat
  njobs=`qstat | egrep " idx[0-9]+" | wc -l`
  if [ $njobs -eq 0 ]; then break; fi
  sleep 3
done
