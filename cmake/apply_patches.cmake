# Dependency patches belong to the aggregate, shared by source and release builds.
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
    "-DPATCH=${CMAKE_CURRENT_LIST_DIR}/patches/third-party-squirrel-ssq-export.patch"
    "-DPATCH_DIR=${TP_ROOT}"
    -DPATCH_NORMALIZE_CRLF=ON
    -P "${CMAKE_CURRENT_LIST_DIR}/apply_patch.cmake"
    COMMAND_ERROR_IS_FATAL ANY)
execute_process(COMMAND "${CMAKE_COMMAND}"
    "-DPATCH=${CMAKE_CURRENT_LIST_DIR}/patches/simplesquirrel-constructor-exceptions.patch"
    "-DPATCH_DIR=${TP_ROOT}"
    -DPATCH_NORMALIZE_CRLF=ON
    -P "${CMAKE_CURRENT_LIST_DIR}/apply_patch.cmake"
    COMMAND_ERROR_IS_FATAL ANY)
execute_process(COMMAND "${CMAKE_COMMAND}"
    "-DPATCH=${CMAKE_CURRENT_LIST_DIR}/patches/simplesquirrel-object-move-init.patch"
    "-DPATCH_DIR=${TP_ROOT}"
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
    "-DPATCH=${CMAKE_CURRENT_LIST_DIR}/patches/sdl2-android-alooper-pollonce.patch"
    "-DPATCH_DIR=${TP_ROOT}"
    -DPATCH_NORMALIZE_CRLF=ON
    -P "${CMAKE_CURRENT_LIST_DIR}/apply_patch.cmake"
    COMMAND_ERROR_IS_FATAL ANY)
execute_process(COMMAND "${CMAKE_COMMAND}"
    "-DPATCH=${CMAKE_CURRENT_LIST_DIR}/patches/squirrel-binding-contract-expression-scope.patch"
    "-DPATCH_DIR=${TP_ROOT}"
    -DPATCH_NORMALIZE_CRLF=ON
    -P "${CMAKE_CURRENT_LIST_DIR}/apply_patch.cmake"
    COMMAND_ERROR_IS_FATAL ANY)
execute_process(COMMAND "${CMAKE_COMMAND}"
    "-DPATCH=${CMAKE_CURRENT_LIST_DIR}/patches/simplesquirrel-stable-type-hash.patch"
    "-DPATCH_DIR=${TP_ROOT}"
    -DPATCH_NORMALIZE_CRLF=ON
    -P "${CMAKE_CURRENT_LIST_DIR}/apply_patch.cmake"
    COMMAND_ERROR_IS_FATAL ANY)
execute_process(COMMAND "${CMAKE_COMMAND}"
    "-DPATCH=${CMAKE_CURRENT_LIST_DIR}/patches/squirrel-nested-call-signature-lifetime.patch"
    "-DPATCH_DIR=${TP_ROOT}"
    -DPATCH_NORMALIZE_CRLF=ON
    -P "${CMAKE_CURRENT_LIST_DIR}/apply_patch.cmake"
    COMMAND_ERROR_IS_FATAL ANY)
execute_process(COMMAND "${CMAKE_COMMAND}"
    "-DPATCH=${CMAKE_CURRENT_LIST_DIR}/patches/mpg123-apple-fpu-detection.patch"
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
    "-DPATCH=${CMAKE_CURRENT_LIST_DIR}/patches/box2d-shared-library.patch"
    "-DPATCH_DIR=${TP_ROOT}"
    -DPATCH_NORMALIZE_CRLF=ON
    -P "${CMAKE_CURRENT_LIST_DIR}/apply_patch.cmake"
    COMMAND_ERROR_IS_FATAL ANY)
execute_process(COMMAND "${CMAKE_COMMAND}"
    "-DPATCH=${CMAKE_CURRENT_LIST_DIR}/patches/sdl2-static-library-crt-entry.patch"
    "-DPATCH_DIR=${TP_ROOT}"
    -DPATCH_NORMALIZE_CRLF=ON
    -P "${CMAKE_CURRENT_LIST_DIR}/apply_patch.cmake"
    COMMAND_ERROR_IS_FATAL ANY)
execute_process(COMMAND "${CMAKE_COMMAND}"
    "-DPATCH=${CMAKE_CURRENT_LIST_DIR}/patches/sdl2-shared-wayland-link.patch"
    "-DPATCH_DIR=${TP_ROOT}"
    -DPATCH_NORMALIZE_CRLF=ON
    -P "${CMAKE_CURRENT_LIST_DIR}/apply_patch.cmake"
    COMMAND_ERROR_IS_FATAL ANY)
execute_process(COMMAND "${CMAKE_COMMAND}"
    "-DPATCH=${CMAKE_CURRENT_LIST_DIR}/patches/jolt-solved-contacts-rolling.patch"
    "-DPATCH_DIR=${TP_ROOT}/jolt"
    -DPATCH_NORMALIZE_CRLF=ON
    -P "${CMAKE_CURRENT_LIST_DIR}/apply_patch.cmake"
    COMMAND_ERROR_IS_FATAL ANY)
