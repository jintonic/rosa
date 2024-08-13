#!/bin/bash
echo "check ROOT files in current folder..."
njobs=`ls -1 Integrated_*_1.root 2>/dev/null | wc -l`
if [ $njobs -le 0 ]; then echo "no ROOT file found. Quit."; exit; fi

echo "process ROOT files..."
for run in `ls -1 Integrated_*_1.root`; do
  id=${run%_*.root}; n=`ls -1 ${id}_*.root | wc -l`

  echo "#!/bin/sh" > $id.sh
  echo "root -b -q $PWD/combine.C'(\"$PWD/$id\",$n)'" >> $id.sh
  qsub -V -N $id -o $id.log -e $id.err $id.sh
done

echo "check progress..."
while true; do
  njobs=`qstat | egrep " ...[0-9]+" | wc -l`
  if [ $njobs -eq 0 ]; then break; fi
  qstat | head -n 1; qstat | egrep " ...[0-9]+"; sleep 3
done
chmod 664 *.err *.log &>/dev/null; chmod 775 *_*.sh &>/dev/null
