# When debugging the timezone widget, add this debugging definition
# to have a debugging-friendly timezone widget, debug logging,
# and no intrusive timezone-setting while clicking around.
option( DEBUG_TIMEZONES "Debug-friendly timezone widget." OFF )
if( DEBUG_TIMEZONES )
    add_definitions( -DDEBUG_TIMEZONES )
endif()

include_directories( ${PROJECT_BINARY_DIR}/src/libcalamaresui )

calamares_add_plugin( locale
    TYPE viewmodule
    EXPORT_MACRO PLUGINDLLEXPORT_PRO
    SOURCES
        ${geoip_src}
        Config.cpp
        LCLocaleDialog.cpp
        LocaleConfiguration.cpp
        LocalePage.cpp
        LocaleViewStep.cpp
        SetTimezoneJob.cpp
        timezonewidget/timezonewidget.cpp
        timezonewidget/localeglobal.cpp
    UI
    RESOURCES
        locale.qrc
    LINK_PRIVATE_LIBRARIES
        calamaresui
        Qt5::Network
        ${geoip_libs}
        yamlcpp
    SHARED_LIB
)

calamares_add_test(
    localetest
    SOURCES
        Tests.cpp
        LocaleConfiguration.cpp
)
