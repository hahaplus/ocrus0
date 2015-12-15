#!/bin/bash

if [ ! -f "$2" ] ; then
    echo "Usage: $0 PROG_BOUNDING_BOX PATH_IMAGE_LIST PATH_RESULT"
    exit 1
fi

PROG_BOUNDING_BOX="$1"
PATH_IMAGE_LIST="$2"
PATH_STATS=${PATH_IMAGE_LIST}_stats.json
PATH_STATS_DATE=${PATH_IMAGE_LIST}_stats_date.json
PATH_STATS_MONEY=${PATH_IMAGE_LIST}_stats_money.json
PATH_RESULT="$3"

# echo Copy groud truth files ...
# cp gt_files_0000-0027/*_gt.json /home/csuncs89/3-useful/2015-11-useful/ocrus0_build/Photos/
# cp gt_files_0201-0262/*_gt.json /media/sf_D_DRIVE/3-useful/2015-12-useful/61TestingData/

echo Build and install ...
make
sudo make install

echo Recognize and generate bounding box files ...
ocrus_draw_all.py $PROG_BOUNDING_BOX both 4 symbol $PATH_IMAGE_LIST > ocrus_draw_all_time.txt

echo Generate ocr_lines files ...
ocrus_to_ocr_lines.py symbol $PATH_IMAGE_LIST

echo Draw ocr_lines ...
ocrus_draw_ocr_lines_all.py $PATH_IMAGE_LIST

echo Calculate running time ...
ocrus_running_time.py ocrus_draw_all_time.txt

echo Calculate accuracy ...
ocrus_calc_accuracy.py $PATH_IMAGE_LIST $PATH_STATS_DATE date
ocrus_calc_accuracy.py $PATH_IMAGE_LIST $PATH_STATS_MONEY money
ocrus_calc_accuracy.py $PATH_IMAGE_LIST $PATH_STATS
echo Accuracy results stored in $PATH_STATS

echo Copying results ...
cp /home/csuncs89/3-useful/2015-11-useful/ocrus0_build/Photos/*_symbol.* "$PATH_RESULT"
cp /media/sf_D_DRIVE/3-useful/2015-12-useful/61TestingData/*_symbol.* "$PATH_RESULT"

cp $PATH_STATS $PATH_RESULT
cp $PATH_STATS_DATE $PATH_RESULT
cp $PATH_STATS_MONEY $PATH_RESULT

echo Exited
