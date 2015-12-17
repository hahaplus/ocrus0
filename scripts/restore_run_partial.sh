#!/bin/bash

if [ ! -f "$2" ] ; then
    echo "Usage: $0 PROG_BOUNDING_BOX PATH_IMAGE_LIST PATH_RESULT"
    exit 1
fi

PROG_BOUNDING_BOX="$1"
PATH_IMAGE_LIST="$2"
PATH_STATS=${PATH_IMAGE_LIST}_stats.json
PATH_RESULT="$3"

# echo Copy groud truth files ...
# cp gt_files_0000-0027/*_gt.json /home/csuncs89/3-useful/2015-11-useful/ocrus0_build/Photos/
# cp gt_files_0201-0262/*_gt.json /media/sf_D_DRIVE/3-useful/2015-12-useful/61TestingData/

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
rm -r ${PATH_RESULT}/bad_lines
mkdir ${PATH_RESULT}/bad_lines
for PATH_IMG in `cat $PATH_IMAGE_LIST`
do
  cp ${PATH_IMG}_symbol.png "$PATH_RESULT"
  mv `dirname ${PATH_IMG}`/bad_lines/*.png ${PATH_RESULT}/bad_lines/ 2> /dev/null
done
cp $PATH_STATS $PATH_RESULT

echo Exited
