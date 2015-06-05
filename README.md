# OCRus Java Wrapper

## Overview
This is the Java wrapper for OCRus back-end part that can be used in Windows and Linux system. You can use command line or GUI to do OCR. 

## How to use

### Prerequisites
You need the following installed and available in your environment variables:

* [Java 1.7](http://java.oracle.com)

* [Apache maven 3.0.3 or greater](http://maven.apache.org/)

### 1. Step: Install Tesseract

#### Windows Users:
Download [Tesseract 3.02.02](https://code.google.com/p/tesseract-ocr/downloads/detail?name=tesseract-ocr-setup-3.02.02.exe&can=2&q=) and install it.

#### Linux users:
If they are not already installed, you need the following libraries (Ubuntu):

```
sudo apt-get install autoconf automake libtool
sudo apt-get install libpng12-dev
sudo apt-get install libjpeg62-dev
sudo apt-get install libtiff4-dev
sudo apt-get install zlib1g-dev
```

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
sudo ldconfig
```

### 2. Step: Add Language Training Data
Download [English](https://code.google.com/p/tesseract-ocr/downloads/detail?name=tesseract-ocr-3.02.eng.tar.gz&can=2&q=), [Japanese](https://code.google.com/p/tesseract-ocr/downloads/detail?name=tesseract-ocr-3.02.jpn.tar.gz&can=2&q=), [Chinese](https://code.google.com/p/tesseract-ocr/downloads/detail?name=tesseract-ocr-3.02.chi_sim.tar.gz&can=2&q=) language trained data. Uncompress these 3 files.

#### Windows Users:
Put the `eng.traineddata`, `jpn.traineddata`, `chi_sim.traineddata` to `Tesseract_install_dir\tessdata`, for example `C:\Program Files (x86)\Tesseract-OCR\tessdata`.

#### Linux Users:
Put the `eng.traineddata`, `jpn.traineddata`, `chi_sim.traineddata` to directory `/usr/local/share/tessdata/`.

* If you want to use other language, please [download] the corresponding trained data and put the `*.traineddata` to the above two directories according to the OS.

### 3. Step: Generate GUI and Command Line Tool
Git clone this project and run the following commands:

```
cd ocrus-pc-wrapper
mvn clean package
```

Then you can find `ocrus-java-wrapper-0.0.1-SNAPSHOT-jar-with-dependencies.jar` in `target/` directory.

You can run following command to do OCR by using a GUI:

```
java -jar target/ocrus-java-wrapper-0.0.1-SNAPSHOT-jar-with-dependencies.jar -u
```

The GUI looks like this:

![OCRus GUI](screenshot/GUI.png "OCRus GUI")

You can specify the OCR mode(single image or a directory), input path, output directory, OCR language, and then click `Run OCR` to check the result.

Or, you can run following command to do OCR by using command line:

```
java -jar target/ocrus-java-wrapper-0.0.1-SNAPSHOT-jar-with-dependencies.jar -i input_path -o output_dir [OPTIONS]
```

OPTIONS explanation:

* -h	Print help for Tesseract Java Wrapper
* -s	Single image mode(Default)
* -d	Directory mode
* -l	OCR language(Default is English)
* -i	Input file or input directory (depends on mode). NECESSARY!
* -o	OCR result output directory. NECESSARY!

For example, if you want to do OCR for an image `img.jpg`, the OCR output directory is `ocr-output`, the OCR language is `jpn`, the you can run the following command:

```
java -jar target/ocrus-java-wrapper-0.0.1-SNAPSHOT-jar-with-dependencies.jar -s -i img.jpg -o ocr-output -l jpn
```

After OCR, you can open `ocr-output/img.txt` to check the ocr result.

Another example, if you want to do OCR for all images in directory `imgs`, the OCR output directory is `ocr-output`, the OCR language is `eng`, the you can run the following command:

```
java -jar target/ocrus-java-wrapper-0.0.1-SNAPSHOT-jar-with-dependencies.jar -d -i imgs -o ocr-output -l eng
```

After OCR, you can open `ocr-output/*.txt` to check the corresponding ocr result for different image.

## Q&A
* Why does the command line throw "Error opening data file ..." exception?
>
>The language trained data is not put in the correct directory.
>
>Please put the `.traineddata` file into the tessdata directory that required by the command line tips.
>

* Why is the GUI not working by throw "Headless" exception?
>
>The Java version doesn't include graphics library.
>
>Please make sure the Java version is Oracle 1.7 or higher.
>