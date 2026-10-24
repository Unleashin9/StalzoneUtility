# ћинимальна€ верси€ CMake, котора€ поддерживает то, что мы используем.
# 3.16 Ч разумный минимум дл€ Qt 6.
cmake_minimum_required(VERSION 3.16)

# »м€ проекта и верси€. VERSION потом используетс€ дл€ "ќ программе".
project(StalzoneUtility VERSION 0.1.0 LANGUAGES CXX)

# —тандарт C++17 Ч современный и поддерживаетс€ всеми компил€торами.
# Qt 6 сам требует C++17, так что это не прихоть, а необходимость.
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ј¬“ќћј“»„≈— »≈ »Ќ—“–”ћ≈Ќ“џ QT:
# AUTOMOC   Ч прогон€ет moc (Meta-Object Compiler) по заголовкам,
#             чтобы работали сигналы/слоты (Q_OBJECT).
# AUTORCC   Ч компилирует .qrc (файлы ресурсов: иконки, стили).
# AUTOUIC   Ч компилирует .ui (файлы из Qt Designer) в C++ код.
# ¬ключаем всЄ три Ч это стандарт дл€ Qt-проектов.
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# »щем Qt 6 и конкретные модули, которые нам нужны:
# Core    Ч базовые типы (QString, QObject)
# Widgets Ч интерфейс (QMainWindow, QPushButton и т.д.)
# Sql     Ч работа с базой данных (SQLite)
# Network Ч сетевые запросы к API аукциона
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Sql Network)

# »сполн€емый файл Ч собираетс€ из перечисленных .cpp файлов.
# «аголовки (.h) писать не об€зательно, но полезно дл€ IDE.
add_executable(StalzoneUtility
    src/main.cpp
    src/ui/MainWindow.cpp
    src/ui/MainWindow.h
)

# ѕрив€зываем к проекту библиотеки Qt.
# PRIVATE Ч значит "только дл€ этой цели", наружу не торчат.
target_link_libraries(StalzoneUtility PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::Sql
    Qt6::Network
)