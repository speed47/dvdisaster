# dvdisaster provides additional ECC protection for optical media

It can loosely be compared to *par2* files, but the protection works at the *iso* level instead of working at the file level. This way, even if metadata from the optical media filesystem is damaged, dvdisaster can still work flawlessly.

This version of dvdisaster supports the following platforms:
Linux, FreeBSD, NetBSD on x86, PowerPC, Sparc, and Windows.

Three protection codecs are supported, they're quickly detailed below. Please see the documentation/manual.pdf for more information about these, and everything else.

# The codecs

## RS01

RS01 creates error correction files which are stored separately from the image they
belong to. The artefact is an **ecc** file, which must be stored on another media than the one we're protecting.

## RS02

RS02 creates error correction data which is added to the medium to protect, we call this *augmenting* the image we're protecting. Damaged sectors in the error correction information reduce the data recovering capacity, but do not make recovery impossible - a second medium for keeping or protecting the error correction
information is not required.

## RS03

RS03 is a further development of RS01 and RS02. It can create both error correction files and
augmented images, with the following added features:

- RS03 can distribute work over multiple processor cores and is therefore much faster than
RS01/RS02 on modern hardware.
- RS03 error correction files are - contrary to RS01 - robust against damage. This should
not delude you into careless handling of your error correction files though - the disadvantages
of reading at the filesystem level are still valid.
- RS03 augmented images do not require so-called master blocks holding important in-
formation. This makes RS03 a bit more robust, but also more restrictive: The augmented
image must completely fill the medium now while the size of augmented images can be
freely chosen in RS02.
The changes for parallel computation and higher robustness make RS03 a bit less space efficient,
e.g. RS03 error correction data has slighly less error correction capacity than its RS01/RS02
counterparts on images with equal size.

# Unofficial version

The last upstream version by Carsten Gnörlich is dated 2017, and could be found on the [official](https://web.archive.org/web/20180428070843/http://dvdisaster.net/en/index.html) [website](https://web.archive.org/web/20180509154525/http://dvdisaster.org/en/index.html) which is [now](http://www.dvdisaster.net) [down](http://www.dvdisaster.org). The original source code [repository](https://sourceforge.net/projects/dvdisaster/files/dvdisaster) doesn't have it, but [Debian sources](https://sources.debian.org/src/dvdisaster/) does, thanks to the maintainer there.
The original README has been left untouched in this repository.
This version is built on top of the latest upstream version, with the following notable enhancements:

- Most Debian patches have been applied, those specific to Debian have been omitted
- Windows build supported again, it was dropped upstream a few versions back
- A Linux CLI-only version is now supported, without depending on gtk (`CLI_ONLY=1 ./configure && make clean && make -j4`)
- Regression tests confirmed working on Linux64 (normal and CLI-only), Windows32 and Windows64
- Added pre-defined sizes for BD-R Triple Layer (100GB), BD-R Quadruple Layer (128GB)

# Rationale

Even if the optical media era is sunsetting now, and has been for a few years, it's still of some value for off-site backups. In any case, we still have media in our hands that we want to be able to repair, should it be damaged, during the next years/decades. Repairing is actually pretty much the very reason of dvdisaster existence (as long as parity data has been added, of course).
The idea of this unofficial version is to ensure dvdisaster doesn't get hard to find, use or compile, should upstream development never resume (we hope it does!).
This is also why precompiled Windows binaries and a precompiled static CLI-only Linux version are available here.

# Screenshots

### Scanning a damaged CD under Windows

![dvdisaster_damaged_cd](https://user-images.githubusercontent.com/218502/91434313-edaaf880-e864-11ea-8e41-7b58a1e97a70.PNG)
