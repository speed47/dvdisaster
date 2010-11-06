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

news_headline("Neuigkeiten - Archiv von 2009");

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
