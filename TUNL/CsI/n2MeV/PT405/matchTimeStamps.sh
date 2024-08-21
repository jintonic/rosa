#!/bin/bash
# https://stackoverflow.com/a/23930212/1801749
read -r -d '' HELP << END
Submit matchTimeStamps.C jobs to SLURM
 
Usage:
./matchTimeStamp.sh /path/to/input/files/folder
END

echo "parse arguments..."
if [ $# -lt 1 ]; then echo "$HELP"; exit; fi # missing arguments
test -d "$1" || { echo "$1 doesn't exist. Quit."; exit; }
if [ ${1:0:1} != '/' ]; then # relative path
  exp=$PWD/$1
else # absolute path
  exp=$1
fi

echo "check ROOT files in $1..."
njobs=`ls -1 $1/SIS*Raw_*_*.root 2>/dev/null | wc -l`
if [ $njobs -le 0 ]; then echo "no ROOT file found. Quit."; exit; fi

echo "process $njobs ROOT files..."
for file in `ls -1 $exp/SIS*Raw_*_*.root`; do
  name=$(basename -- $file) # remove path from file name
  out=BDchannels${name#SIS3316Raw}; out=${out%root}txt
  if [ -f $out ]; then continue; fi # skip processed files

  log=${name%root}log; err=${name%root}err; script=${name%root}sh
  echo "#!/bin/sh" > $exp/$script
  echo "root -b -q $PWD/matchTimeStamps.C'(\"$file\")'" >> $exp/$script
  number=${name##*_}; number=${number%.root} # get number from file name
  sbatch -J match$number -o $log -e $err $exp/$script
done

echo "check progress..."
while true; do
  njobs=`squeue | egrep " match[0-9]+" | wc -l`
  if [ $njobs -eq 0 ]; then break; fi
  squeue | head -n 1; squeue | egrep " match[0-9]+"; sleep 3
done
chmod 664 *.err *.log &>/dev/null; chmod 775 *_*.sh &>/dev/null
