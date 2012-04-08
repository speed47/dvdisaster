<?php
# dvdisaster: German homepage translation
# Copyright (C) 2004-2012 Carsten Gnörlich
#
# UTF-8 trigger: äöüß 
#
# Include our PHP sub routines, then call begin_page()
# to start the HTML page, insert the header, 
# navigation and news if appropriate.

require("../include/dvdisaster.php");
require("../include/screenshot.php");
begin_page();
?>

<!-- Insert actual page content below -->

<h3 class="top">Fehler berichten</h3>
Wie jede komplexe Software kann dvdisaster Programmierfehler enthalten oder
mit bestimmten Laufwerken und/oder Softwareprodukten nicht zusammenarbeiten.
Geben Sie uns daher Rückmeldungen über Schwierigkeiten, die Sie beim Einsatz
des Programms oder auch der Dokumentation beobachten. Auf diese Weise helfen Sie
mit, zukünftige Versionen von dvdisaster zu verbessern.<p>

Um sicherzustellen, daß wir die benötigten Informationen bekommen, haben wir
eine Liste mit Anhaltspunkten für Fehlerberichte zusammengestellt:

<h4>Bitte vergewissern Sie sich zuerst, daß wirklich ein Fehler vorliegt:</h4>

<ul>
<li>Verwenden Sie die aktuellste Originalversion von dvdisaster, die Sie von
<a href="http://dvdisaster.net/de/download.php">unseren Projektseiten
   über SourceForge</a> herunterladen können. 
dvdisaster-Versionen, die von jemand anders
angeboten werden können Funktionen und Fehler enthalten, die es in der
Originalversion nicht gibt (und wir können dessen Fehler nicht beheben).
</li>
<li>Schauen Sie im <a href="qa20.php">Fragen und Antworten</a>-Bereich nach, ob das
Problem dort bereits behandelt wird.</li>
<li>Bitte beachten Sie daß dvdisaster nur mit (wieder-)beschreibbaren Datenträgern
arbeitet. Es ist <b>kein Fehler</b>, daß dvdisaster <b>DVD-ROM</b> und <b>BD-ROM</b>
nicht akzeptiert. Ebenso werden CD-Audio, VCD, SVCD 
und CDs mit Mehrfachsitzungen ("multisession") sowie alle HD-DVD-Formate 
nicht unterstützt. Siehe auch die
<a href="qa10.php#media">vollständige Liste der unterstützten Formate</a>.</li>
<li>dvdisaster funktioniert nur mit echten Laufwerken. Nicht unterstützt sind
Netzwerklaufwerke, Software-Laufwerke (z.B. die Alkohol-Laufwerke :-), und
Laufwerke in virtuellen Maschinen.
</li>
</ul>

<h4>Einsenden von Fehlerberichten:</h4>

Bitte schicken Sie Ihre Fehlerberichte pe E-Mail an
<img src="../images/email.png" alt="als Grafik dargestellte E-Mail-Adresse" class="valigntt">. Ihr Bericht sollte die folgenden
Informationen enthalten:<p>

<ul>
<li>Informationen über das verwendete Betriebssystem und die Versionsnummer von dvdisaster;</li>
<li>die Typbezeichnung des Laufwerks und des Datenträgers, mit dem der Fehler auftritt;</li>
<li>eine kurze Beschreibung des Fehlers, den Sie beobachtet haben;</li>
<li>ein Bildschirmfoto der Fehlermeldungen und/oder Ausgaben, die weitere Informationen
   über das Problem liefern;</li>
<li>falls das Problem nur auf bestimmten Laufwerken oder Computern auftritt, eine Beschreibung
   der Unterschiede zwischen den funktionierenden und nicht funktionierenden Konfigurationen;</li>
   <li>eine Protokolldatei (s.u.) sofern das Problem mit dem Laufwerk oder Datenträger
zusammenzuhängen scheint.</li>
</ul>

<a name="log"></a>
<?php begin_screen_shot("Protokolldatei erzeugen", "activate-logfile.png"); ?>

<b>Erzeugen der Protokolldatei:</b> 
Wenn Sie Kompatibilitätsprobleme mit Ihrem Laufwerk oder den Datenträgern vermuten,
   aktivieren Sie bitte die Erstellung einer Protokolldatei. Füllen Sie dazu 
den Einstellungs-Dialog wie im Bildschirmfoto gezeigt aus. 
Führen Sie dann eine "Lesen"- oder "Überprüfen"-Aktion
durch und fügen Sie die erzeugte Protokolldatei Ihrer Fehlerbeschreibung hinzu.
<?php end_screen_shot(); ?>

Vielen Dank für Ihre Mithilfe!

<!-- do not change below -->

<?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>
