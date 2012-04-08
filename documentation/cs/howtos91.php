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

<h3 class="top">Testování kompatibility mezi soubory pro opravu chyb a bitovými kopiemi</h3><b>Záměr:</b> Chcete zapsat data na disk a vytvořit pro ně soubor s daty pro opravu chyb. Pro ušetření času provedete následující:<ol>
<li>Pomocí vypalovacího programu vytvoříte bitovou kopii ve formátu ISO.</li>
<li>Bitovou kopii vypálíte na disk.</li>
<li>Pro stejnou bitovou kopii vytvoříte soubor pro opravu chyb.</li>
</ol><b>Možná nekompatibilita:</b> Vypalovací program vytvoří disk, který není zcela shodný s bitovou kopií. To může způsobit, že soubor pro opravu nebude možné použít k opravě poškození disku.<p><b>Jak otestovat kompatibilitu:</b><p>Předesíláme, že některé kroky jsou zde jen načrtnuty; podrobnější informace a postup najdete v odkazovaných sekcích.<p><table>
<tr>
<td class="w200x" align="center"><img src="../images/good-image.png" alt="Ikona: Kompletní bitová kopie"><p><img src="../images/down-fork-arrow.png" alt="Ikona: Dvojitá šipka"></td>
<td>  </td>
<td class="valignt">Z dat která chcete vypálit na disk <b>vytvořte bitovou kopii ve formátu ISO</b>. Pokud potřebujete poradit jak vytvářet bitové kopie ISO, podívejte se na <a href="howtos33.php?way=1">příklad vytváření bitových kopií</a>.</td>
</tr>
</table>

<table>
<tr>
<td class="w100x" align="center"><img src="../images/good-cd.png" alt="Ikona: Nepoškozený disk (bez chyb)" class="nobordervalignm"><p><img src="../images/down-arrow.png" alt="Ikona: Šipka dolů" class="noborder"></td>
<td class="w100x" align="center" valign="top"><img src="../images/ecc.png" alt="Ikona: Samostatný soubor s daty pro opravu chyb" class="nobordervalignm"></td>
<td>  </td>
<td class="valignt"><b>Vypalte disk a vytvořte data pro opravu chyb.</b> Pro <a href="howtos33.php?way=3#c">vypálení disku</a> použijte právě vytvořenou bitovou kopii. Poté proveďte tato <a href="howtos22.php#ecc">základní nastavení</a> a z bitové kopie <a href="howtos23.php?way=2">vytvořte soubor pro opravu chyb</a>.</td>
</tr>
</table>

<table>
<tr>
<td class="w100x" align="center"><img src="../images/good-image2.png" alt="Ikona: Kompletní bitová kopie z předtím zapsaného disku" class="noborder"><p><img src="../images/down-arrow.png" alt="Ikona: Šipka dolů" class="noborder"></td>
<td class="w100x" align="center"></td>
<td>  </td>
<td class="valignt"><b>Vytvořte <i>druhou</i> bitovou kopii z <i>vypáleného</i> disku. </b> Použijte tato <a href="howtos22.php#read">nastavení</a> a disk načtěte jak je popsáno ve <a href="howtos23.php?way=1">vytváření bitové kopie</a> pro vytvoření souboru pro opravu chyb. Postup však můžete přerušit po vytvoření bitové kopie, protože soubor pro opravu chyb už nemusíme znova vytvářet.</td>
</tr>
</table>

<table>
<tr>
<td class="w200x" align="center"><a href="howtosa2.php"> <img src="../images/select-image.png" alt="Ovládací prvky dvdisaster: Výběr souboru bitové kopie (vstupní pole a tlačítko)" class="noborder"> <br><img src="../images/down-arrow.png" alt="Ikona: Šipka dolů" class="noborder"></a></td>
<td>  </td>
<td class="valignt"><b>Zadejte název <i>druhé</i> bitové kopie</b> kterou jste právě vytvořili z disku. Zapamatujte si, že následující test je zbytečné provádět s bitovou kopií vytvořenou vypalovacím programem.</td>
</tr>
</table>

<table>
<tr>
<td class="w200x" align="center"><a href="howtosa3.php"> <img src="../images/select-ecc.png" alt="Ovládací prvky dvdisaster: Výběr souboru pro opravu chyb (vstupní pole a tlačítko)" class="noborder"> <br><img src="../images/down-arrow.png" alt="Ikona: Šipka dolů" class="noborder"></a></td>
<td>  </td>
<td class="valignt">Pokud již není přednastaven z předchozích kroků, <b>zadejte název souboru pro opravu chyb</b>.</td>
</tr>
</table>

<table>
<tr>
<td class="w200x" align="center"><a href="howtosa4.php"> <img src="images/compare-icon.png" alt="Ovládací prvky dvdisaster: Ověřit (tlačítko)" class="noborder"> <br><img src="../images/down-arrow.png" alt="Ikona: Šipka dolů" class="noborder"></a></td>
<td>  </td>
<td class="valignt"><b>Spusťte vyhodnocení</b> pomocí tlačítka &quot;Ověřit&quot;.</td>
</tr>
</table>

<?php begin_howto_shot("zobrazení informací.","compat-okay-rs01.png", ""); ?><b>Prohlédněte si výsledky ověření.</b> Pokud jsou zobrazeny zelené hlášky &quot;Bitová kopie je v pořádku.&quot; a &quot;Soubor pro opravu chyb je v pořádku.&quot;, jsou váš vypalovací program a dvdisaster kompatibilní. Soubory pro opravu chyb pak můžete vytvářet přímo z bitových kopií vytvořených vypalovacím programem.<?php end_howto_shot(); ?><hr><a name="err"> </a> <b>Možné příčiny chyb a jejich odstranění:</b><p><?php begin_howto_shot("špatná velikost bitové kopie.","compat-150-rs01.png", "down-arrow.png"); ?> <b>Typický problém: špatná velikost bitové kopie.</b> Ověření může zjistit, že je bitová kopie větší než bylo očekáváno. Rozdíl je typicky 150 nebo 300 sektorů pro CD a 1-15 sektorů pro DVD/BD. V tomto případě se může jednat o padding. Pro ověření zkuste:<?php end_howto_shot(); ?><table>
<tr>
<td class="w200x" align="center"><img src="images/fix-icon.png" alt="Ovládací prvky dvdisaster: Opravit (tlačítko)" class="noborder"><p><img src="../images/down-arrow.png" alt="Ikona: Šipka dolů" class="noborder"></td>
<td>  </td>
<td class="valignt"><b>Spusťte proces opravy.</b></td>
</tr>
</table>

<?php begin_howto_shot("Zkrácení bitové kopie.","compat-dialog-rs01.png", "down-arrow.png"); ?><b>Potvrďte dialog.</b> Objeví se dialog s dotazem zda odstranit přebytečné sektory. Odpovězte &quot;OK&quot;.<?php end_howto_shot(); ?><table>
<tr>
<td class="w200x" align="center"><img src="images/stop-icon.png" alt="Ovládací prvky dvdisaster: Zastavit (tlačítko)" class="noborder"><p><img src="../images/down-arrow.png" alt="Ikona: Šipka dolů" class="noborder"></td>
<td>  </td>
<td class="valignt"><b>Zastavte proces opravy,</b> protože po zkrácení bitové kopie již není třeba provádět žádné další akce.</td>
</tr>
</table>

<table>
<tr>
<td class="w200x" align="center"><a href="howtosa4.php"> <img src="images/compare-icon.png" alt="Ovládací prvky dvdisaster: Ověřit (tlačítko)" class="noborder"> <br><img src="../images/down-arrow.png" alt="Ikona: Šipka dolů" class="noborder"></a></td>
<td>  </td>
<td class="valignt"><b>Znovu spusťte proces ověření</b> kliknutím na tlačítko &quot;Ověřit&quot;.</td>
</tr>
</table>

<?php begin_howto_shot("zobrazení informací.","compat-okay-rs01.png", ""); ?><b>Zkontrolujte nové výsledky.</b> Pokud se nyní zobrazí zelené hlášky &quot;Bitová kopie je v pořádku.&quot; a &quot;Soubor pro opravu chyb je v pořádku.&quot; je váš problém pouze kosmetický: Vypalovací program pouze přidal vynulované padding sektory. <?php end_howto_shot(); ?> <span class="red">Pokud však problém přetrvá i po provedení výše uvedených kroků, <i>nepředpokládejte</i>, že jsou dvdisaster a váš vypalovací program kompatibilní. Vytvořený soubor pro opravu chyb bude pravděpodobně nepoužitelný.</span><p>Pro vytváření souborů pro opravu chyb použijte následující metodu:<hr>

<pre> </pre><b>Alternativní metoda obcházející nekompatibilitu:</b><ol>
<li>Nejdříve vypalte data na disk.</li>
<li>Z vypáleného disku vytvořte pomocí dvdisaster bitovou kopii.</li>
<li>Pro vytvoření souboru pro opravu chyb použijte tuto bitovou kopii.</li>
</ol>Tato metoda zabere více času díky dodatečnému procesu čtení, ale výhodou je, že zároveň otestuje čitelnost vytvořeného disku.<!-- do not change below --> <?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>