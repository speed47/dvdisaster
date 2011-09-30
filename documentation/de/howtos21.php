<?php
# dvdisaster: German homepage translation
# Copyright (C) 2004-2010 Carsten Gnörlich
#
# UTF-8 trigger: äöüß 
#
# Include our PHP sub routines, then call begin_page()
# to start the HTML page, insert the header, 
# navigation and news if appropriate.

require("../include/dvdisaster.php");
require("../include/footnote.php");
$script_path = current(get_included_files());
$script_name = basename($script_path, ".php");
begin_page();
$answer=$_GET["answer"];
howto_headline("Fehlerkorrektur-Daten erstellen", "Entscheidungshilfe", "images/create-icon.png");
?>

<!--- Insert actual page content below --->

<h3>Entscheidungshilfe</h3>

Fehlerkorrektur-Daten können entweder in einer eigenständigen Fehlerkorrektur-Datei 
oder direkt auf dem Datenträger abgelegt werden.
Klicken Sie auf die Antworten zu den folgenden Fragen, um eine Empfehlung für das Aufbewahren 
der Fehlerkorrektur-Daten zu erhalten.<p>

<i>Benötigen Sie Fehlerkorrektur-Daten für einen bereits gebrannten Datenträger?</i>
<ul>
<?php
if($answer <= 1) $mode="active"; else $mode="passive";
  echo "<li><a href=\"howtos21.php?answer=1\" class=\"$mode\">Ja, der Datenträger existiert schon</a></li>\n";;
if($answer != 1 || $answer >= 2) $mode="active"; else $mode="passive";
  echo "<li><a href=\"howtos21.php?answer=2\" class=\"$mode\">Nein, ich werde den Datenträger erst später brennen</a></li>\n";
echo "</ul>\n";

if($answer == 1) 
{  echo "Sie müssen eine <a href=\"howtos22.php\">Fehlerkorrektur-Datei</a> erstellen,\n";
   echo "denn ein bereits bestehender Datenträger kann nicht mehr nachträglich mit Fehlerkorrektur-Daten\n";
   echo "erweitert werden.\n";
}

if($answer >= 2)
{  echo "<i>Wieviel Platz ist noch auf dem Datenträger vorhanden?</i>\n";
   echo "<ul>\n";
   if($answer >= 2 && $answer != 4) $mode="active"; else $mode="passive";
      echo "<li><a href=\"howtos21.php?answer=3\" class=\"$mode\">Auf dem Datenträger sind mehr als 20% frei.</a></li>\n";
   if($answer >= 2 && $answer != 3) $mode="active"; else $mode="passive";
      echo "<li><a href=\"howtos21.php?answer=4\" class=\"$mode\">Der Datenträger ist schon fast voll (weniger als 20% frei)</a></li>\n";
   echo "</ul>\n";

   if($answer == 3)
   {  echo "Sie können die Fehlerkorrektur-Daten direkt <a href=\"howtos32.php\">auf dem Datenträger ablegen</a>.\n";
      echo "Dazu müssen Sie erst ein ISO-Abbild erzeugen und dieses um Fehlerkorrektur-Daten erweitern,\n";
      echo "bevor Sie den Datenträger brennen.\n";
   }
   if($answer == 4)
   {  echo "Der Datenträger hat nicht genügend Platz, um die Fehlerkorrektur-Daten aufzunehmen.\n";
      echo "Erzeugen Sie besser eine eigenständige <a href=\"howtos22.php\">Fehlerkorrektur-Datei</a>.\n";
   }
}
?>

<h3>Mehr Informationen zum Aufbewahren von Fehlerkorrektur-Daten</h3>

dvdisaster hilft Ihnen, Datenträger vor Datenverlust zu schützen, 
indem Sie vorsorglich<sup>*)</sup> Fehlerkorrektur-Daten erzeugen.
Fehlerkorrektur-Daten müssen wie eine normale Datensicherung behandelt werden.
Das bedeutet, daß Sie die Fehlerkorrektur-Daten während der gesamten
Lebensdauer des Datenträgers aufbewahren müssen. <p>

Am bequemsten ist es, die Fehlerkorrektur-Daten direkt auf dem Datenträger
abzulegen, den Sie schützen möchten. Das ist aber nur möglich, wenn Sie
den Datenträger noch nicht gebrannt haben: Sie müssen bei dieser Methode nämlich 
zunächst ein ISO-Abbild erzeugen und dieses mit dvdisaster um Fehlerkorrektur-Daten
erweitern. Anschließend schreiben Sie das ISO-Abbild, das nun die Original-Daten und
die Fehlerkorrektur-Daten enthält, auf den Datenträger.<p>

Wenn der Datenträger schon geschrieben ist oder nicht mehr genügend Platz darauf frei ist,
können Sie die Fehlerkorrektur-Daten in Form einer eigenständigen Datei erzeugen.
Diese Datei müssen Sie dann auf einem anderen Datenträger speichern, d.h. Sie müssen
zusätzliche Vorkehrungen treffen, um Ihre Fehlerkorrektur-Dateien  
<a href="howtos24.php">zu archivieren</a>.<p>

In der <a href="http://dvdisaster.net/legacy/de/background30.html">alten Dokumentation</a>
finden Sie weitere Informationen zu den Vor- und Nachteilen der beiden Methoden. 

<pre> </pre>

<!--- do not change below --->

<?php
footnote("*","footnote","An dieser Stelle noch einmal der Hinweis: Fehlerkorrektur-Daten müssen erstellt werden, bevor der Datenträger kaputt geht. Von defekten Datenträgern können keine Fehlerkorrektur-Daten mehr erstellt und damit sehr wahrscheinlich auch keine unlesbaren Sektoren wiederhergestellt werden.");

# end_page() adds the footer line and closes the HTML properly.

end_page();
?>
