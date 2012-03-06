<?php
# dvdisaster: Russian homepage translation
# Copyright (C) 2007-2010 Igor Gorbounov
#
# UTF-8 trigger: äöüß 
#
# Include our PHP sub routines, then call begin_page()
# to start the HTML page, insert the header, 
# navigation and news if appropriate.

require("../include/dvdisaster.php");
require("../include/screenshot.php");

begin_page();

howto_headline("Восстановление образов носителей", "Прохождение", "images/fix-icon.png");
?>

<!--- Insert actual page content below --->

Убедитесь, что dvdisaster сконфигурирован, как описано в разделе
<a href="howtos41.php">основные параметры</a>.
Затем выполните следующие шаги:<p>

<hr>

<a name="a"></a>
<table>
<tr>
<td width="200px" align="center"><img src="../images/slot-in.png">
<br><img src="../images/down-arrow.png"></td>
<td>&nbsp;&nbsp;</td>
<td valign="top"><b>Вставьте поврежденный носитель в привод,</b> 
которые непосредственно подключен к компьютеру. Нельзя использовать сетевые приводы,
виртуальные приводы и приводы в виртуальных машинах.</td>
</tr>
</table>

<table>
<tr>
<td width="200px" align="center"><img src="../images/winbrowser.png">
<br><img src="../images/down-arrow.png"></td>
<td>&nbsp;&nbsp;</td>
<td valign="top"><b>Закройте все окна,</b> которые могут быть открыты операционной системой
для просмотра или запуска на исполнение содержимого носителя.
Подождите, пока привод не распознает носитель и носитель не замедлит вращение.</td>
</tr>
</table>

<table>
<tr>
<td width="200px" align="center"><a href="howtosa1.php">
<img src="../images/select-drive.png" border="0">
<br><img src="../images/down-arrow.png" border="0"></a></td>
<td>&nbsp;&nbsp;</td>
<td valign="top"><b>Выберите привод,</b> содержащий поврежденный носитель, 
в выпадающем меню dvdisaster'а.</td>
</tr>
</table>

<table>
<tr>
<td width="200px"align="center">
<img src="../images/select-ecc.png" border="0" align="center"></a></td>
<td>&nbsp;&nbsp;</td>
<td valign="top">
Если используются <a href="howtos20.php">файлы для исправления ошибок,</a>
введите имя файла в показанное поле. 
Оставьте это поле пустым, если носитель был 
<a href="howtos30.php">дополнен данными для исправления ошибок</a>.<br>
</td>
</tr>
<td width="200px" align="center"><a href="howtosa1.php">
<img src="../images/down-arrow.png" border="0"></a></td>
</table>

<table>
<tr>
<td width="200px" align="center">
<a href="howtosa4.php">
<img src="images/read-icon.png" border="0">
<br><img src="../images/down-arrow.png" border="0"></a></td>
<td>&nbsp;&nbsp;</td>
<td valign="top"><b>Нажмите кнопку "Читать"</b> для запуска процесса чтения.</td>
</tr>
</table>

<?php begin_howto_shot("Чтение носителя.","adaptive-progress.png", ""); ?>
<b>Наблюдайте за процессом чтения.</b>
При адаптивной стратегии чтения выполняется систематический поиск читаемых секторов. Будут видны временные пропуски, которые будут заполняться на следующих этапах. Обычно этот эффект не так выражен, как показано на картинке. Если все поврежденные 
сектора размещены в конце носителя, процесс чтения может даже остановиться, не добравшись до первого поврежденного сектора.
<?php end_howto_shot(); ?>
<p>

<table>
<tr>
<td width="200px" align="center">
<img src="../images/down-arrow.png" border="0"></a></td>
</tr>
</table>

<?php begin_howto_shot("Процесс чтения завершился успешно.","adaptive-success.png", ""); ?>
<b>Следующие действия зависят от результата процесса чтения.</b> 
Процесс чтения заканчивается автоматически, когда собрано достаточно данных для успешного восстановления (обратите внимание на сообщение, отмеченное зеленым). В таком случае продолжайте восстановление, для чего нажмите кнопку "Исправить", как описывается ниже.
<?php end_howto_shot(); ?>

<?php begin_howto_shot("Процесс чтения завершился неудачей.","adaptive-failure.png", ""); ?>
Процесс чтения прервется также в случае, если ему не удастся найти досточного количества читаемых секторов (смотрите сообщение, помеченное красным). Образ в этом незавершенном состоянии еще <b>не</b> восстановим.
Постарайтесь собрать дополнительные данные, следуя советам в 
<a href="howtos43.php">дополнительных параметрах</a>.
<?php end_howto_shot(); ?>

<table>
<tr>
<td width="200px" align="center"><img src="../images/down-arrow.png" border="0"></td>
<td></td><td></td>
</tr>

<a name="b"></a>
<tr>
<td width="200px" align="center"><a href="howtosa4.php">
<img src="images/fix-icon.png" border="0">
</td>
<td>&nbsp;&nbsp;</td>
<td valign="top">Нажмите на кнопку "Исправить" для запуска
<b>восстановления образа</b> (работает <b>только</b> в случае, когда вышеописанный процесс чтения завершился успешно!).</td>
</tr>

<tr>
<td width="200px" align="center"><img src="../images/down-arrow.png" border="0"></td>
<td></td><td></td>
</tr>
</table>

<?php begin_howto_shot("Наблюдение за восстановлением.","fix-success.png", ""); ?>
<b>Наблюдение за процессом восстановления.</b> Адаптивное чтение остановится, как только будет собрано достаточно данных для успешного восстановления; следовательно, исправление ошибок всегда будет максимально нагружено. Это приводит к появлению большой красной области на графике "Ошибки/Ecc-блок" и не является причиной для беспокойства. В зависимости от размера носителя и производительности компьютера на восстановление может потребоваться от нескольких минут до часов.
<?php end_howto_shot(); ?>

<table>
<tr>
<td width="200px"align="center">
<img src="../images/down-arrow.png" border="0">
</td>
</tr>
</table>

<table>
<tr>
<td width="200px"align="center">
<img src="../images/good-image.png" border="0" align="center"></a></td>
<td>&nbsp;&nbsp;</td>
<td valign="top">После завершения восстановления все данные в ISO-образе будут снова в порядке.
</td>
</tr>
</table>

<table>
<tr>
<td width="200px"align="center">
<img src="../images/down-arrow.png" border="0">
</td>
</tr>
</table>

<table>
<tr>
<td width="200px" align="center">
<a href="howtos33.php?way=2#c">
<img src="thumbnails/write-iso1.png" border="0"></a></td>
<td>&nbsp;&nbsp;</td>
<td valign="top"><b>Запишите восстановленный ISO-образ</b> 
на новый носитель. Выполните такие же действия, как описано в разделе
о <a href="howtos33.php?way=2#c">записи носителей,</a> которые были
<a href="howtos33.php">дополнены данными для исправления ошибок</a>.
</td>
</tr>
</table>

<table>
<tr>
<td width="200px"align="center">
<img src="../images/down-arrow.png" border="0">
</td>
</tr>
</table>

<table>
<tr>
<td width="200px"align="center">
<img src="../images/old-cd.png" border="0" align="center">
<img src="../images/old-image.png" border="0" align="center">
<img src="../images/good-cd.png" border="0" align="center"></a></td>
<td>&nbsp;&nbsp;</td>
<td valign="top">Теперь вы создали новый носитель, содержащий полностью восстановленные данные.
Обязательно <a href="howtos10.php">проверьте их на ошибки чтения</a>. 
Затем можно выбросить поврежденный носитель и удалить ISO-образ. Однако если для старого носителя был создан файл для исправления ошибок, его можно хранить для защиты вновь созданного носителя.
</td>
</tr>
</table>

<!--- do not change below --->

<?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>
