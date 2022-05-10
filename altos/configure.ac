dnl
dnl  Copyright © 2008,2009 Keith Packard <keithp@keithp.com>
dnl
dnl  This program is free software; you can redistribute it and/or modify
dnl  it under the terms of the GNU General Public License as published by
dnl  the Free Software Foundation; either version 2 of the License, or
dnl  (at your option) any later version.
dnl
dnl  This program is distributed in the hope that it will be useful, but
dnl  WITHOUT ANY WARRANTY; without even the implied warranty of
dnl  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
dnl  General Public License for more details.
dnl
dnl  You should have received a copy of the GNU General Public License along
dnl  with this program; if not, write to the Free Software Foundation, Inc.,
dnl  59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
dnl
dnl Process this file with autoconf to create configure.

AC_PREREQ(2.57)
AC_INIT([altos], 1.9.10.4)
ANDROID_VERSION=34
AC_CONFIG_SRCDIR([src/kernel/ao.h])
AM_INIT_AUTOMAKE([foreign dist-bzip2])
AM_MAINTAINER_MODE

RELEASE_DATE=2022-02-21
AC_SUBST(RELEASE_DATE)

DOC_DATE=`LC_ALL=C date -d $RELEASE_DATE +'%d %b %Y'`

AC_SUBST(DOC_DATE)

VERSION_DASH=`echo $VERSION | sed 's/\./-/g'`
AC_SUBST(VERSION_DASH)
AC_SUBST(ANDROID_VERSION)

dnl ==========================================================================
dnl Java library versions

ALTOSUILIB_VERSION=14
ALTOSLIB_VERSION=14

AC_SUBST(ALTOSLIB_VERSION)
AC_DEFINE(ALTOSLIB_VERSION,$ALTOSLIB_VERSION,[Version of the AltosLib package])
AC_SUBST(ALTOSUILIB_VERSION)
AC_DEFINE(ALTOSUILIB_VERSION,$ALTOSUILIB_VERSION,[Version of the AltosUILib package])

dnl ==========================================================================

AM_CONFIG_HEADER(config.h)

AC_ARG_WITH(freetts, AS_HELP_STRING([--with-freetts=PATH],
	[Set freetts class path (default /usr/share/java)]),
	[FREETTS=$withval], [FREETTS=/usr/share/java])

AC_SUBST(FREETTS)

AC_ARG_WITH(jfreechart, AS_HELP_STRING([--with-jfreechart=PATH],
	[Set jfreechart class path (default /usr/share/java)]),
	[JFREECHART=$withval], [JFREECHART=/usr/share/java])

AC_SUBST(JFREECHART)

AC_ARG_WITH(jcommon, AS_HELP_STRING([--with-jcommon=PATH],
	[Set jcommon class path (default /usr/share/java)]),
	[JCOMMON=$withval], [JCOMMON=/usr/share/java])

AC_SUBST(JCOMMON)

AC_ARG_WITH(jvm, AS_HELP_STRING([--with-jvm=PATH],
	[Set jvm path for java builds (default searches in /usr/lib/jvm)]),
	[JVM=$withval], [JVM=auto])

if test "x$JVM" = "xauto"; then
	AC_MSG_CHECKING([JVM])
	for jvm in default-java java-6-openjdk java-7-openjdk java-8-openjdk java-9-openjdk java-10-openjdk java-11-openjdk java-12-openjdk java-13-openjdk java-6-sun java-8-openjdk-amd64; do
		if test "x$JVM" = "xauto"; then
			INCLUDE="/usr/lib/jvm/$jvm/include"
			if test -f "$INCLUDE"/jni.h; then
				JVM=/usr/lib/jvm/"$jvm"
			fi
		fi
	done
	if test "x$JVM" = "xauto"; then
		AC_MSG_ERROR([no JVM files found])
	fi
	AC_MSG_RESULT([$JVM])
fi

AC_ARG_WITH(java-version, AS_HELP_STRING([--with-java-version=7],
					 [Set java language compatibility version (default is 7)]),
	   [JAVA_VERSION=$withval], [JAVA_VERSION=7])

JAVAC="$JVM"/bin/javac
JAVA="$JVM"/bin/java
JVM_INCLUDE="$JVM"/include
JAVAC_VERSION_FLAGS="-target 1.$JAVA_VERSION -source 1.$JAVA_VERSION"

AC_SUBST(JAVAC)
AC_SUBST(JAVA)
AC_SUBST(JVM)
AC_SUBST(JVM_INCLUDE)
AC_SUBST(JAVAC_VERSION_FLAGS)


AC_ARG_WITH(android, AS_HELP_STRING([--with-android=PATH],
	[Set android SDK path (default searches in a variety of places)]),
	[ANDROID_SDK=$withval], [ANDROID_SDK=${ANDROID_SDK:-auto}])

if test "x$ANDROID_SDK" = "xauto"; then
	AC_MSG_CHECKING([Android SDK])
	for sdk in ../android/android-sdk-linux ../android/android-sdk ../android-sdk ../android-sdk-linux $HOME/android; do
		if test "x$ANDROID_SDK" = "xauto"; then
			SDK="$sdk"
			if test -f "$SDK/SDK Readme.txt"; then
				ANDROID_SDK=`readlink -m "$SDK"`
			fi
		fi
	done
	if test "x$ANDROID_SDK" = "xauto"; then
		AC_MSG_NOTICE([no Android SDK found])
		ANDROID_SDK=no
	fi
	AC_MSG_RESULT([$ANDROID_SDK])
fi
if test "x$ANDROID_SDK" != "xno"; then
	AC_MSG_CHECKING([Android SDK version])
	SDK_VERSION=`grep -i '^Pkg.Revision=' $ANDROID_SDK/tools/source.properties | cut -f2- -d= | sed s/^r//`
	SDK_MAJOR_VERSION="`echo $SDK_VERSION | sed 's/[[^0-9]].*//'`"
	if test "$SDK_MAJOR_VERSION" -lt 17 ; then
		AC_MSG_NOTICE([SDK version $SDK_VERSION is too old. Need >= r17])
		ANDROID_SDK=no
	else
		AC_MSG_RESULT([$SDK_VERSION])
	fi
fi
if test "x$ANDROID_SDK" != "xno"; then
	AC_MSG_CHECKING([Android SDK API Level])
	if test -f "$ANDROID_SDK/platforms/android-10/source.properties"; then
		API_LEVEL=`sed -ne 's/^AndroidVersion.ApiLevel=//p' $ANDROID_SDK/platforms/android-10/source.properties`
		if test "$API_LEVEL" = "10"; then
			AC_MSG_RESULT([$API_LEVEL])
		else
			AC_MSG_NOTICE([API Level 10 not installed correctly.])
			ANDROID_SDK=no
		fi
	else
		AC_MSG_NOTICE([API Level 10 not installed, but is required.])
		ANDROID_SDK=no
	fi
fi
if test "x$ANDROID_SDK" != "xno"; then
	AC_MSG_CHECKING([Android SDK required extras])
	if test -f "$ANDROID_SDK/extras/android/support/source.properties"; then
		ANDROID_SUPPORT_LIB_NAME=`sed -ne 's/^Extra.NameDisplay=//p' $ANDROID_SDK/extras/android/support/source.properties`
		if test -f "$ANDROID_SDK/extras/google/google_play_services/source.properties"; then
			GOOGLE_PLAY_SERVICES_NAME=`sed -ne 's/^Extra.NameDisplay=//p' $ANDROID_SDK/extras/google/google_play_services/source.properties`
			AC_MSG_RESULT([$ANDROID_SUPPORT_LIB_NAME, $GOOGLE_PLAY_SERVICES_NAME])
		else
			AC_MSG_NOTICE([Google Play Services library not installed.])
			ANDROID_SDK=no
		fi
	else
		AC_MSG_NOTICE([Android Support Library not installed.])
		ANDROID_SDK=no
	fi
fi

ANDROID_RELEASE=no
if test "x$ANDROID_SDK" != "xno"; then
	HAVE_ANDROID_SDK="yes"
	ANDROID_RELEASE=yes
else
	HAVE_ANDROID_SDK="no"
fi

AM_CONDITIONAL([ANDROID], [test x$ANDROID_SDK != xno])
AM_CONDITIONAL([ANDROID_RELEASE], [test x$ANDROID_RELEASE = xyes])

AC_SUBST(ANDROID_SDK)

AC_ARG_WITH(fat-dir, AS_HELP_STRING([--with-fat-dir=PATH],
	    [Set the directory to install the 'fat-install' distribution files to (defaults to no such target)]),
	    [FATDIR=$withval], [FATDIR=none])

AM_CONDITIONAL(FATINSTALL, [test "x$FATDIR" != "xnone"])

AC_SUBST(FATDIR)

AC_ARG_WITH(google-key, AS_HELP_STRING([--with-google-key=PATH],
	    [Set the file to read the AltosDroid maps API key from (defaults to ~/altusmetrumllc/google-altosdroid-maps-api-key)]),
	    [GOOGLEKEYFILE=$withval], [GOOGLEKEYFILE=$HOME/altusmetrumllc/google-altosdroid-maps-api-key])

if test -r "$GOOGLEKEYFILE" -a -s "$GOOGLEKEYFILE"; then
	GOOGLEKEY=`cat "$GOOGLEKEYFILE"`
	HAVE_GOOGLE_KEY="yes"
else
	GOOGLEKEY='null'
	HAVE_GOOGLE_KEY="no"
fi

WARN_CFLAGS="-Wall -Wpointer-arith -Wstrict-prototypes -Wmissing-prototypes -Wmissing-declarations -Wnested-externs"
AC_SUBST(WARN_CFLAGS)

AC_ARG_ENABLE(faketime, AS_HELP_STRING([--enable-faketime],
              [Use faketime program to ensure pdf files are reproducible (default=no)]),
	      [FAKETIME=$enableval], [FAKETIME=no])

AM_CONDITIONAL(FAKETIME, [test x$FAKETIME = xyes])

AC_SUBST(GOOGLEKEY)

AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_LIBTOOL
PKG_PROG_PKG_CONFIG

WARN_CFLAGS=""
if test "x$GCC" = "xyes"; then
	WARN_CFLAGS="-Wall -Wpointer-arith -Wstrict-prototypes \
	-Wmissing-prototypes -Wmissing-declarations \
	-Wnested-externs -fno-strict-aliasing"
	AC_DEFINE_UNQUOTED(HAVE_WARNING_CPP_DIRECTIVE,1,
	[Can use #warning in C files])
fi
AC_SUBST(WARN_CFLAGS)

#
# Configure ARM compiler for STM32L and LPC11U14
#

AC_ARG_WITH([arm-cc],
	    [AS_HELP_STRING([--with-arm-cc],
	                    [Name of ARM C compiler])],
	    [],
	    [with_arm_cc=auto])

if test "x$with_arm_cc" != "xno"; then	    
	if test "x$with_arm_cc" = "xauto"; then
		with_arm_cc="arm-none-eabi-gcc"
		AC_CHECK_PROG([HAVE_ARM_CC],[$with_arm_cc], yes, no)
	else
		HAVE_ARM_CC=yes
	fi
else
	HAVE_ARM_CC=no
fi

if test "x$HAVE_ARM_CC" = "xno"; then
	AC_MSG_WARN([Arm compiler not found, ARM binaries will not be built])
else
	ARM_CC=$with_arm_cc
fi
AC_SUBST(HAVE_ARM_CC)
AC_SUBST(ARM_CC)

if test "x$HAVE_ARM_CC" = "xyes"; then
	save_CC="$CC"
	save_CFLAGS="$CFLAGS"
	save_LIBS="$LIBS"
	CC="$ARM_CC"
	CFLAGS="-mthumb -mcpu=cortex-m0"
	LIBS="--specs=picolibc.specs"
	AC_LANG_PUSH([C])

	AC_MSG_CHECKING([if ]$ARM_CC[ supports cortex-m0])
	AC_LINK_IFELSE([AC_LANG_PROGRAM([])],
			  [HAVE_ARM_M0_CC=yes],
			  [HAVE_ARM_M0_CC=no])
        AC_MSG_RESULT([$HAVE_ARM_M0_CC])

	CFLAGS="-mthumb -mcpu=cortex-m3"
	AC_MSG_CHECKING([if ]$ARM_CC[ supports cortex-m3])
	AC_LINK_IFELSE([AC_LANG_PROGRAM([])],
			  [HAVE_ARM_M3_CC=yes],
			  [HAVE_ARM_M3_CC=no])
        AC_MSG_RESULT([$HAVE_ARM_M3_CC])

	AC_LANG_POP([C])
	LIBS="$save_LIBS"
	CFLAGS="$save_CFLAGS"
	CC="$save_CC"
else
	HAVE_ARM_M3_CC=no
	HAVE_ARM_M0_CC=no
fi
AC_SUBST(HAVE_ARM_M3_CC)
AC_SUBST(HAVE_ARM_M0_CC)
	
if test "x$HAVE_ARM_M3_CC" = "xno"; then
	AC_MSG_WARN([No cortex-m3 arm compiler found, STM32L binaries will not be built])
fi

if test "x$HAVE_ARM_M0_CC" = "xno"; then
	AC_MSG_WARN([No cortex-m0 arm compiler found, LPC11U14 and STM32F0 binaries will not be built])
fi



#
# Configure AVR compiler
#

AC_ARG_WITH([avr-cc],
	    [AS_HELP_STRING([--with-avr-cc],
	                    [Name of AVR C compiler])],
	    [],
	    [with_avr_cc=auto])

if test "x$with_avr_cc" != "xno"; then	    
	if test "x$with_avr_cc" = "xauto"; then
		with_avr_cc="avr-gcc"
		AC_CHECK_PROG([HAVE_AVR_CC],[$with_avr_cc], yes, no)
	else
		HAVE_AVR_CC=yes
	fi
else
	HAVE_AVR_CC=no
fi

AC_ARG_WITH([avr-objcopy],
	    [AS_HELP_STRING([--with-avr-objcopy],
	                    [Name of AVR objcopy])],
	    [],
	    [with_avr_objcopy=auto])

if test "x$with_avr_objcopy" != "xno"; then	    
	if test "x$with_avr_objcopy" = "xauto"; then
		with_avr_objcopy="avr-objcopy"
		AC_CHECK_PROG([HAVE_AVR_OBJCOPY],[$with_avr_objcopy], yes, no)
	else
		HAVE_AVR_OBJCOPY=yes
	fi
else
	HAVE_AVR_OBJCOPY=no
fi

if test "x$HAVE_AVR_CC" = "xno" -o "x$HAVE_AVR_OBJCOPY" = "xno"; then
	AC_MSG_WARN([AVR compiler and objcopy not found, atmel binaries will not be built])
	HAVE_AVR_CC=no
else
	save_CC="$CC"
	save_CFLAGS="$CFLAGS"
	save_LIBS="$LIBS"

	CC="$with_avr_cc"
	CFLAGS="-mmcu=attiny85"
	AC_LANG_PUSH([C])
	AC_MSG_CHECKING([if ]$with_avr_cc[ can link programs])
	AC_LINK_IFELSE([AC_LANG_PROGRAM([])],
			  [AVR_LINK=yes],
			  [AVR_LINK=no])
        AC_MSG_RESULT([$AVR_LINK])
	AC_LANG_POP([C])

	LIBS="$save_LIBS"
	CFLAGS="$save_CFLAGS"
	CC="$save_CC"

	if test "x$AVR_LINK" = xyes; then
		AVR_CC=$with_avr_cc
		AVR_OBJCOPY=$with_avr_objcopy
	else
		HAVE_AVR_CC=no;
	fi
fi

AC_SUBST(AVR_CC)
AC_SUBST(AVR_OBJCOPY)
AC_SUBST(HAVE_AVR_CC)

AC_CHECK_PROG([HAVE_NICKLE], [nickle], yes, no)
if test "x$HAVE_NICKLE" = "xno"; then
	AC_MSG_ERROR([Please install nickle to build AltOs])
fi

AC_CHECK_PROG([HAVE_ASCIIDOCTOR_PDF], [asciidoctor-pdf], yes, no)
if test "x$HAVE_ASCIIDOCTOR_PDF" = "xno"; then
	AC_MSG_WARN([asciidoctor-pdf not found, PDF docs will not be built])
fi
AM_CONDITIONAL([ASCIIDOCTOR_PDF], [test x$HAVE_ASCIIDOCTOR_PDF != xno])

AC_ARG_WITH([strip-nondeterminism],
	    [AS_HELP_STRING([--with-strip-nondeterminism],
		            [Name of non-deterministic build data stripping tool])],
	    [],
	    [with_strip_nondeterminism=auto])

if test "x$with_strip_nondeterminism" != "xno"; then
	if test "x$with_strip_nondeterminism" = "xauto"; then
		with_strip_nondeterminism="strip-nondeterminism"
		AC_CHECK_PROG([HAVE_STRIP_NONDETERMINISM],[$with_strip_nondeterminism], yes, no)
	else
		HAVE_STRIP_NONDETERMINISM=yes
	fi
else
	HAVE_STRIP_NONDETERMINISM=no
fi
AM_CONDITIONAL([STRIP_NONDETERMINISM], [test x$HAVE_STRIP_NONDETERMINISM != xno])
AC_SUBST(STRIP_NONDETERMINISM, "$with_strip_nondeterminism")

PKG_CHECK_MODULES([JANSSON], [jansson])

AC_ARG_WITH([readline],
	    [AS_HELP_STRING([--with-readline],
			      [enable readline functionality in ao-dbg @<:@default=auto@:>@])],
	      [],
	      [with_readline=auto])
			      
LIBREADLINE_LIBS=

if test x"$with_readline" != "xno"; then
	AC_CHECK_LIB([readline], [main],
                      [AC_SUBST([LIBREADLINE], ["-lreadline -lncurses"])
	               AC_DEFINE([HAVE_LIBREADLINE], [1],
                         [Define if you have libreadline])],
		      [if test "x$with_readline" != xauto; then
	                 AC_MSG_ERROR([--with-readline was given, but test for readline failed])
	               fi],
		       -lncurses)
fi

PKG_CHECK_MODULES([LIBUSB], [libusb-1.0])

AC_CHECK_HEADERS(libelf.h libelf/libelf.h, [break])
AC_CHECK_HEADERS(gelf.h libelf/gelf.h, [break])

AC_ARG_WITH([stlink],
	    [AS_HELP_STRING([--with-stlink],
	                    [Build tools that use the stlink library (default: auto)])],
	    [],
	    [with_stlink=auto])

if test x"$with_stlink" != "xno"; then
	PKG_CHECK_MODULES([STLINK], [stlink], [HAVE_STLINK=yes], [HAVE_STLINK=no])
	if test x"$HAVE_STLINK" = "xno" -a x"$with_stlink" != "xauto"; then
		AC_MSG_ERROR([--with-stlink was given, but stlink was not found])
	fi
else
	HAVE_STLINK=no
fi

if test x"$HAVE_STLINK" = "xyes"; then
	AC_DEFINE(HAVE_STLINK,1,[Using STlink library])
fi

AM_CONDITIONAL([LIBSTLINK], [test x$HAVE_STLINK = xyes])

AC_ARG_ENABLE([multi-arch],
	      [AS_HELP_STRING([--enable-multi-arch],
			      [enable building both i386 and amd64 libraries (default=auto)])],
	      [MULTI_ARCH=$enableval],
	      [MULTI_ARCH=auto])

case x"$MULTI_ARCH" in
xauto)
	arch=`uname -m`
	case x"$arch" in
	xx86_64|xi*86)
		save_CFLAGS="$CFLAGS"
		save_LIBS="$LIBS"
		LIBS="-ldl"
		CFLAGS="-m64"
		AC_MSG_CHECKING([if ]$CC[ ]$CFLAGS[ can link programs])
		AC_LINK_IFELSE([AC_LANG_PROGRAM([])],
				[M64_LINK=yes],
				[M64_LINK=no])
		AC_MSG_RESULT([$M64_LINK])
		CFLAGS="-m32"
		AC_MSG_CHECKING([if ]$CC[ ]$CFLAGS[ can link programs])
		AC_LINK_IFELSE([AC_LANG_PROGRAM([])],
				[M32_LINK=yes],
				[M32_LINK=no])
		AC_MSG_RESULT([$M32_LINK])
		CFLAGS="$save_CFLAGS"
		LIBS="$save_LIBS"
		case x"$M64_LINK"x"$M32_LINK" in
		xyesxyes)
			MULTI_ARCH=yes
			;;
		*)
			MULTI_ARCH=no
			;;
		esac
		;;
	*)
		MULTI_ARCH=no
		;;
	esac
	;;
xyes|xno)
	;;
*)
	MULTI_ARCH="no"
	;;
esac
	
AM_CONDITIONAL([MULTI_ARCH], [test x$MULTI_ARCH = xyes])

AC_ARG_ENABLE([install-shared-mime-info],
	      [AS_HELP_STRING([--disable-install-shared-mime-info],
	                      [disable installing shared mime info files (default=yes)])],
	      [INSTALL_SHARED_MIME_INFO=$enableval],
	      [INSTALL_SHARED_MIME_INFO=yes])

AM_CONDITIONAL([INSTALL_SHARED_MIME_INFO], [test x$INSTALL_SHARED_MIME_INFO = xyes])

AC_OUTPUT([
Makefile
src/Makedefs
src/chaoskey-v1.0/org.altusmetrum.ChaosKey.metainfo.xml
doc/Makefile
altoslib/Makefile
altoslib/AltosVersion.java
icon/Makefile
altosuilib/Makefile
altosui/Makefile
altosui/Info.plist
altosui/altos-windows.nsi
libaltos/Makefile
micropeak/Makefile
micropeak/Info.plist
micropeak/micropeak-windows.nsi
telegps/Makefile
telegps/Info.plist
telegps/telegps-windows.nsi
altosdroid/Makefile
altosdroid/local.properties
altosdroid/app/src/main/AndroidManifest.xml
altosdroid/app/build.gradle
ao-tools/Makefile
ao-tools/lib/Makefile
ao-tools/ao-rawload/Makefile
ao-tools/ao-dbg/Makefile
ao-tools/ao-bitbang/Makefile
ao-tools/ao-eeprom/Makefile
ao-tools/ao-list/Makefile
ao-tools/ao-load/Makefile
ao-tools/ao-telem/Makefile
ao-tools/ao-stmload/Makefile
ao-tools/ao-send-telem/Makefile
ao-tools/ao-sky-flash/Makefile
ao-tools/ao-dumpflash/Makefile
ao-tools/ao-edit-telem/Makefile
ao-tools/ao-dump-up/Makefile
ao-tools/ao-elftohex/Makefile
ao-tools/ao-usbload/Makefile
ao-tools/ao-flash/Makefile
ao-tools/ao-test-igniter/Makefile
ao-tools/ao-test-baro/Makefile
ao-tools/ao-test-flash/Makefile
ao-tools/ao-cal-accel/Makefile
ao-tools/ao-cal-freq/Makefile
ao-tools/ao-test-gps/Makefile
ao-tools/ao-usbtrng/Makefile
ao-tools/ao-makebin/Makefile
ao-utils/Makefile
map-server/Makefile
map-server/altos-mapd/Makefile
map-server/altos-map/Makefile
map-server/altos-mapj/Makefile
])

echo ""
echo "  Package: ${PACKAGE_NAME} ${PACKAGE_VERSION}"
echo ""
echo "  Configuration"
echo "    Arm compiler................: ${ARM_CC}"
echo "    STM32L support..............: ${HAVE_ARM_M3_CC}"
echo "    LPC11U14 support............: ${HAVE_ARM_M0_CC}"
echo "    AVR compiler................: ${AVR_CC} ${AVR_OBJCOPY}"
echo "    AVR support.................: ${HAVE_AVR_CC}"
echo "    Android support.............: ${HAVE_ANDROID_SDK}"
echo "    Android release support.....: ${ANDROID_RELEASE}"
echo "    STlink support..............: ${HAVE_STLINK}"
echo "    i386 and amd64 libaltos.....: ${MULTI_ARCH}"
echo "    install shared mime info....: ${INSTALL_SHARED_MIME_INFO}"
echo "    Strip jar timestamps........: ${STRIP_NONDETERMINISM}"
echo ""
echo "  Java"
echo "    freetts.....................: ${FREETTS}"
echo "    jfreechart..................: ${JFREECHART}"
echo "    jcommon.....................: ${JCOMMON}"
echo "    JVM include.................: ${JVM_INCLUDE}"
echo "    AltosDroid maps API key.....: ${HAVE_GOOGLE_KEY}"
if test x${ANDROID_SDK} != "xno"; then
echo ""
echo "  Android path"
echo "    Android SDK.................: ${ANDROID_SDK}"
fi
echo ""
