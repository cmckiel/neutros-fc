function(add_doxygen_target)
    find_program(DOXYGEN_EXECUTABLE doxygen REQUIRED)
    add_custom_target(doxygen
        COMMAND ${DOXYGEN_EXECUTABLE} Doxyfile
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/docs/_sphinx/_breathe/_doxygen
        COMMENT "Generating Doxygen XML"
        VERBATIM
    )
endfunction()
