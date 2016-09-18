#-------------------------------------------------
#
# Project created by QtCreator 2016-05-04T16:58:05
#
#-------------------------------------------------
TARGET = PBDfluid
# where to put the .o files
OBJECTS_DIR=obj

QT       +=gui opengl core xml

CONFIG   += console
CONFIG   -= app_bundle

# as I want to support 4.8 and 5 this will set a flag for some of the mac stuff
# mainly in the types.h file for the setMacVisual which is native in Qt5
isEqual(QT_MAJOR_VERSION, 5) {
        cache()
        DEFINES +=QT5BUILD
}
# where to put moc auto generated files
MOC_DIR=moc

# add the Qt form
FORMS += ui/MainWindow.ui

# openmp
QMAKE_CXXFLAGS += -fopenmp
QMAKE_LFLAGS += -fopenmp

# alembic
linux*:INCLUDEPATH+= /usr/local/alembic-1.5.8/include/
linux*:LIBS+=-L/usr/local/alembic-1.5.8/lib/static

linux*:LIBS+= -lAbcWFObjConvert         -lAlembicAbcCoreOgawa \
-lAlembicAbc              -lAlembicAbcGeom \
-lAlembicAbcCollection   -lAlembicAbcMaterial \
-lAlembicAbcCoreAbstract  -lAlembicAbcOpenGL \
-lAlembicAbcCoreFactory   -lAlembicOgawa \
-lAlembicAbcCoreHDF5      -lAlembicUtil

linux*:LIBS+=-L/usr/local/lib -lHalf

macx:LIBS+=-lAlembic


SOURCES += $$PWD/src/main.cpp\
           $$PWD/src/Simulation.cpp \
           $$PWD/src/MainWindow.cpp \
           $$PWD/src/PBD.cpp \
           $$PWD/src/Timer.cpp \
           $$PWD/src/NeighborsSearch.cpp \
           $$PWD/src/Draw.cpp \
           $$PWD/src/Boundary.cpp \
           $$PWD/src/Kernel.cpp \
           $$PWD/src/Particle.cpp \
           $$PWD/src/Definition.cpp \

HEADERS+= $$PWD/include/Simulation.h \
          $$PWD/include/MainWindow.h \
          $$PWD/include/PBD.h \
          $$PWD/include/Timer.h \
          $$PWD/include/NeighborsSearch.h \
          $$PWD/include/Draw.h \
          $$PWD/include/Boundary.h \
          $$PWD/include/Kernel.h \
          $$PWD/include/Particle.h \
          $$PWD/include/Definition.h \

# and add the include dir into the search path for Qt and make
INCLUDEPATH +=./include

INCLUDEPATH+=/usr/local/include/OpenEXR

# where our exe is going to live (root of project)
DESTDIR=./
# add the glsl shader files
OTHER_FILES+= shaders/*.glsl \
              glm/*.hpp \



# note each command you add needs a ; as it will be run as a single line
# first check if we are shadow building or not easiest way is to check out against current
!equals(PWD, $${OUT_PWD}){
        copydata.commands = echo "creating destination dirs" ;
        # now make a dir
        copydata.commands += mkdir -p $$OUT_PWD/shaders ;
        copydata.commands += mkdir -p $$OUT_PWD/textures ;
        copydata.commands += mkdir -p $$OUT_PWD/data ;
        copydata.commands += echo "copying files" ;
        # then copy the files
        copydata.commands += $(COPY_DIR) $$PWD/shaders/* $$OUT_PWD/shaders/ ;
        copydata.commands += $(COPY_DIR) $$PWD/textures/* $$OUT_PWD/textures/ ;
        copydata.commands += $(COPY_DIR) $$PWD/data/* $$OUT_PWD/data/ ;
        # now make sure the first target is built before copy
        first.depends = $(first) copydata
        export(first.depends)
        export(copydata.commands)
        # now add it as an extra target
        QMAKE_EXTRA_TARGETS += first copydata
}
NGLPATH=$$(NGLDIR)
isEmpty(NGLPATH){ # note brace must be here
        message("including $HOME/NGL")
        include($(HOME)/NGL/UseNGL.pri)
}
else{ # note brace must be here
        message("Using custom NGL location")
        include($(NGLDIR)/UseNGL.pri)
}
