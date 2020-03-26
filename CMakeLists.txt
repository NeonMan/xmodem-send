CMAKE_MINIMUM_REQUIRED(VERSION 3.7)
PROJECT(xmodem-send)

# --- Qt5 related stuff ---
SET(CMAKE_INCLUDE_CURRENT_DIR ON)
SET(CMAKE_AUTOMOC ON)
#SET(CMAKE_AUTOUIC ON) #Not using .ui
FIND_PACKAGE(Qt5 COMPONENTS Core SerialPort Widgets REQUIRED)

# --- Main program ---
SET(CFILES
	main.cpp
	mainwindow.cpp
	crc16-xmodem.c
	transfer.cpp
)

SET(HFILES
	mainwindow.h
	crc16-xmodem.h
	transfer.cpp
)

ADD_EXECUTABLE(${PROJECT_NAME} ${CFILES} ${HFILES})
TARGET_LINK_LIBRARIES(${PROJECT_NAME} Qt5::Widgets Qt5::SerialPort)
