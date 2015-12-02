set (LIBS
     opencv_core opencv_imgcodecs opencv_imgproc opencv_highgui 
     opencv_ml opencv_video opencv_features2d opencv_calib3d 
     opencv_objdetect opencv_flann opencv_photo 
     opencv_stitching opencv_superres opencv_ts opencv_videostab 
     lept tesseract gsl gslcblas)
     
include_directories(include)

FILE(GLOB SRC api/*.cpp
              preprocessing/*/*.cpp)

add_executable(ImageProcess main.cpp ${SRC})
target_link_libraries(ImageProcess ${LIBS})

add_executable(ocrus_bounding_box ocrus_bounding_box.cc recognition/recognition.cc)
target_link_libraries(ocrus_bounding_box ${LIBS})

add_executable(ocrus_ocr_draw ocrus_ocr_draw.cc recognition/recognition.cc ${SRC})
target_link_libraries(ocrus_ocr_draw ${LIBS})

INSTALL(TARGETS ocrus_bounding_box
        RUNTIME DESTINATION bin)
