#!/bin/bash

INPUT_FOLDER=$1
OUTPUT_FOLDER=$PWD/raw_videos

if [ -z "$INPUT_FOLDER" ]; then
    echo ERROR: pass in folder where *.mpg files are stored!
    exit -1
fi

if [ ! -e "$INPUT_FOLDER" ]; then
    echo ERROR: input folder: $INPUT_FOLDER does not exist! Place Ref_* and Test_* videos in there.
    exit -1
fi

VIDEOS=$(find $INPUT_FOLDER -name "*.mpg")
if [ -z "$VIDEOS" ]; then
    echo ERROR: did not find any *.mpg files in $INPUT_FOLDER!
    exit -1
fi

mkdir -p $OUTPUT_FOLDER

for video_file in $VIDEOS
do
	filename=$(basename $video_file)
	filename="${filename%.*}"
	format=$(echo $video_file | grep -oP "yuv[0-9]{3}p")
	output_file=$OUTPUT_FOLDER/$filename.yuv
	echo ""
	echo "########################################################################"
	echo ""
	echo "Converting: $(basename $video_file) to $(basename $output_file) using -pix_fmt: $format ..."
	echo ""
	echo "########################################################################"
	ffmpeg -i $video_file -c:v rawvideo -pix_fmt $format $output_file
	echo
done

