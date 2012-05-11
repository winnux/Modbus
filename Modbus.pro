TEMPLATE = app
CONFIG += console
CONFIG -= qt

SOURCES += main.cpp \
    libmodbus/modbus-tcp.c \
    libmodbus/modbus-rtu.c \
    libmodbus/modbus-data.c \
    libmodbus/modbus.c \
    config.cpp

HEADERS += \
    libmodbus/modbus-version.h \
    libmodbus/modbus-tcp-private.h \
    libmodbus/modbus-tcp.h \
    libmodbus/modbus-rtu-private.h \
    libmodbus/modbus-rtu.h \
    libmodbus/modbus-private.h \
    libmodbus/modbus.h \
    config.h \
    NetServer.h \
    ezlogger/ezlogger_verbosity_level_policy.hpp \
    ezlogger/ezlogger_output_policy.hpp \
    ezlogger/ezlogger_misc.hpp \
    ezlogger/ezlogger_macros.hpp \
    ezlogger/ezlogger_headers.hpp \
    ezlogger/ezlogger_format_policy.hpp \
    ezlogger/ezlogger.hpp
win32{
    INCLUDEPATH += E:\boost_1_49_0

    CONFIG(debug,debug|release){
        LIBS +=E:\boost_1_49_0\stage\lib\libboost_thread-vc100-mt-gd-1_49.lib
        LIBS +=E:\boost_1_49_0\stage\lib\libboost_system-vc100-mt-gd-1_49.lib
        LIBS += E:\boost_1_49_0\stage\lib\libboost_regex-vc100-mt-gd-1_49.lib
    }else{
       LIBS +=E:\boost_1_49_0\stage\lib\libboost_thread-vc100-mt-s-1_49.lib
       LIBS +=E:\boost_1_49_0\stage\lib\libboost_system-vc100-mt-s-1_49.lib
        LIBS += E:\boost_1_49_0\stage\lib\libboost_regex-vc100-mt-s-1_49.lib
    }
}
unix{
    INCLUDEPATH += /home/lihaibo/dev/boost_1_49_0
    LIBS += /home/lihaibo/dev/boost_1_49_0/stage/lib/libboost_thread.a
    LIBS +=/home/lihaibo/dev/boost_1_49_0/stage/lib/libboost_system.a
    LIBS += -lpthread -lrt
}
INCLUDEPATH +=libmodbus
win32:LIBS +=ws2_32.lib
win32:LIBS +=wsock32.lib
win32:LIBS +=setupapi.lib

OTHER_FILES += \
    configuration.xml
