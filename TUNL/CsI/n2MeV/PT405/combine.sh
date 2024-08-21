#!/bin/bash
# https://stackoverflow.com/a/23930212/1801749
read -r -d '' HELP << END
Submit combine.C jobs to SLURM
 
Usage:
./combine.sh /path/to/input/files/folder
END

echo "parse arguments..."
if [ $# -lt 1 ]; then echo "$HELP"; exit; fi # missing arguments
test -d "$1" || { echo "$1 doesn't exist. Quit."; exit; }
if [ ${1:0:1} != '/' ]; then # relative path
  dir=$PWD/$1
else # absolute path
  dir=$1
fi
echo "check Integrated*.root in $dir..."
njobs=`ls -1 $dir/Integrated_*_1.root 2>/dev/null | wc -l`
if [ $njobs -le 0 ]; then echo "no Integrated*.root found. Quit."; exit; fi

echo "process ROOT files..."
for run in `ls -1 $dir/Integrated_*_1.root`; do
  name=$(basename -- $run) # remove path from file name
  id=${name%_*.root}; n=`ls -1 $dir/${id}_*.root | wc -l`

  echo "#!/bin/sh" > $id.sh
  echo "root -b -q $PWD/combine.C'(\"$dir/$id\",$n)'" >> $id.sh
  sbatch -J $id -o $id.log -e $id.err $id.sh
done

echo "check progress..."
while true; do
  njobs=`squeue | grep " Integr" | wc -l`
  if [ $njobs -eq 0 ]; then break; fi
  squeue | head -n 1; squeue | grep " Integr"; sleep 5
done
chmod 664 *.err *.log &>/dev/null; chmod 775 *_*.sh &>/dev/null
