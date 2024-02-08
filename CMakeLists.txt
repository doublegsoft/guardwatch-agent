cmake_minimum_required(VERSION 3.6)

project(guardwatch-agent)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pthread")

set(LIBUV       "3rd/libuv-1.44.1")
set(SQLITE      "3rd/sqlite-3.43.2")
set(NETSNMP     "3rd/net-snmp-5.9.4")
set(JSON_C      "3rd/json-c-0.17")
set(RRDTOOL     "3rd/rrdtool-1.8.0")
set(GFC         "3rd/gfc-0.1.1")

set(GW_SOURCES
  src/mustach.c
  src/mustach-wrap.c
  src/mustach-json-c.c
  src/mongoose.c
  src/gw_snmpd.c
  src/gw_sql.c
  src/gw_sqlite.c
  src/gw_httpd.c
  src/gw_app.c
)

set(GW_LIBS
  sqlite3
  netsnmp
  netsnmpagent
  netsnmpmibs
  json-c
  rrd
  gfc
)

file(GLOB_RECURSE GW_WWW_FILES
  www/*
)

function(copy_directory SOURCE_PATH)
  file(GLOB INNER_FILES ${SOURCE_PATH}/*)
  foreach (RESOURCE ${INNER_FILES})
    if(IS_DIRECTORY ${RESOURCE})
      file(RELATIVE_PATH RELPATH "${CMAKE_CURRENT_SOURCE_DIR}" ${RESOURCE})
      copy_directory(${RELPATH})
    else()
      file(RELATIVE_PATH RELPATH "${CMAKE_CURRENT_SOURCE_DIR}" ${RESOURCE})
      get_filename_component(RELPATH ${RELPATH} DIRECTORY)
      set_property(SOURCE ${RESOURCE} PROPERTY MACOSX_PACKAGE_LOCATION "Resources/${RELPATH}")
    endif()
  endforeach(RESOURCE)
endfunction()

include_directories(
  "src"
  "/opt/local/include"
  "${GFC}/include"
  "${CMAKE_BINARY_DIR}/${SQLITE}"
  "${CMAKE_BINARY_DIR}/${NETSNMP}/include"
  "${NETSNMP}/include"
  "${NETSNMP}/snmplib"
  "${JSON_C}"
  "${CMAKE_BINARY_DIR}/${JSON_C}"
  "${RRDTOOL}/src"
)

link_directories(
  "/opt/local/lib"
  "${SQLITE}/.libs"
  "${CMAKE_BINARY_DIR}/${SQLITE}/.libs"
  "${CMAKE_BINARY_DIR}/${NETSNMP}/agent/.libs"
  "${CMAKE_BINARY_DIR}/${NETSNMP}/snmplib/.libs"
  "${CMAKE_BINARY_DIR}/${RRDTOOL}/src/.libs"
)

add_subdirectory("${JSON_C}")
set(WITH_GFC_CRYPTO OFF CACHE BOOL "gfc_crypto.c" FORCE)
set(WITH_GFC_JVM OFF CACHE BOOL "gfc_jvm.c" FORCE)
add_subdirectory("${GFC}")

if(APPLE)

  set(RESOURCES_ICONS
    res/macosx/guardwatch.icns
    res/macosx/guardwatch-running.icns
    res/macosx/guardwatch-status.png
  )

  set(RESOURCES_MIBS
    mib/guardwatch-agent.mib
  )

  set(RESOURCES_WWW
    ${GW_WWW_FILES}
  )

  set(FRAMEWORKS_LIBS
    ${CMAKE_BINARY_DIR}/${SQLITE}/.libs/libsqlite3.0.dylib
    ${CMAKE_BINARY_DIR}/${NETSNMP}/agent/.libs/libnetsnmpagent.40.dylib
    ${CMAKE_BINARY_DIR}/${NETSNMP}/agent/.libs/libnetsnmpmibs.40.dylib
    ${CMAKE_BINARY_DIR}/${NETSNMP}/snmplib/.libs/libnetsnmp.40.dylib
  )

  add_executable(guardwatch-agent MACOSX_BUNDLE
    ${GW_SOURCES}
    src/macosx/gw_main.m

    res/macosx/Info.plist
    ${RESOURCES_ICONS}
    ${RESOURCES_MIBS}
    ${RESOURCES_WWW}
    ${FRAMEWORKS_LIBS}
  )
  target_link_libraries(guardwatch-agent PRIVATE
    ${GW_LIBS}
    "-framework Foundation"
    "-framework Cocoa"
  )

  set_source_files_properties(${RESOURCES_ICONS} PROPERTIES
    MACOSX_PACKAGE_LOCATION                       Resources
  )

  set_source_files_properties(${RESOURCES_MIBS} PROPERTIES
    MACOSX_PACKAGE_LOCATION                       Resources/mib
  )

  set_source_files_properties(${RESOURCES_WWW} PROPERTIES
    MACOSX_PACKAGE_LOCATION                       Resources/www
  )
  copy_directory("www")


  set_source_files_properties(${FRAMEWORKS_LIBS} PROPERTIES
    MACOSX_PACKAGE_LOCATION                       Frameworks
  )

  set_target_properties(guardwatch-agent PROPERTIES
    OUTPUT_NAME                                   "Guardwatch Agent"
    MACOSX_BUNDLE                                 TRUE
    MACOSX_RPATH                                  TRUE
    XCODE_ATTRIBUTE_LD_RUNPATH_SEARCH_PATHS       "@executable_path/../Frameworks"
    MACOSX_BUNDLE_INFO_PLIST                      ../../res/macosx/Info.plist
  )
ELSEIF(WIN32)

  add_executable(guardwatch-agent
    ${GW_SOURCES}
    "src/windows/gw_os.c"
    "src/windows/gw_win.c"
    "src/windows/gw_main.c"
  )
  target_link_libraries(guardwatch-agent PRIVATE
    ${GW_LIBS}
    pthread
  )
ELSEIF(CMAKE_SYSTEM_NAME STREQUAL "Linux")

  add_executable(guardwatch-agent
    ${GW_SOURCES}
    "src/linux/gw_main.c"
    "src/linux/gw_os.c"
  )
  target_link_libraries(guardwatch-agent PRIVATE
    ${GW_LIBS}
  )
endif(APPLE)