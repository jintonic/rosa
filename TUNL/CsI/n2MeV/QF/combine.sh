#!/bin/bash
echo "check ROOT files in current folder..."
njobs=`ls -1 Integrated_*_1.root 2>/dev/null | wc -l`
if [ $njobs -le 0 ]; then echo "no ROOT file found. Quit."; exit; fi

echo "process ROOT files..."
for run in `ls -1 Integrated_*_1.root`; do
  id=${run%_*.root}; n=`ls -1 ${id}_*.root | wc -l`

  echo "#!/bin/sh" > $id.sh
  echo "root -b -q $PWD/combine.C'(\"$PWD/$id\",$n)'" >> $id.sh
  sbatch -J $id -o $id.log -e $id.err $id.sh
done

echo "check progress..."
while true; do
  njobs=`squeue | grep " Integr" | wc -l`
  if [ $njobs -eq 0 ]; then break; fi
  squeue | head -n 1; squeue | grep " Integr"; sleep 5
done
chmod 664 *.err *.log &>/dev/null; chmod 775 *_*.sh &>/dev/null
