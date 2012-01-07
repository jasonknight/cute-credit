#############################################################################
# Makefile for building: cute-credit
# Generated by qmake (2.01a) (Qt 4.7.0) on: Thu Dec 22 15:36:41 2011
# Project:  ArtemaHybrid.pro
# Template: app
# Command: /usr/bin/qmake -o Makefile ArtemaHybrid.pro
#############################################################################

####### Compiler, tools and options

CC            = gcc
CXX           = g++
DEFINES       = -DQT_NO_DEBUG -DQT_SQL_LIB -DQT_CORE_LIB -DQT_SHARED
CFLAGS        = -pipe -O2 -Wall -W -D_REENTRANT $(DEFINES)
CXXFLAGS      = -pipe -O2 -Wall -W -D_REENTRANT $(DEFINES)
INCPATH       = -I/usr/share/qt4/mkspecs/linux-g++ -I. -I/usr/include/qt4/QtCore -I/usr/include/qt4/QtSql -I/usr/include/qt4 -I.
LINK          = g++
LFLAGS        = -Wl,-O1
LIBS          = $(SUBLIBS)  -L/usr/lib -lQtSql -lQtCore -lpthread 
AR            = ar cqs
RANLIB        = 
QMAKE         = /usr/bin/qmake
TAR           = tar -cf
COMPRESS      = gzip -9f
COPY          = cp -f
SED           = sed
COPY_FILE     = $(COPY)
COPY_DIR      = $(COPY) -r
STRIP         = strip
INSTALL_FILE  = install -m 644 -p
INSTALL_DIR   = $(COPY_DIR)
INSTALL_PROGRAM = install -m 755 -p
DEL_FILE      = rm -f
SYMLINK       = ln -f -s
DEL_DIR       = rmdir
MOVE          = mv -f
CHK_DIR_EXISTS= test -d
MKDIR         = mkdir -p

####### Output directory

OBJECTS_DIR   = ./

####### Files

SOURCES       = main.cpp \
		artematimer.cpp \
		reader.cpp \
		fifocontroller.cpp \
		mock.cpp \
		database.cpp \
		artemahybrid.cpp moc_artematimer.cpp \
		moc_reader.cpp \
		moc_fifocontroller.cpp \
		moc_mock.cpp \
		moc_database.cpp \
		moc_artemahybrid.cpp
OBJECTS       = main.o \
		artematimer.o \
		reader.o \
		fifocontroller.o \
		mock.o \
		database.o \
		artemahybrid.o \
		moc_artematimer.o \
		moc_reader.o \
		moc_fifocontroller.o \
		moc_mock.o \
		moc_database.o \
		moc_artemahybrid.o
DIST          = /usr/share/qt4/mkspecs/common/g++.conf \
		/usr/share/qt4/mkspecs/common/unix.conf \
		/usr/share/qt4/mkspecs/common/linux.conf \
		/usr/share/qt4/mkspecs/qconfig.pri \
		/usr/share/qt4/mkspecs/modules/qt_webkit_version.pri \
		/usr/share/qt4/mkspecs/features/qt_functions.prf \
		/usr/share/qt4/mkspecs/features/qt_config.prf \
		/usr/share/qt4/mkspecs/features/exclusive_builds.prf \
		/usr/share/qt4/mkspecs/features/default_pre.prf \
		/usr/share/qt4/mkspecs/features/release.prf \
		/usr/share/qt4/mkspecs/features/default_post.prf \
		/usr/share/qt4/mkspecs/features/warn_on.prf \
		/usr/share/qt4/mkspecs/features/qt.prf \
		/usr/share/qt4/mkspecs/features/unix/thread.prf \
		/usr/share/qt4/mkspecs/features/moc.prf \
		/usr/share/qt4/mkspecs/features/resources.prf \
		/usr/share/qt4/mkspecs/features/uic.prf \
		/usr/share/qt4/mkspecs/features/yacc.prf \
		/usr/share/qt4/mkspecs/features/lex.prf \
		/usr/share/qt4/mkspecs/features/include_source_dir.prf \
		ArtemaHybrid.pro
QMAKE_TARGET  = cute-credit
DESTDIR       = 
TARGET        = cute-credit

first: all
####### Implicit rules

.SUFFIXES: .o .c .cpp .cc .cxx .C

.cpp.o:
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o "$@" "$<"

.cc.o:
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o "$@" "$<"

.cxx.o:
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o "$@" "$<"

.C.o:
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o "$@" "$<"

.c.o:
	$(CC) -c $(CFLAGS) $(INCPATH) -o "$@" "$<"

####### Build rules

all: Makefile $(TARGET)

$(TARGET):  $(OBJECTS)  
	$(LINK) $(LFLAGS) -o $(TARGET) $(OBJECTS) $(OBJCOMP) $(LIBS)

Makefile: ArtemaHybrid.pro  /usr/share/qt4/mkspecs/linux-g++/qmake.conf /usr/share/qt4/mkspecs/common/g++.conf \
		/usr/share/qt4/mkspecs/common/unix.conf \
		/usr/share/qt4/mkspecs/common/linux.conf \
		/usr/share/qt4/mkspecs/qconfig.pri \
		/usr/share/qt4/mkspecs/modules/qt_webkit_version.pri \
		/usr/share/qt4/mkspecs/features/qt_functions.prf \
		/usr/share/qt4/mkspecs/features/qt_config.prf \
		/usr/share/qt4/mkspecs/features/exclusive_builds.prf \
		/usr/share/qt4/mkspecs/features/default_pre.prf \
		/usr/share/qt4/mkspecs/features/release.prf \
		/usr/share/qt4/mkspecs/features/default_post.prf \
		/usr/share/qt4/mkspecs/features/warn_on.prf \
		/usr/share/qt4/mkspecs/features/qt.prf \
		/usr/share/qt4/mkspecs/features/unix/thread.prf \
		/usr/share/qt4/mkspecs/features/moc.prf \
		/usr/share/qt4/mkspecs/features/resources.prf \
		/usr/share/qt4/mkspecs/features/uic.prf \
		/usr/share/qt4/mkspecs/features/yacc.prf \
		/usr/share/qt4/mkspecs/features/lex.prf \
		/usr/share/qt4/mkspecs/features/include_source_dir.prf \
		/usr/lib/libQtSql.prl \
		/usr/lib/libQtCore.prl
	$(QMAKE) -o Makefile ArtemaHybrid.pro
/usr/share/qt4/mkspecs/common/g++.conf:
/usr/share/qt4/mkspecs/common/unix.conf:
/usr/share/qt4/mkspecs/common/linux.conf:
/usr/share/qt4/mkspecs/qconfig.pri:
/usr/share/qt4/mkspecs/modules/qt_webkit_version.pri:
/usr/share/qt4/mkspecs/features/qt_functions.prf:
/usr/share/qt4/mkspecs/features/qt_config.prf:
/usr/share/qt4/mkspecs/features/exclusive_builds.prf:
/usr/share/qt4/mkspecs/features/default_pre.prf:
/usr/share/qt4/mkspecs/features/release.prf:
/usr/share/qt4/mkspecs/features/default_post.prf:
/usr/share/qt4/mkspecs/features/warn_on.prf:
/usr/share/qt4/mkspecs/features/qt.prf:
/usr/share/qt4/mkspecs/features/unix/thread.prf:
/usr/share/qt4/mkspecs/features/moc.prf:
/usr/share/qt4/mkspecs/features/resources.prf:
/usr/share/qt4/mkspecs/features/uic.prf:
/usr/share/qt4/mkspecs/features/yacc.prf:
/usr/share/qt4/mkspecs/features/lex.prf:
/usr/share/qt4/mkspecs/features/include_source_dir.prf:
/usr/lib/libQtSql.prl:
/usr/lib/libQtCore.prl:
qmake:  FORCE
	@$(QMAKE) -o Makefile ArtemaHybrid.pro

dist: 
	@$(CHK_DIR_EXISTS) .tmp/cute-credit1.0.0 || $(MKDIR) .tmp/cute-credit1.0.0 
	$(COPY_FILE) --parents $(SOURCES) $(DIST) .tmp/cute-credit1.0.0/ && $(COPY_FILE) --parents helpers.h artematimer.h reader.h cute_credit.h fifocontroller.h mock.h database.h artemahybrid.h paylife_structs.h .tmp/cute-credit1.0.0/ && $(COPY_FILE) --parents main.cpp artematimer.cpp reader.cpp fifocontroller.cpp mock.cpp database.cpp artemahybrid.cpp .tmp/cute-credit1.0.0/ && (cd `dirname .tmp/cute-credit1.0.0` && $(TAR) cute-credit1.0.0.tar cute-credit1.0.0 && $(COMPRESS) cute-credit1.0.0.tar) && $(MOVE) `dirname .tmp/cute-credit1.0.0`/cute-credit1.0.0.tar.gz . && $(DEL_FILE) -r .tmp/cute-credit1.0.0


clean:compiler_clean 
	-$(DEL_FILE) $(OBJECTS)
	-$(DEL_FILE) *~ core *.core


####### Sub-libraries

distclean: clean
	-$(DEL_FILE) $(TARGET) 
	-$(DEL_FILE) Makefile


check: first

mocclean: compiler_moc_header_clean compiler_moc_source_clean

mocables: compiler_moc_header_make_all compiler_moc_source_make_all

compiler_moc_header_make_all: moc_artematimer.cpp moc_reader.cpp moc_fifocontroller.cpp moc_mock.cpp moc_database.cpp moc_artemahybrid.cpp
compiler_moc_header_clean:
	-$(DEL_FILE) moc_artematimer.cpp moc_reader.cpp moc_fifocontroller.cpp moc_mock.cpp moc_database.cpp moc_artemahybrid.cpp
moc_artematimer.cpp: artematimer.h
	/usr/bin/moc-qt4 $(DEFINES) $(INCPATH) artematimer.h -o moc_artematimer.cpp

moc_reader.cpp: cute_credit.h \
		helpers.h \
		artematimer.h \
		reader.h \
		fifocontroller.h \
		reader.h
	/usr/bin/moc-qt4 $(DEFINES) $(INCPATH) reader.h -o moc_reader.cpp

moc_fifocontroller.cpp: cute_credit.h \
		helpers.h \
		artematimer.h \
		reader.h \
		fifocontroller.h \
		fifocontroller.h
	/usr/bin/moc-qt4 $(DEFINES) $(INCPATH) fifocontroller.h -o moc_fifocontroller.cpp

moc_mock.cpp: mock.h
	/usr/bin/moc-qt4 $(DEFINES) $(INCPATH) mock.h -o moc_mock.cpp

moc_database.cpp: database.h
	/usr/bin/moc-qt4 $(DEFINES) $(INCPATH) database.h -o moc_database.cpp

moc_artemahybrid.cpp: cute_credit.h \
		helpers.h \
		artematimer.h \
		reader.h \
		fifocontroller.h \
		artemahybrid.h
	/usr/bin/moc-qt4 $(DEFINES) $(INCPATH) artemahybrid.h -o moc_artemahybrid.cpp

compiler_rcc_make_all:
compiler_rcc_clean:
compiler_image_collection_make_all: qmake_image_collection.cpp
compiler_image_collection_clean:
	-$(DEL_FILE) qmake_image_collection.cpp
compiler_moc_source_make_all:
compiler_moc_source_clean:
compiler_uic_make_all:
compiler_uic_clean:
compiler_yacc_decl_make_all:
compiler_yacc_decl_clean:
compiler_yacc_impl_make_all:
compiler_yacc_impl_clean:
compiler_lex_make_all:
compiler_lex_clean:
compiler_clean: compiler_moc_header_clean 

####### Compile

main.o: main.cpp cute_credit.h \
		helpers.h \
		artematimer.h \
		reader.h \
		fifocontroller.h \
		mock.h \
		artemahybrid.h \
		database.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o main.o main.cpp

artematimer.o: artematimer.cpp artematimer.h \
		cute_credit.h \
		helpers.h \
		reader.h \
		fifocontroller.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o artematimer.o artematimer.cpp

reader.o: reader.cpp reader.h \
		cute_credit.h \
		helpers.h \
		artematimer.h \
		fifocontroller.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o reader.o reader.cpp

fifocontroller.o: fifocontroller.cpp fifocontroller.h \
		cute_credit.h \
		helpers.h \
		artematimer.h \
		reader.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o fifocontroller.o fifocontroller.cpp

mock.o: mock.cpp mock.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o mock.o mock.cpp

database.o: database.cpp database.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o database.o database.cpp

artemahybrid.o: artemahybrid.cpp artemahybrid.h \
		cute_credit.h \
		helpers.h \
		artematimer.h \
		reader.h \
		fifocontroller.h \
		paylife_structs.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o artemahybrid.o artemahybrid.cpp

moc_artematimer.o: moc_artematimer.cpp 
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o moc_artematimer.o moc_artematimer.cpp

moc_reader.o: moc_reader.cpp 
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o moc_reader.o moc_reader.cpp

moc_fifocontroller.o: moc_fifocontroller.cpp 
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o moc_fifocontroller.o moc_fifocontroller.cpp

moc_mock.o: moc_mock.cpp 
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o moc_mock.o moc_mock.cpp

moc_database.o: moc_database.cpp 
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o moc_database.o moc_database.cpp

moc_artemahybrid.o: moc_artemahybrid.cpp 
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o moc_artemahybrid.o moc_artemahybrid.cpp

####### Install

install:   FORCE

uninstall:   FORCE

FORCE:

