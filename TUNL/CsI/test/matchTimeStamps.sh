#!/bin/bash
echo "check ROOT files in current folder..."
njobs=`ls -1 SIS*Raw_*_*.root 2>/dev/null | wc -l`
if [ $njobs -le 0 ]; then echo "no ROOT file found. Quit."; exit; fi

echo "submit $njobs matching jobs..."
for file in `ls -1 $PWD/SIS*Raw_*_*.root`; do
  name=$(basename -- $file) # remove path from file name
  number=${name##*_}; number=${number%.root} # get number from file name
  log=${name%.root}.log; err=${name%.root}.err; script=${name%.root}.sh
  echo "#!/bin/sh" > $script
  echo "root -b -q $PWD/matchTimeStamps.C'(\"$file\")'" >> $script
  # man qsub. -V: copy ENV to node; err & output must be separated at hcdata
  qsub -V -N match$number -o $log -e $err $script
done

echo "check progress..."
while true; do
  qstat
  njobs=`qstat | egrep " match[0-9]+" | wc -l`
  if [ $njobs -eq 0 ]; then break; fi
  sleep 3
done

