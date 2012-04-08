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

<h3 class="top">Ověření kompatibility pro bitové kopie rozšířené o data pro opravu chyb</h3><b>Záměr:</b> dvdisaster může umístit data pro opravu chyb <a href="howtos30.php">na disk společně s uživatelskými daty</a>. Aby se předešlo konfliktům, jsou data pro opravu chyb do bitové kopie přidána způsobem, který je činí pro většinu ostatních aplikací neviditelnými.<p><b>Možná nekompatibilita:</b> Vypalovací program také nemusí být schopný vidět tato data. I když je to nepravděpodobné, může se stát, že vypalovací program data pro opravu chyb během vypalování disku vynechá, nebo je poškodí. Oprava chyb pak nebude fungovat.<p><b>Jak otestovat kompatibilitu:</b><p>Předesíláme, že některé kroky jsou zde jen načrtnuty; podrobnější informace a postup najdete v odkazovaných sekcích.<p><table>
<tr>
<td class="w200x" align="center"><img src="../images/good-cd-ecc.png" alt="Ikona: Disk obsahující data pro opravu chyb"><p><img src="../images/down-arrow.png" alt="Ikona: Šipka dolů"></td>
<td>  </td>
<td class="valignt"><b>Nejdříve vytvořte disk rozšířený o data pro opravu chyb</b>. Nezapomeňte použít správná <a href="howtos32.php">nastavení</a> a postupujte podle <a href="howtos33.php">návodu</a>. <br>Nepoužívejte přepisovatelné DVD nebo BD disky, protože v některých případech mohou test ovlivnit (viz <a href="qa20.php#rw">položka 3.4 v otázkách a odpovědích</a>).</td>
</tr>
</table>

<table>
<tr>
<td class="w200x" align="center"><img src="../images/good-image2.png" alt="Ikona: Kompletní bitová kopie z předtím zapsaného disku" class="noborder"><p><img src="../images/down-arrow.png" alt="Ikona: Šipka dolů" class="noborder"></td>
<td>  </td>
<td class="valignt"><b>Vytvořte <i>druhou</i> bitovou kopii z <i>vypáleného</i> disku.</b> Použijte stejné <a href="howtos22.php">nastavení</a> a postup jako v <a href="howtos23.php?way=1">načtení disku</a> pro vytvoření souboru pro opravu chyb, postup však můžete přerušit po vytvoření bitové kopie, protože soubor pro opravu chyb nepotřebujeme vytvářet.</td>
</tr>
</table>

<table>
<tr>
<td class="w200x" align="center"><a href="howtosa2.php"> <img src="../images/select-image.png" alt="Ovládací prvky dvdisaster: Výběr souboru bitové kopie (vstupní pole a tlačítko)" class="noborder"> <br><img src="../images/down-arrow.png" alt="Ikona: Šipka dolů" class="noborder"></a></td>
<td>  </td>
<td class="valignt"><b>Zadejte název <i>druhé</i> bitové kopie</b> kterou jste právě vytvořili z disku. Zapamatujte si, že následující test je zbytečné provádět s bitovou kopií vytvořenou vypalovacím programem a rozšířenou o data pro opravu chyb prostřednictvím dvdisaster.</td>
</tr>
</table>

<table>
<tr>
<td class="w200x" align="center"><a href="howtosa4.php"> <img src="images/compare-icon.png" alt="Ovládací prvky dvdisaster: Ověřit (tlačítko)" class="noborder"> <br><img src="../images/down-arrow.png" alt="Ikona: Šipka dolů" class="noborder"></a></td>
<td>  </td>
<td class="valignt"><b>Spusťte vyhodnocení</b> pomocí tlačítka &quot;Ověřit&quot;.</td>
</tr>
</table>

<?php begin_howto_shot("zobrazení informací.","compat-okay-rs02.png", ""); ?><b>Prohlédněte si výsledky ověření.</b> Pokud jsou zobrazeny zelené hlášky &quot;Bitová kopie je v pořádku.&quot; a &quot;Data pro opravu chyb jsou v pořádku.&quot; jsou váš vypalovací program a dvdisaster kompatibilní.<?php end_howto_shot(); ?><hr><a name="err"> </a> <b>Možné příčiny chyb a jejich odstranění:</b><p><?php begin_howto_shot("špatná velikost bitové kopie.","compat-150-rs02.png", "down-arrow.png"); ?> <b>Typický problém: špatná velikost bitové kopie.</b> Ověření může zjistit, že je bitová kopie větší než bylo očekáváno. Rozdíl je typicky 150 nebo 300 sektorů pro CD a 1-15 sektorů pro DVD/BD. V tomto případě se může jednat o padding. Pro ověření zkuste:<?php end_howto_shot(); ?><table>
<tr>
<td class="w200x" align="center"><img src="images/fix-icon.png" alt="Ovládací prvky dvdisaster: Opravit (tlačítko)" class="noborder"><p><img src="../images/down-arrow.png" alt="Ikona: Šipka dolů" class="noborder"></td>
<td>  </td>
<td class="valignt"><b>Spusťte proces opravy.</b></td>
</tr>
</table>

<?php begin_howto_shot("Zkrácení bitové kopie.","compat-dialog-rs02.png", "down-arrow.png"); ?><b>Potvrďte dialog.</b> Objeví se dialog s dotazem zda odstranit přebytečné sektory. Odpovězte &quot;OK&quot;.<?php end_howto_shot(); ?><table>
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

<?php begin_howto_shot("zobrazení informací.","compat-okay-rs02.png", ""); ?><b>Zkontrolujte nové výsledky.</b> Pokud se nyní zobrazí zelené hlášky &quot;Bitová kopie je v pořádku.&quot; a &quot;Data pro opravu chyb jsou v pořádku.&quot; je váš problém pouze kosmetický: Vypalovací program pouze přidal vynulované padding sektory. <?php end_howto_shot(); ?> <span class="red">Pokud však problém přetrvá i po provedení výše uvedených kroků, <i>nemůžete</i> váš vypalovací program používat k vypalování rozšířených programů. Proveďte test s programem jiného výrobce. </span><p><!-- do not change below --> <?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>