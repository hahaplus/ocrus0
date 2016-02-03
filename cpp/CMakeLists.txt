set (LIBS
     opencv_core opencv_imgcodecs opencv_imgproc opencv_highgui 
     opencv_ml opencv_video opencv_features2d opencv_calib3d 
     opencv_objdetect opencv_flann opencv_photo 
     opencv_stitching opencv_superres opencv_ts opencv_videostab)

FILE(GLOB SRC preprocessing/*/*.cc)

add_executable(ocrus_binarize_img programs/ocrus_binarize_img.cc
               preprocessing/binarize/binarize.cc)
target_link_libraries(ocrus_binarize_img ${LIBS})

add_executable(ocrus_preprocess_img programs/ocrus_preprocess_img.cc
               ${SRC})
target_link_libraries(ocrus_preprocess_img ${LIBS})

install(TARGETS 
        ocrus_binarize_img
        ocrus_preprocess_img                
        RUNTIME DESTINATION bin)
