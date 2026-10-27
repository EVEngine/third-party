# Only external submodule patches remain; aggregate-owned sources contain their fixes directly.
get_filename_component(TP_ROOT "${CMAKE_CURRENT_LIST_DIR}/.." ABSOLUTE)
execute_process(COMMAND "${CMAKE_COMMAND}"
    "-DPATCH=${CMAKE_CURRENT_LIST_DIR}/patches/medialoader-smooth-normals.patch"
    "-DPATCH_DIR=${TP_ROOT}/medialoader"
    -DPATCH_NORMALIZE_CRLF=ON
    -P "${CMAKE_CURRENT_LIST_DIR}/apply_patch.cmake"
    COMMAND_ERROR_IS_FATAL ANY)
execute_process(COMMAND "${CMAKE_COMMAND}"
    "-DPATCH=${CMAKE_CURRENT_LIST_DIR}/patches/medialoader-image-formats.patch"
    "-DPATCH_DIR=${TP_ROOT}/medialoader"
    -DPATCH_NORMALIZE_CRLF=ON
    -P "${CMAKE_CURRENT_LIST_DIR}/apply_patch.cmake"
    COMMAND_ERROR_IS_FATAL ANY)
execute_process(COMMAND "${CMAKE_COMMAND}"
    "-DPATCH=${CMAKE_CURRENT_LIST_DIR}/patches/medialoader-audio-gapless.patch"
    "-DPATCH_DIR=${TP_ROOT}/medialoader"
    -DPATCH_NORMALIZE_CRLF=ON
    -P "${CMAKE_CURRENT_LIST_DIR}/apply_patch.cmake"
    COMMAND_ERROR_IS_FATAL ANY)
execute_process(COMMAND "${CMAKE_COMMAND}"
    "-DPATCH=${CMAKE_CURRENT_LIST_DIR}/patches/mpg123-signal-handler.patch"
    "-DPATCH_DIR=${TP_ROOT}/medialoader"
    -DPATCH_NORMALIZE_CRLF=ON
    -P "${CMAKE_CURRENT_LIST_DIR}/apply_patch.cmake"
    COMMAND_ERROR_IS_FATAL ANY)
execute_process(COMMAND "${CMAKE_COMMAND}"
    "-DPATCH=${CMAKE_CURRENT_LIST_DIR}/patches/box3d-dynamic-crt.patch"
    "-DPATCH_DIR=${TP_ROOT}/box3d"
    -DPATCH_NORMALIZE_CRLF=ON
    -P "${CMAKE_CURRENT_LIST_DIR}/apply_patch.cmake"
    COMMAND_ERROR_IS_FATAL ANY)
execute_process(COMMAND "${CMAKE_COMMAND}"
    "-DPATCH=${CMAKE_CURRENT_LIST_DIR}/patches/box3d-shared-library.patch"
    "-DPATCH_DIR=${TP_ROOT}/box3d"
    -DPATCH_NORMALIZE_CRLF=ON
    -P "${CMAKE_CURRENT_LIST_DIR}/apply_patch.cmake"
    COMMAND_ERROR_IS_FATAL ANY)
execute_process(COMMAND "${CMAKE_COMMAND}"
    "-DPATCH=${CMAKE_CURRENT_LIST_DIR}/patches/jolt-solved-contacts-rolling.patch"
    "-DPATCH_DIR=${TP_ROOT}/jolt"
    -DPATCH_NORMALIZE_CRLF=ON
    -P "${CMAKE_CURRENT_LIST_DIR}/apply_patch.cmake"
    COMMAND_ERROR_IS_FATAL ANY)
