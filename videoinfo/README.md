# Problem

Develop an application that computes frame and sequence level peak signal to noise ratio
(PSNR) scores. Input to the application should be reference and test video files in YUV format.
The application should support 4:4:4, 4:2:2, and 4:2:0 subsampling. The files can
be converted to YUV format using ffmpeg Sequence level PSNR score and processing speed in frames/sec
should be printed on a console. The source code should be divided in two parts, video read and PSNR calculation.

_Hints:_

As YUV file format does not contain metadata, therefore, a user of the application should provide
video parameters such as width and height to the application through the command line
interface.
PSNR is to be computed for all the color planes of an image. Color planes PSNR scores should be
aggregated to a frame score by taking an average.
Sequence level score should be computed by taking an average of frame level scores.

# Solution

## Pre-requistes

`sudo apt install ffmpeg`


## Prepare Videos

```
./mpg_to_yuv.sh <path/to/folder/with/mpg/videos>
```

Running this command will create a `raw_videos/` folder in the directory where the script was run.

## Run

```
root@root:~/Git/fun_coding_questions/videoinfo$ ./build/videoinfo --help

USAGE: videoinfo -s SAMPLING -w WIDTH -h HEIGHT ref-file test-file

    Computes PSNR between a reference and test video streams.
Options:
  --help                Print help messages
  -s [ --sampling ] arg One of '4:4:4', '4:2:2' , or '4:2:0'
  -h [ --height ] arg   Height of video file
  -w [ --width ] arg    Width of video file


Positional Arguments: 
  ref-file              Reference video file
  test-file             Test video file

v0.1.0

root@root:~/Git/fun_coding_questions/videoinfo$ time ./build/videoinfo -s 4:4:4 -h 768 -w 432 raw_videos/Ref_768x432_yuv444p.yuv raw_videos/Test_768x432_yuv444p.yuv
Sequence Score: 32.6901dB
FPS: 316.627/sec

real    0m1.906s
user    0m0.532s
sys     0m1.372s

root@root:~/Git/fun_coding_questions/videoinfo$ time ./build/videoinfo -s 4:2:2 -h 768 -w 432 raw_videos/Ref_768x432_yuv422p.yuv raw_videos/Test_768x432_yuv422p.yuv
Sequence Score: 34.1514dB
FPS: 365.266/sec

real    0m1.653s
user    0m0.620s
sys     0m1.032s

root@root:~/Git/fun_coding_questions/videoinfo$ time ./build/videoinfo -s 4:2:0 -h 768 -w 432 raw_videos/Ref_768x432_yuv420p.yuv raw_videos/Test_768x432_yuv420p.yuv 
Sequence Score: 32.7141dB
FPS: 460.481/sec

real    0m1.312s
user    0m0.468s
sys     0m0.840s
```

## Build

```
root@root:~/Git/fun_coding_questions/videoinfo$ ./configure.sh && make -C build -j8
-- The C compiler identification is GNU 5.4.0
-- The CXX compiler identification is GNU 5.4.0
-- Check for working C compiler: /usr/bin/cc
-- Check for working C compiler: /usr/bin/cc -- works
-- Detecting C compiler ABI info
-- Detecting C compiler ABI info - done
-- Detecting C compile features
-- Detecting C compile features - done
-- Check for working CXX compiler: /usr/bin/c++
-- Check for working CXX compiler: /usr/bin/c++ -- works
-- Detecting CXX compiler ABI info
-- Detecting CXX compiler ABI info - done
-- Detecting CXX compile features
-- Detecting CXX compile features - done
-- Configuring done
-- Generating done
-- Build files have been written to: /home/root/Git/fun_coding_questions/videoinfo/build
make: Entering directory '/home/root/Git/fun_coding_questions/videoinfo/build'
make[1]: Entering directory '/home/root/Git/fun_coding_questions/videoinfo/build'
make[2]: Entering directory '/home/root/Git/fun_coding_questions/videoinfo/build'
Scanning dependencies of target videoinfo
make[2]: Leaving directory '/home/root/Git/fun_coding_questions/videoinfo/build'
make[2]: Entering directory '/home/root/Git/fun_coding_questions/videoinfo/build'
[ 50%] Building CXX object CMakeFiles/videoinfo.dir/src/main.cpp.o
[100%] Linking CXX executable videoinfo
make[2]: Leaving directory '/home/root/Git/fun_coding_questions/videoinfo/build'
[100%] Built target videoinfo
make[1]: Leaving directory '/home/root/Git/fun_coding_questions/videoinfo/build'
make: Leaving directory '/home/root/Git/fun_coding_questions/videoinfo/build'
```


