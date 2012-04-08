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

<h3 class="top">Technical properties of the error correction</h3>

This page outlines the basic ideas behind dvdisaster
so that you can see for yourself whether
it meets your demands on data safety.
If in doubt, you should not use dvdisaster
or deploy additional data backup strategies.<p>

<b>Method of error correction.</b> &nbsp; dvdisaster uses a
<a href="http://en.wikipedia.org/wiki/Reed-Solomon_error_correction">Reed-Solomon</a> code
together with an error correction algorithm optimized for the treatment of erasures.
The implementation draws a lot of inspiration and program code from the excellent
<a href="http://www.ka9q.net/code/fec/">Reed-Solomon library</a> written by
<a href="http://www.ka9q.net/">Phil Karn</a>.

<p>

Using the <a href="howtos22.php#ecc">standard settings</a> of an 
<a href="howtos20.php">error correction file</a> 
223 medium sectors are combined into one error correction code ("ECC") block.
Medium read errors are regarded as "erasures"; therefore a maximum 
of 32 bad medium sectors<sup><a href="#footnote1">*)</a></sup>
are correctable per ECC block. <p>

The 223 sectors are selected so that they are evenly distributed over the medium surface.
Therefore large contigous areas of defective sectors can be corrected before the threshold
of 32 defects per ECC block<sup><a href="#footnote1">*)</a></sup>
is reached. This kind of error pattern is especially
common for an aging medium where the outer area is starting to degenerate, 
and for scratches along the data spiral. <p>

On the other hand, radial or diagonal scratches are expected to be correctable in the
CD/DVD drive itself. If not, the employed error correction strategy performs rather neutral
in these cases (neither especially good nor extraordinary bad). <p>

<b>Limits of error correction.</b> &nbsp; In the wost case, 
33 defective sectors<sup><a href="#footnote1">*)</a></sup> are sufficient
to prevent a full data restoration. However to achieve this effect, the errors have to
be distributed over the medium in such a way that they occur in the same ECC block - such a pattern
is very unlikely.<br>
Empirical tests have shown that on aging media about 10% of the overall sector count
may become defective before
the threshold of 33 defects per ECC block<sup><a href="#footnote1">*)</a></sup>
 is reached. <br>
Scratches will cause the threshold to be reached earlier, 
therefore it is recommended to visually check the media in regular intervals.
Media with read errors caused by scratches should be replaced immediately.<p>

<b>Hardware limits.</b> &nbsp; Most drives will not recognize media when the lead-in area
before the first sector
(near the center hole) is damaged. In such cases, dvdisaster will not be able to recover
any content from the media. <p>

It is <i>not feasible</i> to enhance the reliability of poor quality media by using
dvdisaster. Cheap media can decay within a few days to an extent which will exceed
the capabilities of the error correction code. <p>

<pre> </pre>
<table width="50%"><tr><td><hr></td></tr></table>

<span class="fs">
<a name="footnote1"><sup>*)</sup></a> 
The given threshold of 32 correctable errors per ECC block is the standard setting.
It is possible to <a href="howtos22.php#ecc">select other values</a>
for higher or lower error correction capabilities. When 
<a href="howtos30.php">error correction data is put directly on the medium</a>, the number of correctable
errors depends on the free space on the medium.
</span>

<!-- do not change below -->

<?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>
