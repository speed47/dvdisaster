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

news_headline("Neuigkeiten - Archiv von 2009");

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

news_item("14.04.2009", "Nachgelegt: dvdisaster 0.72.rc1 für Mac OS X", "
Die native Version für Mac OS X ist im Bereich der
Benutzeroberfläche <a href=\"download30.php#mac\">noch etwas hakelig</a>,
da die Portierung der GTK+-Bibliothek in einem frühen Entwicklungsstadium ist.
Doch mit der heute aktualisierten dvdisaster-Version läßt sich schon auf dem Mac arbeiten...
", 9, "2009-04-14T00:00:00Z", "2009-04-14T00:00:00Z");

news_item("11.04.2009", "dvdisaster 0.72.rc1 veröffentlicht", "
Der erste Veröffentlichungskandidat für den stabilen 0.72er Zweig 
steht nun bereit. Unter anderem gibt es Unterstützung für Blu-Ray-Datenträger und
\"Raw\"-Lesen sowie C2-Überprüfungen für CD. 
Auf der <a href=\"download.php\">Herunterladen</a>-Seite
finden Sie mehr Informationen.
", 8, "2009-04-11T00:00:00Z", "2009-04-11T00:00:00Z");

news_item("08.03.2009", "dvdisaster 0.71.28 veröffentlicht", "
Diese Version behebt Abstürze durch unzulässige Befehle 
(\"illegal Instruction\") auf x86-Maschinen die SSE2 nicht unterstützen.
Sie enthält weitere kleinere Änderungen auf dem Weg zum 
Veröffentlichungskandidaten für den stabilen Zweig.
", 7, "2009-03-08T00:00:00Z", "2009-03-08T00:00:00Z");

news_item("18.01.2009", "dvdisaster 0.71.27 veröffentlicht", "
Die Dokumentation ist wieder komplett. Einige Crashes und
Inkompatibilitäten mit ungewöhnlichen Laufwerken und Datenträgern wurden behoben.
Diese Version enthält die letzte große interne Änderung, die für die 0.71.x-Serie
geplant war. Wenn alles gut geht wird die nächste Version ein 
Veröffentlichungskandidat für den stabilen Zweig (0.72).
", 6, "2009-01-18T00:00:00Z", "2009-01-18T00:00:00Z");

if($news_flash == 0) 
   end_page();
?>
