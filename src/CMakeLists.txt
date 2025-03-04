# 配置 QRC 文件
set(QRC_SOURCE_FILE ../resources/res.qrc)
qt5_add_resources(QT_RESOURCES ${QRC_SOURCE_FILE})

# 定义目标
add_executable(${PROJECT_NAME}
        WIN32
        widget.ui
        ${QT_RESOURCES}
        ../icon.rc
        main.cpp
        widget.cpp
        monitor/TriggerMonitor.cpp
        utils/regex/re.c
        task/SyncTask.cpp
        init/Env.cpp
        utils/SyncUtils.cpp
        utils/rule/RuleHighlighter.cpp
        utils/rule/RuleEditDialog.cpp
        utils/rule/RulesParser.cpp
        utils/db/LevelDBWrapper.cpp
)

# 定义 xxHash 的根目录
set(XXHASH_ROOT "${CMAKE_SOURCE_DIR}/libs/xxHash")
set(XXHASH_BUILD_DIR "${XXHASH_ROOT}/cmake_unoffical/build")
message(STATUS "XXHash build dir: ${XXHASH_BUILD_DIR}")

# add QXlsx、xxHash libs
target_include_directories(${PROJECT_NAME}
        PRIVATE
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/libs/QXlsx/QXlsx/header
        ${CMAKE_SOURCE_DIR}/libs/leveldb/include
        ${CMAKE_SOURCE_DIR}/libs/xxHash
)

# 链接 Qt 库
target_link_libraries(${PROJECT_NAME} Qt5::Core Qt5::Sql Qt5::Gui Qt5::Widgets Qt5::Svg Qt5::Network Qt5::Concurrent QXlsx leveldb xxHash::xxhash)
message(STATUS "Linking with: ${XXHASH_BUILD_DIR}/libxxhash.dll.a")
set(MINGW_DLLS
        "${MINGW_BIN_PATH}/${LIBGCC_DLL}"
        "${MINGW_BIN_PATH}/libstdc++-6.dll"
        "${MINGW_BIN_PATH}/libwinpthread-1.dll"
)

# Windows 特定设置
if (WIN32 AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(DEBUG_SUFFIX)
    if (MSVC AND CMAKE_BUILD_TYPE MATCHES "Debug")
        set(DEBUG_SUFFIX "d")
    endif ()

    set(QT_INSTALL_PATH "${CMAKE_PREFIX_PATH}")
    if (NOT EXISTS "${QT_INSTALL_PATH}/bin")
        set(QT_INSTALL_PATH "${QT_INSTALL_PATH}/..")
        if (NOT EXISTS "${QT_INSTALL_PATH}/bin")
            set(QT_INSTALL_PATH "${QT_INSTALL_PATH}/..")
        endif ()
    endif ()

    # 创建插件目录并复制平台插件
    if (EXISTS "${QT_INSTALL_PATH}/plugins/platforms/qwindows${DEBUG_SUFFIX}.dll")
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:${PROJECT_NAME}>/plugins/platforms/")
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy "${QT_INSTALL_PATH}/plugins/platforms/qwindows${DEBUG_SUFFIX}.dll" "$<TARGET_FILE_DIR:${PROJECT_NAME}>/plugins/platforms/")
    endif ()

    # 复制所需的 Qt DLL 文件
    foreach (QT_LIB Core Gui Widgets)
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy "${QT_INSTALL_PATH}/bin/Qt5${QT_LIB}${DEBUG_SUFFIX}.dll" "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
        )
    endforeach ()

    # 复制 MinGW DLL
    foreach (DLL ${MINGW_DLLS})
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different "${DLL}" "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
        )
    endforeach ()

    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND "${QT_INSTALL_PATH}/bin/windeployqt.exe"
            --no-compiler-runtime
            --no-opengl-sw
            --release
            "$<TARGET_FILE:${PROJECT_NAME}>"
    )
endif ()

# 设置安装前缀为 build/install
set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/install)

# 安装可执行文件
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)

# 安装资源文件
install(FILES ${QRC_SOURCE_FILE} DESTINATION resources)

# 安装 Qt DLL 文件
install(CODE "
    file(GLOB QT_DLLS \"${CMAKE_PREFIX_PATH}/../../bin/Qt5*.dll\")
    foreach (DLL ${QT_DLLS})
        file(INSTALL ${DLL} DESTINATION \"${CMAKE_INSTALL_PREFIX}/bin\")
    endforeach()
")

# 安装 Windows 平台插件
if (WIN32)
    install(CODE "
        file(MAKE_DIRECTORY \"${CMAKE_INSTALL_PREFIX}/bin/plugins/platforms\")
        file(INSTALL \"${CMAKE_PREFIX_PATH}/../../plugins/platforms/qwindows.dll\"
            DESTINATION \"${CMAKE_INSTALL_PREFIX}/bin/plugins/platforms\")
    ")
endif ()

# 调试信息
message(STATUS "Qt Install Path: ${QT_INSTALL_PATH}")
