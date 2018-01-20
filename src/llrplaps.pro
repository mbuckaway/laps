#-------------------------------------------------
#
# Project created by QtCreator 2017-06-22T19:05:05
#
#-------------------------------------------------

QT += core widgets gui sql network svg

#greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

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
  QMAKE_CXXFLAGS += -Wno-write-strings
  QMAKE_RPATHDIR = $$LIBDIR
  QWTDIR == ../qwt-6.1.2-linux
  LIBDIR = ../libs/linux
  LIBS += $$LTKLIBDIR/libltkcpp.a
  LIBS += /usr/lib64/libxml2.so.2
#  INCLUDEPATH += /usr/include
#  LIBS += -L/usr/lib/mysql -lrt
#  LIBS += -L/home/icunning/Qt5.7.0/5.7/gcc_64/plugins/sqldrivers
  LIBS += $$LIBDIR/libqwt.a
}


INCLUDEPATH += $$QWTDIR/src

  #message(QMAKE_REL_RPATH_BASE: $$QMAKE_REL_RPATH_BASE)

  message(INCLUDE: $$INCLUDEPATH)
  message(LIBS: $$LIBS)
  #message(PATH: $$PATH)









SOURCES += main.cpp\
        mainwindow.cpp \
    creader.cpp \
    cdbase.cpp \
    crider.cpp \
    csmtp.cpp \
    cplot.cpp
#    cplotform.cpp

HEADERS  += mainwindow.h \
    creader.h \
    main.h \
    cdbase.h \
    crider.h \
    csmtp.h \
    cplot.h
#    cplotform.h

FORMS    += mainwindow.ui
#    cplotform.ui
