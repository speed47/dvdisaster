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

?>

<!--- Insert actual page content below --->

<h3>Выбор файла образа</h3>

Файл образа содержит данные со всех секторов носителя, в том числе информацию
о читаемости сектора. dvdisaster работает с файлами образов, потому что они
хранятся на жестком диске, который делает гораздо быстрее некоторые режимы произвольного доступа.
Применение такого произвольного доступа к приводам CD/DVD/BD значительно замедлит их работу и, в конечном итоге, приведет к их износу (файлы образов создаются посредством
последовательного доступа, с которым, в отличие от произвольного доступа, приводы эффективно справляются).
По умолчанию расширение имени файлов для образов ".iso".<p>

<?php begin_screen_shot("Выбор файла образа","dialog-iso-full.png"); ?>
Есть два способа выбора файла образа:
<ul>
<li>используя <a href="#filechooser">диалог выбора файла</a> (кнопка, отмеченная зеленым), или</li>
<li>непосредственно вводя местоположение файла (текстовое поле, отмеченное голубым).</li><p>
</ul>
Прямой ввод полезен, когда ведется обработка нескольких файлов
в одном и том же каталоге. В этом случае просто измените имя файлв в текстовом поле.
<p>
<?php end_screen_shot(); ?>

<? require("howtos_winfile.php"); ?>


<!--- do not change below --->

<?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>
