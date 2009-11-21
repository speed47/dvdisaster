<?php
# dvdisaster: English homepage translation
# Copyright (C) 2004-2009 Carsten Gnörlich
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

<!--- Insert actual page content below --->

<h3>Выбор привода</h3>

<?php begin_screen_shot("Выбор привода","dialog-drive-full.png"); ?>
Меню выбора привода находится в верхнем левом углу
панели инструментов (выделено зеленым). Нажмите курсором мыши в поле справа от значка CD,
и выпадет список для выбора привода. Затем выберите привод, содержащий носитель, который нужно обработать с помощью dvdisaster.<p>

Чтобы упростить определение приводов, в списке дается следующая информация:
<ul>
<li>Идентификация устройства, которая обычно состоит из имени изготовителя
и номера модели привода. Эти значения были запрограммированы в приводе
изготовителем. Поскольку dvdisaster отображает их без дополнительной обработки,
то здесь можно увидеть все, что изготовитель посчитал нужным показать. Иногда
эта идентификация не очень содержательна.<p></li>
<li>Идентификатор, с помощью которого этот привод управляется в операционной системе 
(например, /dev/hda в GNU/Linux или F: в Windows)</li>
</ul>
<?php end_screen_shot(); ?>


<p>
<b>Примеры:</b>
<table width="100%">
<tr>
<td width="50%" align="center"><img src="images/select-drive-linux.png"><br>
Развернутый список выбора в GNU/Linux</td>
<td width="50%" align="center"><img src="images/select-drive-win.png"><br>
Развернутый список выбора в Windows</td>
</tr>
</table><p>



<!--- do not change below --->

<?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>
