<?php
# dvdisaster: Russian homepage translation
# Copyright (C) 2007-2010 Igor Gorbounov
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

news_item("03.10.2011", "dvdisaster 0.72.3 released", "
The \"Verify\" function hangs when working on RS01 error
correction files which are larger than 2GB (the error correction
files are correctly generated though).
Volodymyr Bychkoviak discovered the problem and sent in
a bug fix.");

news_item("31.10.2010", "dvdisaster 0.72.2 released", "
This version introduces a workaround which prevents parallel SCSI
adapters from freezing under Linux. RS03 images from 0.79.x
versions will no longer cause spurious error messages.");

news_item("10.08.2009", "Project represention on SourceForge may be outdated", "
Due to the recent functionality changes on SourceForge we are unable
to upload the News, Downloads and CVS contents in time. Please refer
to the respective sections on this site (http://dvdisaster.net) instead.
");

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
