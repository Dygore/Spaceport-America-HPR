#
# Copyright © 2018 Keith Packard <keithp@keithp.com>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.
#

if FAKETIME
FAKETIME=TZ=UTC faketime -f '$(RELEASE_DATE) 00:00:00 i0'
endif

RELNOTES_INC=\
	release-notes-1.9.10.inc \
	release-notes-1.9.9.inc \
	release-notes-1.9.8.inc \
	release-notes-1.9.7.inc \
	release-notes-1.9.6.inc \
	release-notes-1.9.5.inc \
	release-notes-1.9.4.inc \
	release-notes-1.9.3.inc \
	release-notes-1.9.2.inc \
	release-notes-1.9.1.inc \
	release-notes-1.9.inc \
	release-notes-1.8.7.inc \
	release-notes-1.8.6.inc \
	release-notes-1.8.5.inc \
	release-notes-1.8.4.inc \
	release-notes-1.8.3.inc \
	release-notes-1.8.2.inc \
	release-notes-1.8.1.inc \
	release-notes-1.8.inc \
	release-notes-1.7.inc \
	release-notes-1.6.8.inc \
	release-notes-1.6.5.inc \
	release-notes-1.6.4.inc \
	release-notes-1.6.3.inc \
	release-notes-1.6.2.inc \
	release-notes-1.6.1.inc \
	release-notes-1.6.inc \
	release-notes-1.5.inc \
	release-notes-1.4.2.inc \
	release-notes-1.4.1.inc \
	release-notes-1.4.inc \
	release-notes-1.3.2.inc \
	release-notes-1.3.1.inc \
	release-notes-1.3.inc \
	release-notes-1.2.1.inc \
	release-notes-1.2.inc \
	release-notes-1.1.1.inc \
	release-notes-1.1.inc \
	release-notes-1.0.1.inc \
	release-notes-0.9.2.inc \
	release-notes-0.9.inc \
	release-notes-0.8.inc \
	release-notes-0.7.1.inc

IMAGES=\
	altosui.png \
	ascent.png \
	configure-altimeter.png \
	configure-altosui.png \
	configure-groundstation.png \
	configure-pyro.png \
	descent.png \
	device-selection.png \
	easymega.svg \
	easymega-v1.0-bottom.jpg \
	easymega-v1.0-top.jpg \
	easymini.svg \
	easymini-top.jpg \
	easytimer.jpg \
	fire-igniter.png \
	graph-configure.png \
	graph-map.png \
	graph.png \
	graph-stats.png \
	ignitor.png \
	landed.png \
	launch-pad.png \
	load-maps.png \
	micropeak-app.png \
	micropeak-back.jpg \
	micropeak-device-dialog.png \
	micropeak-dime.jpg \
	micropeak-download.png \
	micropeak-graph-configure.png \
	micropeak-graph.png \
	micropeak-nofont.svg \
	micropeak-preferences.png \
	micropeak-raw-data.png \
	micropeak-save-dialog.png \
	micropeak-statistics.png \
	MicroPeakUSB-2.0-inuse.jpg \
	MicroPeakUSB-2.0.jpg \
	monitor-idle.png \
	scan-channels.png \
	site-map.png \
	table.png \
	telegps-configure.png \
	telegps-graph-configure.png \
	telegps-graph-graph.png \
	telegps-graph-map.png \
	telegps-graph-stats.png \
	telegps-info.png \
	telegps-location.png \
	telegps-map.png \
	telegps-preferences.png \
	telegps-scan.png \
	telegps-status.png \
	telegps-table.png \
	telegps-v1.0-top.jpg \
	telemega.svg \
	telemega-v1.0-top.jpg \
	telemetrum.svg \
	telemetrum-v1.1-thside.jpg \
	telemetrum-v2.0-th.jpg \
	telemini-v1.svg \
	telemini-v1-top.jpg \
	telemini-v3.svg \
	telemini-v3.0-top.jpg \
	telemini-v3.0-bottom.jpg \
	altusmetrum-oneline.svg \
	telegps-oneline.svg \
	micropeak-oneline.svg

TXT_FILES=altusmetrum.txt

COMMON_INC_FILES=\
	config-device.inc \
	config-ui.inc \
	load-maps.inc \
	aprs-operation.inc \
	pyro-examples.inc \
	handling.inc \
	release-head.inc

INC_FILES=\
	header.inc \
	dedication.inc \
	intro.inc \
	getting-started.inc \
	usage.inc \
	telemetrum.inc \
	telemini.inc \
	easymini-device.inc \
	telemega.inc \
	easymega.inc \
	easytimer.inc \
	installation.inc \
	using-am-products.inc \
	updating-firmware.inc \
	frequency-cal.inc \
	altosui.inc \
	altosdroid.inc \
	system-operation.inc \
	pyro-channels.inc \
	flight-data-recording.inc \
	specs.inc \
	$(COMMON_INC_FILES) \
	release-notes.inc \
	$(RELNOTES_INC)

ADOC_FILES=$(TXT_FILES:.txt=.adoc) $(INC_FILES:.inc=.adoc)

MOTORTEST_TXT_FILES=motortest.txt

MOTORTEST_INC_FILES=\
	motortest-configuration.inc \
	motortest-installation.inc \
	motortest-intro.inc \
	motortest-operation.inc

MOTORTEST_ADOC_FILES=$(MOTORTEST_TXT_FILES:.txt=.adoc) $(MOTORTEST_INC_FILES:.inc=.adoc)

TELELAUNCH_TXT_FILES=telelaunch.txt

TELELAUNCH_INC_FILES=\
	header.inc \
	telelaunch-acknowledgements.inc \
	telelaunch-intro.inc \
	telelaunch-configuration.inc \
	telelaunch-operation.inc \
	telelaunch-specs.inc \
	telelaunch-updating-firmware.inc \
	telelaunch-cables.inc \
	telelaunch-troubleshooting.inc 

TELELAUNCH_ADOC_FILES=$(TELELAUNCH_TXT_FILES:.txt=.adoc) $(TELELAUNCH_INC_FILES:.inc=.adoc)

TELEGPS_INC_FILES=\
	telegps-dedication.inc \
	telegps-quick-start.inc \
	telegps-using.inc \
	telegps-system-operation.inc \
	telegps-application.inc \
	telegps-specs.inc \
	telegps-updating-firmware.inc \
	telegps-release-notes.inc \
	$(COMMON_INC_FILES)

TELEGPS_TXT_FILES=\
	telegps.txt

TELEGPS_ADOC_FILES=$(TELEGPS_TXT_FILES:.txt=.adoc) $(TELEGPS_INC_FILES:.inc=.adoc)

MICROPEAK_TXT_FILES=\
	micropeak.txt

MICROPEAK_INC_FILES=

MICROPEAK_ADOC_FILES=$(MICROPEAK_TXT_FILES:.txt=.adoc) $(MICROPEAK_INC_FILES:.inc=.adoc)

EASYMINI_TXT_FILES=\
	easymini.txt

EASYMINI_INC_FILES=$(INC_FILES) easymini-release-notes.inc


EASYMINI_ADOC_FILES=$(EASYMINI_TXT_FILES:.txt=.adoc) $(EASYMINI_INC_FILES:.inc=.adoc)

OUTLINE_TXT_FILES=\
	easymega-outline.txt \
	easymini-outline.txt \
	telemega-outline.txt \
	telemetrum-outline.txt \
	telemini-v1-outline.txt \
	telemini-v3-outline.txt \
	telegps-outline.txt

OUTLINE_ADOC_FILES=$(OUTLINE_TXT_FILES:.txt=.adoc)

OUTLINE_PDF_FILES=$(OUTLINE_TXT_FILES:.txt=.pdf)

SVG=\
	easymini.svg \
	telemega.svg \
	telemetrum.svg \
	telemini-v1.svg \
	telemini-v3.svg \
	easymega.svg

RELNOTES_HTML=$(RELNOTES_INC:.inc=.html)

ONEFILE_TXT_FILES=\
	altos.txt \
	companion.txt \
	telemetry.txt \
	map-loading.txt

ONEFILE_ADOC_FILES=$(ONEFILE_TXT_FILES:.txt=.adoc)
ONEFILE_PDF_FILES=$(ONEFILE_TXT_FILES:.txt=.pdf)
ONEFILE_HTML_FILES=$(ONEFILE_TXT_FILES:.txt=.html)

AM_HTML=am.html

PUBLISH_HTML=altusmetrum.html micropeak.html telegps.html easymini.html motortest.html telelaunch.html $(ONEFILE_HTML_FILES)

HTML=$(PUBLISH_HTML) $(RELNOTES_HTML)

if ASCIIDOCTOR_PDF
PDF=altusmetrum.pdf micropeak.pdf telegps.pdf easymini.pdf motortest.pdf telelaunch.pdf $(ONEFILE_PDF_FILES) \
	$(OUTLINE_PDF_FILES)
endif

MAP_DOT_FILES=map-loading.dot
MAP_SVG_FILES=$(MAP_DOT_FILES:.dot=.svg)
STYLESHEET=am.css am-notoc.css

FONTS=\
	fonts/DejaVuSansMono-BoldOblique.ttf \
	fonts/DejaVuSansMono-Bold.ttf \
	fonts/DejaVuSansMono-Oblique.ttf \
	fonts/DejaVuSansMono.ttf \
	fonts/OpenSans-Light.ttf \
	fonts/OpenSans-LightItalic.ttf \
	fonts/OpenSans-Regular.ttf \
	fonts/OpenSans-Italic.ttf \
	fonts/OpenSans-Semibold.ttf \
	fonts/OpenSans-SemiboldItalic.ttf

ICONS=\
	images/icons/caution.svg \
	images/icons/important.svg \
	images/icons/note.svg \
	images/icons/tip.svg \
	images/icons/warning.svg

PDF_CONFIG_FILES=altusmetrum-theme.yml
HTML_CONFIG_FILES=

PUBLISH_DOC=$(PUBLISH_HTML) $(PDF) $(IMAGES) $(MAP_SVG_FILES) $(STYLESHEET)

DOC=$(HTML) $(PDF) $(IMAGES) $(MAP_SVG_FILES) $(STYLESHEET)

SUFFIXES = .dot .svg .inc .txt .adoc .pdf .html

ATTRIBUTES=--attribute="revdate=$(DOC_DATE)" --attribute="version=$(VERSION)"

.dot.svg:
	dot -Tsvg -o$@ $*.dot

.txt.adoc:
	sed -e 's/^[ 	]*//' -e 's/^\\//' $*.txt > $@

.inc.adoc:
	sed -e 's/^[ 	]*//' -e 's/^\\//' $*.inc > $@

.adoc.html:
	asciidoctor $(ATTRIBUTES) -b html5 $*.adoc

.adoc.pdf:
	asciidoctor-pdf $(ATTRIBUTES) $*.adoc

all:	$(HTML) $(PDF)

map-loading.adoc: $(MAP_SVG_FILES)

altusmetrum.pdf altusmetrum.html: $(ADOC_FILES) $(IMAGES)

motortest.pdf motortest.html: $(MOTORTEST_ADOC_FILES) $(IMAGES)

telelaunch.pdf telelaunch.html: $(TELELAUNCH_ADOC_FILES) $(IMAGES)

telegps.html telegps.pdf: $(TELEGPS_ADOC_FILES) $(IMAGES)

micropeak.pdf micropeak.html: $(MICROPEAK_ADOC_FILES) $(IMAGES)

easymini.pdf easymini.html: $(EASYMINI_ADOC_FILES) $(IMAGES)

telemini-v1-outline.pdf: telemini-v1-outline.txt telemini-v1.svg

telemini-v3-outline.pdf: telemini-v3-outline.txt telemini-v3.svg

install:	all

WEB_ROOT=/home/bdale/web/

publish:	$(PUBLISH_DOC) $(FONTS) $(ICONS)
	cp -a $(PUBLISH_DOC) $(WEB_ROOT)/altusmetrum/AltOS/doc/
	mkdir -p $(WEB_ROOT)/altusmetrum/AltOS/doc/fonts/
	cp $(FONTS) $(WEB_ROOT)/altusmetrum/AltOS/doc/fonts/
	mkdir -p $(WEB_ROOT)/altusmetrum/AltOS/doc/images/icons/
	cp $(ICONS) $(WEB_ROOT)/altusmetrum/AltOS/doc/images/icons/
	(cd $(WEB_ROOT)/altusmetrum ; \
	 git add $(WEB_ROOT)/altusmetrum/AltOS/doc/* ; \
	 git add $(WEB_ROOT)/altusmetrum/AltOS/doc/fonts/* ; \
	 git add $(WEB_ROOT)/altusmetrum/AltOS/doc/images/icons/* ;\
	 echo "update docs" | \
	 git commit -F - $(WEB_ROOT)/altusmetrum/AltOS/doc/* $(WEB_ROOT)/altusmetrum/AltOS/doc/fonts/* $(WEB_ROOT)/altusmetrum/AltOS/doc/images/icons ; \
	 git push)

publish-keithp:	am.html $(DOC) $(FONTS)
	ssh keithp.com 'mkdir -p ~keithp/public_html/altos/fonts ~keithp/public_html/altos/images/icons'
	scp -p am.html $(DOC) keithp.com:~keithp/public_html/altos
	scp -p $(FONTS) keithp.com:~keithp/public_html/altos/fonts
	scp -p $(ICONS) keithp.com:~keithp/public_html/altos/images/icons

clean:
	rm -f am.html $(HTML) $(PDF) $(ADOC_FILES) $(TELEGPS_ADOC_FILES) $(MICROPEAK_ADOC_FILES) $(TELELAUNCH_ADOC_FILES) $(MOTORTEST_ADOC_FILES)

distclean: clean
	rm -f $(HTML) $(PDF)

$(PDF): $(PDF_CONFIG_FILES)
$(HTML): $(HTML_CONFIG_FILES)

am.html: Makefile make-am-html $(HTML)
	sh ./make-am-html $(HTML) > $@
