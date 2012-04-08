<?php
# dvdisaster: English homepage translation
# Copyright (C) 2004-2012 Carsten Gnörlich
#
# UTF-8 trigger: äöüß 
?>

<a name="filechooser"></a>
<b>Notes for using the file chooser under Windows and Mac OS X</b><p>

dvdisaster uses the <a href="http://www.gtk.org">GTK+</a> user interface toolkit
which makes sure that it runs on many different operating systems. Since the
GTK+ file chooser behaves differently from its Windows and Mac OS X counterparts (which we
can't use), a small introduction is included further down this page.
Please note that creating the Windows or Mac OS X version would have required
a huge effort without GTK+ - we would probably just have concentrated 
on doing the GNU/Linux version instead ;-)<p>

<?php begin_screen_shot("Choosing files in the Windows version","filebrowser.png"); ?>

<b>Choosing existing files.</b>
The areas marked <span class="green">green</span> are used for choosing existing files.
Directories are picked and navigated in the left half of the dialog; the files
contained are shown and selectable in the right half.<p>

<b>Choosing a name and location for new files.</b> 
First choose a directory for the new file using the
<span class="green">green</span> area in the left half of the dialog.
Then enter the name for the new file in the 
text field marked <span class="blue">blue</span>. 
To double check that you are creating the file in the right place please review
the "Selection:" caption of the <span class="blue">blue</span> field; it contains the
drive letter and directory path under which the new file will be created.<p>

<b>Switching between partitions ("drive letters").</b>

The available partitions are listed in 
the <span class="yellow">yellow</span> marked area. If the currently selected drive
contains lots of subdirectories you might have to scroll down to see the
drive letters.<p>

<b>Going back to the parent directory.</b>
Click on the two dots (..) marked <span class="red">red</span>
to go back one directory level. All directories leading up to the current one
are contained in the drop down menu centered at the top of the dialog
(also marked <span class="red">red</span>). 
Note that this drop down menu is <i>not</i> used for switching drive letters;
please use the <span class="yellow">yellow</span> part of the selection for that.

<?php end_screen_shot(); ?>
