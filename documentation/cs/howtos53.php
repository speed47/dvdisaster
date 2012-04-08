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

<h3 class="top">Výsledky pro bitové kopie rozšířené o data pro opravu chyb</h3>

<?php begin_howto_shot("Bitová kopie s daty pro opravu chyb.","compat-okay-rs02.png", ""); ?>Při ověření bitové kopie podle vložených dat pro opravu chyb budou informace zobrazeny ve vztahu k:<ul>
<li>celé (rozšířené) bitové kopii</li>
<li>části z daty pro opravu chyb:</li>
</ul>
<?php end_howto_shot(); ?>

<table>
<tr><td colspan="2">Výstupní pole <b>&quot;Informace o souboru bitové kopie&quot;:</b><br><hr></td><td></td></tr>
<tr>
<td class="valignt">Sektory bitové kopie:</td>
<td>Počet sektorů rozšířené bitové kopie (včetně sektorů které přidal dvdisaster; jeden sektor = 2KB).</td>
</tr>
<tr><td> </td><td></td></tr>

<tr>
<td class="valignt">Kontrolní součet dat:</td>
<td>MD5 součet originální bitové kopie (před jejím rozšířením o data pro opravu chyb).</td>
</tr>
<tr><td> </td><td></td></tr>

<tr>
<td>ECC hlavičky:<br> Datová sekce:<br> CRC sekce:<br> ECC sekce:</td>
<td class="valignt">Rozšířená bitová kopie je složena ze tří sekcí a několika sektorů s ECC hlavičkami. Tyto hodnoty uvádí počet nečitelných sektorů v jednotlivých sekcích.</td>
</tr>
<tr><td> </td><td></td></tr>

<tr><td colspan="2">Pokud jsou všechny hodnoty v tomto výstupním poli správné, objeví se hlášení &quot;<span class="green">Bitová kopie je v pořádku.</span>&quot;. Jinak se objeví vysvětlení nejvýznamnější chyby.</td>
</tr>

<tr><td> </td><td></td></tr>
<tr><td colspan="2">Výstupní pole <b>&quot;Data pro opravu chyb&quot;:</b><br><hr></td><td></td></tr>
<tr>
<td class="valignt">Vytvořeno pomocí:</td>
<td>Zobrazí verzi dvdisaster která byla použita k vytvoření dat pro opravu chyb. Alpha/vývojové verze jsou zvýrazněny červeně.</td>
</tr>
<tr><td> </td><td></td></tr>

<tr>
<td class="valignt">Metoda:</td>
<td>Metoda a redundance použitá při vytváření dat pro opravu chyb.</td>
</tr>
<tr><td> </td><td></td></tr>

<tr>
<td class="valignt">Vyžaduje:</td>
<td>Nejnižší možná verze dvdisaster vyžadovaná pro zpracování daných dat pro opravu chyb.</td>
</tr>
<tr><td> </td><td></td></tr>

<tr>
<td class="valignt">Sektory bitové kopie:</td>
<td>První hodnota uvádí počet sektorů rozšířené bitové kopie; druhá pak počet sektorů které bitová kopie obsahovala před zpracováním pomocí dvdisaster. Protože jsou data pro opravu chyb umístěna za uživatelskými daty, může být kontrolní součet originální bitové kopie získán následujícím způsobem (za pomoci příkazového řádku GNU/Linux):<br><tt>head -c $((2048*121353)) medium.iso md5sum</tt><br>První parametr pro <i>head</i> je velikost sektoru (2048) vynásobená původní velikostí bitové kopie (121353). Tato vlastnost rozšířených bitových kopií může být také použita k oříznutí dat pro opravu chyb:<br><tt>head -c $((2048*121353)) medium.iso >stripped_image.iso</tt></td>
</tr>
<tr><td> </td><td></td></tr>

<tr>
<td class="valignt">Kontrolní součet dat:</td>
<td>MD5 součet originální bitové kopie (viz předchozí popis).</td>
</tr>
<tr><td> </td><td></td></tr>

<tr>
<td class="valignt">Kontrolní součet CRC:<br> Kontrolní součet ECC:</td>
<td>MD5 součet CRC a ECC sekcí rozšířené bitové kopie. Tyto dvě hodnoty nelze jednoduše vytvořit jinými nástroji než je dvdisaster.</td>
</tr>
<tr><td> </td><td></td></tr>

<tr><td colspan="2">Pokud jsou všechny hodnoty v tomto výstupním poli správné, objeví se hlášení &quot;<span class="green">Data pro opravu chyb jsou v pořádku.</span>&quot;. Jinak se objeví vysvětlení nejvýznamnější chyby.</td>
</tr>

</table>

<!-- do not change below -->

<?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>
