QT       += core gui printsupport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets opengl xml

CONFIG += c++14
CONFIG += file_copies

DEFINES += QCUSTOMPLOT_USE_OPENGL
# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

INCLUDEPATH += ../thirdparty/boost/
INCLUDEPATH +=   ../thirdparty/
INCLUDEPATH +=   ../thirdparty/inja
INCLUDEPATH +=   ../thirdparty/qcustomplot

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
    ./serial/src/serial.cc \
    ./serial/src/impl/win.cc \
    ./serial/src/impl/unix.cc \
    ./serial/src/impl/list_ports/list_ports_linux.cc \
    ./serial/src/impl/list_ports/list_ports_win.cc \
    ./traces/abstracttrace.cpp \
    ./helpers/analisyswidget.cpp \
    ./circle/circularanalisiswidget.cpp \
    ./contour/contouranalisiswidget.cpp \
    ./dataproviders/dataprovider.cpp \
    ./device/device.cpp \
    ./dataproviders/filedataprovider.cpp \
    contour/measurements/anglemeasurements.cpp \
    contour/measurements/curvaturemeasurments.cpp \
    contour/measurements/distancemeasurements.cpp \
    contour/measurements/errormeasurement.cpp \
    contour/measurements/layflatmeasurement.cpp \
    contour/measurements/measurement.cpp \
    contour/reperpointcontroller.cpp \
    dataproviders/devicedataprovider.cpp \
    dataproviders/serialdataprovider.cpp \
    device/calibrationdialog.cpp \
    device/devicecontrolwidget.cpp \
    device/settings.cpp \
    device/settingsdialog.cpp \
    device/usersdialog.cpp \
    helpers/reportrenderer.cpp \
    main.cpp \
    mainwindow.cpp \
    ./profile/profileanalisiswidget.cpp \
    ../thirdparty/qcustomplot/qcustomplot.cpp \
    ./math/polinomialregression.cpp \
    math/algos.cpp \
    profile/tracegraphsholder.cpp \
    traces/contourtrace.cpp \
    contour/measurementtreewidget.cpp \
    traces/postprocessors.cpp \
    traces/singlepointprocessors.cpp



HEADERS += \
    ../thirdparty/qcustomplot/qcustomplot.h \
    ./traces/abstracttrace.h \
    ./helpers/analisyswidget.h \
    ./dataproviders/dataprovider.h \
    ./helpers/datatypes.h \
    ./device/device.h \
    ./serial/include/serial/serial.h \
    ./serial/include/serial/impl/win.h \
    ./serial/include/serial/impl/unix.h \
    ./circle/circularanalisiswidget.h \
    ./contour/contouranalisiswidget.h \
    ./dataproviders/filedataprovider.h \
    contour/measurements/anglemeasurements.h \
    contour/measurements/curvaturemeasurments.h \
    contour/measurements/distancemeasurements.h \
    contour/measurements/errormeasurement.h \
    contour/measurements/layflatmeasurement.h \
    contour/measurements/measurement.h \
    contour/reperpointcontroller.h \
    dataproviders/devicedataprovider.h \
    dataproviders/serialdataprovider.h \
    device/calibrationdialog.h \
    device/devicecontrolwidget.h \
    device/settings.h \
    device/settingsdialog.h \
    device/usersdialog.h \
    helpers/constants.h \
    helpers/reportrenderer.h \
    mainwindow.h \
    ./profile/profileanalisiswidget.h \
    ./math/StaticTraceParams.h \
    ./math/polinomialregression.h \
    math/algos.h \
    math/histogram.hpp \
    profile/tracegraphsholder.h \
    traces/contourtrace.h \
    contour/measurementtreewidget.h \
    traces/postprocessors.h \
    traces/singlepointprocessors.h





FORMS += \
    ./circle/circularanalisiswidget.ui \
    ./contour/contouranalisiswidget.ui \
    device/calibrationdialog.ui \
    device/devicecontrolwidget.ui \
    device/settingsdialog.ui \
    device/usersdialog.ui \
    mainwindow.ui \
    ./profile/profileanalisiswidget.ui

COPIES += reportTemplates
reportTemplates.files = $$files(doctemplates/*)
reportTemplates.path = $$OUT_PWD/reports

unix {

    INCLUDEPATH += /usr/lib/gcc/x86_64-linux-gnu/9/include
    INCLUDEPATH += ../thirdparty/opencascade-7.6.0/build/include/opencascade
}

windows
{
    INCLUDEPATH += ../thirdparty/opencascade-7.6.0/build_win/inc/
}

INCLUDEPATH += ./serial/include
INCLUDEPATH += ../thirdparty/inja/third_party/include/

LIBS += -lz -lnlopt
QMAKE_LIBDIR_FLAGS =

unix {
    #DEFINES+=IGES
    #LIBS += -L"/home/dpuzyrkov/projects/mtrlg/thirdparty/opencascade-7.6.0/build/lin64/gcc/lib"  -lTKernel -lTKMath -lTKService -lTKV3d -lTKOpenGl \
    #    -lTKBRep -lTKIGES -lTKSTL -lTKVRML -lTKSTEP -lTKSTEPAttr -lTKSTEP209 \
    #    -lTKSTEPBase -lTKGeomBase -lTKGeomAlgo -lTKG3d -lTKG2d \
    #    -lTKXSBase -lTKShHealing -lTKHLR -lTKTopAlgo -lTKMesh -lTKPrim \
    #    -lTKCDF -lTKBool -lTKBO -lTKFillet -lTKOffset -lTKLCAF
}

#LIBS *= -L/home/dpuzyrkov/projects/mtrlg/thirdparty/opencascade-7.6_win/build/win32/gcc/lib/
#windows {
#    SHARED_LIB_FILES = $$files(/home/dpuzyrkov/projects/mtrlg/thirdparty/opencascade-7.6_win/build/win32/gcc/lib/*dll.a)
#    for(FILE, SHARED_LIB_FILES) {
#        BASENAME = $$basename(FILE)
#        LIBS += -l$$replace(BASENAME,.dll.a,)
#    }
#}

windows {
    #LIBS +=  \
    #    /home/dpuzyrkov/projects/mtrlg/thirdparty/opencascade-7.6_win/build/win32/gcc/lib/libTKernel.dll.a  \
    #    /home/dpuzyrkov/projects/mtrlg/thirdparty/opencascade-7.6_win/build/win32/gcc/lib/libTKMath.dll.a  \
    #    /home/dpuzyrkov/projects/mtrlg/thirdparty/opencascade-7.6_win/build/win32/gcc/lib/libTKBRep.dll.a \
    #    /home/dpuzyrkov/projects/mtrlg/thirdparty/opencascade-7.6_win/build/win32/gcc/lib/libTKIGES.dll.a \
    #    /home/dpuzyrkov/projects/mtrlg/thirdparty/opencascade-7.6_win/build/win32/gcc/lib/libTKXSBase.dll.a\
    #     /home/dpuzyrkov/projects/mtrlg/thirdparty/opencascade-7.6_win/build/win32/gcc/lib/libTKGeomBase.dll.a \
    #     /home/dpuzyrkov/projects/mtrlg/thirdparty/opencascade-7.6_win/build/win32/gcc/lib/libTKGeomAlgo.dll.a \
    #    /home/dpuzyrkov/projects/mtrlg/thirdparty/opencascade-7.6_win/build/win32/gcc/lib/libTKTopAlgo.dll.a \
    #     /home/dpuzyrkov/projects/mtrlg/thirdparty/opencascade-7.6_win/build/win32/gcc/lib/libTKPrim.dll.a \
    #    /home/dpuzyrkov/projects/mtrlg/thirdparty/opencascade-7.6_win/build/win32/gcc/lib/libTKService.dll.a \
    #     /home/dpuzyrkov/projects/mtrlg/thirdparty/opencascade-7.6_win/build/win32/gcc/lib/libTKG3d.dll.a \
    #     /home/dpuzyrkov/projects/mtrlg/thirdparty/opencascade-7.6_win/build/win32/gcc/lib/libTKG2d.dll.a \
    #     /home/dpuzyrkov/projects/mtrlg/thirdparty/opencascade-7.6_win/build/win32/gcc/lib/libTKShHealing.dll.a \
    #     /home/dpuzyrkov/projects/mtrlg/thirdparty/opencascade-7.6_win/build/win32/gcc/lib/libTKBool.dll.a \
    #     /home/dpuzyrkov/projects/mtrlg/thirdparty/opencascade-7.6_win/build/win32/gcc/lib/libTKBO.dll.a \
         
}


RC_ICONS = icons/favicon.ico

windows {
    LIBS += -lhid -lsetupapi
    LIBS += -lglu32 -lopengl32

}

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES     = icons.qrc

