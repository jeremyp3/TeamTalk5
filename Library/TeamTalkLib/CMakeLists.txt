cmake_minimum_required(VERSION 3.1)
project (TeamTalk5Lib)

set (TEAMTALKLIB_ROOT ${CMAKE_CURRENT_LIST_DIR})

set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG -D_DEBUG")
set (CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -DDEBUG -D_DEBUG")

# WebRTC makes use of std::make_reverse_iterator which is C++14
set (CMAKE_CXX_STANDARD 14)

if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  option (STATICCPP "Link static C and C++ libraries" ON)
  if (STATICCPP)
    set (LINK_LIBS -static-libgcc -static-libstdc++)
  endif()
endif()

# Build Multi-threaded instead of Multi-Treaded DLL to avoid MSVC
# dependency hell
if (MSVC)
  set (COMPILE_FLAGS -DUNICODE -D_UNICODE
    -D_CRT_NONSTDC_NO_WARNINGS
    -D_CRT_SECURE_NO_WARNINGS
    -D_CRT_SECURE_NO_DEPRECATE
    -D_CRT_NONSTDC_NO_DEPRECATE
    -D_WINSOCK_DEPRECATED_NO_WARNINGS)

  foreach (flag_var
      CMAKE_C_FLAGS CMAKE_C_FLAGS_DEBUG CMAKE_C_FLAGS_RELEASE
      CMAKE_C_FLAGS_MINSIZEREL CMAKE_C_FLAGS_RELWITHDEBINFO
      CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
      CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
    if (${flag_var} MATCHES "/MD")
      STRING(REGEX REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
    endif()
  endforeach(flag_var)

  # Use for building TeamTalk toolchain
  set (PLATFORMTOOLSET "/property:PlatformToolset=v142")

else()
  set (COMPILE_FLAGS -Wno-deprecated-declarations)
endif()

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

  if (NOT CMAKE_OSX_DEPLOYMENT_TARGET)
    set (CMAKE_OSX_DEPLOYMENT_TARGET 10.9)
    message(WARNING "No deployment target specified. Defaulting to ${CMAKE_OSX_DEPLOYMENT_TARGET}")
  endif()

  if (NOT CMAKE_OSX_ARCHITECTURES)
    set (CMAKE_OSX_ARCHITECTURES arm64;x86_64)
    message(WARNING "No architecture specified. Defaulting to ${CMAKE_OSX_ARCHITECTURES}")
  endif()
  
elseif (${CMAKE_SYSTEM_NAME} MATCHES "iOS")

  if (NOT CMAKE_OSX_DEPLOYMENT_TARGET)
    set (CMAKE_OSX_DEPLOYMENT_TARGET 7.0)
    message(WARNING "No deployment target specified. Defaulting to ${CMAKE_OSX_DEPLOYMENT_TARGET}")
  endif()
  list (APPEND COMPILE_FLAGS -fembed-bitcode)

elseif (${CMAKE_SYSTEM_NAME} MATCHES "Android")

  if (NOT ANDROID_PLATFORM)
    set (ANDROID_PLATFORM android-21)
    message(WARNING "No Android API level specified. Defaulting to ${ANDROID_NATIVE_API_LEVEL}")
  endif()

endif()

function(set_output_dir target dir)
  set_target_properties (${target} PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${dir}
    LIBRARY_OUTPUT_DIRECTORY_DEBUG ${dir}
    LIBRARY_OUTPUT_DIRECTORY_RELEASE ${dir}
    RUNTIME_OUTPUT_DIRECTORY ${dir}
    RUNTIME_OUTPUT_DIRECTORY_DEBUG ${dir}
    RUNTIME_OUTPUT_DIRECTORY_RELEASE ${dir}
    ARCHIVE_OUTPUT_DIRECTORY ${dir}
    ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${dir}
    ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${dir})
endfunction()

option (FEATURE_LIBVPX "Build TeamTalk DLL with libVPX support" ON)
option (FEATURE_OGG "Build TeamTalk DLL with libOgg support" ON)
option (FEATURE_OPUS "Build TeamTalk DLL with OPUS codec support" ON)
option (FEATURE_OPUSTOOLS "Build TeamTalk DLL with Opus-tools support" ON)
option (FEATURE_SPEEX "Build TeamTalk DLL with Speex codec support" ON)
option (FEATURE_SPEEXDSP "Build TeamTalk DLL with SpeexDSP audio processing support" ON)
option (FEATURE_WEBRTC "Build TeamTalk DLL with WebRTC audio processing support" ON)

if (MSVC)
  option (FEATURE_DSHOW "Build TeamTalk DLL with Microsoft DirectShow Streaming support (obsolete)" OFF)
  option (FEATURE_MEDIAFOUNDATION "Build TeamTalk DLL with Media Foundation Streaming support" ON)
  option (FEATURE_MSDMO "Build TeamTalk DLL with Microsoft DirectX Media Objects (DMO) Resampler support" ON)
  option (FEATURE_VIDCAP "Build TeamTalk DLL with DirectShow Video Capture support (obsolete)" OFF)
  option (FEATURE_LAMEMP3 "Build TeamTalk DLL with MP3 Lame support (obsolete)" OFF)
else()
  option (FEATURE_FFMPEG "Build TeamTalk DLL with FFmpeg support" ON)
endif()
if (FEATURE_FFMPEG)
  if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    option(FEATURE_V4L2 "Build TeamTalk DLL with Video for Linux 2 (V4L2) support (video capture)" ON)
  elseif (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    option(FEATURE_AVF "Build TeamTalk DLL with AVFoundation support (video capture)" ON)
  endif()
endif()

# Sound system options
if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin" OR 
    ${CMAKE_SYSTEM_NAME} MATCHES "Linux" OR
    ${CMAKE_SYSTEM_NAME} MATCHES "Windows")
  option (FEATURE_PORTAUDIO "Build using PortAudio as sound system" ON)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Android")
  option (FEATURE_OPENSLES "Build using OpenSL ES as sound system" ON)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "iOS")
  option (FEATURE_AUDIOUNIT "Build using AudioUnit as sound system" ON)
endif()

add_subdirectory (build)

list (APPEND CMAKE_MODULE_PATH "${TEAMTALKLIB_ROOT}/build")

include (ttsrv)
include (ttdll)
include (unittest)


######################
# libTeamTalk5.so #
######################

option (BUILD_TEAMTALK_DLL "Build TeamTalk5 shared library" ON)

if (BUILD_TEAMTALK_DLL)

  if (MSVS_UNITTEST)
    list (APPEND TTCLIENTDLL_SOURCES ${MSVS_UNITTEST_SOURCES})
  endif()

  add_library (TeamTalk5DLL SHARED ${TTCLIENTDLL_SOURCES} ${TTCLIENTDLL_HEADERS})

  target_include_directories (TeamTalk5DLL PRIVATE ${TTCLIENTDLL_INCLUDE_DIR} INTERFACE ${TEAMTALK_ROOT}/Library/TeamTalk_DLL)
  target_compile_options (TeamTalk5DLL PRIVATE ${TTCLIENTDLL_COMPILE_FLAGS} ${COMPILE_FLAGS})
  target_link_libraries (TeamTalk5DLL PRIVATE ${TTCLIENTDLL_LINK_FLAGS} ${LINK_LIBS})
  set_target_properties (TeamTalk5DLL PROPERTIES OUTPUT_NAME TeamTalk5)

  if (${CMAKE_SYSTEM_NAME} MATCHES "Android")
    set_output_dir(TeamTalk5DLL ${TEAMTALK_ROOT}/Library/TeamTalk_DLL/${CMAKE_ANDROID_ARCH_ABI})
  elseif (${CMAKE_SYSTEM_NAME} MATCHES "iOS")
    set_output_dir(TeamTalk5DLL ${TEAMTALK_ROOT}/Library/TeamTalk_DLL/${CMAKE_OSX_ARCHITECTURES})
  else()
    set_output_dir(TeamTalk5DLL ${TEAMTALK_ROOT}/Library/TeamTalk_DLL)
  endif()

  install (FILES ${CMAKE_CURRENT_LIST_DIR}/../TeamTalk_DLL/TeamTalk.h DESTINATION Library/TeamTalk_DLL)
  install (TARGETS TeamTalk5DLL DESTINATION Library/TeamTalk_DLL)

  if (MSVS_UNITTEST)
    set_target_properties (TeamTalk5DLL PROPERTIES LINK_FLAGS "/LIBPATH:$(VCInstallDir)UnitTest/lib")
    set_target_properties (TeamTalk5DLL PROPERTIES COMPILE_FLAGS "/I$(VCInstallDir)UnitTest/include")
  endif()

endif()

######################
# libTeamTalk5.a #
######################

option (BUILD_TEAMTALK_LIB "Build TeamTalk5 static library" OFF)

if (BUILD_TEAMTALK_LIB)

  if (MSVS_UNITTEST)
    list (APPEND TTCLIENTDLL_SOURCES ${MSVS_UNITTEST_SOURCES})
  endif()

  add_library (TeamTalk5Lib STATIC ${TTCLIENTDLL_SOURCES} ${TTCLIENTDLL_HEADERS})

  target_include_directories (TeamTalk5Lib PRIVATE ${TTCLIENTDLL_INCLUDE_DIR} INTERFACE ${TEAMTALK_ROOT}/Library/TeamTalk_DLL)
  target_compile_options (TeamTalk5Lib PRIVATE ${TTCLIENTDLL_COMPILE_FLAGS} ${COMPILE_FLAGS})
  target_link_libraries (TeamTalk5Lib PRIVATE ${TTCLIENTDLL_LINK_FLAGS} ${LINK_LIBS})
  set_target_properties (TeamTalk5Lib PROPERTIES OUTPUT_NAME TeamTalk5)

  if (${CMAKE_SYSTEM_NAME} MATCHES "Android")
    set_output_dir(TeamTalk5Lib ${TEAMTALK_ROOT}/Library/TeamTalk_DLL/${CMAKE_ANDROID_ARCH_ABI})
  elseif (${CMAKE_SYSTEM_NAME} MATCHES "iOS")
    set_output_dir(TeamTalk5Lib ${TEAMTALK_ROOT}/Library/TeamTalk_DLL/${CMAKE_OSX_ARCHITECTURES})
  else()
    set_output_dir(TeamTalk5Lib ${TEAMTALK_ROOT}/Library/TeamTalk_DLL)
  endif()

  install (TARGETS TeamTalk5Lib DESTINATION Library/TeamTalk_DLL)

endif()



######################
# libTeamTalk5Pro.so #
######################

option (BUILD_TEAMTALK_PRODLL "Build TeamTalk5Pro shared library" ON)

if (BUILD_TEAMTALK_PRODLL)

  if (MSVS_UNITTEST)
    list (APPEND TTCLIENTPRODLL_SOURCES ${MSVS_UNITTEST_SOURCES})
  endif()

  add_library (TeamTalk5ProDLL SHARED
    ${TTCLIENTPRODLL_SOURCES} ${TTCLIENTPRODLL_HEADERS})

  target_include_directories (TeamTalk5ProDLL PRIVATE
    ${TTCLIENTPRODLL_INCLUDE_DIR} INTERFACE ${TEAMTALK_ROOT}/Library/TeamTalk_DLL)

  target_compile_options (TeamTalk5ProDLL PRIVATE
    ${TTCLIENTPRODLL_COMPILE_FLAGS} ${COMPILE_FLAGS})

  target_link_libraries (TeamTalk5ProDLL PRIVATE ${TTCLIENTPRODLL_LINK_FLAGS} ${LINK_LIBS})

  if (${CMAKE_SYSTEM_NAME} MATCHES "Android")
    set_output_dir(TeamTalk5ProDLL ${TEAMTALK_ROOT}/Library/TeamTalk_DLL/${CMAKE_ANDROID_ARCH_ABI})
  elseif (${CMAKE_SYSTEM_NAME} MATCHES "iOS")
    set_output_dir(TeamTalk5ProDLL ${TEAMTALK_ROOT}/Library/TeamTalk_DLL/${CMAKE_OSX_ARCHITECTURES})
  else()
    set_output_dir(TeamTalk5ProDLL ${TEAMTALK_ROOT}/Library/TeamTalk_DLL)
  endif()

  install (FILES ${CMAKE_CURRENT_LIST_DIR}/../TeamTalk_DLL/TeamTalk.h ${CMAKE_CURRENT_LIST_DIR}/../TeamTalk_DLL/TeamTalkSrv.h DESTINATION Library/TeamTalk_DLL)
  install (TARGETS TeamTalk5ProDLL DESTINATION Library/TeamTalk_DLL)
  
  set_target_properties (TeamTalk5ProDLL PROPERTIES OUTPUT_NAME TeamTalk5Pro)

  if (MSVS_UNITTEST)
    set_target_properties (TeamTalk5ProDLL PROPERTIES LINK_FLAGS "/LIBPATH:$(VCInstallDir)UnitTest/lib")
    set_target_properties (TeamTalk5ProDLL PROPERTIES COMPILE_FLAGS "/I$(VCInstallDir)UnitTest/include")
  endif()

endif()

######################
# libTeamTalk5Pro.a #
######################

option (BUILD_TEAMTALK_PROLIB "Build TeamTalk5Pro static library" OFF)

if (BUILD_TEAMTALK_PROLIB)

  if (MSVS_UNITTEST)
    list (APPEND TTCLIENTPRODLL_SOURCES ${MSVS_UNITTEST_SOURCES})
  endif()

  add_library (TeamTalk5ProLib STATIC
    ${TTCLIENTPRODLL_SOURCES} ${TTCLIENTPRODLL_HEADERS})

  target_include_directories (TeamTalk5ProLib PRIVATE
    ${TTCLIENTPRODLL_INCLUDE_DIR} INTERFACE ${TEAMTALK_ROOT}/Library/TeamTalk_DLL)

  target_compile_options (TeamTalk5ProLib PRIVATE
    ${TTCLIENTPRODLL_COMPILE_FLAGS} ${COMPILE_FLAGS})

  target_link_libraries (TeamTalk5ProLib PRIVATE
    ${TTCLIENTPRODLL_LINK_FLAGS} ${LINK_LIBS})

  if (${CMAKE_SYSTEM_NAME} MATCHES "Android")
    set_output_dir(TeamTalk5ProLib ${TEAMTALK_ROOT}/Library/TeamTalk_DLL/${CMAKE_ANDROID_ARCH_ABI})
  elseif (${CMAKE_SYSTEM_NAME} MATCHES "iOS")
    set_output_dir(TeamTalk5ProLib ${TEAMTALK_ROOT}/Library/TeamTalk_DLL/${CMAKE_OSX_ARCHITECTURES})
  else()
    set_output_dir(TeamTalk5ProLib ${TEAMTALK_ROOT}/Library/TeamTalk_DLL)
  endif()

  install (TARGETS TeamTalk5ProLib DESTINATION Library/TeamTalk_DLL)

  set_target_properties (TeamTalk5ProLib PROPERTIES OUTPUT_NAME TeamTalk5Pro)

endif()


##########
# tt5srv #
##########

option (BUILD_TEAMTALK_SRVEXE "Build TeamTalk server" ON)

if (BUILD_TEAMTALK_SRVEXE)
  add_executable (tt5srv
    ${TTSRV_SOURCES} ${TTSRV_HEADERS})

  target_include_directories (tt5srv PRIVATE ${TTSRV_INCLUDE_DIR})

  target_compile_options (tt5srv PRIVATE ${TTSRV_COMPILE_FLAGS} ${COMPILE_FLAGS})

  target_link_libraries (tt5srv PRIVATE ${TTSRV_LINK_FLAGS} ${LINK_LIBS})

  set_output_dir(tt5srv ${TEAMTALK_ROOT}/Server)

  install (TARGETS tt5srv DESTINATION Server)

  if (MSVC)
    set_property(TARGET tt5srv PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../../Server")
  endif()
endif()

#############
# tt5prosrv #
#############

option (BUILD_TEAMTALK_PROSRVEXE "Build TeamTalk server" ON)

if (BUILD_TEAMTALK_PROSRVEXE)

  add_executable (tt5prosrv ${TTSRV_SOURCES} ${TTSRV_HEADERS})

  target_include_directories (tt5prosrv PRIVATE ${TTSRVPRO_INCLUDE_DIR})

  target_compile_options (tt5prosrv PRIVATE ${TTSRVPRO_COMPILE_FLAGS} ${COMPILE_FLAGS})

  target_link_libraries (tt5prosrv PRIVATE ${TTSRVPRO_LINK_FLAGS} ${LINK_LIBS})

  set_output_dir(tt5prosrv ${TEAMTALK_ROOT}/Server)

  install (TARGETS tt5prosrv DESTINATION Server)

  if (MSVC)
    set_property(TARGET tt5prosrv PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../../Server")
  endif()

endif()

if (MSVC)

  ##############
  # tt5svc.exe #
  ##############
  option (BUILD_TEAMTALK_SVCEXE "Build TeamTalk service" ON)

  if (BUILD_TEAMTALK_SVCEXE)
    add_executable (tt5svc ${TTSRV_SOURCES} ${TTSRV_HEADERS})

    target_include_directories (tt5svc PRIVATE ${TTSRV_INCLUDE_DIR})

    target_compile_options (tt5svc PRIVATE ${TTSRV_COMPILE_FLAGS} ${COMPILE_FLAGS}
      -DBUILD_NT_SERVICE)

    target_link_libraries (tt5svc PRIVATE ${TTSRV_LINK_FLAGS} ${LINK_LIBS})

    add_custom_command (
      TARGET tt5svc
      POST_BUILD
      COMMAND mt
      ARGS -manifest tt5svc.exe.manifest "-outputresource:$(TargetPath);#1"
      WORKING_DIRECTORY ${TEAMTALKLIB_ROOT}/build)

    set_output_dir(tt5svc ${TEAMTALK_ROOT}/Server)

    install (TARGETS tt5svc DESTINATION Server)

    set_property(TARGET tt5svc PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../../Server")
  endif()

  #################
  # tt5prosvc.exe #
  #################
  option (BUILD_TEAMTALK_PROSVCEXE "Build TeamTalk Professional service" ON)

  if (BUILD_TEAMTALK_PROSVCEXE)
    add_executable (tt5prosvc ${TTSRV_SOURCES} ${TTSRV_HEADERS})

    target_include_directories (tt5prosvc PRIVATE ${TTSRVPRO_INCLUDE_DIR})

    target_compile_options (tt5prosvc PRIVATE ${TTSRVPRO_COMPILE_FLAGS} ${COMPILE_FLAGS}
      -DBUILD_NT_SERVICE)

    target_link_libraries (tt5prosvc PRIVATE ${TTSRVPRO_LINK_FLAGS} ${LINK_LIBS})

    add_custom_command (TARGET tt5prosvc
      POST_BUILD
      COMMAND mt
      ARGS -manifest tt5svc.exe.manifest "-outputresource:$(TargetPath);#1"
      WORKING_DIRECTORY ${TEAMTALKLIB_ROOT}/build)

    set_output_dir (tt5prosvc ${TEAMTALK_ROOT}/Server)

    install (TARGETS tt5prosvc DESTINATION Server)

    set_property(TARGET tt5prosvc PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../../Server")

  endif()
endif()

####################
# Catch unit-tests #
####################

if (CATCH_UNITTEST)

  add_executable (catchtt ${CATCH_UNITTEST_SOURCES} ${TTCLIENTPRODLL_SOURCES} ${TTCLIENTPRODLL_HEADERS}
      ${TEAMTALKLIB_ROOT}/bin/ttsrv/ServerConfig.cpp ${TEAMTALKLIB_ROOT}/bin/ttsrv/ServerConfig.h
      ${TEAMTALKLIB_ROOT}/bin/ttsrv/ServerGuard.cpp ${TEAMTALKLIB_ROOT}/bin/ttsrv/ServerGuard.h
      ${TEAMTALKLIB_ROOT}/bin/ttsrv/ServerUtil.cpp ${TEAMTALKLIB_ROOT}/bin/ttsrv/ServerUtil.h
      ${TEAMTALKLIB_ROOT}/bin/ttsrv/ServerXML.cpp ${TEAMTALKLIB_ROOT}/bin/ttsrv/ServerXML.h
      ${TEAMTALKLIB_ROOT}/bin/ttsrv/AppInfo.h ${TEAMTALKLIB_ROOT}/settings/Settings.cpp
      ${TEAMTALKLIB_ROOT}/settings/Settings.h)

  if (TOOLCHAIN_CATCH2)
    add_dependencies(catchtt catch2)
    list (APPEND TTCLIENTPRODLL_INCLUDE_DIR ${CATCH_INCLUDE_DIR})
  else()
    # Assuming 'catch' package is installed
  endif()

  target_include_directories (catchtt PRIVATE
    ${TTCLIENTPRODLL_INCLUDE_DIR} ${TINYXML_INCLUDE_DIR})

  target_compile_options (catchtt PRIVATE
    ${TTCLIENTPRODLL_COMPILE_FLAGS} ${COMPILE_FLAGS})

  target_link_libraries (catchtt ${TTCLIENTPRODLL_LINK_FLAGS}
    ${LINK_LIBS} ${CATCH_LINK_FLAGS} ${TINYXML_LINK_FLAGS})

  set_output_dir(catchtt ${TEAMTALK_ROOT}/Library/TeamTalkLib/test)

  set_property(TARGET catchtt PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/test")

  if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
      set_target_properties (catchtt PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_INFO_PLIST ${TEAMTALK_ROOT}/Library/TeamTalkLib/test/Info.plist)
  endif()

endif()
