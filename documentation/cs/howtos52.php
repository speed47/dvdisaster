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

<h3 class="top">Výsledky pro soubory pro opravu chyb</h3>

<?php begin_howto_shot("Bitová kopie a soubor pro opravu chyb.","compat-okay-rs01.png", ""); ?>Porovnání bitové kopie s k ní příslušejícím souborem pro opravu chyb zobrazí informace ve dvou výstupních polích z nichž každé je určeno pro jeden ze souborů:<?php end_howto_shot(); ?><table>
<tr><td colspan="2">Výstupní pole <b>&quot;Informace o souboru bitové kopie&quot;:</b><br><hr></td><td></td></tr>
<tr>
<td class="valignt">Sektory bitové kopie:</td>
<td>Počet sektorů bitové kopie (jeden sektor = 2KB).</td>
</tr>
<tr><td> </td><td></td></tr>
<tr>
<td class="valignt">Chyby kontrolního součtu:</td>
<td>Soubor pro opravu chyb obsahuje CRC32 součty každého sektoru bitové kopie. Pokud je tato hodnota větší než nula, byly některé sektory čitelné, ale kontrolní součet jejich obsahu nesouhlasí. Oprava se pokusí obnovit obsah těchto sektorů.</td>
</tr>
<tr><td> </td><td></td></tr>

<tr>
<td class="valignt">Chybějící sektory:</td>
<td>Udává počet sektorů, které se nepodařilo přečíst. Oprava chyb se pokusí obnovit obsah těchto sektorů.</td>
</tr>
<tr><td> </td><td></td></tr>

<tr>
<td class="valignt">Kontrolní součet bitové kopie:</td>
<td>Pro celou bitovou kopii je vypočítán MD5 součet. Tuto hodnotu můžete v GNU/Linux vytvořit pomocí příkazu:<br><tt>md5sum medium2.iso</tt></td>
</tr>
<tr><td> </td><td></td></tr>

<tr><td colspan="2">Pokud jsou všechny hodnoty v tomto výstupním poli správné, objeví se hlášení &quot;<span class="green">Bitová kopie je v pořádku.</span>&quot;. Jinak se objeví vysvětlení nejvýznamnější chyby.</td>
</tr>

<tr><td> </td><td></td></tr>
<tr>
<td colspan="2">Výstupní pole <b>&quot;Informace o souboru pro opravu chyb&quot;</b>:<br><hr></td><td></td>
</tr>
<tr>
<td class="valignt">Vytvořeno pomocí:</td>
<td>Zobrazí verzi dvdisaster která byla použita k vytvoření dat pro opravu chyb. Alpha/vývojové verze jsou zvýrazněny červeně.</td>
</tr>
<tr><td> </td><td></td></tr>

<tr>
<td class="valignt">Metoda:</td>
<td>Metoda a redundance použitá při vytváření souboru pro opravu chyb.</td>
</tr>
<tr><td> </td><td></td></tr>

<tr>
<td class="valignt">Vyžaduje:</td>
<td>Nejnižší možná verze dvdisaster vyžadovaná pro zpracování daných dat pro opravu chyb.</td>
</tr>
<tr><td> </td><td></td></tr>

<tr>
<td class="valignt">Sektory bitové kopie:</td>
<td>Očekávaný počet sektorů bitové kopie.</td>
</tr>
<tr><td> </td><td></td></tr>

<tr>
<td class="valignt">Kontrolní součet bitové kopie:</td>
<td>Očekávaný MD5 součet bitové kopie.</td>
</tr>
<tr><td> </td><td></td></tr>

<tr>
<td class="valignt">Otisk:</td>
<td>dvdisaster používá k ověření toho zda soubor pro opravu chyb patří k dané bitové kopii kontrolní součet speciálního sektoru.</td>
</tr>
<tr><td> </td><td></td></tr>

<tr>
<td class="valignt">ECC bloky:</td>
<td>Oprava chyb dělí bitovou kopii na malé bloky, které mohou být zpracovávány samostatně. Tato informace je v podstatě zbytečná, pokud je tedy počet ECC bloků správný ;-)</td>
</tr>
<tr><td> </td><td></td></tr>

<tr>
<td class="valignt">Kontrolní součet ECC:</td>
<td>Pro celý soubor pro opravu chyb (s výjimkou prvních 4KB) je vypočítán MD5 součet. Tuto hodnotu můžete v GNU/Linux vytvořit pomocí příkazu:<br><tt>tail -c +4097 medium.ecc | md5sum</tt></td>
</tr>
<tr><td> </td><td></td></tr>

<tr><td colspan="2">Pokud jsou všechny hodnoty v tomto výstupním poli správné, objeví se hlášení &quot;<span class="green">Soubor pro opravu chyb je v pořádku.</span>&quot;. Jinak se objeví vysvětlení nejvýznamnější chyby.</td>
</tr>
</table>

<!-- do not change below -->

<?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>
