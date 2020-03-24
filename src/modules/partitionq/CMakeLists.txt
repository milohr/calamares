# When debugging the partitioning widget, or experimenting, you may
# want to allow unsafe partitioning choices (e.g. doing things to the
# current disk). Set DEBUG_PARTITION_UNSAFE to allow that (it turns off
# some filtering of devices).
option( DEBUG_PARTITION_UNSAFE "Allow unsafe partitioning choices." OFF )
option( DEBUG_PARTITION_LAME   "Unsafe partitioning will error out on exec." ON )

include_directories( ${CMAKE_SOURCE_DIR} )  # For 3rdparty

set( _partition_defs )
if( DEBUG_PARTITION_UNSAFE )
    if( DEBUG_PARTITION_LAME )
        list( APPEND _partition_defs DEBUG_PARTITION_LAME )
    endif()
    list( APPEND _partition_defs DEBUG_PARTITION_UNSAFE )
endif()

find_package(ECM ${ECM_VERSION} REQUIRED NO_MODULE)

find_package( KPMcore 3.3 )
set_package_properties(
    KPMcore PROPERTIES
    PURPOSE "For partition module"
)
find_package( KF5Config CONFIG )
find_package( KF5I18n CONFIG )
find_package( KF5WidgetsAddons CONFIG )

if ( KPMcore_FOUND AND Qt5DBus_FOUND AND KF5CoreAddons_FOUND AND KF5Config_FOUND )
    if ( KPMcore_VERSION VERSION_GREATER "3.3.0")
        list( APPEND _partition_defs WITH_KPMCORE331API) # kpmcore > 3.3.0 with deprecations
    endif()
    if ( KPMcore_VERSION VERSION_GREATER "3.90")
        list( APPEND _partition_defs WITH_KPMCORE4API) # kpmcore 4 with new API
    endif()
    if( KPMcore_VERSION VERSION_GREATER_EQUAL "4.2" )
        list( APPEND _partition_defs WITH_KPMCORE42API) # kpmcore 4.2 with new API
    endif()

    include_directories( ${KPMCORE_INCLUDE_DIR} )
    include_directories( ${PROJECT_BINARY_DIR}/src/libcalamaresui )

    calamares_add_plugin( partitionq
        TYPE viewmodule
        EXPORT_MACRO PLUGINDLLEXPORT_PRO
        SOURCES
            PartitionQmlViewStep.cpp
        LINK_PRIVATE_LIBRARIES
            kpmcore
            calamaresui
            KF5::CoreAddons
        COMPILE_DEFINITIONS ${_partition_defs}
        SHARED_LIB
    )
else()
    if ( NOT KPMcore_FOUND )
        calamares_skip_module( "partition (missing suitable KPMcore)" )
    else()
        calamares_skip_module( "partition (missing dependencies for KPMcore)" )
    endif()
endif()
