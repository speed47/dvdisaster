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
require("../include/footnote.php");
begin_page();

howto_headline("Creating error correction files", "Archival", "images/create-icon.png");
?>

<!-- Insert actual page content below -->
<h3 class="top">Tips for archival of error correction files</h3>

CD/DVD/BD are currently among the most cost-effective exchangeable
mass storage media. Therefore you are probably considering them 
for storing error correction files.<p>

Nothing is wrong with doing so, but be aware that your data and protective
error correction files are kept on media with equal reliability.
When you encounter read errors on a data medium 
it is likely that the medium containing the respective error correction files
has also gone defective. After all both media have been written at the same time,
and they have the same aging characteristics.
<p>

<table width="100%"><tr><td class="vsep"></td>
<td>&nbsp;</td>
<td>This might come at a surprise, but it can not be guaranteed that an
error correction file remains usable when it is stored on a defective medium -
here is a <a href="qa32.php#file">explanation of the technical background</a>.
</td></tr></table><p>

Therefore it is important to protect error correction files just as if 
they were normal data. To be more specific,  
the medium containing error correction files must be protected with
error correction data as well. Here are two ways of doing this:


<ol>
<li>Storing error correction files on separate media:<p>

Use additional media just for keeping the error correction files.
If you use no more than 80% per medium for error correction files
it can 
be <a href="howtos30.php">augmented with error correction data</a>. 
This allows you to recover the medium if you run into problems
reading the error correction files at a later time.<p></li>

<li>Storing error correction files on the next medium in sequence:<p>

Maybe you are using media for an incremental backup strategy. In that 
case you could collect files until the first medium can be filled.
Write that medium as usual and create an error correction file for it.
Include that error correction file into the backup set which will go
onto the second medium. When the second medium has been written, write
the error correction file for it onto the third medium and so on. 
This way all media in the chain are protected with error correction 
files (with the ecc file for the last medium residing on hard disk until
another medium is written).<p>

Of course Murphys Law may strike and result in all media of the chain
becoming defective. In that case you need to recover all media, starting
with the most recent one ;-)
</li>
</ol>

<!-- do not change below -->

<?php

# end_page() adds the footer line and closes the HTML properly.

end_page();
?>
