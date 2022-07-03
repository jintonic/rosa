#!/bin/bash
# https://stackoverflow.com/a/23930212/1801749
read -r -d '' HELP << END
Submit b2r jobs to a Portable Batch System (PBS)
 
Usage:
/path/to/b2r.sh /path/to/input/files/folder
END

echo "parse arguments..."
if [ $# -lt 1 ]; then echo "$HELP"; exit; fi # missing arguments
test -d "$1" || { echo "$1 doesn't exist. Quit."; exit; }
if [ ${1:0:1} != '/' ]; then # relative path
  fullpath=$PWD/$1
else # absolute path
  fullpath=$1
fi

echo "check CSV files in $1..."
njobs=`ls -1 $1/SIS*Raw_*_*.csv 2>/dev/null | wc -l`
if [ $njobs -le 0 ]; then echo "no CSV file in $1. Quit."; exit; fi

echo "submit $njobs b2r jobs..."
for file in `ls -1 $fullpath/SIS*Raw_*_*.csv`; do
  name=$(basename -- $file) # remove path from file name
  number=${name##*_}; number=${number%.csv} # get number from file name
  log=${name%.csv}.log; err=${name%.csv}.err; script=${name%.csv}.sh
  echo "#!/bin/sh" > $script
  echo "root -b -q $PWD/b2r.C'(\"$file\")'" >> $script
  # man qsub. -V: copy ENV to node; err & output must be separated at hcdata
  qsub -V -N b2r$number -o $log -e $err $script
done

echo "check progress..."
while true; do
  qstat
  njobs=`qstat | egrep " b2r[0-9]+" | wc -l`
  if [ $njobs -eq 0 ]; then break; fi
  sleep 3
done