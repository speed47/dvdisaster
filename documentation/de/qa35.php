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

<h3 class="top">Das angepaßte Lese-Verfahren</h3>
<p>

dvdisaster enthält zwei verschiedene Leseverfahren.<p>

<b>Anwendungen für das angepaßte Leseverfahren:</b><p>
<ul>
<li> Inhalt von beschädigten Datenträgern <a href="howtos42.php">rekonstruieren</a>
</li>
</ul>

<b>Anwendungen für das <a href="qa34.php">lineare Leseverfahren:</a></b><p>
<ul>
<li>Abbilder von unbeschädigten Datenträgern zum Erzeugen einer Fehlerkorrekturdatei <a href="howtos23.php">einlesen</a></li>
<li><a href="howtos12.php">Lesegeschwindigkeitskurve</a> zum Prüfen des Datenträger-Zustandes ermitteln</li>
</ul>

<pre> </pre>

<b>Eigenschaften des angepaßten Verfahrens.</b><p>

Das angepaßte Verfahren setzt eine "Teile-und-Herrsche" ("divide-and-conquer") - Strategie ein,
um möglichst schnell die noch lesbaren Stellen eines beschädigten Datenträgers zu ermitteln 
und auszulesen.
Die Strategie geht auf einen Artikel von Harald Bögeholz im c't-Magazin 16/2005 
zurück, wo sie zusammen mit dem Programm <i>h2cdimage</i> veröffentlicht wurde:

<ol>
<li> 
  Zu Anfang wird der Datenträger als ein einziger noch nicht gelesener Bereich betrachtet. 
Das Lesen beginnt mit Sektor Null.<p>
</li>
<li>
Der Lesevorgang wird solange linear fortgesetzt,
bis entweder das Ende des momentanen Bereiches erreicht ist oder ein Lesefehler auftritt.<p>
</li>
<li>
Der Lesevorgang wird entweder beendet, wenn (3a) genügend Sektoren für eine Fehlerkorrektur 
gelesen wurden oder (3b) keine unlesbaren Bereiche oberhalb einer bestimmten Größe 
mehr vorhanden sind.
<p>
</li>
<li>Anderenfalls wird der größte noch nicht gelesene Bereich auf dem Datenträger bestimmt
und in der Mitte aufgeteilt. Der Lesevorgang wird in der Mitte wie in Schritt 2 fortgesetzt.
Die erste Hälfte des aufgeteilten Bereiches verbleibt hingegen als noch nicht gelesener Bereich
für einen späteren Durchlauf.<p>
</li>
</ol>

<?php begin_screen_shot("Angepaßtes Lesen in Aktion","adaptive-progress.png"); ?>
Das Abbruchkriterium (3a) ist besonders wirkungsvoll: Es beendet das Einlesen sofort,
wenn die absolut notwendigen Sektoren zur Wiederherstellung des Abbildes mit Hilfe 
der Fehlerkorrektur gelesen worden sind.
Dies kann die Bearbeitungszeit  gegenüber einem vollständigen Einlese-Versuch um bis zu 90% verkürzen,
erfordert aber natürlich, daß man die zugehörige Fehlerkorrektur-Datei zur Hand hat.
<?php end_screen_shot(); ?><p>

<pre> </pre>

<a name="configure"></a>
<b>Einstellmöglichkeiten</b><p>

<b>Fehlerkorrekturdatei.</b> Angepaßtes Lesen funktioniert am besten, wenn die zum Abbild gehörenden
Fehlerkorrektur-Daten vorhanden sind. 
Das setzt natürlich voraus, daß man diese Daten
zu einem Zeitpunkt <a href="howtos21.php">erzeugt</a> hat, 
als der Datenträger noch vollständig lesbar war.

Um eine Fehlerkorrektur-Datei zu nutzen, muß sie vor Beginn des Lesens 
<a href="howtos42.php#select_eccfile">ausgewählt</a> werden.<p>

<b>Einschränkung des adaptiven Lesebereiches.</b> Der Einlesevorgang 
kann auf einen Teil des Datenträgers <a href="howtos11.php#image">eingeschränkt</a> werden. 

Bei der Verwendung von Fehlerkorrektur-Daten ist das Einschränken 
des Lesebereichs nicht sinnvoll, da es gegebenenfalls das Einlesen von Sektoren
verhindert, die zur Fehlerkorrektur benötigt werden.
Ohne Fehlerkorrektur-Daten kann es hingegen bei mehrfachen Einlese-Versuchen 
von beschädigten Datenträgern hilfreich sein.<p>

<b>Lesen vorzeitig beenden.</b>Wenn keine Fehlerkorrektur-Daten vorhanden sind, wird der Lesevorgang beendet, sobald keine
unlesbaren Bereiche oberhalb 
<a href="howtos41.php#reading_attempts">einer bestimmten Größe</a> mehr vorhanden sind.<p>

Der Wert zum Beenden sollte nicht kleiner als 128 eingestellt werden.
Anderenfalls werden in der Schlußphase des Einlesens sehr viele Neupositionierungen des
Laserschlittens im Laufwerk durchgeführt. Darunter leidet sowohl die Lebensdauer als auch die
Lesefähigkeit des Laufwerks. Günstiger ist es typischerweise, früher mit den adaptiven
Lesen aufzuhören und die letzten Sektoren mit dem <a href="qa34.php">linearen Leseverfahren</a>
zu vervollständigen.

<!-- do not change below -->

<?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>
