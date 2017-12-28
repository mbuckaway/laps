#-------------------------------------------------
#
# Project created by QtCreator 2017-06-22T19:05:05
#
#-------------------------------------------------

QT += core gui sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = llrplaps
TEMPLATE = app

LTKLIBDIR = ../LTK/LTKCPP/Library

INCLUDEPATH += $$LTKLIBDIR


win32:!win32-g++ {
  contains(QT_ARCH, "x86_64") {
    message(Building for win 64bit)
#    LIBS += -lws2_32
  } else {
    message(Building for win 32bit)
  }
}



macx {
  message(Building for macx)
  CONFIG += app_bundle
  QMAKE_LFLAGS_RELEASE += -framework cocoa
  QMAKE_LFLAGS += -Wl,-rpath,@executable_path/../Frameworks
  QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.10
}


message($$PWD)

linux-g++ {
  message(Building for linux-g++)
  QMAKE_CXXFLAGS += -Wno-write-strings
#  QMAKE_RPATHDIR = $$LIBDIR
  LIBS += $$LTKLIBDIR/libltkcpp.a
  LIBS += /usr/lib64/libxml2.so.2
}

#message(QMAKE_REL_RPATH_BASE: $$QMAKE_REL_RPATH_BASE)

message(INCLUDE: $$INCLUDEPATH)
message(LIBS: $$LIBS)
#message(PATH: $$PATH)









SOURCES += main.cpp\
        mainwindow.cpp \
    creader.cpp \
    cdbase.cpp \
    crider.cpp

HEADERS  += mainwindow.h \
    creader.h \
    main.h \
    cdbase.h \
    crider.h

FORMS    += mainwindow.ui
