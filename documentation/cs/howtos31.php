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
require("../include/footnote.php");
$script_path = current(get_included_files());
$script_name = basename($script_path, ".php");
begin_page();
$answer=$_GET["answer"];
howto_headline("Vytvoření dat pro opravu chyb", "Pomoc s rozhodováním", "images/create-icon.png");
?>

<!-- Insert actual page content below -->

<h3 class="top">Pomoc s rozhodováním</h3>Data pro opravu chyb mohou být vytvořena jako samostatný soubor, nebo mohou být uložena přímo na chráněný disk. Aby jste zjistili, která z těchto metod je pro vás vhodnější, zodpovězte následující otázky.<p><i>Potřebujete data pro opravu chyb pro disk, který již existuje?</i><ul>
<?php
if($answer <= 1) $mode="active"; else $mode="passive";
  echo "<li><a href=\"howtos21.php?answer=1\" class=\"$mode\">Ano, disk již byl vytvořen.</a></li>\n";;
if($answer != 1 || $answer >= 2) $mode="active"; else $mode="passive";
  echo "<li><a href=\"howtos21.php?answer=2\" class=\"$mode\">Ne, ale chystám se ho zapsat nyní.</a></li>\n";
echo "</ul>\n";

if($answer == 1) 
{  echo "Musíte vytvořit <a href=\"howtos22.php\">soubor pro opravu chyb,</a>\n";
   echo "protože existující disk nemůže být rozšířen o data pro opravu chyb.\n";
}

if($answer >= 2)
{  echo "<i>Kolik na disku zbývá volného prostoru?</i>\n";
   echo "<ul>\n";
   if($answer >= 2 && $answer != 4) $mode="active"; else $mode="passive";
      echo "<li><a href=\"howtos21.php?answer=3\" class=\"$mode\">Disk má více než 20% volného prostoru.</a></li>\n";
   if($answer >= 2 && $answer != 3) $mode="active"; else $mode="passive";
      echo "<li><a href=\"howtos21.php?answer=4\" class=\"$mode\">Disk je téměř plný (méně než 20% volného místa)</a></li>\n";
   echo "</ul>\n";

   if($answer == 3)
   {  echo "Data pro opravu chyb můžete umístit <a href=\"howtos33.php\">přímo na chráněný disk</a>.\n";
      echo "Toho dosáhnete vytvořením bitové kopie ISO a jejím rozšířením\n";
      echo "o data pro opravu chyb před jeho vypálením na disk.\n";
   }
   if($answer == 4)
   {  echo "Na disku není dostatek volného prostoru pro uložení dat pro opravu chyb.\n";
      echo "Musíte vytvořit samostatný <a href=\"howtos22.php\">soubor pro opravu chyb</a>.\n";
   }
}
?>

<h3>Více informací o uchovávání dat pro opravu chyb</h3>dvdisaster pomáhá chránit vaše disky před ztrátou dat pomocí dopředu<sup>*)</sup> vytvořených dat pro opravu chyb. S daty pro opravu chyb musí být zacházeno jako s jakýmikoliv jinými záložními daty, tedy, musí být k dispozici po celou dobu životnosti chráněného disku.<p>Nejjednodušším způsobem uchování dat pro opravu chyb je jejich uložení na chráněném disku. To je ale možné pouze pokud disk ještě nebyl zapsán: při použití této metody je nutné napřed vytvořit bitovou kopii ve formátu ISO, která je poté rozšířena o data pro opravu chyb a takto rozšířená bitová kopie je poté zapsána na disk.<p>Pokud byl disk již vytvořen, nebo nemá dostatek volného prostoru pro rozšíření, můžete data pro opravu chyb uložit jako samostatný soubor. Tento soubor pak musí být <a href="howtos24.php">archivován</a> samostatně.<p>Více informací o výhodách a nevýhodách těchto metod můžete najít v <a href="qa33.php#table">základních informacích</a>.<pre> </pre>

<!-- do not change below -->

<?php
footnote("*","footnote","Aby to bylo jasné, ještě jednou zopakujeme: data pro opravu chyb musí být vytvořena předtím než dojde k poškození disku. Není možné vytvořit data pro opravu chyb pro poškozený disk; nečitelné sektory není pak možné obnovit.");

# end_page() adds the footer line and closes the HTML properly.

end_page();
?>
