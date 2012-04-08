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

<h3 class="top">The linear reading strategy</h3>

dvdisaster contains two different reading strategies.<p>

<b>The linear reading strategy is recommended for:</b><p>
<ul>
<li><a href="howtos23.php">creating images</a> from undamaged media, e.g. to generate the error correction file</li>
<li><a href="howtos12.php">scanning the medium</a> for reading speed and read errors</li>
</ul>

<b>The <a href="qa35.php">adaptive reading strategy</a> is recommended for:</b><p>
<ul>
<li> <a href="howtos42.php">extracting data</a> from damaged media
</li>
</ul>

<pre> </pre>

<b>Properties of the linear reading strategy.</b><p>

<?php begin_screen_shot("Weak CD","weak-cd.png"); ?>
Optical media are organized into sectors which are 
continously numbered beginning with zero. Each sector contains 2048 bytes of data.<p>

The linear reading strategy reads the medium from the start (sector 0)
until the end (last sector). The reading speed is shown graphically to provide
information about the <a href="#quality">medium quality</a>:
<?php end_screen_shot(); ?><p>

<pre> </pre>


<a name="configure"></a>
<b>Configuration.</b><p>

<b>Number of sectors to skip after a read error.</b>
Reading attempts for defective sectors are slow and may wear out the drive mechanics
under unfavourable conditions. A series of read errors, spread over a continous sector
range, is more common than single spot defects. Therefore a 
<a href="howtos11.php#read_attempts"> configuration option</a> exists so that a certain number
of sectors will be skipped after the occurance of a read error. The skipped sectors are
assumed to be defective without further reading attempts. 
Some guide lines for selecting the number of skipped sectors are:<p>

<ul>
<li>Skipping a large number of sectors (e.g. <b>1024</b>) gives a quick overview of the 
degree of damage, but will usually not collect enough data for repairing the medium image.
<p></li> 
<li>Smaller values like <b>16, 32 or 64</b> are a good trade-off:
The processing time will be considerably shortened, but still enough data for repairing
the image will be collected.<p></li>
</ul>

On DVD media read errors do usually extend over at least 16 sectors for technical
reasons. Therefore a sector skip less than 16 is not recommended for DVD media.
<p>

<a name="range"></a>
<b>Limiting the reading range.</b>
Reading can be
<a href="howtos11.php#image">limited to a given range of sectors</a> in the
"Image" preferences tab. This comes in handy
during multiple read attempts of damaged media.

<pre> </pre>

<a name="quality"></a>
<b>Estimating media quality.</b><p>

<a name="error"></a>
<b>The speed curve.</b>
Most drives will slow down while reading medium areas which are in bad condition:
<ul>
<li>
Drops in the reading speed can be a warning for an imminent medium failure.
</li>
<li>
However some drives will read with full speed until the bitter end.
With such drives media deterioration may not show up in the reading curve
until actual read errors occur.
</li>
</ul><p>

The reading curve is most accurate when using the
<a href="howtos10.php"> "Scan"</a> function. During the
<a href="howtos23.php">"Read"</a> operation the read data will be written to
the hard drive at the same time, which may cause irregularities in the reading
curve depending on the operating system and hardware used.<p>

<b>Read errors.</b>
Read errors cause <a href="howtos13.php#defective">red markings in the spiral</a> or respective
messages at the command line. 
This means that the medium could not be
read at these places during the current reading pass:
<ul>
<li>The medium is most likely defective.</li>
<li>The image should be 
<a href="howtos40.php">repaired</a> as soon as possible and then be transferred
to a new medium.</li>
</ul>

<!-- do not change below -->

<?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>
