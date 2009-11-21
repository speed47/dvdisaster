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

<h3>Выбор файла для исправления ошибок</h3>

Файл для исправления ошибок содержит информацию для реконструирования нечитаемых 
секторов с поврежденного носителя. Он может также использоваться для проверки носителя
на наличие поврежденных или измененных секторов. Стандартное расширение имени файла - ".ecc".<p>

<?php begin_screen_shot("Выбор файла для исправления ошибок","dialog-ecc-full.png"); ?>
Есть два способа выбора файла для исправления ошибок:
<ul>
<li>используя <a href="#filechooser">диалог выбора файлов</a> (кнопка помечена заленым), или</li>
<li>непосредственно вводя местоположение файла для исправления ошибок (текстовое поле, помеченное голубым).</li><p>
</ul>
Прямой ввод удобен, когда вы собираетесь создать несколько файлов для исправления
ошибок в одном и том же каталоге. В этом случае просто измените имя файла в текстовом поле.<p>
<?php end_screen_shot(); ?>

<? require("howtos_winfile.php"); ?>



<!--- do not change below --->

<?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>
