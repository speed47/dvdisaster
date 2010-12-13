<?php
# dvdisaster: German homepage translation
# Copyright (C) 2004-2010 Carsten Gnörlich
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


news_item("01.10.2009", "Einschätzung zu potentiellen Sicherheitslücken in der GTK-Bibliothek für Windows", "
Die Windows-Versionen von dvdisaster 0.70.x/0.72.x werden mit einer alten Version 
der GTK-Bibliothek
ausgeliefert, die Schwachstellen in der Verarbeitung von Bilddateien aufweist.
Um die Lücke auszunutzen, müssen manipulierte Bilder aus
einer externen Quelle nachgeladen werden. Da dvdisaster
keine derartigen Funktionen beinhaltet, ist diese Schwachstelle als nicht 
gefährlich anzusehen.<p>
Ein Auswechseln von GTK innerhalb von dvdisaster 0.70.x/0.72.x wird nicht
empfohlen, da sich in neueren GTK-Versionen einige Schnittstellen geändert
haben. Der Austausch kann daher zu Fehlfunktionen führen.<p>
Die Windows-Version von dvdisaster 0.79.1 wird mit entsprechend überarbeiteten
Schnittstellen und einer aktuellen Version von GTK ausgeliefert werden.<p>
An dieser Stelle vielen Dank an alle Nutzer, die einen Hinweis auf die
Schwachstelle gegeben haben. 
", 13, "2009-10-01T00:00:00Z", "2009-10-01T00:00:00Z");

news_item("10.08.2009", "Projektseiten auf SourceForge nicht aktuell", "
Aufgrund von Änderungen in der Funktionalität von SourceForge
können die Inhalte der News, Downloads und des CVS nicht zeitnah
hochgeladen werden. Bitte nutzen Sie stattdessen die entsprechenden
Rubriken auf diesen Seiten (http://dvdisaster.net).
", 12, "2009-08-10T00:00:00Z", "2009-08-10T00:00:00Z");

news_item("08.08.2009", "dvdisaster 0.72.1 veröffentlicht", "
Pablo Almeida hat die Bildschirmtexte ins Portugiesische übersetzt.
Es wurde eine Umgehungslösung eingebaut um Win XP vom Einfrieren 
bei bestimmten CD-RW/Laufwerks-Kombinationen abzuhalten.<p>
<i>Aktueller Hinweis: Die Umgehungslösung ist nicht vollständig.
Falls das Problem immer noch besteht, probieren Sie bitte Version 0.79.x aus.</i>
", 11, "2009-08-08T00:00:00Z", "2010-02-06T00:00:00Z");

news_item("04.07.2009", "dvdisaster 0.72 veröffentlicht", "
Dies ist die erste stabile Version des 0.72er-Zweiges.
Igor Gorbounov hat die russische online-Dokumentation vervollständigt
und es wurden noch einige kleine Fehler aus dem ersten 
Veröffentlichungskandidaten behoben.<p>
", 10, "2009-07-04T00:00:00Z", "2009-07-04T00:00:00Z");

news_finalize();

if($news_flash == 0) 
   end_page();
?>
