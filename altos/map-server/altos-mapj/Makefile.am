JAVAROOT=classes
AM_JAVACFLAGS=$(JAVAC_VERSION_FLAGS) -encoding UTF-8 -Xlint:deprecation -Xlint:unchecked

altoslibdir=$(libdir)/altos

CLASSPATH_ENV=mkdir -p $(JAVAROOT); CLASSPATH="$(JAVAROOT):../../altoslib/*"

bin_SCRIPTS=altos-mapj

altosmap_JAVA = \
	AltosMap.java

ALTOSLIB_CLASS=\
	altoslib_$(ALTOSLIB_VERSION).jar

JAR=altosmap.jar

FATJAR=altosmap-fat.jar

all-local: classes/altosmap $(JAR) altos-mapj altos-mapj-test altos-mapj-jdb

fat: $(FATJAR)

install-altosmapJAVA: altosmap.jar
	@$(NORMAL_INSTALL)
	test -z "$(altosmapdir)" || $(MKDIR_P) "$(DESTDIR)$(altosmapdir)"
	echo " $(INSTALL_DATA)" "$<" "'$(DESTDIR)$(altosmapdir)/altosmap.jar'"; \
	$(INSTALL_DATA) "$<" "$(DESTDIR)$(altosmapdir)"

classes/altosmap:
	mkdir -p classes/altosmap

$(JAR): classaltosmap.stamp Manifest.txt $(ALTOSLIB_CLASS)
	jar cfm $@ Manifest.txt \
		-C classes altosmap
if STRIP_NONDETERMINISM
	$(STRIP_NONDETERMINISM) $@
endif

altosmapdir=$(datadir)/java

$(FATJAR): classaltosmap.stamp Manifest-fat.txt $(ALTOSLIB_CLASS)
	jar cfm $@ Manifest-fat.txt \
		-C classes altosmap
if STRIP_NONDETERMINISM
	$(STRIP_NONDETERMINISM) $@
endif

altos-mapj: Makefile
	echo "#!/bin/sh" > $@
	echo 'exec java -Djava.library.path="$(altoslibdir)" -jar "$(altosmapdir)/altosmap.jar" "$$@"' >> $@
	chmod +x $@

Manifest.txt: Makefile
	echo 'Main-Class: altosmap.AltosMap' > $@
	echo "Class-Path: $(ALTOSLIB_CLASS) $(ALTOSUILIB_CLASS)" >> $@

Manifest-fat.txt: Makefile
	echo 'Main-Class: altosmap.AltosMap' > $@
	echo "Class-Path: $(ALTOSLIB_CLASS) $(ALTOSUILIB_CLASS)" >> $@

altos-mapj-test: Makefile
	echo '#!/bin/sh' > $@
	echo 'dir="$$(dirname $$0)"' >> $@
	echo 'cd "$$dir"' >> $@
	echo 'altosmap="$$(pwd -P)"' >> $@
	echo 'exec java -jar "$$altosmap/altosmap.jar" "$$@"' >> $@
	chmod +x $@

altos-mapj-jdb: Makefile
	echo "#!/bin/sh" > $@
	echo 'exec jdb altosmap/AltosMap "$$@"' >> $@
	chmod +x $@

$(ALTOSLIB_CLASS):
	-rm -f "$@"
	$(LN_S) ../../altoslib/"$@" .

clean::
	rm -f Manifest.txt Manifest-fat.txt
