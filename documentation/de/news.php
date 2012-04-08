<?php
# dvdisaster: German homepage translation
# Copyright (C) 2004-2012 Carsten Gnörlich
#
# UTF-8 trigger: äöüß 
#
# Include our PHP sub routines, then call begin_page()
# to start the HTML page, insert the header, 
# navigation and news if appropriate.

if($news_flash == 0) 
{  require("../include/dvdisaster.php");
   begin_page();
}

$news_counter = 0;

# The news page is different from the other pages;
# you must only use the news_*() functions below to create content.
# Do not insert plain HTML!

news_headline("Neues über dvdisaster");

news_item("07.04.2012", "dvdisaster 0.72.4 veröffentlicht", "
Anpassung an aktuelle Versionen von GNU/Linux, FreeBSD und NetBSD.
", 21, "2012-04-07T01:00:00Z", "2012-04-07T01:00:00Z");


news_item("05.10.2011", "dvdisaster 0.72.3 veröffentlicht", "
Die \"Vergleichen\"-Funktion hing bei der Verarbeitung von 
RS01-Fehlerkorrektur-Dateien, die größer als 2GB sind (die
Fehlerkorrektur-Dateien selbst werden korrekt erzeugt).
Volodymyr Bychkoviak fand das Problem und schickte eine
Korrektur.
", 20, "2011-10-05T01:00:00Z", "2011-10-05T01:00:00Z");

news_item("28.07.2011", "Dokumentation ergänzt", "
Die zwischenzeitlich verlorengegangenen <a href=\"qa30.php\">Hintergrundinformationen</a> aus der Dokumentation von Version 0.70 wurden überarbeitet und wieder
online gestellt.
", 19, "2011-07-28T00:00:00Z", "2011-07-28T00:00:00Z");

news_item("21.11.2010", "dvdisaster 0.79.3 veröffentlicht", "
Ab dieser Version wird unter Linux per Voreinstellung der
SG_IO-Treiber zum Zugriff auf die optischen Laufwerke verwendet.
Damit liegt jetzt auch im Entwicklungszweig eine Lösung für
hängende parallele SCSI-Kontroller unter Linux vor, die durch den
alten CDROM_SEND_PACKET-Treiber verursacht wurden.
Für den RS03-Kodierer sind jetzt Optimierungen für 
Altivec auf dem PowerPC verfügbar.
", 18, "2010-11-21T00:00:00Z", "2010-11-21T00:00:00Z");

news_item("07.11.2010", "Windows- und Mac OS X-Versionen für 0.72.2 nachgelegt", "
Die Windows- und Mac OS X-Versionen von 0.72.2 sind jetzt verfügbar.
Sie wurden mit der Entwicklungsumgebung von 0.79.x erzeugt.
Daher sind die mitgelieferten Bibliotheken von GTK+
jetzt in neueren Versionen als in 0.72.1 enthalten und es können sich
leichte Änderungen im Aussehen und Verhalten ergeben.
", 17, "2010-11-07T00:00:00Z", "2010-11-07T00:00:00Z");


news_item("31.10.2010", "dvdisaster 0.72.2 veröffentlicht", "
Diese Version führt einen Workaround ein um zu verhindern
daß parallele SCSI-Kontroller unter Linux nicht
mehr reagieren. 
Die Aufwärtskompatibilität mit dvdisaster 0.79.x wurde verbessert.
", 16, "2010-10-31T00:00:00Z", "2010-10-31T00:00:00Z");

news_item("28.02.2010", "dvdisaster 0.79.2 veröffentlicht", "
Binärpakete sind wieder für Mac OS X verfügbar nachdem die
Entwicklungsumgebung auch für Mac OS X aktualisiert wurde.
 Die Entwicklung
von RS03 geht weiter, ist aber noch längst nicht abgeschlossen.
", 15, "2010-02-28T00:00:00Z", "2010-02-28T00:00:00Z");

news_item("07.02.2010", "Beginn des neuen Entwicklungszweiges 0.79", "
Heute wird mit Version 0.79.1 die erste Version des neuen 
Entwicklungszweiges veröffentlicht. Diese Version dient hauptsächlich
zum Ausprobieren neuer Funktionen und wird nicht zum produktiven
Einsatz empfohlen.
Auf der <a href=\"download40.php\">Seite zum Herunterladen</a>
finden Sie Informationen darüber, was gerade geändert wurde und wie
Sie beim Testen mithelfen können.
", 14, "2010-02-07T00:00:00Z", "2010-02-07T00:00:00Z");

news_finalize();

if($news_flash == 0) 
   end_page();
?>
