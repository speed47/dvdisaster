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
require("../include/footnote.php");
begin_page();
$show_all=$_GET["showall"];
?>

<!-- Insert actual page content below -->

<h3 class="top">dvdisaster herunterladen</h3>

<!--
<table width="100%">
<tr>
<td>dvdisaster unterstützt die Betriebssysteme Darwin/Mac OS X, FreeBSD, GNU/Linux, NetBSD und Windows in den <a href="download10.php">aktuellen Versionen</a>.
Es wird Ihnen 
als <a href="http://www.germany.fsfeurope.org/documents/freesoftware.de.html">freie Software</a> 
unter der <a href="http://www.gnu.org/licenses/gpl-3.0.txt">GNU General Public License v3</a>  zur 
Verfügung gestellt.</td>
<td class="w127x" valign="top"><img src="../images/gplv3-127x51.png" alt="GPLv3-Logo" width="127"></td>
</tr>
</table>
-->
dvdisaster unterstützt die Betriebssysteme Darwin/Mac OS X, FreeBSD, GNU/Linux, NetBSD und Windows in den <a href="download10.php">aktuellen Versionen</a>.
Es wird Ihnen 
als <a href="http://www.germany.fsfeurope.org/documents/freesoftware.de.html">freie Software</a> 
unter der <a href="http://www.gnu.org/licenses/gpl-2.0.txt">GNU General Public License v2</a>  zur 
Verfügung gestellt.

<p>

Laden Sie sich bitte entweder den Quellkode oder eine Binärversion 
aus der folgenden Liste herunter. Die Pakete können mit
einer <a href="download20.php">digitalen Unterschrift</a> auf 
ihren Ursprungszustand überprüft werden.<p>

<ul>
<li>Die Quellkode-Version enthält eine Datei <tt>INSTALL</tt> mit weiteren
Informationen zum Übersetzen des Programmes.</li>
<li>Für Mac OS X laden Sie bitte das ZIP-Archiv herunter und packen es an einer
beliebigen Stelle aus. Bitte beachten Sie die 
<a href="download30.php#mac">speziellen Hinweise für Mac OS X</a>.</li>
<li>Um die Binärversion <a href="download30.php#win">für Windows zu installieren</a>, 
rufen Sie das Programm nach dem Herunterladen auf und folgen dem Dialog.</li>
</ul> 

<?php
if(!strcmp($have_experimental, "yes"))
{ ?>
<b>Alpha-/Entwickler-Versionen</b> - neu und experimentell für erfahrene Benutzer!<p> 

Sie sind eingeladen, die nächste dvdisaster-Version auszuprobieren, 
aber beachten Sie bitte, daß diese Version noch Fehler und 
Kompatibilitätsprobleme enthalten kann. Die neueste experimentelle Version 
ist <a href="download40.php"><?php echo $cooked_version ?></a>. 
<p>

<?php
}
?>
<b>Stabile Version</b> - zum Einstieg empfohlen<p> 
<a name="download"></a>

<table class="download" cellpadding="0" cellspacing="5">
<tr><td><b>dvdisaster-0.72</b></td><td align="right">07-Apr-2012</td></tr>
<tr><td colspan="2" class="hsep"></td></tr>
<tr><td colspan="2">
  <table>
    <tr><td align="right">&nbsp;&nbsp;Quellkode für alle Betriebssysteme:&nbsp;</td>
        <td><a href="http://dvdisaster.net/downloads/dvdisaster-0.72.4.tar.bz2">dvdisaster-0.72.4.tar.bz2</a></td></tr>
    <tr><td align="right">Digitale Unterschrift:&nbsp;</td>
        <td><a href="http://dvdisaster.net/downloads/dvdisaster-0.72.4.tar.bz2.gpg">dvdisaster-0.72.4.tar.bz2.gpg</a></td></tr>

<?php
if($mode == "www")
    echo "<tr><td align=\"right\">MD5-Prüfsumme:&nbsp;</td><td>4eb09c1aa3cdbc1dafdb075148fb471d</td></tr>";
?>
    <tr><td><pre> </pre></td><td></td></tr>


    <tr><td align="right">&nbsp;&nbsp;Quellkode für alle Betriebssysteme:&nbsp;</td>
        <td><a href="http://dvdisaster.net/downloads/dvdisaster-0.72.3.tar.bz2">dvdisaster-0.72.3.tar.bz2</a></td></tr>
    <tr><td align="right">Digitale Unterschrift:&nbsp;</td>
        <td><a href="http://dvdisaster.net/downloads/dvdisaster-0.72.3.tar.bz2.gpg">dvdisaster-0.72.3.tar.bz2.gpg</a></td></tr>

<?php
if($mode == "www")
    echo "<tr><td align=\"right\">MD5-Prüfsumme:&nbsp;</td><td>4eb09c1aa3cdbc1dafdb075148fb471d</td></tr>";
?>
    <tr><td colspan="2" class="esep"></td></tr>

    <tr><td align="right">Binärversion für Mac OS X 10.6 / x86:&nbsp;</td>
        <td><a href="http://dvdisaster.net/downloads/dvdisaster-0.72.3.app.zip">dvdisaster-0.72.3.app.zip</a>&nbsp;--&nbsp;bitte erst den <a href="download30.php#mac">Hinweis</a> lesen</td></tr>
    <tr><td align="right">Digitale Unterschrift:&nbsp;</td>
        <td><a href="http://dvdisaster.net/downloads/dvdisaster-0.72.3.app.zip.gpg">dvdisaster-0.72.3.app.zip.gpg</a></td></tr>

<?php
if($mode == "www")
    echo "<tr><td align=\"right\">MD5-Prüfsumme:&nbsp;</td><td>38389bbbeb0d259a3f0a8df89b363f72</td></tr>";
?>
    <tr><td colspan="2" class="esep"></td></tr>

    <tr><td align="right">Binärversion für Windows:&nbsp;</td>
        <td><a href="http://dvdisaster.net/downloads/dvdisaster-0.72.3-setup.exe">dvdisaster-0.72.3-setup.exe</a></td></tr>
    <tr><td align="right">Digitale Unterschrift:&nbsp;</td>
        <td><a href="http://dvdisaster.net/downloads/dvdisaster-0.72.3-setup.exe.gpg">dvdisaster-0.72.3-setup.exe.gpg</a></td></tr>
<?php
if($mode == "www")
    echo "<tr><td align=\"right\">MD5-Prüfsumme:&nbsp;</td><td>b6861ba1e8de6d91a2da5342a14870e0</td></tr>";
?>
    <tr><td colspan="2"> </td></tr>

<?php
  if($show_all == 0) {
?>
    <tr><td colspan="2"><a href="download.php?showall=1#download">Ältere Veröffentlichungen des 0.72er-Versionszweiges anzeigen</a></td></tr>
<?php
  }
  else {
?> 
   <tr><td colspan="2"><a href="download.php?showall=0#download">Ältere Veröffentlichungen des 0.72er-Versionszweiges verbergen</a></td></tr>

   <tr><td colspan="2"> </td></tr>
   <tr><td></td><td>Version 0.72.2</td></tr>
    <tr><td align="right">&nbsp;&nbsp;Quellkode für alle Betriebssysteme:&nbsp;</td>
        <td><a href="http://dvdisaster.net/downloads/dvdisaster-0.72.2.tar.bz2">dvdisaster-0.72.2.tar.bz2</a></td></tr>
    <tr><td align="right">Digitale Unterschrift:&nbsp;</td>
        <td><a href="http://dvdisaster.net/downloads/dvdisaster-0.72.2.tar.bz2.gpg">dvdisaster-0.72.2.tar.bz2.gpg</a></td></tr>

    <tr><td align="right">MD5-Prüfsumme:&nbsp;</td><td>312bceef3bf9c0754cf633ed3b12eb71</td></tr>
    <tr><td colspan="2" class="esep"></td></tr>

    <tr><td align="right">Binärversion für Mac OS X 10.5 / x86:&nbsp;</td>
        <td><a href="http://dvdisaster.net/downloads/dvdisaster-0.72.2.app.zip">dvdisaster-0.72.2.app.zip</a>&nbsp;--&nbsp;bitte erst den <a href="download30.php#mac">Hinweis</a> lesen</td></tr>
    <tr><td align="right">Digitale Unterschrift:&nbsp;</td>
        <td><a href="http://dvdisaster.net/downloads/dvdisaster-0.72.2.app.zip.gpg">dvdisaster-0.72.2.app.zip.gpg</a></td></tr>

    <tr><td align="right">MD5-Prüfsumme:&nbsp;</td><td>52243c1fafb9d2e496b6eb318c3e534f</td></tr>
    <tr><td colspan="2" class="esep"></td></tr>

    <tr><td align="right">Binärversion für Windows:&nbsp;</td>
        <td><a href="http://dvdisaster.net/downloads/dvdisaster-0.72.2-setup.exe">dvdisaster-0.72.2-setup.exe</a></td></tr>
    <tr><td align="right">Digitale Unterschrift:&nbsp;</td>
        <td><a href="http://dvdisaster.net/downloads/dvdisaster-0.72.2-setup.exe.gpg">dvdisaster-0.72.2-setup.exe.gpg</a></td></tr>
    <tr><td align="right">MD5-Prüfsumme:&nbsp;</td><td>f80258d27354061fd9e28850ec4701a6</td></tr>

   <tr><td colspan="2"> </td></tr>
   <tr><td></td><td>Version 0.72.1</td></tr>
    <tr><td align="right">&nbsp;&nbsp;Quellkode für alle Betriebssysteme:&nbsp;</td>
        <td><a href="http://dvdisaster.net/downloads/dvdisaster-0.72.1.tar.bz2">dvdisaster-0.72.1.tar.bz2</a></td></tr>
    <tr><td align="right">Digitale Unterschrift:&nbsp;</td>
        <td><a href="http://dvdisaster.net/downloads/dvdisaster-0.72.1.tar.bz2.gpg">dvdisaster-0.72.1.tar.bz2.gpg</a></td></tr>
    <tr><td align="right">MD5-Prüfsumme:&nbsp;</td>
        <td>4da96566bc003be93d9dfb0109b4aa1d</td></tr>
    <tr><td colspan="2" class="esep"></td></tr>

    <tr><td align="right">Binärversion für Mac OS X 10.5 / x86:&nbsp;</td>
        <td><a href="http://dvdisaster.net/downloads/dvdisaster-0.72.1.app.zip">dvdisaster-0.72.1.app.zip</a>&nbsp;--&nbsp;bitte erst den <a href="download30.php#mac">Hinweis</a> lesen</td></tr>
    <tr><td align="right">Digitale Unterschrift:&nbsp;</td>
        <td><a href="http://dvdisaster.net/downloads/dvdisaster-0.72.1.app.zip.gpg">dvdisaster-0.72.1.app.zip.gpg</a></td></tr>
    <tr><td align="right">MD5-Prüfsumme:&nbsp;</td>
        <td>924b5677f69473b6b87991e01779a541</td></tr>
    <tr><td colspan="2" class="esep"></td></tr>

    <tr><td align="right">Binärversion für Windows:&nbsp;</td>
        <td><a href="http://dvdisaster.net/downloads/dvdisaster-0.72.1-setup.exe">dvdisaster-0.72.1-setup.exe</a></td></tr>
    <tr><td align="right">Digitale Unterschrift:&nbsp;</td>
        <td><a href="http://dvdisaster.net/downloads/dvdisaster-0.72.1-setup.exe.gpg">dvdisaster-0.72.1-setup.exe.gpg</a></td></tr>
    <tr><td align="right">MD5-Prüfsumme:&nbsp;</td>
        <td>34d062ddebe1a648e808d29ca4e9879f</td></tr>

   <tr><td colspan="2"> </td></tr>
   <tr><td></td><td>Version 0.72</td></tr>
    <tr><td align="right">&nbsp;&nbsp;Quellkode für alle Betriebssysteme:&nbsp;</td>
        <td><a href="http://dvdisaster.net/downloads/dvdisaster-0.72.tar.bz2">dvdisaster-0.72.tar.bz2</a></td></tr>
    <tr><td align="right">Digitale Unterschrift:&nbsp;</td>
        <td><a href="http://dvdisaster.net/downloads/dvdisaster-0.72.tar.bz2.gpg">dvdisaster-0.72.tar.bz2.gpg</a></td></tr>
    <tr><td align="right">MD5-Prüfsumme:&nbsp;</td>
        <td>efa35607d91412a7ff185722f270fb8a</td></tr>
    <tr><td colspan="2" class="esep"></td></tr>

    <tr><td align="right">Binärversion für Mac OS X 10.5 / x86:&nbsp;</td>
        <td><a href="http://dvdisaster.net/downloads/dvdisaster-0.72.app.zip">dvdisaster-0.72.app.zip</a>&nbsp;--&nbsp;bitte erst den <a href="download30.php#mac">Hinweis</a> lesen</td></tr>
    <tr><td align="right">Digitale Unterschrift:&nbsp;</td>
        <td><a href="http://dvdisaster.net/downloads/dvdisaster-0.72.app.zip.gpg">dvdisaster-0.72.app.zip.gpg</a></td></tr>
    <tr><td align="right">MD5-Prüfsumme:&nbsp;</td>
        <td>1f28385b2b6d64b664fd416eb4c85e80</td></tr>
    <tr><td colspan="2" class="esep"></td></tr>

    <tr><td align="right">Binärversion für Windows:&nbsp;</td>
        <td><a href="http://dvdisaster.net/downloads/dvdisaster-0.72-setup.exe">dvdisaster-0.72-setup.exe</a></td></tr>
    <tr><td align="right">Digitale Unterschrift:&nbsp;</td>
        <td><a href="http://dvdisaster.net/downloads/dvdisaster-0.72-setup.exe.gpg">dvdisaster-0.72-setup.exe.gpg</a></td></tr>
    <tr><td align="right">MD5-Prüfsumme:&nbsp;</td>
        <td>cc8eb2af384917db8d6d983e1d4aac69</td></tr>
<?php
  }
?>
  </table>
</td></tr>
<tr><td colspan="2" class="hsep"></td></tr>
<tr><td colspan="2">
Grundlegende Neuerungen in dieser Version:<p>
<ul>
<li>Unterstützung für <a href="qa10.php#media">Blu-Ray-Datenträger</a></li>
<li>"Raw"-Lesen und C2-Überprüfungen für CD-Datenträger</li>
<li>Einstellbare Anzahl der Leseversuche</li>
<li>Erstes "natives" Mac OS X-Paket </li>
<li>NetBSD-Port von Sergey Svishchev</li>
<li>Verbesserte Typerkennung für eingelegte Datenträger</li>
<li>Informationsfenster für eingelegten Datenträger</li>
<li>Überarbeiteter und erweiterter Dialog für Programmeinstellungen</li>
<li>Neu gestaltete und erweiterte Dokumentation</li>
<li>Russische Übersetzungen von Igor Gorbounov</li>
<li>... und unzählige weitere kleine Sachen.</li>
</ul>

<b>Patches</b> (kleine Änderungen nach Version 0.72; die obigen Dateien wurden erneuert):<p>

<b>0.72 pl4</b> Anpassungen an aktuelle Versionen und Programmbibiliotheken
von GNU/Linux, FreeBSD und NetBSD. (07-Apr-2012)<p>

<b>0.72 pl3</b> In der "Vergleichen"-Funktion wurde ein Fehler
behoben, der bei der Verarbeitung von RS01-Fehlerkorrektur-Dateien
auftrat, die größer als 2GB waren. Danke an Volodymyr Bychkoviak 
für die Problembeschreibung und die Korrektur. (05-Okt-2011)<p>

<b>0.72 pl2</b> 
Diese Version führt einen Workaround ein
um zu verhindern daß parallele SCSI-Adapter unter Linux
nicht mehr reagieren.
Die Aufwärtskompatibilität mit Version 0.79.x wurde verbessert.<br>
Die Windows- und Mac OS X-Versionen werden nun mit der aktuellen
Entwicklungsumgebung von dvdisaster 0.79.x erzeugt und mit neueren
Versionen der GTK+-Benutzeroberflächenbibliothek ausgeliefert. 
Für diesen Update wurden noch kleinere Änderungen an einigen Skripten
 erforderlich so daß sich die Prüfsumme des Quellkode-Pakets geändert hat 
(das Paket vom 31.10. hatte die md5-Prüfsumme 
86110e212aa1bf336a52ba89d3daa93d und kann selbstverständlich für
Linux, FreeBSD und NetBSD weiter verwendet
werden). (07-11-2010)<p>

<b>0.72 pl1</b> Pablo Almeida hat die Bildschirmtexte ins Portugiesische übersetzt.
Es wurde eine Umgehungslösung eingebaut um das Einfrieren von Windows XP bei bestimmten Kombinationen
von CD-RW-Rohlingen und Laufwerken zu verhindern. (08-Aug-2009)<br>
<i>Hinweis: Die Umgehungslösung hat sich als nicht immer wirksam herausgestellt. Eine bessere Lösung ist in <a href="download40.php">Version 0.79.x</a> enthalten; diese läßt sich leider nicht so einfach in die stabile Version zurückportieren.</i> (06-Feb-2010)<p>

<b>0.72</b> Dies ist die erste stabile Version des 0.72er-Zweiges.
Igor Gorbounov hat die russische online-Dokumentation vervollständigt
und es wurden noch einige kleine Fehler aus dem ersten 
Veröffentlichungskandidaten behoben.<p>
Unter neueren Windows-Versionen wird bei bestimmten Spracheinstellungen
nicht die gewünschte Bildschirmsprache dargestellt. Die Behebung
dieses Problems ist ziemlich komplex und wird erst mit Version 0.73
erfolgen. (04-Jul-2009)<p>
 
<b>0.72-rc1</b> Erster Kandidat für die stabile Version. (11-Apr-2009)
</td></tr></table><p>

Falls kein Herunterladen über die obigen Links möglich ist,
versuchen Sie bitte dvdisaster über
<a href="http://sourceforge.net/projects/dvdisaster/files">SourceForge</a> 
zu bekommen.

<pre> </pre>

<b>Vorangegangene Version</b> - eine Aktualisierung auf Version 0.72 wird empfohlen.<p> 

<table class="download" cellpadding="0" cellspacing="5">
<tr><td><b>dvdisaster-0.70</b></td><td align="right">04-Mär-2008</td></tr>
<tr><td colspan="2" class="hsep"></td></tr>
<tr><td colspan="2">
  <table>
    <tr><td align="right">&nbsp;&nbsp;Quellkode für alle Betriebssysteme:&nbsp;</td>
        <td><a href="http://dvdisaster.net/downloads/dvdisaster-0.70.6.tar.bz2">dvdisaster-0.70.6.tar.bz2</a></td></tr>
    <tr><td align="right">Digitale Unterschrift:&nbsp;</td>
        <td><a href="http://dvdisaster.net/downloads/dvdisaster-0.70.6.tar.bz2.gpg">dvdisaster-0.70.6.tar.bz2.gpg</a></td></tr>
    <tr><td align="right">MD5-Prüfsumme:&nbsp;</td>
        <td>c6d2215d7dd582475b19593dfa4fbdc2</td></tr>
    <tr><td colspan="2" class="esep"></td></tr>

    <tr><td align="right">Binärversion für Windows:&nbsp;</td>
        <td><a href="http://dvdisaster.net/downloads/dvdisaster-0.70.6-setup.exe">dvdisaster-0.70.6-setup.exe</a></td></tr>
    <tr><td align="right">Digitale Unterschrift:&nbsp;</td>
        <td><a href="http://dvdisaster.net/downloads/dvdisaster-0.70.6-setup.exe.gpg">dvdisaster-0.70.6-setup.exe.gpg</a></td></tr>
    <tr><td align="right">MD5-Prüfsumme:&nbsp;</td>
        <td>82f74bebd08ab7ae783ddc5dd0bba731</td></tr>
  </table>
</td></tr>
<tr><td colspan="2" class="hsep"></td></tr>
<tr><td colspan="2">
Das RS02-Fehlerkorrektur-Verfahren
wird in der graphischen Benutzeroberfläche vollständig unterstützt.
Damit erzeugte Abbilder können nun auch
mit dem angepaßten Leseverfahren verarbeitet werden.<p>

Julian Einwag hat damit begonnen, dvdisaster 
für Mac OS X / Darwin anzupassen.<p>

Daniel Nylander hat die Bildschirmtexte ins Schwedische übersetzt.<p>

<b>Patches</b> (kleine Änderungen nach Version 0.70; die obigen Dateien wurden erneuert):<p>

<b>pl6</b> Die Unterstützung von Umlauten in Dateinamen wurde
wieder rückgängig gemacht,
da sie zur fehlerhaften Bearbeitung von Dateien &gt; 2GB unter Windows führte. 
Eine korrekte Behandlung von Dateinamen mit Sonderzeichen
 wird jetzt erst in der
experimentellen Version 0.71.25 erprobt. <i>(04-Mär-2008)</i><p>

<b>pl5</b> Behebt ein Problem mit neueren Linux-Kernen, das unter bestimmten Bedingungen zum
Einfrieren des Systems führen kann. Die Behandlung von Umlauten in Dateinamen wurde
verbessert.
Zurückportierung einiger weiterer
Verbesserungen aus 0.71.24. <i>(24-Feb-2008)</i>.<p>

<b>pl4</b> ist besser kompatibel zu zweischichtigen DVDs 
(DVD-R DL und DVD+R DL).<br> 
Einige kleinere Fehler wurden behoben. <i>(20-Jan-2007)</i>.<p>

<b>pl3</b> behebt einen Fehler bei der Erkennung von nicht unterstützten CDs,
der unter Umständen einen blauen Bildschirm unter Windows erzeugte.
Abbruchmöglichkeit während der RS02-Erkennung für DVD RW hinzugefügt.
<i>(10-Dez-2006)</i>.<p>

<b>pl2</b> behebt eine fehlerhafte Speicherfreigabe beim Schließen des Programms.
Das Auspacken der Bildschirmfotos für die Dokumentation auf PPC-Plattformen wurde
korrigiert. Nur die Quellkode-Archive wurden erneuert. 
<i>(03-Okt-2006)</i>.<p>

<b>pl1</b> behebt einen Fehler im angepaßten Lesen für RS02, durch den unter bestimmten
Bedingungen nicht genügend Daten für eine erfolgreiche Wiederherstellung gelesen wurden.
Ein paar kleine Verbesserungen an der Dokumentation und der Benutzbarkeit sind auch dabei.
<i>(30-Jul-2006)</i>
</td></tr></table><p>

Der Quellkode von dvdisaster kann auch direkt im 
<a href="http://sourceforge.net/cvs/?group_id=157550">CVS-Archiv</a>
betrachtet werden. Einige interessante Dateien darin sind: 
<ul>
<li><a href="http://dvdisaster.cvs.sourceforge.net/dvdisaster/dvdisaster/CHANGELOG?view=markup">CHANGELOG</a>- was hat sich in den letzten Versionen geändert (in Englisch);</li>
<li><a href="http://dvdisaster.cvs.sourceforge.net/dvdisaster/dvdisaster/CREDITS.de?view=markup">CREDITS.de</a>- wer bei dem Projekt bisher mitgemacht hat;</li>
<li><a href="http://dvdisaster.cvs.sourceforge.net/dvdisaster/dvdisaster/INSTALL?view=markup">INSTALL</a> - Installationshinweise (in Englisch);</li>
<li><a href="http://dvdisaster.cvs.sourceforge.net/dvdisaster/dvdisaster/README?view=markup">README</a> - eine Übersicht zum Quellkodearchiv (in Englisch).</li>
</ul>

<!-- do not change below -->

<?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>
