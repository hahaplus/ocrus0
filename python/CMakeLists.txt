install(PROGRAMS
            ocrus_draw_bbox.py ocrus_draw_all.py 
            ocrus_run_psm_all.py ocrus_to_json.py
            ocrus_draw_gt_all.py ocrus_draw_ocr_lines_all.py 
            ocrus_calc_accuracy.py
        DESTINATION bin)

install(CODE "execute_process(COMMAND python setup.py install 
				WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})")
