# define the executable
add_executable(webcatd
    main.cpp)

# libraries this module links to
target_link_libraries(webcatd PRIVATE
    webcat
    webcatserver
    belt.pp
    log
    direct_stream
    Boost::filesystem
    Boost::program_options
    Boost::locale
    )

if(NOT WIN32 AND NOT APPLE)
    find_package(Threads REQUIRED)
    target_link_libraries(webcatd PRIVATE Threads::Threads)
endif()

# what to do on make install
install(TARGETS webcatd
        EXPORT webcat.package
        RUNTIME DESTINATION ${WEBCAT_INSTALL_DESTINATION_RUNTIME}
        LIBRARY DESTINATION ${WEBCAT_INSTALL_DESTINATION_LIBRARY}
        ARCHIVE DESTINATION ${WEBCAT_INSTALL_DESTINATION_ARCHIVE})
