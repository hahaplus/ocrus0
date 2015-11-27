set (LIBS
     opencv_core opencv_imgcodecs opencv_imgproc opencv_highgui 
     opencv_ml opencv_video opencv_features2d opencv_calib3d 
     opencv_objdetect opencv_flann opencv_photo 
     opencv_stitching opencv_superres opencv_ts opencv_videostab 
     lept tesseract gsl gslcblas)

add_executable(ImageProcess main.cpp)
target_link_libraries(ImageProcess ${LIBS})
