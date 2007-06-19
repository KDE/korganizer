include_directories( ${CMAKE_SOURCE_DIR}/korganizer ${CMAKE_BINARY_DIR}/korganizer )

########### next target ###############

set(timezone_SRCS timezone.cpp )

kde4_automoc(${timezone_SRCS})


kde4_add_test_executable(timezone ${timezone_SRCS})

target_link_libraries(timezone  korganizer ${KDE4_KIO_LIBS} )
