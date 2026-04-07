TEMPLATE = app
CONFIG += console c++17
CONFIG -= app_bundle
# CONFIG -= qt

SOURCES += \
        ChessEngine.cpp \
        FEN.cpp \
        MicroMax.cpp \
        ChessPGN.cpp \
        UCIEngine.cpp \
        main.cpp

HEADERS += \
    ChessEngine.h \
    ChessEngineInterface.h \
    ChessPGN.h \
    Converter.h \
    UCIEngine.h
