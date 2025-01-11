# 使用传递下来的项目名称
message(STATUS "Current project name: ${PROJECT_NAME}")

# 配置 QRC 文件
set(QRC_SOURCE_FILE ../resources/res.qrc)
qt5_add_resources(QT_RESOURCES ${QRC_SOURCE_FILE})

# 定义目标
add_executable(${PROJECT_NAME} main.cpp widget.cpp System.cpp widget.ui ${QT_RESOURCES} ../icon.rc
        utils/regex/re.c
        task/FileMonitor.cpp
        task/FileProcessTask.cpp
        task/SyncTask.h
)

# 链接 Qt 库
target_link_libraries(${PROJECT_NAME} Qt5::Core Qt5::Gui Qt5::Widgets Qt5::Svg Qt5::Network Qt5::Concurrent)

# 其他逻辑可以继续使用 ${PROJECT_NAME} 来自动绑定目标
# MinGW 运行时 DLL 安装路径
set(MINGW_DLLS
        "D:/devtools/Qt/5.12.9/mingw73_64/bin/libgcc_s_seh-1.dll"
        "D:/devtools/Qt/5.12.9/mingw73_64/bin/libstdc++-6.dll"
        "D:/devtools/Qt/5.12.9/mingw73_64/bin/libwinpthread-1.dll"
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

    # 使用 windeployqt 工具自动部署 DLL（可选）
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND "${QT_INSTALL_PATH}/bin/windeployqt.exe" "$<TARGET_FILE:${PROJECT_NAME}>"
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
    file(GLOB QT_DLLS \"${CMAKE_PREFIX_PATH}/../..//bin/Qt5*.dll\")
    foreach (DLL \${QT_DLLS})
        file(INSTALL \${DLL} DESTINATION \"\${CMAKE_INSTALL_PREFIX}/bin\")
    endforeach ()
")

# 安装 OpenCV DLL 文件
install(CODE "
    file(GLOB OPENCV_DLLS \"${OpenCV_DIR}/x64/mingw/bin/*.dll\")
    foreach (DLL \${OPENCV_DLLS})
        file(INSTALL \${DLL} DESTINATION \"\${CMAKE_INSTALL_PREFIX}/bin\")
    endforeach ()
")

# 安装 Windows 平台插件
if (WIN32)
    install(CODE "
        file(MAKE_DIRECTORY \"\${CMAKE_INSTALL_PREFIX}/bin/plugins/platforms\")
        file(INSTALL \"${CMAKE_PREFIX_PATH}/../../plugins/platforms/qwindows.dll\"
            DESTINATION \"\${CMAKE_INSTALL_PREFIX}/bin/plugins/platforms\")
    ")
endif ()

# 调试信息
message(STATUS "Qt Install Path: ${QT_INSTALL_PATH}")
