#!/bin/bash
# https://stackoverflow.com/a/23930212/1801749
read -r -d '' HELP << END
Submit integrate.C jobs to SLURM
 
Usage:
./integrate.sh /path/to/input/files/folder
END

echo "parse arguments..."
if [ $# -lt 1 ]; then echo "$HELP"; exit; fi # missing arguments
test -d "$1" || { echo "$1 doesn't exist. Quit."; exit; }
if [ ${1:0:1} != '/' ]; then # relative path
  dir=$PWD/$1
else # absolute path
  dir=$1
fi

echo "check BDchannels*.txt in $dir..."
njobs=`ls -1 $dir/BDchannels*.txt 2>/dev/null | wc -l`
if [ $njobs -le 0 ]; then echo "no BDchannels*.txt found. Quit."; exit; fi

echo "submitting $njobs jobs..."
for file in `ls -1 $dir/BDchannels_*_*.txt`; do
  name=$(basename -- $file) # remove path from file name
  in=SIS3316Raw${name#BDchannels}; in=${in%txt}root;
  out=Integrated${name#BDchannels}; out=${out%txt}root;
  if [ -f $out ]; then continue; fi # skip processed files
  
  log=${out%root}log; err=${out%root}err; script=${out%root}sh
  echo "#!/bin/sh" > $script
  echo "root -b -q $PWD/integrate.C'(\"$dir/$in\")'" >> $script
  number=${name##*_}; number=${number%.root} # get number from file name
  sbatch -J int$number -o $log -e $err $script
done

echo "check progress..."
while true; do
  njobs=`squeue | egrep " int[0-9]+" | wc -l`
  if [ $njobs -eq 0 ]; then break; fi
  squeue | head -n 1; squeue | egrep " int[0-9]+"; sleep 3
done
chmod 664 *.err *.log *.root &>/dev/null; chmod 775 *_*.sh &>/dev/null
