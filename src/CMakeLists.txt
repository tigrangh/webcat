if( WIN32 )
    SET(WEBCAT_INSTALL_DESTINATION_RUNTIME bin)
    SET(WEBCAT_INSTALL_DESTINATION_LIBRARY lib)
    SET(WEBCAT_INSTALL_DESTINATION_ARCHIVE lib)
    SET(WEBCAT_INSTALL_DESTINATION_INCLUDE include)
else( WIN32 )
    SET(WEBCAT_INSTALL_DESTINATION_RUNTIME bin/webcat)
    SET(WEBCAT_INSTALL_DESTINATION_LIBRARY lib/webcat)
    SET(WEBCAT_INSTALL_DESTINATION_ARCHIVE lib/webcat)
    SET(WEBCAT_INSTALL_DESTINATION_INCLUDE include/webcat)
endif( WIN32 )

if( NOT WIN32 )
    # don't skip the full RPATH for the build tree
    SET(CMAKE_SKIP_BUILD_RPATH  FALSE)

    # when building, don't use the install RPATH already
    # (but later on when installing)
    SET(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)

    SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib/webcat;${CMAKE_INSTALL_PREFIX}/lib/belt.pp;${CMAKE_INSTALL_PREFIX}/lib/mesh.pp")

endif()

include(cmake_utility/utility.cmake)

if(NOT TARGET belt.pp)
add_subdirectory(belt.pp)
endif()
#find_package(belt.pp)

add_subdirectory(webcat)
add_subdirectory(webcatd)
add_subdirectory(libwebcatserver)

# following is used for find_package functionality
install(FILES webcat-config.cmake DESTINATION ${WEBCAT_INSTALL_DESTINATION_LIBRARY})
install(EXPORT webcat.package NAMESPACE webcat:: DESTINATION ${WEBCAT_INSTALL_DESTINATION_LIBRARY})
