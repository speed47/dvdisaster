<?php
# dvdisaster: Russian homepage translation
# Copyright (C) 2007-2012 Igor Gorbounov
#
# UTF-8 trigger: äöüß 
#
# Include our PHP sub routines, then call begin_page()
# to start the HTML page, insert the header, 
# navigation and news if appropriate.

require("../include/dvdisaster.php");
begin_page();
?>

<!-- Insert actual page content below -->

<h3 class="top"><a name="top">Технические вопросы</a></h3>

<a href="#nls">2.1 На какие языки переведена программа?</a><p>
<a href="#media">2.2 Какие типы носителей поддерживаются?</a><p>
<a href="#filesystem">2.3 Какие файловые системы поддерживаются?</a><p>

<hr><p>

<b><a name="nls">2.1 На какие языки переведена программа?</a></b><p>

Текущая версия dvdisaster содержит экранные тексты на следующих языках:<p>

<table>
<tr><td>&nbsp;&nbsp;&nbsp;</td><td>Чешский</td><td>--</td><td>полный</td></tr>
<tr><td></td><td>Английский</td><td>--</td><td>полный</td></tr>
<tr><td>&nbsp;&nbsp;&nbsp;</td><td>Немецкий</td><td>--</td><td>полный</td></tr>
<tr><td>&nbsp;&nbsp;&nbsp;</td><td>Итальянский</td><td>--</td><td>до версии 0.65</td></tr>
<tr><td>&nbsp;&nbsp;&nbsp;</td><td>Русский</td><td>--</td><td>полный</td></tr>
<tr><td>&nbsp;&nbsp;&nbsp;</td><td>Португальский</td><td>--</td><td>полный<td></tr>
<tr><td>&nbsp;&nbsp;&nbsp;</td><td>Шведский</td><td>--</td><td>до версии 0.70</td></tr>
</table><p>

Переводчики для других языков - добро пожаловать!<p>

dvdisaster автоматически получает языковые настройки из операционной системы.
Если локальный язык еще не поддерживается, то будет использоваться английский текст. 
С помощью переменных окружения можно выбрать другие языки.<p>

Пример для оболочки bash и немецкого языка:

<pre>export LANG=de_DE</pre>

Если специальные символы, такие как немецкие умлауты, отображаются неправильно,
попробуйте следующее:<p>

<tt>export OUTPUT_CHARSET=iso-8859-1</tt> (X11, XTerm)

<div class="talignr"><a href="#top">&uarr;</a></div>


<b><a name="media">2.2 Какие типы носителей поддерживаются?</a></b><p>

dvdisaster поддерживает (пере-)записываемые CD и DVD. <br>
Носители с многосессионными записями или с защитой от копирования <i>не</i> могут быть использованы.<p>

Пригодные носители по типу:<p>

<b>CD-R, CD-RW</b><p>

<ul>
 <li>поддерживаются только Data CD.</li>
</ul>

<b>DVD-R, DVD+R</b><p>

<ul>
<li>Дополнительные ограничения неизвестны.</li>
</ul>

<b>DVD-R DL, DVD+R DL (два слоя)</b>
<ul>
<li>Привод должен быть способен <a href="qa20.php#dvdrom">идентифицировать тип носителя</a>.
Обычно это имеет место только в тех случаях, когда приводы могут также и писать на двухслойный носитель.</li>
</ul>

<b>DVD-RW, DVD+RW</b><p>

<ul>
<li>Некоторые приводы сообщают неправильные <a href="qa20.php#rw">размеры образов</a>.<br>
Выход из положения: определяйте размер образа из файловой системы ISO/UDF или файла ECC/RS02.
</li></ul>

<b>DVD-RAM</b><p>
<ul>
<li>Пригодны только, если записывать с помощью ISO/UDF образов, как DVD-R/-RW;</li>
<li>Не поддерживаются, если используются как извлекаемый носитель / для пакетной записи.</li>
<li>То же самое и с распознаванием <a href="qa20.php#rw">размера образа</a>,
как уже упоминалось выше.</li>
</ul>

<b>Непригодные типы</b> (образ нельзя извлечь):<p>
DVD-ROM (печатаемые DVD), CD-Audio и CD-Video.

<div class="talignr"><a href="#top">&uarr;</a></div><p>


<b><a name="filesystem">2.3 Какие файловые системы поддерживаются?</a></b><p>

dvdisaster работает исключительно на уровне образа,
доступ к которому осуществляется посекторно.
Это означает, что не имеет значения, под какую файловую систему сформатирован носитель.<p>

Поскольку dvdisaster как не знает, так и не использует структуру файловой системы,
он не может исправлять логические ошибки на уровне файловой системы.
Он не может восстанавливать потерянные или удаленные файлы.
<div class="talignr"><a href="#top">&uarr;</a></div><p>

<!-- do not change below -->

<?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>
