#!/bin/bash
echo "check ROOT files in current folder..."
njobs=`ls -1 SIS*Raw_*_*.root 2>/dev/null | wc -l`
if [ $njobs -le 0 ]; then echo "no ROOT file found. Quit."; exit; fi

echo "process $njobs ROOT files..."
for file in `ls -1 $PWD/SIS*Raw_*_*.root`; do
  name=$(basename -- $file) # remove path from file name
  out=BDchannels${name#SIS3316Raw}; out=${out%root}txt
  if [ -f $out ]; then continue; fi # skip processed files

  log=${name%root}log; err=${name%root}err; script=${name%root}sh
  echo "#!/bin/sh" > $script
  echo "root -b -q $PWD/matchTimeStamps.C'(\"$file\")'" >> $script
  number=${name##*_}; number=${number%.root} # get number from file name
  # man qsub. -V: copy ENV to node; err & output must be separated at hcdata
  echo qsub -V -N match$number -o $log -e $err $script
done

echo "check progress..."
while true; do
  njobs=`qstat | egrep " match[0-9]+" | wc -l`
  if [ $njobs -eq 0 ]; then break; fi
  qstat | head -n 1; qstat | egrep " match[0-9]+"; sleep 3
done
chmod 664 *.err *.log &>/dev/null; chmod 775 *_*.sh &>/dev/null
