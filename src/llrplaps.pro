#-------------------------------------------------
#
# Project created by QtCreator 2017-06-22T19:05:05
#
#-------------------------------------------------

QT += core widgets gui sql network svg
TARGET = llrplaps
TEMPLATE = app
#RESOURCES = ../qrc/llrplaps.qrc
#RC_FILE += ../qrc/llrplaps.rc
#ICON = ../qrc/icons/llrplaps.icns

LTKLIBDIR = ../LTK/LTKCPP/Library

INCLUDEPATH += $$LTKLIBDIR

message($$PWD)


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

linux-g++ {
  message(Building for linux-g++)
  QWTDIR = ../qwt-6.1.2-linux
  LIBDIR = ../libs/linux
#  LIBS += $$LIBDIR
  LIBS += $$LTKLIBDIR/libltkcpp.a
  LIBS += /usr/lib64/libxml2.so.2
  LIBS += -ldl
#  INCLUDEPATH += /usr/include
#  LIBS += -L/usr/lib/mysql -lrt
#  LIBS += -L/home/icunning/Qt5.7.0/5.7/gcc_64/plugins/sqldrivers
  LIBS += $$QWTDIR/lib/libqwt.a
  QMAKE_CXXFLAGS += -Wall -Wno-write-strings
  QMAKE_RPATHDIR += $$LIBDIR #$$LIBDIR/platforms
}

message(QMAKE_RPATHDIR: $$QMAKE_RPATHDIR);
message($$QMAKE_REL_RPATH_BASE);

INCLUDEPATH += $$QWTDIR/src

  message(LFLAGS: $$QMAKE_LFLAGS)
  message(INCLUDE: $$INCLUDEPATH)
  message(LIBS: $$LIBS)
  #message(PATH: $$PATH)



SOURCES += main.cpp \
    mainwindow.cpp \
    creader.cpp \
    cdbase.cpp \
    crider.cpp \
    csmtp.cpp \
    cplot.cpp
#    cmessages.cpp \
#    cmembers.cpp \
#    csessions.cpp \
#    csettings.cpp
#    cplotform.cpp

HEADERS  += mainwindow.h \
    creader.h \
    main.h \
    cdbase.h \
    crider.h \
    csmtp.h \
    cplot.h
#    cmessages.h \
#    cmembers.h \
#    csessions.h \
#    csettings.h
#    cplotform.h

FORMS    += mainwindow.ui
#    cmessages.ui \
#    cmembers.ui \
#    csessions.ui \
#    csettings.ui
#    cplotform.ui
