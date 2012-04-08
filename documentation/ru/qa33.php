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

<h3 class="top">The RS01, RS02 and RS03 methods</h3>

dvdisaster contains three error correction methods.
RS01 and RS02 are the existing and proven methods while RS03 is still
under development.<p>

<b>Comparison of the methods.</b>

All three methods build on the same
<a href="qa31.php">Reed-Solomon</a> error correction.
They calculate error correction information for ISO images which
is used to recover unreadable sectors if the disc becomes damaged afterwards.<p>

The methods differ in the way the error correction information is stored:<p>

<ul>
<li>
<a name="file"> </a>
RS01 creates <b>error correction files</b> which are stored separately from 
the image they belong to. Since data protection at the
<a href="qa32.php#file">file level</a> is difficult,
error correction files must be stored on media which are protected
against data loss by dvdisaster, too.<p></li>

<li>
<a name="image"> </a>
To apply the RS02 method an image is first created on hard disc using a
CD/DVD writing software. Before the image is written on the medium,
dvdisaster is used to <b>augment the image</b> with error correction data.
Therefore the data to be protected and the error correction information
are located at the same medium. Damaged sectors in the error correction
information reduce the data recovery capacity, but do not make recovery
impossible - a second medium for keeping or protecting the error correction
information is not required.<p></li>
</ul>

RS03 is a further development of RS01 and RS02. It can create both
error correction files and augmented images:
<ul>
<li>RS03 can distribute work over multiple processor cores and is therefore
much faster than RS01/RS02 on modern hardware.</li>
<li>RS03 <b>error correction files</b> are - contrary to RS01 - robust against
damage. This should not delude you into careless handling of your error
correction files though - the disadvantages of <a href="qa32.php#file">reading
at the filesystem level</a> are still valid.</li>
<li>RS03 <b>augmented images</b> do not require so-called master blocks
holding important information. This makes RS03 a bit more robust, but also
more restrictive: The augmented image must completely fill the medium now
while the size of augmented image can be freely chosen in RS02.
</li>
</ul>
The changes for parallel computation and higher robustness make RS03 a bit less
space efficient, e.g. RS03 error correction data has slighly less error
correction capacity than its RS01/RS02 counterpart with equal size.<p>

<a name="table"> </a>
<b>Comparison of error correction storage.</b><p>
The following table summarizes the differences between 
error correction files (RS01, RS03) and augmented images (RS02, RS03):<p>

<table width="100%" border="1" cellspacing="0" cellpadding="5">
<tr>
<td class="w50p"><i>Error correction files</i></td>
<td class="w50p"><i>Image augmented with error correction data</i></td>
</tr>
<tr valign="top">
<td> 
any possible redundancy can be chosen</td>
<td> redundancy is limited by free space on medium<br>
(= medium capacity - size of data image)</td>
</tr>

<tr valign="top">
<td>already effective at 15% redundancy
since error correction files are required to be free of damage</td>
<td>requires more redundancy (recommended: 20-30%)
to compensate defects in the error correction data</td> 
</tr>

<tr valign="top">
<td>medium can be completely filled with data</td>
<td>usable medium capacity is reduced by amount of error correction data</td> 
</tr>

<tr valign="top">
<td> can be created for already existing media</td>
<td> only applicable before writing the new medium since
the image must be augmented with error correction information in advance</td>
</tr>

<tr valign="top">
<td> separately storing the error correction file from user data
strengthens data protection</td>
<td> common storage of user data and error correction data may reduce error
correction capacity</td>
</tr>

<tr valign="top">
<td>Mapping between error correction files and media must be kept.
Error correction files must be protected against damage.</td>
<td>Easy one-medium solution; error correction information
needs not to be cataloged or explicitly protected.</td></tr>

<tr valign="top">
<td> no compatibilty issues with play-back units</td>
<td> media with augmented images may not play correctly on all units</td>
</tr>
</table><p>

<!-- do not change below -->

<?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>
