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
begin_page();
?>

<!-- Insert actual page content below -->

<h3 class="top">Die Idee der Fehlerkorrektur</h3>

<table width="100%">
<tr valign="top">
<td><img src="../images/bad-cd.png" alt="Symbol: Beschädigter Datenträger (teilweise unlesbar)"></td>
<td> </td>
<td><img src="../images/ecc.png" alt="Symbol: Eigenständige Fehlerkorrektur-Datei"></td>
<td> &nbsp; </td>
<td rowspan="3">
In dem Beispiel auf der letzten Seite haben wir gesehen, daß dvdisaster 
die Daten aus den noch lesbaren Teilen des defekten Datenträgers und den
Fehlerkorrektur-Daten wiederherstellt.<p>

Ein grundlegendes Verständnis der Fehlerkorrektur ist auf jeden Fall hilfreich,
um dvdisaster nutzbringend einzusetzen. Bei der Gelegenheit können wir mit
der ab und zu gemachten Vermutung aufräumen, daß die Fehlerkorrektur-Daten 
nur eine Kopie der letzten 20% des Datenträgers seien. 
So einfach geht es nun auch wieder nicht ;-)
</td>
</tr>

<tr>
<td align="right" class="w65x">80%<img src="../images/rdiag-arrow.png" alt="Symbol: Diagonaler Pfeil nach rechts"></td>
<td> </td>
<td align="left" class="w65x"><img src="../images/ldiag-arrow.png" alt="Symbol: Diagonaler Pfeil nach links">20%</td>
<td> </td>
</tr>

<tr>
<td> </td>
<td> <img src="../images/good-image.png" alt="Symbol: Vollständiges Abbild"></td>
<td> </td>
<td> </td>
</tr>
</table><p>

<b>Beispiel: Annas Schreibtisch-PIN</b><p>

Anna besitzt einen Schreibtisch, dessen Schubladen nur aufgehen, wenn man
die Zahlenkombination "8 6 2 3" eingibt. Da die Schubladen keinen wertvollen Inhalt
enthalten, beschließt Anna die Zahlenkombination auf dem Schreibtisch zu notieren:<p>

<img src="../images/ecc-example1.png" alt="Zahlenkombination 8 6 2 3"><p>

Allerdings befürchtet sie, daß eine der Zahlen durch einen Tintenfleck
unlesbar werden könnte. Daher schreibt sie zusätzlich noch die Summe
der vier Zahlen dazu ("+" und "=" - Zeichen sind nur zur Verdeutlichung 
dazugeschrieben):<p>

<img src="../images/ecc-example2.png" alt="8+6+2+3=19"><p>

Nach einer Weile wird tatsächlich eine Zahl durch einen Tintenfleck
verdeckt:<p>

<img src="../images/ecc-example3.png" alt="8+(unlesbar)+2+3=19"><p>

Das ist aber kein Problem, da Anna sich die fehlende Zahl <i>x</i>
durch Umstellen der Gleichung wieder ausrechnen kann:<p>

8 + x + 2 + 3 = 19, also<p>

x = 19 - 8 - 2 - 3, demnach ist x = 6.<p>

Man kann sich leicht davon überzeugen, daß auf diese Weise jede Zahl aus den
anderen vier Zahlen wieder berechnet werden kann. Das Rechenbeispiel 
verdeutlicht außerdem die grundlegenden Eigenschaften der Fehlerkorrektur:
<p>

<table><tr><td><img src="../images/ecc-example4.png" alt="8+6+2+3 (CD)=19 (ecc)"></td><td>&nbsp;&nbsp;</td>
<td class="valignt">
Zu einer gegebenen Menge von Daten (hier die Zahlenfolge "8 6 2 3") 
kann man zusätzliche Fehlerkorrektur-Daten berechnen (hier die Summe "19"), 
mit denen sich verlorengegangene Daten wieder errechnen lassen.<p>

Bei dvdisaster ist es ganz ähnlich; hier ist die zu schützende Zahlenfolge
nichts anderes als das ISO-Abbild einer CD, DVD oder BD.</td>
</tr></table><p>

Der Begriff der <b>Redundanz</b> läßt sich damit wie folgt erklären:

<ul>
<li>Zu 4 Eingabezahlen wird eine weitere "Fehlerkorrektur-Zahl" berechnet.
1 von 4 (oder 1/4) entspricht einer Redundanz von 25%.</li>
<li> Aus der Fehlerkorrektur-Zahl kann eine andere Zahl wieder berechnet
werden, also höchstens 25% der Daten. Die Kapazität der Fehlerkorrektur entspricht
der Redundanz.</li>
<li> Der zusätzliche Speicherverbrauch durch die Fehlerkorrektur-Daten 
entspricht ebenfalls der gewählten Redundanz, also 25%</li>
</ul>

In dvdisaster wird der Begriff der Redundanz entsprechend verwendet.
Außerdem kann man sich durch Nachrechnen überlegen, daß
<ul>
<li>keine Daten wiederhergestellt werden können, wenn der Datenverlust
größer als die Redundanz ist (für zwei oder mehr fehlende Zahlen kann die
Gleichung nicht eindeutig gelöst werden).</li>
<li>die Fehlerkorrektur-Daten berechnet werden müssen, wenn noch alle
Daten vorhanden sind.</li>
</ul><p>

Das hier gezeigte Summenbeispiel läßt sich allerdings nicht zur einer
Fehlerkorrektur verallgemeinern, die mehr als einen fehlenden Datenwert
korrigieren kann. Dazu braucht man ein mächtigeres Gleichungssystem,
das sich auch für mehrere fehlende Werte (= Variablen) eindeutig lösen
läßt. Der in dvdisaster 
verwendete <a href="http://de.wikipedia.org/wiki/Reed-Solomon-Code">Reed-Solomon-Kode</a> hat diese Eigenschaften; er ist jedoch mit der üblichen Schulmathematik
nicht anschaulich zu erklären. Daher mögen an weiterführenden
Informationen interessierte Leser die entsprechende Literatur aus der
Kodierungstheorie hinzuziehen.

<!-- do not change below -->

<?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>
