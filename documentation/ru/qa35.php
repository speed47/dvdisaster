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
require("../include/screenshot.php");

begin_page();
?>

<!-- Insert actual page content below -->

<h3 class="top">The adaptive reading strategy</h3>

dvdisaster contains two different reading strategies.<p>

<b>The adaptive reading strategy is recommended for:</b><p>
<ul>
<li><a href="howtos42.php">extracting data</a> from damaged media
</li>
</ul>

<b>The <a href="qa34.php">linear reading strategy</a> is recommended for:</b><p>
<ul>
<li><a href="howtos23.php">creating images</a> from undamaged media, e.g. to generate the error correction file</li>
<li><a href="howtos12.php">scanning the medium</a> for reading speed and read errors</li>
</ul>

<pre> </pre>

<b>Properties of the adaptive reading strategy.</b><p>

The adaptive reading strategy uses a "divide and conquer" approach for locating
still readable portions of a damaged medium. The strategy is based upon an article
published by Harald Bögeholz in c't-Magazin 16/2005 where it was published together
with the program <i>h2cdimage</i>:

<ol>
<li> 
At the beginning the medium is considered as a single unread interval. Reading begins
with sector zero.<p>
</li>
<li>
The reading process continues sequentially unless either the end of the current interval
or a read error is encountered.<p>
</li>
<li>
The reading process is terminated if either (3a) sufficient sectors for a successful
error correction have been read or (3b) no unreadable intervals exceeding a given size
remain.
<p>
</li>
<li>
Otherwise the largest remaining unread interval will be determined. Reading continues in the middle
(e.g. second half) of this interval; 
the first half of this interval is kept for a later reading pass.
</li>
</ol>

<?php begin_screen_shot("Adaptive reading in progress","adaptive-progress.png"); ?>
The termination criterium (3a) is especially efficient: Reading will stop as soon
as enough sectors have been collected for a successful image recovery using the
error correction file. This can reduce the reading time by as much as 90 percent
compared with a full read attempt, but does of course only work when 
an error correction file is available.<p>
<?php end_screen_shot(); ?><p>

<pre> </pre>

<a name="configure"></a>
<b>Configuration</b><p>

<b>Error correction file.</b> 
Adaptive reading works best when error correction data is available. 
Obviously the error correction data must have been 
<a href="howtos21.php">created</a>
at a time where the medium was still fully readable. To use a error correction file
during adaptive reading, 
<a href="howtos42.php#select_eccfile">enter its name</a> before starting the reading process.<p>

<b>Limiting the adaptive reading range.</b> Reading can be 
<a href="howtos11.php#image">limited</a> to a part of the medium.

This is not recommended when error correction data is used since the limit
might prevent sectors from being read which are required 
for a succesful error correction. 
If no error correction data is available, 
limiting the reading range might be helpful
during multiple reading attempts.<p>

<b>Early reading termination.</b>
If no error correction data is available, adaptive reading will stop when no unread
intervals
<a href="howtos41.php#reading_attempts">larger than a selectable size</a> remain.<p>

The termination value should not be smaller than 128.
Otherwise the laser head will have to carry out lots of repositioning moves during the
final phase of the reading process. This will negatively affect both the life expectancy
of the drive and its reading capability. A better approach is to stop adaptive 
reading earlier and then try reading the remaining sectors with an additional
<a href="qa34.php">linear reading</a> pass.

<!-- do not change below -->

<?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>
