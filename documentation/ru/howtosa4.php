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

<h3 class="top">Запуск действий</h3>

<?php begin_screen_shot("Запуск действий","action-buttons.png"); ?>
Для запуска действия в dvdisaster'е, нажмите одну из кнопок, отмеченную зеленым:<p>

<table>
<tr>
<td class="valignt"><img src="images/read-icon.png" alt="dvdisaster UI: Read (button)"> &nbsp;</td>
<td><b>Чтение содержимого носителя в файл образа</b> для:
<ul>
<li>чтения <a href="howtos42.php#a">поврежденного носителя</a> для последующего  восстановления.
<li>чтения <a href="howtos23.php?way=1&expand=0">носителя без ошибок</a> для создания файла для исправления ошибок.</ul></td>
</tr>

<tr>
<td class="valignt"><img src="images/create-icon.png" alt="dvdisaster UI: Create (button)"> &nbsp;</td>
<td><b><a href="howtos20.php">Создание файла для исправления ошибок</a></b><br>
(возможно только со свободных от дефектов носителей!)</td>
</tr>

<tr>
<td class="valignt"><img src="images/scan-icon.png" alt="dvdisaster UI: Scan (button)"> &nbsp;</td>
<td><b><a href="howtos10.php">Поиск ошибок чтения на носителе.</a></b>
</td>
</tr>

<tr>
<td class="valignt"><img src="images/fix-icon.png" alt="dvdisaster UI: Fix (button)"> &nbsp;</td>
<td><b><a href="howtos40.php">Востановление образа поврежденного носителя</a></b><br>
при условии, что имеются
 <a href="howtos20.php">данные для исправления ошибок</a>.
</td>
</tr>

<tr>
<td class="valignt"><img src="images/compare-icon.png" alt="dvdisaster UI: Verify (button)"> &nbsp;</td>
<td>Вывод <a href="howtos50.php">информации об образах и данных для исправления ошибок</a>.
</td>
</tr>
</table><p>

<b>Другие кнопки, имеющие отношение к вышеописанным действиям:</b>

<table>
<tr>
<td class="valignt"><img src="images/log-icon.png" alt="dvdisaster UI: View log (button)"> &nbsp;</td>
<td><b>Просмотреть журнальный файл выполняющегося действия</b> (отмечено желтым).<br>
См. также: <a href="feedback.php#log">Создание журнального файла</a>.
</td>
</tr>

<tr>
<td class="valignt"><img src="images/stop-icon.png" alt="dvdisaster UI: Stop (button)"> &nbsp;</td>
<td><b>Прерывание выполняющегося действия</b> (отмечено красным).<br>
Для прерывания некоторых действий требуется некоторые время; в особенности, если эта
кнопка нажимается во время чтения поврежденного сектора.
</td>
</tr>
</table>

<?php end_screen_shot(); ?>

<!-- do not change below -->

<?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>
