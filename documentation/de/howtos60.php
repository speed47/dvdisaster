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

<h3 class="top">Das große Bild - Ein Vergleich von dvdisaster mit normaler Datensicherung</h3>

dvdisaster archiviert Daten so auf CD/DVD/BD, daß sie auch dann noch wiederherstellbar sind, 
wenn der Datenträger bereits einige Lesefehler enthält. Dabei verbraucht dvdisaster 
weniger Speicherplatz (oder zusätzliche Datenträger)
als dies bei einer vollständigen Sicherungskopie der Fall wäre. 
Die Erklärungen auf diesen Seiten sollen Ihnen helfen, 
die Gemeinsamkeiten und die Unterschiede zwischen dvdisaster und
einer normalen Datensicherung zu verstehen. <p>

Zunächst sei die Funktionsweise einer normalen Datensicherung betrachtet:<p>

<table width="100%">
<tr>
<td class="w65x"><img src="../images/backup1.png" alt="Symbol: Vorhandener Datenträger"></td>
<td class="w65x">Kopie<br><img src="../images/right-arrow.png" alt="Symbol: Pfeil nach rechts"></td>
<td class="w65x"><img src="../images/backup2.png" alt="Symbol: Backup-Datenträger"></td>
<td> &nbsp; </td>
<td>Zu einem vorhandenen Datenträger (1) wird eine identische Sicherungskopie (2)
erstellt.</td>
</tr>

<tr>
<td align="center"><img src="../images/down-arrow.png" alt="Symbol: Pfeil nach unten">&nbsp;&nbsp;</td>
<td></td>
<td align="center"><img src="../images/down-arrow.png" alt="Symbol: Pfeil nach unten">&nbsp;&nbsp;</td>
<td> </td>
</tr>

<tr>
<td class="w65x"><img src="../images/bad-cd1.png" alt="Symbol: Beschädigter Datenträger"></td>
<td class="w65x"> </td>
<td class="w65x"><img src="../images/backup2.png" alt="Symbol: Backup-Datenträger"></td>
<td></td>
<td>Falls anschließend einer der beiden Datenträger beschädigt wird,
verbleibt immer noch eine weitere vollständige Kopie der Daten.</td>
</tr>
</table><p>

Tatsächlich gibt es Fälle, in denen es unerläßlich ist ein zweites Exemplar
einer CD/DVD/BD zu haben: Man kann einen Datenträger verlieren, er kann
im Laufwerk platzen oder durch falsche Behandlung komplett zerstört werden.
Bei einer sachgerechten Handhabung ist ein solcher Totalverlust allerdings
eher unwahrscheinlich.<p>

Viel häufiger passiert es, daß der Datenträger nach einigen 
Jahren allmählich Daten verliert - ein praktisch unabwendbarer Alterungsprozeß.
Wenn man den Datenträger regelmäßig benutzt (oder prüft), 
bemerkt man den Datenverlust typischerweise,
nachdem 5% bis 10% des Datenträgers unlesbar geworden sind.
Umgekehrt heißt dies, daß zu diesem Zeitpunkt noch vielleicht 90% des
Datenträgers lesbar sind. <i>Man braucht also keine vollständige Kopie des 
Datenträgers, sondern nur die Möglichkeit, die fehlenden 10% wiederherzustellen.</i><p>

An dieser Stelle setzt dvdisaster an. Beispiel:<p>

<table width="100%">
<tr>
<td class="w65x"><img src="../images/good-cd.png" alt="Symbol: Guter Datenträger (ohne Lesefehler)"></td>
<td class="w65x">ECC<br><img src="../images/right-arrow.png" alt="Symbol: Pfeil nach rechts"><br>erstellen</td>
<td class="w65x"><img src="../images/ecc.png" alt="Symbol: Eigenständige Fehlerkorrektur-Datei"></td>
<td> &nbsp; </td>
<td>
Anstelle einer kompletten Sicherungskopie erzeugt dvdisaster Fehlerkorrektur-Daten (genannt "ECC"), 
mit denen zum Beispiel bis zu 20% eines gealterten Datenträgers repariert werden können.
20% wurden anstelle der oben genannten 5-10% gewählt, um eine Sicherheitsreserve zu haben.
</td>
</tr>

<tr>
<td align="center"><img src="../images/down-arrow.png" alt="Symbol: Pfeil nach unten">&nbsp;&nbsp;</td>
<td></td>
<td align="center"><img src="../images/down-arrow.png" alt="Symbol: Pfeil nach unten">&nbsp;&nbsp;</td>
<td> </td>
</tr>

<tr>
<td><img src="../images/bad-cd.png" alt="Symbol: Beschädigter Datenträger (teilweise unlesbar)"></td>
<td> </td>
<td><img src="../images/ecc.png" alt="Symbol: Eigenständige Fehlerkorrektur-Datei"></td>
<td> &nbsp; </td>
<td>
Wenn der Datenträger später kaputt geht,
wird sein Inhalt aus den noch lesbaren Teilen des Datenträgers und den Fehlerkorrektur-Daten
rekonstruiert. 
</td>
</tr>

<tr>
<td align="right" class="w65x">80%<img src="../images/rdiag-arrow.png" alt="Symbol: Diagonaler Pfeil nach rechts"></td>
<td> </td>
<td align="left" class="w65x"><img src="../images/ldiag-arrow.png" alt="Symbol: Diagonaler Pfeil nach links">20%</td>
<td> </td>
<td>Dazu muß der Datenträger noch mindestens 80% der Daten liefern, und die verbleibenden 20%
kommen aus den Fehlerkorrektur-Daten.</td>
</tr>

<tr>
<td> </td>
<td> <img src="../images/good-image.png" alt="Symbol: Vollständiges Abbild"></td>
<td> </td>
<td> </td>
<td>Die vollständig wiederherstellten Daten liegen nun als ISO-Abbild vor (den kaputten Datenträger selbst
kann man nicht mehr reparieren).
</td>
</tr>

<tr>
<td> </td>
<td align="center"><img src="../images/down-arrow.png" alt="Symbol: Pfeil nach unten"></td>
<td> </td>
<td> </td>
<td>Davon können Sie mit Hilfe einer beliebigen Brennsoftware wieder einen neuen Datenträger brennen.</td>
</tr>

<tr>
<td> </td>
<td align="center"><img src="../images/good-cd.png" alt="Symbol: Guter Datenträger (ohne Lesefehler)"></td>
<td> </td>
<td> </td>
<td>Schließlich haben Sie einen neuen, wieder vollständigen Datenträger.</td>
</tr>
</table><p>

Die Wiederherstellung von Daten benötigt mehr Schritte als bei einer normalen Datensicherung.
Damit grenzt sich dvdisaster gegenüber einer normalen Datensicherung wie folgt ab:<p>

<table>
<tr valign="top"><td>Vorteile</td>
<td><ul>
<li>Es wird weniger zusätzlicher Speicherplatz benötigt. Wenn Sie Fehlerkorrektur-Daten
mit einer 20%igen-Wiederherstellungsrate erzeugen, brauchen Sie zum Absicherung
von 5 Datenträgern nur einen zusätzlichen Datenträger.</li>
<li>Datenträger altern typischerweise gleich schnell und an den gleichen Stellen
(typischerweise außen). Daher hilft eine 1:1-Sicherungskopie nur bedingt, 
da nach einigen Jahren möglicherweise beide Kopien an den gleichen Stellen 
defekt sind.</li>
</ul></td></tr>
<tr valign="top"><td>Gemeinsamkeit</td>
<td><ul><li>Sicherungskopien und Fehlerkorrektur-Daten müssen erzeugt werden, bevor der Datenträger
kaputt geht. Von einem bereits defekten Datenträger können sie nicht mehr erzeugt werden und die Daten
können nicht wiederhergestellt werden.</li></ul></td></tr>
<tr valign="top"><td>Nachteil</td>
<td><ul><li>Wenn die Wiederherstellungsrate der Fehlerkorrektur-Daten überschritten wird
oder der Datenträger komplett verloren geht, sind die Daten weg!
Insbesondere gilt: Falls die Fehlerkorrektur-Datei 20% reparieren kann, aber vom
Datenträger nur noch 75% lesbar sind, dann sind nicht etwa noch 95% des
Datenträgers wiederherstellbar, sondern gar nichts mehr kann repariert werden!</li></ul></td></tr>
</table> 


Die nächsten drei Seiten enthalten weitere Informationen zu dvdisaster:<p>

<ul>
<li>Die nächste Seite verdeutlicht, wie die <a href="howtos61.php">Fehlerkorrektur funktioniert.</a><p></li>
<li>Am Beispiel von Jane wird gezeigt, wie man dvdisaster <a href="howtos62.php"> in der vorgesehenen Weise</a>
verwendet. Sie erstellt rechtzeitig Fehlerkorrektur-Daten und kann damit alle Daten von 
einem kaputt gegangenen Datenträger retten.<p></li>
<li>Das Beispiel von Joe zeigt hingegen <a href="howtos63.php">wie man es nicht machen sollte</a>. 
Er verwendet keine Fehlerkorrektur-Daten und stellt fest,
daß er defekte Datenträger auch durch mehrmalige Versuche nicht mehr
komplett einlesen kann. Auf diese Weise verliert er Daten von einem 
kaputt gegangenen Datenträger.<p></li>
</ul>

Die Geschichten sind natürlich frei erfunden und Ähnlichkeiten mit real existierenden
Personen oder Situationen sind rein zufällig.


<!-- do not change below -->

<?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>
