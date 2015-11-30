# OCRus OCR Core

## Overview
This is the OCRus back-end part that preprocess the image and do OCR. 

## How to configure and develop

### 1. Step: Install OpenCV and GSL
Download and install [OpenCV](http://docs.opencv.org/3.0-last-rst/doc/tutorials/introduction/linux_install/linux_install.html).

* Currently we use OpenCV 2.4.10 because the greater version has some compile issues in Windows.

Download and install [GSL](http://www.gnu.org/software/gsl/)

### 2. Step: Install Tesseract
If they are not already installed, you need the following libraries (Ubuntu):

```
sudo apt-get install autoconf automake libtool
sudo apt-get install libpng12-dev
sudo apt-get install libjpeg62-dev
sudo apt-get install libtiff4-dev
sudo apt-get install zlib1g-dev
```

**Note**: if you want to use the recommended CMake way (See 4.1), please install the libraries to default location which is usually '/usr/local'.

Download [Leptonica 1.72](http://www.leptonica.org/download.html).

Run following commands to install Leptonica:

```
tar -xzvf leptonica-1.72.tar.gz
cd leptonica-1.72
./configure --prefix=/path/to/install/leptonica
make
make install (use `sudo make install` if `/path/to/install/leptonica directory` has permission limits)
```
Download [Tesseract 3.02.02](https://code.google.com/p/tesseract-ocr/downloads/detail?name=tesseract-ocr-3.02.02.tar.gz&can=2&q=)　source code.

Run following commands to install Tesseract:

```
./autogen.sh
LIBLEPT_HEADERSDIR=/path/to/install/leptonica/include ./configure --prefix=/path/to/install/tesseract --with-extra-libraries=/path/to/install/leptonica/lib
make
make install (use `sudo make install` if /path/to/install/leptonica directory has permission limits)
```

Create a file `/etc/ld.so.conf.d/tesseract.conf` and add these two lines into the file:

```
/path/to/install/tesseract/lib
/path/to/install/leptonica/lib
```

Run following command to link shared libraries:

```
sudo ldconfig -v
```

### 3. Step: Add Language Training Data
Download [English](https://code.google.com/p/tesseract-ocr/downloads/detail?name=tesseract-ocr-3.02.eng.tar.gz&can=2&q=), [Japanese](https://code.google.com/p/tesseract-ocr/downloads/detail?name=tesseract-ocr-3.02.jpn.tar.gz&can=2&q=), [Chinese](https://code.google.com/p/tesseract-ocr/downloads/detail?name=tesseract-ocr-3.02.chi_sim.tar.gz&can=2&q=) language trained data. Uncompress these 3 files.

Put the `eng.traineddata`, `jpn.traineddata`, `chi_sim.traineddata` to directory `/path/to/install/tesseract/share/tessdata/`.

Run following command to add `TESSDATA_PREFIX` variable to your environment variables.

```
export TESSDATA_PREFIX=/path/to/install/tesseract/share/
```

* NOTE: Language data are in `/path/to/install/tesseract/share/tessdata/`, but `TESSDATA_PREFIX` is `/path/to/install/tesseract/share/`, no `tessdata`.

* If you want to use other language, please [download](https://code.google.com/p/tesseract-ocr/downloads/list) the corresponding trained data and put the `*.traineddata` to the above directory.

### 4. Step: Configure Eclipse IDE for C/C++ Developers
1. Download [Eclipse IDE for C/C++ Developers](http://www.eclipse.org/downloads/packages/eclipse-ide-cc-developers/heliossr2).
1. Start Eclipse. Just run the executable that comes in the folder.
2. Go to **File -> New -> C/C++ Project**
3. Choose a name for your project (i.e. `ImageProcess`). An Empty Project should be okay.
        ![Create Project](screenshot/createProject.png "Create Project")
4. Leave everything else by default. Press Finish.
5. Git clone this project or download the zip file, extract all the file into this project root directory.
6. Add OpenCV, Tesseract, GSL header files and libraries to the project. Do the following:
 + Go to **Project–>Properties**
 + In **C/C++ Build**, click on **Settings**.
 + In **GCC C++ Compiler**, go to **Includes**. In **Include paths(-l)** you should include the path of the folder where OpenCV, Leptonica, Tesseract, GSL were installed:
 
        ![Header Files](screenshot/headerFiles.png "Header Files")
 + In **GCC C++ Linker**, go to **Libraries**. In **Library search path (-L)** you should write the path to where the OpenCV, Leptonica, Tesseract, GSL libraries reside:
 + Then in **Libraries(-l)** add the OpenCV, Leptonica, Tesseract, GSL libraries that you may need. We use the following whole bunch:
        ```
        opencv_core opencv_imgproc opencv_highgui opencv_ml opencv_video opencv_features2d opencv_calib3d opencv_objdetect opencv_flann opencv_photo opencv_stitching opencv_superres opencv_ts opencv_videostab
        lept
        tesseract
        gsl
        gslcblas
        ```
        ![Libraries](screenshot/libraries.png "Libraries")
    Now you are done. Click OK.
7. Your project should be ready to be built. For this, go to **Project->Build all**.

#### 4.1. (Recommended Alternative) Configure with CMake in Eclipse
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
  
4. Run the cmake command to generate makefile and Eclipse CDT files

  ```
  cmake -G"Eclipse CDT4 - Unix Makefiles" -D CMAKE_BUILD_TYPE=Debug -DCMAKE_ECLIPSE_GENERATE_SOURCE_PROJECT=TRUE ../ocrus0
  ```
  
5. In Eclipse, import the existing Eclipse projects both in ocrus0 and ocrus0_build. Build the projects as usual.

6. Trace tesseract source code in Debug mode: first you need to import tesseract project as makefile project. Then select the ocrus0_build imported project properties and in C/C++ General -> C/C++ Project Paths -> Projects, check tesseract.

### 5. Step: Run the Executable
After build, the binary `Debug/ImageProcess` will be generated in project root directory.

Run the following command to do OCR by using command line:

```
Debug/ImageProcess -i input_path -o output_dir [OPTIONS]
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
Debug/ImageProcess -s -c config/sn.conf -i img.jpg -o ocr-output -l jpn
```

Another example, if you want to do OCR for all images in directory `imgs`, the OCR output directory is `ocr-output`, the OCR language is `eng`, the you can run the following command:

```
Debug/ImageProcess -d -c config/sn.conf -i imgs -o ocr-output -l eng
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