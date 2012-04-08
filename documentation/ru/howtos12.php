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

howto_headline("Поиск ошибок на носителях", "Прохождение", "images/scan-icon.png");
?>

<!-- Insert actual page content below -->

Убедитесь, что dvdisaster настроен так, как описано в разделе 
<a href="howtos11.php">основные настройки</a>, так как некоторые параметры могут
отрицательно влиять на результаты проверки. Затем выполните следующие шаги:
<p>

<hr>

<table>
<tr>
<td class="w200x" align="center"><img src="../images/slot-in.png" alt="Icon: Insert the medium into a drive">
<br><img src="../images/down-arrow.png" alt="Icon: Arrow down"></td>
<td>&nbsp;&nbsp;</td>
<td class="valignt"><b>Вставьте проверяемый носитель в привод</b>
, который непосредственно подключен к компьютеру. 
Нельзя использовать сетевые приводы, виртуальные приводы и приводы в виртуальных машинах.
</td>
</tr>
</table>

<table>
<tr>
<td class="w200x" align="center"><img src="../images/winbrowser.png" alt="Icon: Close any windows opened by the Autoplay function">
<br><img src="../images/down-arrow.png" alt="Icon: Arrow down"></td>
<td>&nbsp;&nbsp;</td>
<td class="valignt"><b>Закройте все окна</b>, которые могли быть открыты операционной системой 
для просмотра или выполнения содержимого носителя. 
Подождите, пока привод не распознает носитель и не замедлится его вращение.
</td>
</tr>
</table>

<table>
<tr>
<td class="w200x" align="center"><a href="howtosa1.php">
<img src="../images/select-drive.png" alt="dvdisaster UI: Drive selection (dropdown menu)" class="noborder">
<br><img src="../images/down-arrow.png" alt="Icon: Arrow down" class="noborder"></a></td>
<td>&nbsp;&nbsp;</td>
<td class="valignt"><b>Выберите привод, содержащий носитель,</b>
в выпадающем меню dvdisaster.
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
<td class="valignt"><b>Выберите файл коррекции ошибок для этого носителя</b>,
если он имеется. Ecc-данные носителя, дополненного по методу RS02,
используются автоматически.</td>
</tr>
</table>

<table>
<tr>
<td class="w200x" align="center">
<a href="howtosa4.php">
<img src="images/scan-icon.png" alt="dvdisaster UI: Scan (button)" class="noborder">
<br><img src="../images/down-arrow.png" alt="Icon: Arrow down" class="noborder"></a></td>
<td>&nbsp;&nbsp;</td>
<td class="valignt">Начните проверку носителя <b>нажатием на кнопку "Проверить"</b>.</td>
</tr>
</table>

<?php begin_howto_shot("Проверка носителя.","good-cd.png", ""); ?>
<b>Наблюдайте за процессом проверки.</b>
Пока идет проверка, больше ничего не делайте в компьютере.
Открывание других программ или работа с ними, а также перемещение окон могут повлиять на результаты проверки.
<?php end_howto_shot(); ?>
<p>

<hr>

<a href="howtos13.php">Объяснение результатов...</a>


<!-- do not change below -->

<?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>
