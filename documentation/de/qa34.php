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

<h3 class="top"><b>Das lineare Lese-Verfahren</b></h3>
<p>

In dvdisaster sind zwei verschiedene Leseverfahren enthalten.<p>

<b>Anwendungen für das lineare Leseverfahren:</b><p>
<ul>
<li>Abbilder von unbeschädigten Datenträgern zum Erzeugen einer Fehlerkorrekturdatei <a href="howtos23.php">einlesen</a>.</li>
<li><a href="howtos12.php">Lesegeschwindigkeitskurve</a> zum Prüfen des Datenträger-Zustandes ermitteln.</li>
</ul>

<b>Anwendungen für das <a href="qa35.php">angepaßte Leseverfahren:</a></b><p>
<ul>
<li> Inhalt von beschädigten Datenträgern <a href="howtos42.php">rekonstruieren</a>
</li>
</ul>

<pre> </pre>

<b>Eigenschaften des linearen Verfahrens.</b><p>

<?php begin_screen_shot("Schwache CD","weak-cd.png"); ?>
Optische Datenträger 
sind in Sektoren aufgeteilt, die mit Null beginnend numeriert sind 
und jeweils 2048 Bytes an Daten enthalten.<p>

Das lineare Leseverfahren liest den Datenträger vom Anfang (Sektor 0)
bis zum Ende (letzter Sektor) ein. Die Lesegeschwindigkeit wird 
graphisch dargestellt, um die <a href="#quality">Qualität des Datenträgers</a>
abschätzen zu können:
<?php end_screen_shot(); ?><p>

<pre> </pre>


<a name="configure"></a>
<b>Einstellmöglichkeiten.</b><p>

<b>Anzahl der zu überspringenden Sektoren nach einem Lesefehler.</b>
Leseversuche von defekten Sektoren kosten viel Zeit und bewirken in ungünstigen Fällen
einen erhöhten Verschleiß des Laufwerks. Lesefehler treten aber typischerweise nicht einzeln,
sondern über längere Bereiche auf. Daher gibt es eine 
<a href="howtos11.php#read_attempts"> Einstellmöglichkeit</a> nach einem Lesefehler
eine Anzahl nachfolgender Sektoren zu überspringen. Diese Sektoren werden ohne weitere Leseversuche 
als defekt angenommen. Dies hat die folgenden Auswirkungen:<p>


<ul>
<li>Das Überspringen einer großen Anzahl von Sektoren (z.B <b>1024</b>) ergibt eine schnelle
Übersicht über die Beschädigung des Datenträgers.<br>
Es liefert aber in der Regel nicht genügend Daten für eine erfolgreiche Fehlerkorrektur.<p></li> 
<li>Kleinere Werte von <b>16, 32 oder 64</b> sind ein guter Kompromiß zwischen verringerter 
Bearbeitungszeit und Wiederherstellbarkeit des Datenträger-Abbildes.<p></li>
</ul>

Auf DVD-Datenträgern erstrecken sich Lesefehler aus technischen Gründen meist über
mindestens 16 Sektoren. Daher lohnt es sich für DVD nicht, 
einen Wert kleiner als 16 einzustellen.
<p>

<a name="range"></a>
<b>Einschränkung des Lesebereiches.</b>
Der Einlesevorgang kann in der Reiterkarte "Abbild"
<a href="howtos11.php#image"> auf einen Teil des Datenträgers eingeschränkt</a> werden.
Dies ist bei mehrfachen Einlese-Versuchen von beschädigten Datenträgern hilfreich.

<pre> </pre>

<a name="quality"></a>
<b>Abschätzung der Datenträger-Qualität.</b><p>

<a name="error"></a>
<b>Die Geschwindigkeitskurve.</b>
Viele Laufwerke verringern ihre Lesegeschwindigkeit in Bereichen
des Datenträgers, die sich in einem schlechten Zustand befinden:
<ul>
<li>Einbrüche in der Lesegeschwindigkeit können ein Warnzeichen für ein
baldiges Versagen des Datenträgers darstellen.</li>
<li>
Es gibt aber auch Laufwerke, die "bis zum bitteren Ende" mit voller
Geschwindigkeit lesen. Man kann sich also nicht darauf verlassen,
daß sich ein Versagen des Datenträgers durch Unterbrechungen in der 
Geschwindigkeitskurve ankündigt.
</li>
</ul><p>

Die Lesekurve ist bei der
<a href="howtos10.php"> "Prüfen"</a>-Funktion am aussagekräftigsten.
In der 
<a href="howtos23.php"> "Lesen"</a>-Betriebsart
werden die gelesenen Daten gleichzeitig auf der
Festplatte abgelegt, was je nach Betriebssystem und verwendeter Hardware kleine
Verzögerungen und damit Unregelmäßigkeiten in der Lesekurve bewirkt.<p>

<b>Lesefehler.</b>
Lesefehler werden <a href="howtos13.php#defective">rot in der Spirale markiert</a> bzw. in der Kommandozeile ausgegeben.
An diesen Stellen konnte der Datenträger im momentanen Durchlauf nicht vollständig gelesen werden:
<ul>
<li>Es ist damit wahrscheinlich, daß der Datenträger defekt ist. </li>
<li>Das Abbild sollte jetzt schnellstmöglich
<a href="howtos40.php"> rekonstruiert</a> und auf einen neuen Datenträger geschrieben werden.</li>
</ul>

<!-- do not change below -->

<?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>
