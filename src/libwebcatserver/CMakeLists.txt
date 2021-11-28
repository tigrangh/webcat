# generate model.gen.hpp from model.idl
add_custom_command (
  OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/model.gen.hpp
  MAIN_DEPENDENCY model.idl
  COMMAND idl ${CMAKE_CURRENT_SOURCE_DIR}/model.idl ${CMAKE_CURRENT_SOURCE_DIR}/model.gen
)

add_definitions(-DWEBCATSERVER_LIBRARY)

# define the library
add_library(webcatserver
    global.hpp
    model.hpp
    model.gen.hpp
    http.hpp
    server.cpp
    server.hpp
    common.cpp
    common.hpp)

# libraries this module links to
target_link_libraries(webcatserver
    PUBLIC
        webcat
        direct_stream
    INTERFACE
        belt.pp
    PRIVATE
#        systemutility
        packet
        socket
#        processor
        utility)

# what to do on make install
install(TARGETS webcatserver
        EXPORT webcat.package
        RUNTIME DESTINATION ${WEBCAT_INSTALL_DESTINATION_RUNTIME}
        LIBRARY DESTINATION ${WEBCAT_INSTALL_DESTINATION_LIBRARY}
        ARCHIVE DESTINATION ${WEBCAT_INSTALL_DESTINATION_ARCHIVE})

install(FILES
    global.hpp
    server.hpp
    DESTINATION ${WEBCAT_INSTALL_DESTINATION_INCLUDE}/libwebcatserver)
