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

howto_headline("Получение информации об образах и данных для исправления ошибок", "Вывод информации", "images/compare-icon.png");
?>

<!-- Insert actual page content below -->

Для этой функции нет настроек; однако нужен файл образа
и, возможно, 
<a href="howtos20.php">файл для исправления ошибок</a> от него.

<hr>

<table>
<tr>
<td class="w200x" align="center">
<a href="howtosa2.php">
<img src="../images/select-image.png" alt="dvdisaster UI: Image file selection (input field and button)" class="noborder">
<br><img src="../images/down-arrow.png" alt="Icon: Arrow down" class="noborder"></a></td>
<td>&nbsp;&nbsp;</td>
<td class="valignt"><b>Введите имя файла ISO-образа,</b>
для которого нужно получить информацию. Образ уже должен присутствовать на
жестком диске; в противном случае используйте функцию "Читать" для его получения с носителя. 
</td>
</tr>
</table>

<table>
<tr>
<td class="w200x" align="center">
<a href="howtosa3.php">
<img src="../images/select-ecc.png" alt="dvdisaster UI: Error correction file selection (input field and button)" class="noborder">
<br><img src="../images/down-arrow.png" alt="Icon: Arrow down" class="noborder"></a></td>
<td>&nbsp;&nbsp;</td>
<td class="valignt">
<b>Введите имя файла для исправления ошибок</b>
от этого носителя. Оставьте это поле пустым, если образ
был
<a href="howtos30.php">дополнен данными для исправления ошибок</a>.
</td>
</tr>
</table>

<table>
<tr>
<td class="w200x" align="center">
<a href="howtosa4.php">
<img src="images/compare-icon.png" alt="dvdisaster UI: Verify (button)" class="noborder">
<br><img src="../images/down-arrow.png" alt="Icon: Arrow down" class="noborder"></a></td>
<td>&nbsp;&nbsp;</td>
<td class="valignt"><b>Запустите получение информации,</b> нажав кнопку
"Проверить".</td>
</tr>
</table>

<?php begin_howto_shot("Вывод информации.","compat-okay-rs01.png", ""); ?>
<b>Наблюдайте за процессом проверки.</b>
Чтобы отобразить всю информацию, файл образа и файл для исправления ошибок должны быть полностью прочитаны. 
<?php end_howto_shot(); ?>

<hr>

<a name="examine">Дополнительная информация по объяснению результатов:</a><p>

<ul>
<li><a href="howtos52.php">Объяснение результатов для файлов исправления ошибок</a><p></li>
<li><a href="howtos53.php">Объяснение результатов для дополненных образов</a><p></li>
<li><a href="howtos59.php">Примеры</a><p></li>
</ul>


<!-- do not change below -->

<?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>
