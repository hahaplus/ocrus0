set (LIBS
     opencv_core opencv_imgcodecs opencv_imgproc opencv_highgui 
     opencv_ml opencv_video opencv_features2d opencv_calib3d 
     opencv_objdetect opencv_flann opencv_photo 
     opencv_stitching opencv_superres opencv_ts opencv_videostab 
     lept tesseract gsl gslcblas)

FILE(GLOB SRC api/*.cpp
              preprocessing/*/*.cpp
              util/*.cpp
              binarization/*.cc
              
              textDetect/*.cpp
              recognition/recognition.cc)

add_executable(ImageProcess main.cpp ${SRC})
target_link_libraries(ImageProcess ${LIBS})

add_executable(ocrus_bounding_box
               ocrus_bounding_box.cc ${SRC})
target_link_libraries(ocrus_bounding_box ${LIBS})

add_executable(ocrus_candidates
               ocrus_candidates.cc ${SRC})
target_link_libraries(ocrus_candidates ${LIBS})

add_executable(ocrus_ocr_draw ocrus_ocr_draw.cc ${SRC})
target_link_libraries(ocrus_ocr_draw ${LIBS})

add_executable(wap_ocr_test wap_ocr_test.cpp ${SRC})
target_link_libraries(wap_ocr_test ${LIBS})

INSTALL(TARGETS ocrus_bounding_box ocrus_candidates ocrus_ocr_draw wap_ocr_test
        RUNTIME DESTINATION bin)
