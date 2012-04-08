<?php
# dvdisaster: English homepage translation
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

<h3 class="top">Digitální podpis</h3>Všechny stažitelné balíčky dvdisaster byly digitálně podepsány pomocí <a href="http://www.gnupg.org/gnupg.html">GnuPG</a> aby bylo možné ověřit jejich autentičnost.<p>Podpis byl vytvořen pomocí následujícího <a href="../pubkey.asc">veřejného klíče</a>:<pre>
pub   1024D/F5F6C46C 2003-08-22
      Otisk klíče = 12B3 1535 AF90 3ADE 9E73  BA7E 5A59 0EFE F5F6 C46C
uid                  dvdisaster (pkg signing key #1)
sub   1024g/091AD320 2003-08-22
</pre>Pokud chcete získat klíč přímo od autorů, zašlete email na adresu <img src="../images/email.png" alt="carsten@dvdisaster.org" class="valigntt">. Do pole předmět prosím uveďte &quot;GPG finger print&quot;.<h3>Kontrolní součet MD5</h3>Na rozdíl od digitálního podpisu jsou kontrolní součty MD5 kryptograficky slabé: Je možné vytvořit upravený balíček se stejným kontrolním součte jako měl originál. Kontrolní součty MD5 jsou ale dostatečné pro rychlé ověření, zda se stažení zdařilo a proběhlo bez chyb.<!-- do not change below --> <?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>