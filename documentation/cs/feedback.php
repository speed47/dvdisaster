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
require("../include/screenshot.php");
begin_page();
?>

<!-- Insert actual page content below -->

<h3 class="top">Hlášení chyb</h3>Stejně jako všechny složitější programy může dvdisaster obsahovat chyby (chybné programování) a může být nekompatibilní s některým hardwarem (mechanikami) a jinými programy v systému. Uvítáme, pokud nám oznámíte jakékoliv problémy na které narazíte, ať už v programu nebo v dokumentaci, aby jsme se mohli pokusit dané věci v dalších verzích vylepšit nebo opravit.<p>Aby jsme vždy dostali správné informace, vytvořili jsme následující seznam základních zásad pro hlášení chyb:<h4>Zkontrolujte napřed zda se skutečně jedná o chybu v programu:</h4>

<ul>
<li>Ujistěte se, že používáte nejnovější verzi programu staženou z <a href="http://dvdisaster.net/en/download.php">naší stránky na SourceForge</a>. Verze dvdisaster poskytované třetími stranami mohou obsahovat funkce a chyby, které se v originální verzi nevyskytují (a mi nemůžeme opravovat jejich problémy).</li>
<li>Ujistěte se, že problém na který jste narazili již není popsán v sekci <a href="qa20.php">Otázky a odpovědi</a>.</li>
<li>Upozorňujeme, že dvdisaster funguje pouze se zapisovatelnými variantami disků, takže to že <b>odmítne</b> pracovat s <b>DVD-ROM</b> a <b>BD-ROM</b> disky <b>není chyba</b>. Dále nejsou podporovány ani CD-Audio, VCD, SVCD a multisession disky, stejně jako všechny formáty HD-DVD (<a href="qa10.php#media">seznam podporovaných formátů</a>).</li>
<li>dvdisaster funguje pouze s reálnými mechanikami. Nejsou podporovány síťové mechaniky, softwarové mechaniky (např. Alcohol) ani mechaniky ve virtuálních počítačích.</li>
</ul>

<h4>Jak nahlásit problémy s programem:</h4>Zašlete jakékoliv zjištěné závady na email <img src="../images/email.png" alt="carsten@dvdisaster.org" class="valigntt">. Hlášení o chybě by mělo obsahovat:<p><ul>
<li>Informace o operačním systému a použité verzi dvdisaster;</li>
<li>informace o mechanikách a typech disků u kterých se problém vyskytl ;</li>
<li>textový popis pozorovaného problému;</li>
<li>snímek obrazovky se zobrazeným chybovým hlášením a/nebo výstupem který by mohl poskytnout další informace o problému;</li>
<li>rozdíly mezi fungující a nefungující konfigurací pokud se problém vyskytuje pouze na určitých mechanikách/počítačích;</li>
<li>soubor se záznamem, pokud si myslíte, že jde o nekompatibilitu mechaniky nebo disku.</li>
</ul>

<?php begin_screen_shot("Vytvoření souboru se záznamem.", "activate-logfile.png"); ?><b>Jak vytvořit soubor se záznamem:</b> Pokud máte podezření, že za problém může nekompatibilita vaší mechaniky a/nebo disku, aktivujte v nastavení ukládání záznamu do souboru, jak je zobrazeno na níže uvedeném snímku. Poté proveďte čtení nebo kontrolu a výsledný soubor se záznamem připojte k vašemu hlášení. <?php end_screen_shot(); ?>Děkujeme za váš názor!<!-- do not change below --> <?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>