set(SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/EspressoUI.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/EspressoBrewTab.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/EspressoConnectionScreen.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/EspressoSettingsTab.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Settings/SettingsManagerDefaults.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Logging/Logging.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/images/espresso_logo.c

)

set(INCLUDES
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/Settings
        ${CMAKE_CURRENT_SOURCE_DIR}/Logging
)

set(ESPRESSO-UI-INCLUDES ${INCLUDES} PARENT_SCOPE)
set(ESPRESSO-UI-SOURCE ${SOURCES} PARENT_SCOPE)
