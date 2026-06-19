function(add_sphinx_target)
    find_program(SPHINX_EXECUTABLE sphinx-build
        HINTS ${CMAKE_SOURCE_DIR}/.venv/bin
        REQUIRED
    )
    add_custom_target(sphinx
        COMMAND ${SPHINX_EXECUTABLE} -b html docs docs/_build/html # Consider -W (warnings as errors) and -n (nitpick - raise error for any missing link) flags later
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Building Sphinx documentation"
        VERBATIM
        DEPENDS doxygen generate-trace
    )
endfunction()
