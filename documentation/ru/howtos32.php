<?php
# dvdisaster: English homepage translation
# Copyright (C) 2004-2010 Carsten Gnörlich
#
# UTF-8 trigger: äöüß 
#
# Include our PHP sub routines, then call begin_page()
# to start the HTML page, insert the header, 
# navigation and news if appropriate.

require("../include/dvdisaster.php");
require("../include/screenshot.php");
begin_page();

howto_headline("Дополнение образов данными для исправления ошибок", "Основные параметры", "images/create-icon.png");
?>

<!--- Insert actual page content below --->

<?php begin_screen_shot("Открытие диалога настроек.","global-prefs-invoke.png"); ?>
<table><tr><td valign="top"><img src="../images/prefs-icon.png" valign="bottom"></td>
<td>В диалоге настроек находятся следующие вкладки.
Откройте диалог, выбрав значок, отмеченный зеленым на снимке с экрана
(нажмите на картинку, и она увеличится). Этот значок может выглядеть по-разному
в зависимости от используемой темы.</td>
</tr></table>
<?php end_screen_shot(); ?>

<pre> </pre>

<?php begin_screen_shot("Вкладка \"Исправление ошибок\".","create-prefs-ecc2.png"); ?>
<b>Вкладка "Исправление ошибок".</b> Выберите в качестве способа сохранения "Расширенный образ (RS02)"
(зеленое меню). Выберите "Использовать наименьший возможный размер из следующей таблицы", если используются носители стандартных размеров.
Затем dvdisaster выберет наименьший возможный тип носителя, который может быть
использован для записи образа. Оставшееся свободное место на носителе
будет использоваться для данных для исправления ошибок, и образ будет подготовлен
соответствующим образом.
<?php end_screen_shot(); ?>


<pre> </pre>

<b>Не используемые вкладки</b><p>

Вкладка "Разное" в настоящее время содержит лишь функции для создания
журнальных файлов. Это полезно для отправки с <a href="feedback.php">сообщениями об ошибках</a>,
но во время нормальной работы должно быть выключено.
Вкладка "Внешний вид" дает возможность подобрать цвета вывода по своему вкусу, 
но кроме этого не имеет никакого влияния на создание данных для исправления ошибок.

<pre> </pre>

<a href="howtos33.php">Дополнение образа данными для исправления ошибок...</a>


<!--- do not change below --->

<?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>
