export PATH=$(shell echo "$$PWD:$$PATH")
AM_CFLAGS=-DLINUX -DPOSIX_TTY -I$(JVM_INCLUDE) -I$(JVM_INCLUDE)/linux
AM_JAVACFLAGS=$(JAVAC_VERSION_FLAGS) -encoding UTF-8 -Xlint:deprecation -Xlint:unchecked

altoslibdir=$(libdir)/altos

altoslib_LTLIBRARIES=libaltos.la
libaltos_la_LDFLAGS=-version-info 1:0:1 -Wl,-znoexecstack

libaltos_la_SOURCES=\
	libaltos_common.c \
	libaltos_posix.c \
	libaltos_linux.c \
	libaltos_wrap.c \
	libaltos.h \
	libaltos_posix.h \
	libaltos_private.h

WINDOWS_SRC=\
	libaltos_common.c\
	libaltos_windows.c\
	libaltos_wrap.c

WINDOWS_H=\
	libaltos.h

noinst_PROGRAMS=cjnitest btletest

cjnitest_SOURCES=cjnitest.c
cjnitest_LDADD=libaltos.la

btletest_SOURCES=btletest.c

btletest_LDADD=-lbluetooth

if MULTI_ARCH
altoslib_LTLIBRARIES+=libaltos_i686.la libaltos_amd64.la libaltos_aarch64.la libaltos_armel.la libaltos_armhf.la

I686=i686-linux-gnu
libaltos_i686_la_LDFLAGS=-Wl,arch=$(I686) $(libaltos_la_LDFLAGS)
libaltos_i686_la_CFLAGS=-Warch=i686-linux-gnu $(AM_CFLAGS)
libaltos_i686_la_SOURCES=$(libaltos_la_SOURCES)

AMD64=x86_64-linux-gnu
libaltos_amd64_la_LDFLAGS=-Wl,arch=$(AMD64) $(libaltos_la_LDFLAGS)
libaltos_amd64_la_CFLAGS=-Warch=x86_64-linux-gnu $(AM_CFLAGS)
libaltos_amd64_la_SOURCES=$(libaltos_la_SOURCES)

AARCH64=aarch64-linux-gnu
libaltos_aarch64_la_LDFLAGS=-Wl,arch=$(AARCH64) $(libaltos_la_LDFLAGS)
libaltos_aarch64_la_CFLAGS=-Warch=$(AARCH64) $(AM_CFLAGS)
libaltos_aarch64_la_SOURCES=$(libaltos_la_SOURCES)

ARMEL=arm-linux-gnueabi
libaltos_armel_la_LDFLAGS=-Wl,arch=$(ARMEL) $(libaltos_la_LDFLAGS)
libaltos_armel_la_CFLAGS=-Warch=$(ARMEL) $(AM_CFLAGS)
libaltos_armel_la_SOURCES=$(libaltos_la_SOURCES)

ARMHF=arm-linux-gnueabihf
libaltos_armhf_la_LDFLAGS=-Wl,arch=$(ARMHF) $(libaltos_la_LDFLAGS)
libaltos_armhf_la_CFLAGS=-Warch=$(ARMHF) $(AM_CFLAGS)
libaltos_armhf_la_SOURCES=$(libaltos_la_SOURCES)

noinst_PROGRAMS+=cjnitest_i686 cjnitest_amd64 cjnitest_aarch64 cjnitest_armel cjnitest_armhf

cjnitest_i686_CFLAGS=$(libaltos_i686_la_CFLAGS)
cjnitest_i686_LDFLAGS=-Wl,arch=$(I686)
cjnitest_i686_SOURCES=$(cjnitest_SOURCES)
cjnitest_i686_LDADD=libaltos_i686.la

cjnitest_amd64_CFLAGS=$(libaltos_amd64_la_CFLAGS)
cjnitest_amd64_LDFLAGS=-Wl,arch=$(AMD64)
cjnitest_amd64_SOURCES=$(cjnitest_SOURCES)
cjnitest_amd64_LDADD=libaltos_amd64.la

cjnitest_aarch64_CFLAGS=$(libaltos_aarch64_la_CFLAGS)
cjnitest_aarch64_LDFLAGS=-Wl,arch=$(AARCH64)
cjnitest_aarch64_SOURCES=$(cjnitest_SOURCES)
cjnitest_aarch64_LDADD=libaltos_aarch64.la

cjnitest_armel_CFLAGS=$(libaltos_armel_la_CFLAGS)
cjnitest_armel_LDFLAGS=-Wl,arch=$(ARMEL)
cjnitest_armel_SOURCES=$(cjnitest_SOURCES)
cjnitest_armel_LDADD=libaltos_armel.la

cjnitest_armhf_CFLAGS=$(libaltos_armhf_la_CFLAGS)
cjnitest_armhf_LDFLAGS=-Wl,arch=$(ARMHF)
cjnitest_armhf_SOURCES=$(cjnitest_SOURCES)
cjnitest_armhf_LDADD=libaltos_armhf.la

endif

LIBS=-ldl

HFILES=libaltos.h

SWIG_FILE=libaltos.swig

CLASSDIR=libaltosJNI

$(SWIG_FILE): libaltos.i0 $(HFILES)
	sed 's;//%;%;' libaltos.i0 $(HFILES) > $(SWIG_FILE)

all-local: classlibaltos.stamp

test:
	which gcc

libaltos_wrap.c: classlibaltos.stamp

classlibaltos.stamp: $(SWIG_FILE)
	swig -java -package libaltosJNI $(SWIG_FILE)
	mkdir -p libaltosJNI
	$(JAVAC) -d . $(AM_JAVACFLAGS) $(JAVACFLAGS) *.java && \
	touch classlibaltos.stamp

MINGCC32=i686-w64-mingw32-gcc
MINGCC64=x86_64-w64-mingw32-gcc
MINGFLAGS=-Wall -Wextra -DWINDOWS -DBUILD_DLL -mconsole -I$(JVM_INCLUDE) -I$(JVM_INCLUDE)/linux
MINGLIBS=-lsetupapi -lws2_32

fat: all altos.dll altos64.dll

altos.dll: $(WINDOWS_SRC) $(WINDOWS_H)
	SOURCE_DATE_EPOCH=0 $(MINGCC32) -o $@ $(MINGFLAGS) -shared $(WINDOWS_SRC) $(MINGLIBS)

altos64.dll: $(WINDOWS_SRC) $(WINDOWS_H)
	SOURCE_DATE_EPOCH=0 $(MINGCC64) -o $@ $(MINGFLAGS) -shared $(WINDOWS_SRC) $(MINGLIBS)

clean-local:
	-rm -rf libaltosJNI *.class *.java classlibaltos.stamp $(SWIG_FILE) libaltos_wrap.c altos.dll altos64.dll
