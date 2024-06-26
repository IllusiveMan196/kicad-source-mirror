option( KICAD_SHOW_GTK_MESSAGES
    "Show all the GTK error messages in the terminal"
    OFF )

mark_as_advanced( KICAD_SHOW_GTK_MESSAGES )

set( PLATFORM_COMPILE_DEFS "" )

# Add the appropriate source files
if( APPLE )
    set( PLATFORM_SRCS
        osx/app.mm
        osx/drivers.mm
        osx/environment.mm
        osx/io.mm
        osx/policy.mm
        osx/secrets.mm
        osx/ui.mm
        )

    set( PLATFORM_LIBS
        "-framework Cocoa"
        "-framework AppKit"
        "-framework CoreData"
        "-framework Foundation"
        )
elseif( WIN32 )
    set( PLATFORM_SRCS
        msw/app.cpp
        msw/drivers.cpp
        msw/environment.cpp
        msw/io.cpp
        msw/policy.cpp
        msw/secrets.cpp
        msw/ui.cpp
        )

    set( PLATFORM_LIBS
        "Shlwapi"
        "winhttp"
        "wintrust"
        "Imm32"
        )
elseif( UNIX )
    set( PLATFORM_SRCS
        gtk/app.cpp
        gtk/drivers.cpp
        gtk/environment.cpp
        gtk/io.cpp
        gtk/policy.cpp
        gtk/secrets.cpp
        gtk/ui.cpp
        )

    find_package(PkgConfig REQUIRED)
    pkg_check_modules(GTK3 REQUIRED gtk+-3.0)

    # Detect GTK3 and configure it
    set( PLATFORM_LIBS
        ${GTK3_LIBRARIES}
        )

    include_directories( SYSTEM ${GTK3_INCLUDE_DIRS} )
    link_directories( ${GTK3_LIBRARY_DIRS} )
    add_definitions( ${GTK3_CFLAGS_OTHER} )

    # Detect the secret library and configure it
    pkg_check_modules(secret REQUIRED libsecret-1)
    include_directories( SYSTEM ${secret_INCLUDE_DIRS} )
    list( APPEND PLATFORM_LIBS ${secret_LIBRARIES} )

    if( KICAD_WAYLAND )
        find_package(Wayland 1.20 COMPONENTS Client REQUIRED)

        list( APPEND PLATFORM_SRCS gtk/wayland-pointer-constraints-unstable-v1.c )
        list( APPEND PLATFORM_LIBS Wayland::Client  )
    endif()

    # Add the definition to show the GTK error messages if desired (they are hidden
    # by us by default)
    if( KICAD_SHOW_GTK_MESSAGES )
        message( STATUS "Configuring KiCad not to hide any GTK error messages" )
        string( APPEND PLATFORM_COMPILE_DEFS "-DKICAD_SHOW_GTK_MESSAGES" )
    endif()
endif()

include_directories(
    ${INC_AFTER}
    )

add_library( kiplatform STATIC
    ${PLATFORM_SRCS}
    )

target_compile_definitions( kiplatform PRIVATE
    ${PLATFORM_COMPILE_DEFS}
    )

target_include_directories( kiplatform PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    )

target_link_libraries( kiplatform
    core
    ${wxWidgets_LIBRARIES}
    ${PLATFORM_LIBS}
    )

if( APPLE )
    find_library(COREFOUNDATION_LIBRARY CoreFoundation REQUIRED)
    find_library(SECURITY_LIBRARY Security REQUIRED)

    target_link_libraries( kiplatform
            ${COREFOUNDATION_LIBRARY}
            ${SECURITY_LIBRARY}
    )
endif()
