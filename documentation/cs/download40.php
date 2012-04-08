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
require("../include/download.php");
begin_page();
$show_all=$_GET["showall"];
?>

<!-- Insert actual page content below -->

<h3 class="top">Alpha (vývojové) verze</h3><b>Pomozte nám s testováním!</b> Tato stránka obsahuje experimentální verze dvdisaster které budou vytvářeny během vývoje příští stabilní verze.<p><b>Upozornění:</b> Tato verze se stále vyvíjí a některé funkce ještě nejsou implementovány. Může obsahovat závažné chyby a může ne vždy viditelně selhat i u funkcí, které v předchozích verzích fungovali bez chyby. Nepoužívejte tuto verzi pro zpracování důležitých dat a nepoužívejte bitové kopie, nebo data pro opravu chyb pro archivační účely; k tomu je určena <a href="download.php">stabilní verze 0.72</a>.<hr>

<h3>Plánované změny v nové verzi</h3>Všechny platformy:<ul>
<li>Zavedení některých malých změn, které byly odloženy v průběhu vývoje 0.72. <i>[dosud nezačalo]</i></li>
<li>Odstranění nepotřebných funkcí. <i>[hotovo]</i></li>
<li>Pročištění zdrojového kódu a jeho příprava pro vícevláknové zpracování (podpora vícejádrových procesorů). <i>[probíhá]</i></li>
<li>Zavedení vícevláknového RS03 kodeku. <i>[probíhá]</i></li>
<li>Dokumentace použití RS03. <i>[dosud nezačalo]</i></li>
</ul>Windows:<ul>
<li>Aktualizace sady nástrojů GTK+ a vývojového systému. <i>[hotovo]</i></li>
<li>Zvýšení systémových požadavků na Windows 2000 nebo novější (starší verze Windows již nejsou podporovány vývojovými nástroji). To činí podporu ASPI ovladačů a dělení souborů na 2GB segmenty nepotřebné. <i>[hotovo]</i></li>
</ul>MacOS:<ul>
<li>Aktualizace sady nástrojů GTK+ a zavedení podpůrných funkcí pro obejití chyb v zobrazení uživatelského rozhraní. <i>[probíhá]</i></li>
</ul>

<hr>

<h3>Stažení</h3><a name="download"></a>Alpha verze používají stejný formát balíčků jako běžné verze.<p><table class="download" cellpadding="0" cellspacing="5">
<tr><td><b>dvdisaster-0.79</b></td><td align="right">21-Lis-2010</td></tr>
<tr><td colspan="2" class="hsep"></td></tr>
<tr><td colspan="2"><table>
<?php
    download_version("0.79.3", 0, "764977ab3d492a1ea9e346bfa9975e90", "1678bea3f81164ee4982398ce2227664",  "bff4788342d02aaa5d82ce7b78de5b04");

    if($show_all == 0) 
    {  echo "<tr><td colspan=\"2\"><a href=\"download40.php?showall=1#download\">Zobrazit starší verze z 0.79 větve</a></td></tr>\n";
    }
    else 
    {  echo "<tr><td colspan=\"2\"><a href=\"download40.php?showall=0#download\">Skrýt starší verze z 0.79 větve</a></td></tr>\n";
       echo "    <tr><td colspan=\"2\"> </td></tr>\n";

       download_version("0.79.2", 1, "378ed135c2faf0eaf643125d1f7726c6", "f673e41b5ddc31a6ecb48a5f053de885", "0b4c0b46e827c7f796416473511ab036");

       download_version("0.79.1", 1, "ba6d0178dc03119080e07ef0a2967c38", "žádný", "b4c62833a2447097950b563e4a7b2065");
   }
?>
  </table>
</td></tr>
<tr><td colspan="2" class="hsep"></td></tr>
<tr><td colspan="2"><b>Všechny platformy:</b> Tyto verze obsahují oproti verzi 0.72.x významné vnitřní změny. Používejte je opatrně.<p><b>0.79.3</b> (21-Lis-2010)<br><ul>
<li>GNU/Linux: Od této verze je pro přístup k optickým mechanikám využíván ovladač SG_IO; předtím používaný ovladač CDROM_SEND_PACKET je možné zvolit manuálně. Výchozí nastavení ovladačů bylo v předchozích verzích obrácené, ale v novějších jádrech Linuxu zajišťuje ovladač SG_IO lepší kompatibilitu.</li>
<li>Michael Klein poskytl Altivec optimalizace pro RS03 kodek.</li>
</ul><b>0.79.2</b> (28-Úno-2010)<br><ul>
<li>Nyní je k dispozici binární balíček pro Mac OS X. Bylo aktualizováno vývojové prostředí pro Mac OS X; díky tomu bylo odstraněno několik chyb v zobrazení uživatelského rozhraní.</li>
<li>Nadále pokračuje vývoj kodeku RS03, ale k dokončení má stále ještě daleko.</li>
</ul><b>0.79.1</b> (07-Úno-2010)<br><ul>
<li>Do SCSI vrstvy byly přidány funkce pro obejití chyb v čipsetech současných mechanik. Na těchto mechanikách mělo spuštění čtení nebo kontroly za následek zamrznutí systému. Problém se projevuje především pod Windows XP, ale ostatní operační systémy mohou vykazovat stejné chyby. Zkontrolujte prosím, zda tyto mechaniky nyní fungují a také nahlaste, pokud přestaly fungovat některé z mechanik, které předtím fungovali bez problémů.</li> 
<li>Obsahuje ukázkovou implementaci RS03 kodeku. Tato verze je poskytována pouze aby ji mohli zainteresovaní lidé porovnat s jeho <a href="download50.php">specifikací</a>. Nepoužívejte ho pro běžnou práci. Finální verze bude vydána ve verzi 0.80.</li>
</ul><b>Windows:</b> Byly aktualizovány všechny komponenty vývojového prostředí a používané knihovny. Zkontrolujte, zda nedochází k problémům s uživatelským rozhraním a lokalizací.</td></tr></table><p><!-- do not change below --> <?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>