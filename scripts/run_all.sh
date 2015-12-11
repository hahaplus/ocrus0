#!/bin/bash

if [ ! -f "$1" ] ; then
    echo "Usage: $0 PATH_IMAGE_LIST PATH_RESULT"
    exit 1
fi

PATH_IMAGE_LIST="$1"
PATH_STATS=${PATH_IMAGE_LIST}_stats.json
PATH_RESULT="$2"

# echo Copy groud truth files ...
# cp gt_files_0000-0027/*_gt.json /home/csuncs89/3-useful/2015-11-useful/ocrus0_build/Photos/
# cp gt_files_0201-0262/*_gt.json /media/sf_D_DRIVE/3-useful/2015-12-useful/61TestingData/

echo Build and install ...
make
sudo make install

echo Recognize and generate bounding box files ...
ocrus_draw_all.py ocrus_bounding_box both 4 symbol $PATH_IMAGE_LIST

echo Generate ocr_lines files ...
ocrus_to_ocr_lines.py symbol $PATH_IMAGE_LIST

# echo Draw ocr_lines ...
# ocrus_draw_ocr_lines_all.py $PATH_IMAGE_LIST

echo Calculate accuracy ...
ocrus_calc_accuracy.py $PATH_IMAGE_LIST $PATH_STATS
echo Accuracy results stored in $PATH_STATS

echo Copying results ...
cp /home/csuncs89/3-useful/2015-11-useful/ocrus0_build/Photos/*_symbol.* "$PATH_RESULT"
cp /media/sf_D_DRIVE/3-useful/2015-12-useful/61TestingData/*_symbol.* "$PATH_RESULT"

cp $PATH_STATS $PATH_RESULT

echo Exited
