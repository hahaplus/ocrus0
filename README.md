OCRus OCR Core
==============

Overview
--------
This is the OCRus OCR CORE that preprocesses the image and wraps around the Tesseract OCR Engine.

Installation
------------

### Operation System Prerequisites
Tested on ubuntu 14.04 64bit and other similar Linux systems might work as well

### 1. Step: Install Libraries

```
sudo apt-get install autoconf automake libtool
sudo apt-get install libpng12-dev
sudo apt-get install libjpeg62-dev
sudo apt-get install libtiff4-dev
sudo apt-get install zlib1g-dev
```

Install OpenCV 3.0.0 [Compile and Install OpenCV 3.0.0 on Ubuntu 14.04](http://www.humbug.in/2015/compile-and-install-opencv-3-0-0-on-ubuntu-14-04/).

Download [Leptonica 1.72](http://www.leptonica.org/download.html).

Run following commands to install Leptonica:

```
tar -xzvf leptonica-1.72.tar.gz
cd leptonica-1.72
./configure
make
sudo make install
```

Download [Tesseract 3.02.02](https://code.google.com/p/tesseract-ocr/downloads/detail?name=tesseract-ocr-3.02.02.tar.gz&can=2&q=)　source code.

Run following commands to install Tesseract:

```
./autogen.sh
./configure
make
sudo make install
```

Link shared libraries:

```
sudo ldconfig
```

Install some other libraries

```
sudo apt-get install python-pil
sudo apt-get install cmake
```

### 2. Step: Add Language Training Data
Download [English](https://code.google.com/p/tesseract-ocr/downloads/detail?name=tesseract-ocr-3.02.eng.tar.gz&can=2&q=), [Japanese](https://code.google.com/p/tesseract-ocr/downloads/detail?name=tesseract-ocr-3.02.jpn.tar.gz&can=2&q=), [Chinese](https://code.google.com/p/tesseract-ocr/downloads/detail?name=tesseract-ocr-3.02.chi_sim.tar.gz&can=2&q=) language trained data. Uncompress these 3 files.

Put the `eng.traineddata`, `jpn.traineddata`, `chi_sim.traineddata` to directory `/path/to/install/tesseract/share/tessdata/`.

Run following command to add `TESSDATA_PREFIX` variable to your environment variables.

```
export TESSDATA_PREFIX=/path/to/install/tesseract/share/
```

* NOTE: Language data are in `/path/to/install/tesseract/share/tessdata/`, but `TESSDATA_PREFIX` is `/path/to/install/tesseract/share/`, no `tessdata`.

* If you want to use other language, please [download](https://code.google.com/p/tesseract-ocr/downloads/list) the corresponding trained data and put the `*.traineddata` to the above directory.

### 3. Step: Configure OCRus
1. Clone the repository https://github.com/zhangli140/ocrus0.git
  ```
  git clone https://github.com/zhangli140/ocrus0.git
  ```

2. Checkout the branch that you will work on, usually the 'develop' branch
  ```
  cd ocrus0
  git checkout develop
  ```
  
3. Create a directory that is silbing to ocrus0 for out-source CMake build, here 'ocrus0_build`
  ```
  cd .. 
  mkdir ocrus0_build
  cd ocrus0_build
  ```

4. Generate makefile and Eclipse CDT files
  ```
  cmake -G"Eclipse CDT4 - Unix Makefiles" -D CMAKE_BUILD_TYPE=Debug \
  -DCMAKE_ECLIPSE_GENERATE_SOURCE_PROJECT=TRUE ../ocrus0
  ```

5. In Eclipse, import the existing Eclipse projects both in ocrus0 and ocrus0_build. Now you can build ocrus0. You can also build the project by make.
  ```
  make -j2
  ```

6. Install the program
  ```
  sudo make install
  ```

7. (Optional) Trace tesseract source code in Debug mode: first you need to import tesseract project as makefile project. Then select the ocrus0_build imported project properties and in C/C++ General -> C/C++ Project Paths -> Projects, check tesseract.

### 4. Step: Run the Programs

Programs:

* ocrus_draw_all.py

	Process all the Photo-0000 to Photo-0027 images.
	Output bounding box result and draw the result to image
  
* ocrus_draw_bbox.py

	Draw the bounding box result to image
	
* ocrus_bounding_box

	Print the bounding boxes of an image

* ImageProcess

	The original program that integrates all the preprocessing and recoginition steps

#### Run ImageProcess
Enter into the directory where ImageProcess is located. The directory is 'ocrus0_build/src' if you follow the steps

```
./ImageProcess -i input_path -o output_dir [OPTIONS]
```

OPTIONS explanation:

* -s	Single image mode(Default)
* -d	Directory mode
* -l	OCR language(Default is English)
* -c    Config file path.
* -i	Input file or input directory (depends on mode). NECESSARY!
* -o	OCR result output directory. NECESSARY!

Config File explanation:

* salient   Salient object result directory.
* border    Border dection result directory.
* turn      Transform result directory.
* text      Text detection object result directory.
* binarize  Binarilization result directory.
* denoise   Denoise result directory.
* deskew    Deskew result directory.

Config File is used to control the workflow and store intermediate result. You can check [config/sn.conf](http://192.168.140.36/snapnote/snapnoteocrcore/blob/master/config/sn.conf) as an example.

* NOTE: Please make sure these directories in Config File exist.

For example, if you want to do OCR for an image `img.jpg`, the OCR output directory is `ocr-output`, the OCR language is `jpn`, the you can run the following command:

```
./ImageProcess -s -c config/sn.conf -i img.jpg -o ocr-output -l jpn
```

Another example, if you want to do OCR for all images in directory `imgs`, the OCR output directory is `ocr-output`, the OCR language is `eng`, the you can run the following command:

```
./ImageProcess -d -c config/sn.conf -i imgs -o ocr-output -l eng
```

After OCR, you can check directories in `config/sn.conf` to check the intermediate result and open `ocr-output/*.txt` to check the ocr result.

## More Explanation about Source Code
In the /src folder you can find different source code for different function

* /borderPostition: used to detect the border of the the object in image
* /preprocessing:
 + /binarize: used for making colorful image into high quality white-and-black image
 + /deskew: (find the skew of the texts, and turn it horizontally, not completed)
        an alternative is /textDetect/textorient.h
 + /noiseLevel: detect the noise level of the image
 + /GaussianSPDenoise: denoise
 + /shadow: remove shadow in image
 + /salientRecognition: find main object range in image
 + /simpleNLP: this is actually post-processing after ocr, which will be running on phone
        ```
        contact.h: recognize phone number and email
        namecardPost.h: a untrained model to analyze structure of namecard
        textClassifier.h: a classifier to judge whether a text is a namecard or not
        ```

## Related Project
[Here](http://192.168.140.36/snapnote/snapnotenlp/tree/master) is the server-side post-processing code.

You will find services for named-entity recognition, keywords extraction and event extraction in package `jp.co.worksap.snapnote.nlp` and `jp.co.worksap.snapnote.services`.
* Note: Java Restful Services, currently not linked to our release

You will find service for English spellcorrection in package `jp.co.worksap.spellcorrection.service`.
