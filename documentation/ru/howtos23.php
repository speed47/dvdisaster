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

$way=$_GET["way"];
switch($way)
{  case 1: $action="с носителя"; break;
   case 2: $action="с ISO-образа"; break;
   default: $action="Прохождение"; break;
}

howto_headline("Создание файлов для исправления ошибок", $action, "images/create-icon.png");
?>

<!--- Insert actual page content below --->

Убедитесь, что dvdisaster настроен, как описано в разделе
<a href="howtos22.php">основных настроек</a>, так как выбор некоторых параметров
может привести к созданию недостаточно оптимальных данных для исправления ошибок.
<p>

Следующие шаги зависят от источника данных для исправления ошибок. Сделайте выбор из
этих двух способов:<p>

<table width="100%" cellspacing="5">
<tr>

<?php
$expand=$_GET["expand"];
if($expand=="") $expand=0;
echo "<td><a href=\"howtos23.php?way=1&expand=$expand\"><img src=\"../images/good-cd.png\" border=\"0\"></a></td>\n";
echo "<td><a href=\"howtos23.php?way=1&expand=$expand\">Создать файл для исправления ошибок с CD/DVD/BD-носителя</a></td>\n";
echo "<td><a href=\"howtos23.php?way=2&expand=$expand\"><img src=\"../images/good-image.png\" border=\"0\"></a></td>\n";
echo "<td><a href=\"howtos23.php?way=2&expand=$expand\">Создать файл для исправления ошибок с ISO-образа</a></td>\n";
?>

</tr>
</table>

<?php
if($way==1){
?>
<hr><p>

<table>
<tr>
<td width="200px" align="center"><img src="../images/slot-in.png">
<br><img src="../images/down-arrow.png"></td>
<td>&nbsp;&nbsp;</td>
<td valign="top"><b>Вставьте носитель, который нужно прочитать, в привод,</b>
непосредственно подключенный к вашему компьютеру. 
Нельзя использовать сетевые приводы, виртуальные приводы и приводы в виртуальных машинах.
</td>
</tr>
</table>

<table>
<tr>
<td width="200px" align="center"><img src="../images/winbrowser.png">
<br><img src="../images/down-arrow.png"></td>
<td>&nbsp;&nbsp;</td>
<td valign="top"><b>Закройте все окна</b>, которые могут быть открыты вашей операционной системой 
для просмотра или запуска содержимого носителя. 
Подождите, пока привод не распознает носитель и вращение носителя не замедлится.
</td>
</tr>
</table>

<table>
<tr>
<td width="200px" align="center"><a href="howtosa1.php">
<img src="../images/select-drive.png" border="0">
<br><img src="../images/down-arrow.png" border="0"></a></td>
<td>&nbsp;&nbsp;</td>
<td valign="top"><b>Выберите привод, содержащий носитель,</b>
в выпадающем меню dvdisaster.
</tr>
</table>

<table>
<tr>
<td width="200px" align="center">
<a href="howtosa2.php">
<img src="../images/select-image.png" border="0">
<br><img src="../images/down-arrow.png" border="0"></a></td>
<td>&nbsp;&nbsp;</td>
<td valign="top"><b>Выберите каталог и имя файла</b> 
для хранения ISO-образа.</td>
</tr>
</table>

<table>
<tr>
<td width="200px" align="center">
<a href="howtosa4.php">
<img src="images/read-icon.png" border="0">
<br><img src="../images/down-arrow.png" border="0"></a></td>
<td>&nbsp;&nbsp;</td>
<td valign="top"><b>Создайте ISO-образ</b> носителя, нажав на
кнопку "Читать".</td>
</tr>
</table>

<?php begin_howto_shot("Чтение образа.","watch-read1.png", "down-arrow.png"); ?>
<b>Наблюдайте за процессом чтения.</b>
Подождите, пока носитель не будет полностью прочитан. Если окажется. что носитель
содержит поврежденные секторы, будет невозможно создать данные для исправления ошибок.
<?php end_howto_shot(); 
 }  /* end of if($way == 1) */

if($way == 2) {
?>
<hr><p>

<table>
<tr>
<td width="200px" align="center">
<a href="howtosa2.php">
<img src="../images/select-image.png" border="0">
<br><img src="../images/down-arrow.png" border="0"></a></td>
<td>&nbsp;&nbsp;</td>
<td valign="top"><b>Выберите каталог и имя ISO-образа</b>,
для которого вы хотите создать данные для исправления ошибок.
(Предполагается, что ISO-образ создан какими-либо другими средствами,
например, с помощью вашей программы для создания CD/DVD/BD.)</td>
</tr>
</table>
<?php
}

if($way != 0) {
?>
<table>
<tr>
<td width="200px" align="center">
<a href="howtosa3.php">
<img src="../images/select-ecc.png" border="0">
<br><img src="../images/down-arrow.png" border="0"></a></td>
<td>&nbsp;&nbsp;</td>
<td valign="top"><b>Выберите каталог и имя</b> 
для сохранения файла с данными для исправления данных.</td>
</tr>
</table>

<table>
<tr>
<td width="200px" align="center">
<a href="howtosa4.php">
<img src="images/create-icon.png" border="0">
<br><img src="../images/down-arrow.png" border="0"></a></td>
<td>&nbsp;&nbsp;</td>
<td valign="top"><b>Создайте файл для исправления ошибок</b>,
нажав кнопку "Создать".</td>
</tr>
</table>

<?php begin_howto_shot("Создание файла для исправления ошибок.","watch-create.png", "down-fork-arrow.png"); ?>
<b>Ждите, пока не закончится процесс создания.</b>
Это может занять некоторое время в зависимости от размера образа и выбранной избыточности.
Например, создание файла для исправления ошибок с использованием "нормальной" избыточности
займет приблизительно 5 минут для DVD-образа размером 4ГБ на стандартном оборудовании.
<?php end_howto_shot(); ?>

<table>
<tr>
<td width="200px"align="center">
<img src="../images/old-image.png" border="0" align="center">
&nbsp;&nbsp;&nbsp;
<img src="../images/ecc.png" border="0" align="center"></a></td>
<td>&nbsp;&nbsp;</td>
<td valign="top"><b>Заключение.</b> Теперь можно удалить файл образа. Однако, необходимо 
хранить файл для исправления ошибок и, что гораздо важнее, защищать его от повреждений. На следующей странице есть несколько предложений по
<a href="howtos24.php">архивации файла для исправления ошибок</a>.
</td>
</tr>
</table>

<p>
<a href="howtos24.php">Архивация файла для исправления ошибок...</a>
<?php
} /* end of if($way != 0) */
?>
<!--- do not change below --->

<?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>
