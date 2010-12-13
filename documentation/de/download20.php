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
begin_page();
?>

<!--- Insert actual page content below --->

<h3>Digitale Unterschrift</h3>

Die herunterladbaren Pakete enthalten
eine mit <a href="http://www.gnupg.org/gnupg.html">GnuPG</a> erstellte 
digitale Unterschrift, damit Sie nachprüfen können,
ob sich die Software in ihrem ursprünglichen Zustand befindet.<p>

Die Unterschrift wurde mit dem folgenden 
<a href="../pubkey.asc">Öffentlichen Schlüssel</a> erzeugt:

<a href="../pubkey.asc">
<pre>
pub   1024D/F5F6C46C 2003-08-22
      Key fingerprint = 12B3 1535 AF90 3ADE 9E73  BA7E 5A59 0EFE F5F6 C46C
uid                  dvdisaster (pkg signing key #1)
sub   1024g/091AD320 2003-08-22
</pre></a>

Sie können den Fingerabdruck des öffentlichen Schlüssels auch direkt von
den Entwicklern erhalten, indem Sie eine E-Mail 
an <img src="../images/email.png" align="top"> schreiben. Bitte verwenden Sie
den Betreff "GPG finger print".

<h3>MD5-Prüfsummen</h3>

MD5-Prüfsummen sind im Gegensatz zu der digitalen Unterschrift
kryptographisch schwach: Es ist möglich, ein
manipuliertes Softwarepaket zu erzeugen, das die gleiche Prüfsumme
wie das Original hat. Die Prüfsummen reichen jedoch aus, um schnell
zu testen ob das Softwarepaket vollständig und korrekt heruntergeladen
worden ist.

<!--- do not change below --->

<?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>
