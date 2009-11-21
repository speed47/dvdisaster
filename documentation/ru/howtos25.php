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

howto_headline("Создание файлов для исправления ошибок", "Дополнительные параметры", "images/create-icon.png");
?>

<?php begin_screen_shot("Вкладка \"Привод\".","create-prefs-drive-adv.png"); ?> 
<b>Извлекать носитель после успешного чтения</b>. Это свойство полезно,  
когда обрабатывается партия носителей. Используйте его вместе 
с параметрами, показанными на втором снимке внизу.<p> 
dvdisaster попытается извлечь носитель после прочтения
образа.  Однако извлечению носителя может препятствовать
операционная система, поэтому нет гарантии, что это сработает. Например,
если после вставки носителя открывается окно для запуска содержимого,
может оказаться невозможным автоматическое извлечение носителя.
<?php end_screen_shot(); ?>

<?php begin_screen_shot("Вкладка \"Файлы\".","create-prefs-file-adv.png"); ?>
<b>Автоматическое создание и удаление файлов.</b> 
Используя эти параметры, можно автоматизировать процесс создания файлов для исправления ошибок. Первый параметр позволяет dvdisaster создавать файл для исправления ошибок
сразу после (полного) чтения носителя. Второй параметр
удаляет образ, когда создан файл для исправления ошибок.<p>

<b>Обратите внимание:</b> Не забудьте выбрать другое имя для файла для 
исправления ошибок после вставки нового носителя. В противном случае 
предыдущий файл будет перезаписан.
<?php end_screen_shot(); ?>



<!--- do not change below --->

<?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>
