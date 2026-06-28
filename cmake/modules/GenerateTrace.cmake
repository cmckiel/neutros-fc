function(add_generate_trace_target)
    find_program(PYTHON_EXECUTABLE python3
        HINTS ${CMAKE_SOURCE_DIR}/.venv/bin
        REQUIRED
    )
    add_custom_target(generate-trace
        COMMAND ${PYTHON_EXECUTABLE}
            ${CMAKE_SOURCE_DIR}/tools/extract_trace_tags.py
            --src src
            --tests tests
            --output docs/_generated/trace_tags.rst
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Extracting trace tags"
        VERBATIM
    )
endfunction()
