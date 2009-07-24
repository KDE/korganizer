set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories( ${CMAKE_SOURCE_DIR}/korganizer ${CMAKE_BINARY_DIR}/korganizer ${CMAKE_SOURCE_DIR}/korganizer/interfaces )

########### next target ###############

set(korgplugins_SRCS korgplugins.cpp )


kde4_add_executable(korgplugins TEST ${korgplugins_SRCS})

target_link_libraries(korgplugins korganizerprivate ${KDE4_KIO_LIBS})

