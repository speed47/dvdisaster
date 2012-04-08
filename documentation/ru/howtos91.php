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

<h3 class="top">Проверка совместимости между файлами для исправления ошибок и ISO-образами</h3>


<b>Обоснование:</b> Вы хотите записать данные на носитель и создать для них файл для
исправления ошибок. Для того, чтобы сэкономить время, вы делаете следующее:

<ol>
<li>Вы создаете ISO-образ, используя свое ПО для записи CD/DVD/BD.</li>
<li>Вы записываете образ на носитель.</li>
<li>Вы создаете файл для исправления ошибок с того же образа.</li>
</ol>

<b>Возможная несовместимость:</b> ПО для записи создает носитель, 
который в точности не соответствует образу. Это может не дать алгоритму исправления
ошибок восстановить содержимое носителя, когда он станет поврежденным. 
<p>

<b>Как проверить совместимость:</b><p>

Имейте в виду, что некоторые шаги здесь намечены лишь в общих чертах;
для получения подробных инструкций и примеров смотрите ссылки в соответствующих разделах.<p>

<table>
<tr>
<td class="w200x" align="center"><img src="../images/good-image.png" alt="Icon: Complete image">
<p><img src="../images/down-fork-arrow.png" alt="Icon: Forked arrow"></td>
<td>&nbsp;&nbsp;</td>
<td class="valignt"><b>Создайте ISO-образ данных,</b> которые нужно
записать на носитель. Если вам нужна справка по созданию ISO-образов,
обратитесь к 
<a href="howtos33.php?way=1">примеру создания ISO-образов</a>.
</td>
</tr>
</table>

<table>
<tr>
<td class="w100x" align="center">
<img src="../images/good-cd.png" alt="Icon: Good medium (without read errors)" class="nobordervalignm"><p>
<img src="../images/down-arrow.png" alt="Icon: Arrow down" class="noborder">
</td>
<td class="w100x" align="center" valign="top">
<img src="../images/ecc.png" alt="Icon: Separate file with error correction data" class="nobordervalignm">
</td>
<td>&nbsp;&nbsp;</td>
<td class="valignt"><b>Запишите носитель и создайте файл для исправления ошибок.</b>
Используйте только что созданный образ для
<a href="howtos33.php?way=3#c">записи носителя</a>. 
Потом проведите эти <a href="howtos22.php#ecc">основные настройки</a> и
<a href="howtos23.php?way=2">создайте файл для исправления данных</a>
с этого образа.
</td>
</tr>
</table>

<table>
<tr>
<td class="w100x" align="center">
<img src="../images/good-image2.png" alt="Icon: Complete image from the previously written medium" class="noborder"><p>
<img src="../images/down-arrow.png" alt="Icon: Arrow down" class="noborder">
</td>
<td class="w100x" align="center"> </td>
<td>&nbsp;&nbsp;</td>
<td class="valignt"><b>Создайте <i>второй</i> образ из <i>записанного</i> 
носителя.
</b> Используйте эти <a href="howtos22.php#read">настройки</a>
и прочитайте носитель, как описано
в <a href="howtos23.php?way=1">создании образа</a> 
для построения файла для исправления ошибок. Но вы можете остановить это пошаговое прохождение,
когда чтение закончится, так как не требуется опять создавать файл для исправления ошибок.
</td>
</tr>
</table>

<table>
<tr>
<td class="w200x" align="center">
<a href="howtosa2.php">
<img src="../images/select-image.png" alt="dvdisaster UI: Image file selection (input field and button)" class="noborder">
<br><img src="../images/down-arrow.png" alt="Icon: Arrow down" class="noborder"></a></td>
<td>&nbsp;&nbsp;</td>
<td class="valignt"><b>Введите имя <i>второго</i> ISO-образа,</b>
который вы только что считали с носителя. Имейте в виду, что следующая
проверка бесполезна, когда приходится работать с образом, первоначально созданным с помощью
ПО для записи CD/DVD/BD.
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
в случае, если оно еще не введено в предыдущих действиях.
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
<td class="valignt"><b>Запустите проверку,</b> нажав кнопку
"Проверить".</td>
</tr>
</table>

<?php begin_howto_shot("Показать информацию.","compat-okay-rs01.png", ""); ?>
<b>Взгляните на результаты проверки.</b>
Если вы получите зеленые сообщения "Хороший образ." и "Хороший файл для исправления ошибок.",
ваше ПО для записи дисков и dvdisaster совместимы. Можно по-прежнему
создавать файлы для исправления ошибок непосредственно с ISO-образов, созданных
ПО для записи.
<?php end_howto_shot(); ?>

<hr>

<a name="err"> </a>
<b>Возможные причины возникновения ошибок и способы исправления:</b><p>

<?php begin_howto_shot("Неправильный размер образа.","compat-150-rs01.png", "down-arrow.png"); ?>
<b>Типичная проблема: неправильный размер образа.</b>
В результате проверки может оказаться, что образ больше, чем ожидалось.
Обычно разница составляет 150 или 300 секторов для CD-носителей и
1-15 секторов для DVD/BD. Это могут быть просто нулевые заполняющие секторы, добавляемые
записывающим программным обеспечением. Чтобы выяснить, действительно ли это так, сделайте следующее:
<?php end_howto_shot(); ?>

<table>
<tr>
<td class="w200x" align="center">
<img src="images/fix-icon.png" alt="dvdisaster UI: Fix (button)" class="noborder">
<p><img src="../images/down-arrow.png" alt="Icon: Arrow down" class="noborder"></td>
<td>&nbsp;&nbsp;</td>
<td class="valignt"><b>Запустите процесс восстановления.</b>
</td>
</tr>
</table>

<?php begin_howto_shot("Усечение образа.","compat-dialog-rs01.png", "down-arrow.png"); ?>
<b>Введите подтверждение в диалоговом окне.</b>
Появится диалоговое окно с вопросом, можно ли удалить из образа лишние секторы. Ответьте "OK".
<?php end_howto_shot(); ?>


<table>
<tr>
<td class="w200x" align="center">
<img src="images/stop-icon.png" alt="dvdisaster UI: Stop (button)" class="noborder">
<p><img src="../images/down-arrow.png" alt="Icon: Arrow down" class="noborder"></td>
<td>&nbsp;&nbsp;</td>
<td class="valignt"><b>Остановите процесс восстановления,</b>
так как после усечения образа больше нечего делать.
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
<td class="valignt"><b>Опять запустите проверку,</b>
нажав кнопку "Проверить".</td>
</tr>
</table>

<?php begin_howto_shot("Вывод информации.","compat-okay-rs01.png", ""); ?>
<b>Рассмотрите новые результаты.</b>
Если вы теперь получите зеленые сообщения "Хороший образ." и "Хороший файл для исправления ошибок.", то остается чисто косметическая проблема: действительно, записывающее программное обеспечение добавило нулевые заполняющие секторы при записи носителя.
<?php end_howto_shot(); ?>

<span class="red">Если проблема остается после выполнения вышеописанных шагов, то уже <i>нельзя</i> считать, что dvdisaster и записывающее ПО совместимы. Созданные файлы для исправления ошибок будут непригодными.</span> <p> 
Вместо этого для создания файлов для исправления ошибок используйте следующий метод:

<hr>

<pre> </pre>

<b>Альтернативный метод, позволяющий избежать несовместимости:</b>

<ol>
<li>Сначала запишите данные на носитель.</li>
<li>Используйте dvdisaster для создания ISO-образов с записанного носителя.</li>
<li>Используйте этот образ для создания файла для исправления ошибок.</li>
</ol>
Этот метод требует больше времени из-за дополнительного процесса чтения,
но у него есть и преимущество в виде проверки на читаемость вновь созданного носителя. 

<!-- do not change below -->

<?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>
