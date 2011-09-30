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
begin_page();
?>

<!--- Insert actual page content below --->

<h3>Installation of the source packages</h3>

dvdisaster uses the common <tt>./configure; make; make install</tt> mechanism
for installation from source. Please see the file <tt>INSTALL</tt> in the
source distribution for further information.
<pre> </pre>

<a name="mac"></a>
<h3>Installation of the binary for Mac OS X</h3>
The ZIP archive contains an application bundle made for Mac OS X 10.5
on x86 processors. Unpack the archive at the desired place and click
"dvdisaster.app" to start the program.
dvdisaster is able to run under Mac OS X 10.4 and with PowerPC processors;
however in these cases you need to build your own version from the
source code.<p>

<i>Please note that the graphical user interface is still a bit rough around
the edges.</i> The GTK+ user interface toolkit is still in an early
development state for Mac OS X. The user interface may be sloppy or
freeze for brief periods. There may be glitches in the graphical output.
You can minimize these effects by not tinkering with the dvdisaster window
while any actions are in progress. Especially avoid resizing the window
and sending it to the dock.<p>

During the 0.73 development cycle some workarounds for these effects may
be implemented; otherwise the plan is to wait for improved versions
of the GTK+ toolkit.<p>

There are no plans for developing a dvdisaster version not depending on GTK+
as this would mean to completely re-write the program for Quartz.
Similar reasons hold for the Windows version btw ;-)
<pre> </pre>

<a name="win"></a>
<h3>Installation of the binary for Windows</h3>

Please install the Windows version by executing the setup binary
(e.g. <?php echo ${pkgname}?>-setup.exe) and follow the 
installation dialog. <p>

<b>Caution:</b> dvdisaster can NOT be installed from
manually unpacking the setup.exe program, or by copying it from
an already existing installation. Doing so may result in strange
errors which seem to be totally unrelated to an improper installation.

<!--- do not change below --->

<?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>
