configure_file(version.are.in version.are)
install(DIRECTORY . DESTINATION area
        FILES_MATCHING
        PATTERN "*"
        PATTERN "*.in" EXCLUDE
        PATTERN "version.are" EXCLUDE
        PATTERN "CMakeLists.txt" EXCLUDE)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/version.are DESTINATION area/)
