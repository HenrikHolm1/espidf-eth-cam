idf_component_register(SRCS "eth_cam.c"
                    INCLUDE_DIRS ".")

#target_compile_options(${COMPONENT_LIB} PRIVATE -std=gnu++11)

#function(index_html_to_h_file IN_FILE OUT_FILE)
#    file(READ ${IN_FILE} CONTENTS)
#    file(WRITE ${OUT_FILE} "const char index_html[] = R\"=====(")
#    file(APPEND ${OUT_FILE} "${CONTENTS}")
#    file(APPEND ${OUT_FILE} ")=====\";")
#endfunction()
#index_html_to_h_file("index.html" "index_html.h")
spiffs_create_partition_image(storage ../data FLASH_IN_PROJECT)