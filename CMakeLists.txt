
file(GLOB SOURCES images/*.c *.cpp)

idf_component_register(SRCS ${SOURCES}
                       INCLUDE_DIRS . ./settings
                       REQUIRES main)
