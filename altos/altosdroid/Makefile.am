DBG_APK=app/build/outputs/apk/debug/app-debug.apk
REL_APK=app/build/outputs/apk/release/app-release.apk

if ANDROID
all_target=$(DBG_APK)
all_target += bin/AltosDroid-debug.apk
if ANDROID_RELEASE
all_target+=$(REL_APK)
all_target += bin/AltosDroid-release.apk
endif
else
all_target=
endif

SDK=$(ANDROID_SDK)
SDK_TARGET=$(shell sed -ne 's/^target=//p' project.properties)

DX=$(SDK)/platform-tools/dx
ADB=$(SDK)/platform-tools/adb
AAPT=$(SDK)/platform-tools/aapt
APKBUILDER=$(SDK)/tools/apkbuilder
ZIPALIGN_A=$(SDK)/tools/zipalign
ZIPALIGN_B=$(shell ls $(SDK)/build-tools/*/zipalign | tail -1)

JAVA_SRC_DIR=app/src/main/java/org/altusmetrum/AltosDroid
EXT_LIBDIR=app/libs
RES_DIR=app/src/main/res
DRAWABLE_DIR=$(RES_DIR)/drawable
LAYOUT_DIR=$(RES_DIR)/layout
MENU_DIR=$(RES_DIR)/menu
VALUES_DIR=$(RES_DIR)/values
XML_DIR=$(RES_DIR)/xml
ALTOSLIB_SRCDIR=../altoslib
ALTOSLIB_JAR=altoslib_$(ALTOSLIB_VERSION).jar

ALTOSLIB=$(EXT_LIBDIR)/$(ALTOSLIB_JAR)

JAVA_SRC=$(JAVA_SRC_DIR)/*.java $(JAVA_SRC_DIR)/BuildInfo.java

DRAWABLES=\
    $(DRAWABLE_DIR)/redled.png \
    $(DRAWABLE_DIR)/greenled.png \
    $(DRAWABLE_DIR)/grayled.png

GRADLEW=JAVA_HOME=/usr/lib/jvm/java-8-openjdk-amd64 bash ./gradlew --no-daemon

LAYOUTS=$(LAYOUT_DIR)/*.xml
MENUS=$(MENU_DIR)/*.xml
VALUES=$(VALUES_DIR)/*.xml
XMLS=$(XML_DIR)/*.xml app/src/main/AndroidManifest.xml

RES=$(LAYOUTS) $(MENUS) $(VALUES) $(XMLS)

SRC=$(JAVA_SRC) $(DRAWABLES) $(RES)

all: $(all_target)

bin/AltosDroid-debug.apk: $(DBG_APK)
	mkdir -p bin
	cp $^ $@

bin/AltosDroid-release.apk: $(REL_APK)
	mkdir -p bin
	cp $^ $@

.NOTPARALLEL:

$(ALTOSLIB): $(ALTOSLIB_SRCDIR)/$(ALTOSLIB_JAR)
	mkdir -p $(EXT_LIBDIR)
	cd $(EXT_LIBDIR) && ln -sf $(shell echo $(EXT_LIBDIR) | sed 's|[^/]\+|..|g')/$(ALTOSLIB_SRCDIR)/$(ALTOSLIB_JAR) .

$(JAVA_SRC_DIR)/BuildInfo.java: $(filter-out $(JAVA_SRC_DIR)/BuildInfo.java,$(shell echo $(JAVA_SRC)))
	./buildinfo.sh

$(DRAWABLE_DIR)/%.png: ../icon/%.png
	cd $(DRAWABLE_DIR) && ln -sf $(shell echo $(DRAWABLE_DIR) | sed 's|[^/]\+|..|g')/$< .

if ANDROID
install-release: $(REL_APK)
	$(ADB) install -r $(REL_APK)

install-debug: $(DBG_APK)
	$(ADB) install -r $(DBG_APK)

$(DBG_APK): $(SRC) $(ALTOSLIB)
	$(GRADLEW) assembleDebug

$(REL_APK): $(SRC) $(ALTOSLIB)
	$(GRADLEW) build

release: $(REL_APK)

clean-local: $(GOOGLE_PLAY_SERVICES_LIB)
	$(GRADLEW) clean
	rm -f $(JAVA_SRC_DIR)/BuildInfo.java
	rm -f $(DRAWABLES)
	rm -rf $(EXT_LIBDIR)
	rm -rf $(GOOGLE_PLAY_SERVICES_LIB)

else

clean-local:

endif

clean: clean-local
