#!/bin/bash

if [ ! -f "$2" ] ; then
    echo "Usage: $0 PROG_BOUNDING_BOX PATH_IMAGE_LIST PATH_RESULT"
    exit 1
fi

PROG_BOUNDING_BOX="$1"
PATH_IMAGE_LIST="$2"
PATH_RESULT="$3"

PATH_STATS=`realpath $PATH_RESULT`/${PATH_IMAGE_LIST}_stats.json
PATH_STATS_DATE=`realpath $PATH_RESULT`/${PATH_IMAGE_LIST}_stats_date.json
PATH_STATS_MONEY=`realpath $PATH_RESULT`/${PATH_IMAGE_LIST}_stats_money.json


echo Restore results ...
for PATH_IMG in `cat $PATH_IMAGE_LIST`
do
  cp $PATH_RESULT/`basename $PATH_IMG`_symbol.txt `dirname $PATH_IMG`/
done

echo Build and install ...
make
sudo make install

echo Recognize and generate bounding box files ...
ocrus_draw_all.py $PROG_BOUNDING_BOX draw 4 symbol $PATH_IMAGE_LIST

echo Generate ocr_lines files ...
ocrus_to_ocr_lines.py symbol $PATH_IMAGE_LIST

echo Draw ocr_lines ...
ocrus_draw_ocr_lines_all.py $PATH_IMAGE_LIST

echo Calculate accuracy ...
ocrus_calc_accuracy.py $PATH_IMAGE_LIST $PATH_STATS_DATE date
ocrus_calc_accuracy.py $PATH_IMAGE_LIST $PATH_STATS_MONEY money
ocrus_calc_accuracy.py $PATH_IMAGE_LIST $PATH_STATS
echo Accuracy results stored in $PATH_STATS

echo Copying results ...
rm -rf ${PATH_RESULT}/bad_lines
mkdir -p ${PATH_RESULT}/bad_lines
rm -rf ${PATH_RESULT}/bad
mkdir -p ${PATH_RESULT}/bad
rm -rf ${PATH_RESULT}/good
mkdir -p ${PATH_RESULT}/good
for PATH_IMG in `cat $PATH_IMAGE_LIST`
do
  cp ${PATH_IMG}_symbol.png "$PATH_RESULT"
  mv `dirname ${PATH_IMG}`/bad_lines/*.png ${PATH_RESULT}/bad_lines/ 2> /dev/null
  mv `dirname ${PATH_IMG}`/bad/*_ocr_lines.* ${PATH_RESULT}/bad/ 2> /dev/null
  mv `dirname ${PATH_IMG}`/good/*_ocr_lines.* ${PATH_RESULT}/good/ 2> /dev/null
done

echo Exited
