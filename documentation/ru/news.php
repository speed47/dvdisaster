<?php
# dvdisaster: Russian homepage translation
# Copyright (C) 2007-2009 Igor Gorbounov
#
# UTF-8 trigger: äöüß 
#
# Include our PHP sub routines, then call begin_page()
# to start the HTML page, insert the header, 
# navigation and news if appropriate.

if($news_flash == 0) 
{  require("../include/dvdisaster.php");
   begin_page();
}

$news_counter = 0;

# The news page is different from the other pages;
# you must only use the news_*() functions below to create content.
# Do not insert plain HTML!

news_headline("Новости dvdisaster");

news_item("08.08.2009", "dvdisaster 0.72.1 released", "
Pablo Almeida provided a Portuguese translation for the screen text.
Added workaround to avoid Win XP freezing on certain CD-RW/drive pairs.<p>
");

news_item("04.07.2009", "выпущен dvdisaster 0.72", "
Это первая стабильная версия ветки 0.72.
Игорь Горбунов завершил перевод онлайн-документации на русский язык.
Устранены некоторые незначительные ошибки в первом кандидате на выпуск.<p>
");

news_item("11.04.2009", "выпущен dvdisaster 0.72.rc1", "
  Теперь доступен первый кандидат на выпуск из стабильной ветви 0.72.
  Среди новых возможностей - поддержка носителей Blu-Ray, низкоуровневое чтение и проверки на C2
  для CD, а также \"родная\" версия для Mac OS X.
  Дополнительную информацию см. здесь <a href=\"download.php\">download page</a>.
");

news_item("08.03.2009", "выпущен dvdisaster 0.71.28", "
  В этой версии исправляются падения из-за \"illegal instruction\" на x86-машинах,
  которые не поддерживают SSE2. Много незначительных исправлений перед кандидатом
  на выпуск.
");

news_item("18.01.2009", "выпущен dvdisaster 0.71.27", "
  Эта документация, наконец, завершена. Исправлены некоторые падения и несовместимости
  с необычными носителями и приводами. В этой версии произведен
  последний большой внутренний пересмотр, планировавшийся для версий 0.71.x, поэтому, если
  все будет хорошо, то следующей версией будет стабильный кандидат на выпуск.
");

if($news_flash == 0) 
   end_page();
?>
