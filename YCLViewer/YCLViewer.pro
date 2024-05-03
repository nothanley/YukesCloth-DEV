QT       += core gui

DEFINES += MAJOR_VER=0
DEFINES += MINOR_VER=9
DEFINES += REVIS_VER=23

DEFINES += DEBUG_EDITOR
DEFINES += DEBUG_CONSOLE

TARGET = YclEditor
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
CONFIG += c++17
DEFINES += WIN32_LEAN_AND_MEAN
DEFINES -= UNICODE
DEFINES += _MBCS

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    $$PWD/../YukesCloth/include/BinaryIO.cpp \
    ../YukesCloth/include/common.cpp \
    ../YukesCloth/include/hash.cpp \
    ../YukesCloth/src/Cloth/Mesh/simmesh_col.cpp \
    ../YukesCloth/src/Cloth/Mesh/simmesh_constraint.cpp \
    ../YukesCloth/src/Cloth/Mesh/simmesh_def.cpp \
    ../YukesCloth/src/Cloth/Mesh/simmesh_obj.cpp \
    ../YukesCloth/src/Cloth/Mesh/simmesh_pattern.cpp \
    ../YukesCloth/src/Cloth/Mesh/simmesh_rcn.cpp \
    ../YukesCloth/src/Cloth/Mesh/simmesh_skin.cpp \
    ../YukesCloth/src/Cloth/cgamecloth.cpp \
    ../YukesCloth/src/Cloth/clothstructs.cpp \
    ../YukesCloth/src/Cloth/csimmesh.cpp \
    ../YukesCloth/src/Container/clothfile.cpp \
    ../YukesCloth/src/Encoder/Import/simmeshjson.cpp \
    ../YukesCloth/src/Encoder/clothencoder.cpp \
    ../YukesCloth/src/Encoder/clothsave.cpp \
    ../YukesCloth/src/Encoder/clothserial.cpp \
    ../YukesCloth/src/Encoder/encode_col.cpp \
    ../YukesCloth/src/Encoder/encode_constraint.cpp \
    ../YukesCloth/src/Encoder/encode_def.cpp \
    ../YukesCloth/src/Encoder/encode_obj.cpp \
    ../YukesCloth/src/Encoder/encode_pattern.cpp \
    ../YukesCloth/src/Encoder/encode_rcn.cpp \
    ../YukesCloth/src/Encoder/encode_skin.cpp \
    clothjson.cpp \
    defwidgetitem.cpp \
    main.cpp \
    mainwindow.cpp \
    nodeselectwindow.cpp

HEADERS += \
    $$PWD/include/json.hpp \
    $$PWD/../YukesCloth/include/BinaryIO.h \
    $$PWD/../YukesCloth/YukesCloth \
    ../YukesCloth/YukesSimMesh \
    ../YukesCloth/include/common.h \
    ../YukesCloth/include/hash.h \
    ../YukesCloth/src/Encoder/clothserial.h \
    ../YukesCloth/src/Encoder/encode_col.h \
    ../YukesCloth/src/Encoder/encode_constraint.h \
    ../YukesCloth/src/Encoder/encode_def.h \
    ../YukesCloth/src/Encoder/encode_obj.h \
    ../YukesCloth/src/Encoder/encode_pattern.h \
    ../YukesCloth/src/Encoder/encode_rcn.h \
    ../YukesCloth/src/Encoder/encode_skin.h \
    clothjson.h \
    defwidgetitem.h \
    mainwindow.h \
    nodeselectwindow.h

FORMS += \
    mainwindow.ui \
    nodeselectwindow.ui

INCLUDEPATH += $$PWD/../YukesCloth
INCLUDEPATH += $$PWD/../YukesCloth/src
INCLUDEPATH += $$PWD/../YukesCloth/src/Cloth
INCLUDEPATH += $$PWD/../YukesCloth/src/Encoder
INCLUDEPATH += $$PWD/../YukesCloth/src/Cloth/Mesh
INCLUDEPATH += $$PWD/../YukesCloth/include
INCLUDEPATH += $$PWD/include

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target


RESOURCES += \
    icons.qrc

win32:RC_ICONS += Icons/windowIcon.ico
