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

<h3 class="top">Проверка совместимости образов, дополненных данными для исправления ошибок</h3>

<b>Обоснование:</b> dvdisaster может поместить данные для исправления ошибок
<a href="howtos30.php">вместе с пользовательскими данными на носитель</a>.
Данные для исправления ошибок добавляются к ISO-образу таким образом, что они остаются невидимыми
для большинства приложений и не мешают им.<p>

<b>Возможная несовместимость:</b> ПО для записи CD/DVD/BD также может не
видеть данные для исправления ошибок. Хотя это и маловероятно, но все-таки возможно,
что записывающее ПО отрежет или повредит данные для исправления ошибок 
при создании носителя. В таком случает исправление ошибок не будет работать.<p>


<b>Как проверить совместимость:</b><p>

Имейте в виду, что некоторые шаги намечены здесь лишь в общих чертах;
ищите подробные инструкции и примеры по ссылкам на соответствующие разделы.<p>

<table>
<tr>
<td class="w200x" align="center"><img src="../images/good-cd-ecc.png" alt="Icon: Medium containing error correction data">
<p><img src="../images/down-arrow.png" alt="Icon: Arrow down"></td>
<td>&nbsp;&nbsp;</td>
<td class="valignt"><b>Сначала создайте носитель, к которому были добавлены данные для исправления ошибок</b>. Не забывайте использовать
надлежащие <a href="howtos32.php">настройки</a> и следуйте 
<a href="howtos33.php">пошаговым</a> инструкциям. <br>
Не используйте перезаписываемые DVD- или BD-носители, так как в некоторых обстоятельствах
они могут повлиять на результаты проверки (см. <a href="qa20.php#rw">пункт 3.4 в
вопросах и ответах</a>).
</td>
</tr>
</table>

<table>
<tr>
<td class="w200x" align="center">
<img src="../images/good-image2.png" alt="Icon: Complete image from the previously written medium" class="noborder"><p>
<img src="../images/down-arrow.png" alt="Icon: Arrow down" class="noborder">
</td>
<td>&nbsp;&nbsp;</td>
<td class="valignt"><b>Создайте <i>второй</i> образ с <i>записанного</i> 
носителя</b>.
Используйте те же <a href="howtos22.php">настройки</a> и шаги, как и в
<a href="howtos23.php?way=1">чтении носителя</a> для создания файла для исправления 
ошибок; но можно остановиться после того, как чтение завершится, так как
нам не нужен файл для исправления ошибок.
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
который был только что считан с носителя. Имейте в виду, что следующая проверка
бесполезна, если работать с образом, который первоначально был создан
с помощью ПО для записи CD/DVD/BD и дополнен dvdisaster'ом.
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
<td class="valignt"><b>Запустите проверку,</b> нажав
кнопку "Проверить".</td>
</tr>
</table>

<?php begin_howto_shot("Вывод информации.","compat-okay-rs02.png", ""); ?>
<b>Посмотрите на результаты проверки.</b>
Если вы получили зеленые сообшения "Хороший образ." и "Хорошие данные для исправления ошибок.",
то ваше ПО для записи дисков и dvdisaster совместимы в отношении
дополненных образов.
<?php end_howto_shot(); ?>

<hr>

<a name="err"> </a>
<b>Возможные причины ошибок и способы устранения:</b><p>

<?php begin_howto_shot("Неправильный размер образа.","compat-150-rs02.png", "down-arrow.png"); ?>
<b>Типичная проблема: неправильный размер образа.</b>
В результате проверки может выясниться, что образ больше, чем ожидалось.
Обычно разница составляет 150 или 300 секторов для CD-носителей и
1-15 секторов для DVD/BD-носителей. Это могут быть просто нулевые заполняющие секторы,
добавленные записывающим программным обеспечением. Чтобы выяснить, действительно ли это так,
сделайте следующее:
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

<?php begin_howto_shot("Обрезание образа.","compat-dialog-rs02.png", "down-arrow.png"); ?>
<b>Введите подтверждение в диалоговом окне.</b>
Появится диалоговое окно с вопросом, нужно ли удалять из образа лишние
секторы. Ответьте "OK".
<?php end_howto_shot(); ?>


<table>
<tr>
<td class="w200x" align="center">
<img src="images/stop-icon.png" alt="dvdisaster UI: Stop (button)" class="noborder">
<p><img src="../images/down-arrow.png" alt="Icon: Arrow down" class="noborder"></td>
<td>&nbsp;&nbsp;</td>
<td class="valignt"><b>Остановите процесс восстановления,</b>
так как после усечения образа больше  ничего делать не надо.
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

<?php begin_howto_shot("Вывод информации.","compat-okay-rs02.png", ""); ?>
<b>Рассмотрите новые результаты.</b>
Если вы теперь получите зеленые сообщения "Хороший образ." и "Хорошие данные для исправления ошибок.", то проблема чисто косметическая: записывающее ПО в самом деле 
добавило заполняющие секторы при записи носителя.
<?php end_howto_shot(); ?>

<span class="red">Если после выполнения вышеописанных шагов проблема остается,
<i>нельзя</i> использовать это записывающее ПО для создания носителей с
дополненных образов.
Выполните эту проверку снова, используя ПО другого производителя.
</span> <p> 



<!-- do not change below -->

<?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>
