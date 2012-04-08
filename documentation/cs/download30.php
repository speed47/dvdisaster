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

<h3 class="top">Instalace zdrojových balíčků</h3>dvdisaster používá běžný mechanismus instalace ze zdrojů pomocí <tt>./configure; make; make install</tt>. Podrobnější informace získáte v souboru <tt>INSTALL</tt>.<pre> </pre><a name="mac"></a><h3>Instalace binárního balíčku pro Mac OS X</h3>ZIP archiv obsahuje aplikační balíček vytvořený pro Mac OS X 10.5 na x86 procesorech. Rozbalte archiv do požadovaného umístění a program spusťte pomocí &quot;dvdisaster.app&quot;. dvdisaster může být spuštěn i pod Mac OS X 10.4 a na PowerPC procesorech, musíte však v tomto případě použít zdrojový balíček a sestavit si svou vlastní verzi.<p><i>Mějte na paměti, že grafické uživatelské rozhraní ještě není zcela dokončeno.</i> Sada nástrojů GTK+ pro uživatelské rozhraní na Mac OS X je stále v rané fázi vývoje. Uživatelské rozhraní může vypadat nedotaženě a může občas zamrzat. Může také docházet k chybám zobrazení. Většinu těchto efektů můžete omezit pokud nebudete s oknem dvdisaster pracovat pokud probíhá nějaká akce. Především se vyhněte změně velikosti okna a minimalizaci do doku.<p>V průběhu vývoje verze 0.73 budou možná implementovány některé funkce pro obejití těchto efektů; jinak je v plánu počkat na vylepšení sady nástrojů GTK+.<p>Není v plánu vývoj verze dvdisaster která není závislá na GTK+ protože by to vyžadovalo kompletní přepsání programu pro Quartz. Podobné důvody mimochodem platí i pro verzi pro Windows ;-)<pre> </pre><a name="win"></a><h3>Instalace binárních souborů pro Windows</h3>Verzi pro Windows nainstalujte spuštěním instalačního programu (např. <?php echo ${pkgname}?>-setup.exe) a pokračujte podle zobrazovaných pokynů.<p><b>Varování:</b> dvdisaster není možné nainstalovat jednoduchým rozbalením setup.exe, nebo zkopírováním existující instalace. Pokud to tak uděláte, může docházet k podivným chybám, které s chybnou instalací zdánlivě vůbec nesouvisí.<!-- do not change below --> <?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>