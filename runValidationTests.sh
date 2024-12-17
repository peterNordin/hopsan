#!/bin/bash

# Shell script for running the validation tests on models

failed=0
startDir=$(pwd)
hopsaninstalldir=${startDir}
searchdir=""
if [[ $# -ge 1 ]]; then
  hopsaninstalldir="$1"
fi
if [[ $# -ge 2 ]]; then
  searchdir="$2"
fi

cd ${hopsaninstalldir}/bin
if [ -x hopsancli_d ]; then
  cmd="./hopsancli_d"
elif [ -x hopsancli ]; then
  cmd="./hopsancli"
else
  echo "Error: hopsancli not found in ${hopsaninstalldir}/bin"
  exit 1
fi

echo "Using $cmd for evaluation"
#sleep 1

# Now run hopsancli model unit test on all  hopsanvalidationconfig files found
echo    "**********************************************************" > valtest_failed
echo -n "Validation tests that failed: " >> valtest_failed
echo `date` >> valtest_failed
echo    "**********************************************************" >> valtest_failed
while read line; do
  #echo "Evaluating $line"
  $cmd -t "$line" > valtest_output
  if [ $? -ne 0 ]; then
    failed=1
    echo $line >> valtest_failed
  fi
  cat valtest_output | grep "Test successful: \|failed\|Failed\|Error: \|Warning: " 
done < <(find "$startDir/$searchdir" -name "*.hvc")
if [ -f valtest_output ]; then
  rm valtest_output
fi

if [ $failed -eq 1 ]; then
  setterm -foreground red
  echo
  cat valtest_failed
fi

setterm -default
cd $startDir
exit $failed

