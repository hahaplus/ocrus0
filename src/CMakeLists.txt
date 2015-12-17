set (LIBS
     opencv_core opencv_imgcodecs opencv_imgproc opencv_highgui 
     opencv_ml opencv_video opencv_features2d opencv_calib3d 
     opencv_objdetect opencv_flann opencv_photo 
     opencv_stitching opencv_superres opencv_ts opencv_videostab 
     lept tesseract gsl gslcblas)

FILE(GLOB SRC api/*.cpp
              preprocessing/*/*.cpp
              util/*.cpp
              textExtraction/*.cpp
              recognition/recognition.cc)

add_executable(ImageProcess main.cpp ${SRC})
target_link_libraries(ImageProcess ${LIBS})

add_executable(ocrus_bounding_box
               ocrus_bounding_box.cc ${SRC} binarization/wap_binarize.cc)
target_link_libraries(ocrus_bounding_box ${LIBS})

add_executable(ocrus_ocr_draw ocrus_ocr_draw.cc ${SRC})
target_link_libraries(ocrus_ocr_draw ${LIBS})

add_executable(wap_ocr_test wap_ocr_test.cpp ${SRC})
target_link_libraries(wap_ocr_test ${LIBS})

add_executable(binarizewolfjolion binarization/binarizewolfjolion.cpp)
target_link_libraries(binarizewolfjolion ${LIBS})

INSTALL(TARGETS ocrus_bounding_box ocrus_ocr_draw wap_ocr_test
        RUNTIME DESTINATION bin)
