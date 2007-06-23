include_directories( ${CMAKE_SOURCE_DIR}/korganizer ${CMAKE_BINARY_DIR}/korganizer ${CMAKE_SOURCE_DIR}/korganizer/interfaces )

########### next target ###############

set(timezone_SRCS timezone.cpp )

kde4_automoc(${timezone_SRCS})


kde4_add_test_executable(timezone ${timezone_SRCS})

target_link_libraries(timezone  korganizer ${KDE4_KIO_LIBS} )

########### next target ###############

set(korgplugins_SRCS korgplugins.cpp )

kde4_automoc(${korgplugins_SRCS})

kde4_add_test_executable(korgplugins ${korgplugins_SRCS})

target_link_libraries(korgplugins  korganizer ${KDE4_KIO_LIBS} )



