cmake_minimum_required(VERSION 3.30)
set(CMAKE_CXX_STANDARD 17)

project(qtest)

# 自动查找 Qt 安装路径
if (NOT CMAKE_PREFIX_PATH)
    set(CMAKE_PREFIX_PATH "D:/devtools/Qt/5.12.9/mingw73_64/lib/cmake")
endif ()

# 添加测试源文件
set(TEST_SOURCES test_example.cpp FileMonitorTest.cpp SyncTaskTest.cpp)

# 创建测试可执行文件
add_executable(${PROJECT_NAME} ${TEST_SOURCES})

target_include_directories(${PROJECT_NAME}
        PRIVATE
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/libs/QXlsx/QXlsx/header
)

# 链接Qt Test库
target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::Core Qt5::Gui Qt5::Widgets Qt5::Svg Qt5::Network Qt5::Test QXlsx)

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

# 添加测试到CTest
include(CTest)

# 自动运行测试
add_test(NAME ${PROJECT_NAME} COMMAND ${PROJECT_NAME})
