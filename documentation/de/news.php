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

news_item("31.10.2010", "dvdisaster 0.72.2 veröffentlicht", "
Diese Version führt einen Workaround ein um zu verhindern
daß parallele SCSI-Kontroller unter Linux nicht
mehr reagieren. Mit Version 0.79.x erzeugte RS03-Abbilder
führen nicht mehr zu überflüssigen Fehlermeldungen.");

news_item("10.08.2009", "Projektseiten auf SourceForge nicht aktuell", "
Aufgrund von Änderungen in der Funktionalität von SourceForge
können die Inhalte der News, Downloads und des CVS nicht zeitnah
hochgeladen werden. Bitte nutzen Sie stattdessen die entsprechenden
Rubriken auf diesen Seiten (http://dvdisaster.net).");

news_item("08.08.2009", "dvdisaster 0.72.1 veröffentlicht", "
Pablo Almeida hat die Bildschirmtexte ins Portugiesische übersetzt.
Es wurde ein Workaround eingebaut um Win XP vom Einfrieren 
bei bestimmten CD-RW/Laufwerks-Kominationen abzuhalten.<p>
");

news_item("04.07.2009", "dvdisaster 0.72 veröffentlicht", "
Dies ist die erste stabile Version des 0.72er-Zweiges.
Igor Gorbounov hat die russische online-Dokumentation vervollständigt
und es wurden noch einige kleine Fehler aus dem ersten 
Veröffentlichungskandidaten behoben.<p>
");

news_item("14.04.2009", "Nachgelegt: dvdisaster 0.72.rc1 für Mac OS X", "
Die native Version für Mac OS X ist im Bereich der
Benutzeroberfläche <a href=\"download30.php#mac\">noch etwas hakelig</a>,
da die Portierung der GTK+-Bibliothek in einem frühen Entwicklungsstadium ist.
Doch mit der heute aktualisierten dvdisaster-Version läßt sich schon auf dem Mac arbeiten...
");

news_item("11.04.2009", "dvdisaster 0.72.rc1 veröffentlicht", "
Der erste Veröffentlichungskandidat für den stabilen 0.72er Zweig 
steht nun bereit. Unter anderem gibt es Unterstützung für Blu-Ray-Datenträger und
\"Raw\"-Lesen sowie C2-Überprüfungen für CD. 
Auf der <a href=\"download.php\">Herunterladen</a>-Seite
finden Sie mehr Informationen.
");

news_item("08.03.2009", "dvdisaster 0.71.28 veröffentlicht", "
Diese Version behebt Abstürze durch unzulässige Befehle 
(\"illegal Instruction\") auf x86-Maschinen die SSE2 nicht unterstützen.
Sie enthält weitere kleinere Änderungen auf dem Weg zum 
Veröffentlichungskandidaten für den stabilen Zweig.
");

news_item("18.01.2009", "dvdisaster 0.71.27 veröffentlicht", "
Die Dokumentation ist wieder komplett. Einige Crashes und
Inkompatibilitäten mit ungewöhnlichen Laufwerken und Datenträgern wurden behoben.
Diese Version enthält die letzte große interne Änderung, die für die 0.71.x-Serie
geplant war. Wenn alles gut geht wird die nächste Version ein 
Veröffentlichungskandidat für den stabilen Zweig (0.72).
");

if($news_flash == 0) 
   end_page();
?>
