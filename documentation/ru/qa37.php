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
begin_page();
?>

<!-- Insert actual page content below -->

<h3 class="top">Hints for storing the error correction files</h3>

Currently there are few exchangeable media technologies
which can be a cost-effective alternative to the various CD/DVD/BD formats.
So you will probably not only use CD/DVD/BD for data archival, but store
the respective error correction files on CD, DVD or BD as well.<p>

There is nothing wrong with that, but bear in mind that your archived data
and the error correction files are stored on media with the same degree of
reliability. When read errors occur on the archived data, be prepared that
the disc with the respective error correction file might have aged beyond
full readability, too.<p>

Therefore it is important to protect your error correction files with the same
care as your other data<sup><a href="#footnote1">*)</a></sup>. 
This is best achieved by integrating the error correction
files into your normal data backup scheme. Here are two ideas:<p>

<b>1. Storing the error correction files on dedicated media:</b><p>

If you decide to store error correction files on separate media, it is
<a href="qa32.php#eccfile">important</a> to protect those media
with dvdisaster as well. To avoid a never-ending chain
(error correction files for media of error correction files for ...),
try the following:<p>

Lets assume that five error correction files can be stored at each medium.
Write the first five error correction files to the first medium and create
another error correction file for that medium. Now save that error correction
file together with four other error correction files on the second medium.
If you continue that way, all error correction files except for those from the
last medium (which may still be kept on hard disc) are protected by dvdisaster.<p>

<b>2. Putting the error correction file on the next medium of a series:</b><p>

If you do not fill your media to the max (e.g. with less than 4GB for a single layered DVD), you can store the error correction file of one medium
on the succeeding medium within a series.<p>

<pre> </pre>
<table width="50%"><tr><td><hr></td></tr></table>

<span class="fs">
<a name="footnote1"><sup>*)</sup></a> 
You might also choose an <a href="howtos30.php">augmented image</a> using RS02
or RS03 instead of creating an error correction file.
</span>

<!-- do not change below -->

<?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>
