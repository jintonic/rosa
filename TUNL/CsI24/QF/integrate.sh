#!/bin/bash
echo "check ROOT files in current folder..."
njobs=`ls -1 SIS*Raw_*_*.root 2>/dev/null | wc -l`
if [ $njobs -le 0 ]; then echo "no ROOT file found. Quit."; exit; fi

echo "process $njobs ROOT files on July 30, 2024 ..."
for file in `ls -1 $PWD/SIS*Raw_20240730*_*.root`; do
  name=$(basename -- $file) # remove path from file name
  out=Integrated${name#SIS3316Raw};
  if [ -f $out ]; then continue; fi # skip processed files
  
  log=${name%root}log; err=${name%root}err; script=${name%root}sh
  echo "#!/bin/sh" > $script
  if [[ $name == *130801* ]]; then
    echo "root -b -q $PWD/integrate.C'(\"$file\", 17935)'" >> $script
  elif [[ $name == *182748* ]]; then
    echo "root -b -q $PWD/integrate.C'(\"$file\", 17748)'" >> $script
  else
    echo "root -b -q $PWD/integrate.C'(\"$file\")'" >> $script
  fi
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
