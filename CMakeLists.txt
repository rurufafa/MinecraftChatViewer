cmake_minimum_required(VERSION 3.21.1)
project(MCV VERSION 1.0 LANGUAGES CXX)

# Qtパス
set(CMAKE_PREFIX_PATH "D:/Qt/6.9.1/mingw_64/lib/cmake")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Qt6 Widgets Compat
find_package(Qt6 REQUIRED COMPONENTS Widgets Core5Compat)

# Qtの自動処理設定
qt_standard_project_setup()

# 🔍 .ui の検索パスを追加
set(CMAKE_AUTOUIC_SEARCH_PATHS
    ${CMAKE_SOURCE_DIR}/ui
)

# 実行ファイルに必要なファイル
qt_add_executable(MCV WIN32
    main.cpp
    src/core/config_manager.cpp 
    src/core/config_manager.h 
    src/core/frameless_helper.cpp 
    src/core/frameless_helper.h 
    src/core/log_formatter.cpp 
    src/core/log_formatter.h 
    src/core/log_watcher.cpp 
    src/core/log_watcher.h 
    src/core/record_manager.cpp 
    src/core/record_manager.h
    src/core/rule_models.cpp
    src/core/rule_models.h
    src/core/widget_binding.cpp 
    src/core/widget_binding.h
    src/core/window_manager.cpp 
    src/core/window_manager.h
    src/window/chatwindow.cpp 
    src/window/chatwindow.h
    src/window/groupeditdialog.cpp 
    src/window/groupeditdialog.h 
    src/window/keywordeditdialog.cpp
    src/window/keywordeditdialog.h 
    src/window/keywordwindow.cpp
    src/window/keywordwindow.h
    src/window/ruleeditdialog.cpp 
    src/window/ruleeditdialog.h 
    src/window/rulewindow.cpp 
    src/window/rulewindow.h
    src/window/settingwindow.cpp
    src/window/settingwindow.h
    src/window/timeoutwindow.cpp 
    src/window/timeoutwindow.h 
    src/window/titlewindow.cpp
    src/window/titlewindow.h
    ui/groupeditdialog.ui 
    ui/keywordeditdialog.ui 
    ui/keywordwindow.ui 
    ui/ruleeditdialog.ui 
    ui/rulewindow.ui
    ui/settingwindow.ui 
    ui/timeoutwindow.ui 
    ui/titlewindow.ui
)

target_include_directories(MCV PRIVATE
    ${CMAKE_SOURCE_DIR}/src/window
    ${CMAKE_SOURCE_DIR}/src/core
)

# Qtライブラリリンク
target_link_libraries(MCV PRIVATE Qt6::Widgets Qt6::Core5Compat)