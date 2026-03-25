QT += core widgets concurrent xml

CONFIG += c++17

CONFIG(debug, debug|release) {
 TARGET = ../../bin/QtXMLEditorD
 DESTDIR = $$PWD/bin
# g++ -O0 -g -o app.exe <sources>
} else {
 TARGET = ../../bin/QtXMLEditor
 DESTDIR = $$PWD/bin
# g++ -O2 -s -o app.exe <sources>
}

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    $$PWD/../src/main.cpp \
    $$PWD/../src/xmleditor.cpp \
    $$PWD/../src/xmlnodedialog.cpp \
    $$PWD/../src/xmltree.cpp \
    $$PWD/../src/xmlvalidator.cpp \
    $$PWD/../src/xmldevdesc.cpp
#    $$PWD/../src/xmlattributespopup.cpp

HEADERS += \
    $$PWD/../src/xmleditor.h \
    $$PWD/../src/xmlnodedialog.h \
    $$PWD/../src/xmltree.h \
    $$PWD/../src/xmlvalidator.h \
    $$PWD/../src/xmldevdesc.h
#    $$PWD/../src/xmlattributespopup.h

RESOURCES += \
    xmleditor.qrc

