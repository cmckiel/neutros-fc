function(add_check_trace_target)
    find_program(PYTHON_EXECUTABLE python3
        HINTS ${CMAKE_SOURCE_DIR}/.venv/bin
        REQUIRED
    )
    add_custom_target(check-trace
        COMMAND ${PYTHON_EXECUTABLE}
            ${CMAKE_SOURCE_DIR}/tools/check_trace.py
            ${CMAKE_SOURCE_DIR}/docs/_build/html/needs.json
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Checking trace"
        VERBATIM
        DEPENDS sphinx
    )
endfunction()
