# :cd: What is dvdisaster?

**dvdisaster** provides additional [ECC](https://en.m.wikipedia.org/wiki/Error_correction_code) protection for optical media. If a medium gets damaged, dvdisaster can recover it as long as the amount of damage is smaller than the amount of ECC data you added to protect it.

It can loosely be compared to [.par2](https://en.m.wikipedia.org/wiki/Parchive) files, but the protection works at the *iso* level instead of working at the file level. This way, even if metadata from the optical medium filesystem is damaged, dvdisaster can still work flawlessly.

Please refer to the [PDF manual](documentation/user-manual/manual.pdf) for more information.

# :wrench: Unofficial version

The last upstream version by Carsten Gnörlich is dated 2017, and could be found on the [official](https://web.archive.org/web/20180428070843/http://dvdisaster.net/en/index.html) [website](https://web.archive.org/web/20180509154525/http://dvdisaster.org/en/index.html) which is [now](http://www.dvdisaster.net) [down](http://www.dvdisaster.org). The original source code [repository](https://sourceforge.net/projects/dvdisaster/files/dvdisaster) doesn't have it, but [Debian sources](https://sources.debian.org/src/dvdisaster/) does, thanks to the maintainer there.
The original [README](README) has been left untouched in this repository.
This version is built on top of the latest upstream version, with the following notable enhancements:

- Added pre-defined sizes for BD-R Triple Layer (100GB), BD-R Quadruple Layer (128GB)
- Added an option to use more space for ECC on BD-R
- Windows build supported again (it was dropped upstream a few versions back)
- A new CLI-only version, not depending on gtk (`./configure --with-cli-only && make clean && make -j4`)
- Non-regression tests on each code change, for Linux64 and Windows32/64, CLI and GUI versions
- Prebuilt binaries for Windows32, Windows64, Linux64 (static builds and AppImage builds), CLI and GUI versions
- Fixed a bunch of (minor) quirks, a few (minor) bugs, added a couple (minor) features

Please refer to the [CHANGELOG](CHANGELOG) for all the details.

:loudspeaker: This version will never break compatibility with upstream versions, the goal is to ensure an optical media protected by upstream dvdisaster will still be able to be repaired with this version 10+ years from now. Regression tests are here to ensure this is the case.

## :twisted_rightwards_arrows: Choose between 3 protection modes ("codecs")

For a more detailed explanation of the algorithms, please refer to the [codecs specification PDF](documentation/codecs.pdf).

:one: **RS01** creates error correction files which are stored separately from the image they belong to.
The artefact is an **ecc** file, which must be stored on another media than the one we're protecting.

:two: **RS02** creates error correction data which is added to the medium to protect, we call this *augmenting* the image we're protecting. Damaged sectors in the error correction information reduce the data recovering capacity, but do not make recovery impossible - a second medium for keeping or protecting the error correction
information is not required.

:three: **RS03** is a further development of RS01 and RS02. It can create both error correction files and
augmented images, with the following added features:

- RS03 can use multiple CPU cores and is therefore **much** faster than RS01/RS02 on modern hardware.
- RS03 error correction files are - contrary to RS01, and to a lesser extent RS02 - robust against damage.
- RS03 is more robust, but also more restrictive: The augmented image must completely fill the medium now while the size of augmented images can be freely chosen in RS02.
  The changes for parallel computation and higher robustness make RS03 a bit less space efficient,
  e.g. RS03 error correction data has slighly less (around -3%) error correction capacity than its RS01/RS02 counterparts on images with equal size.

Rough comparison table:

| Codecs           |              RS01              |              RS02              |              RS03              |
|------------------|--------------------------------|--------------------------------|--------------------------------|
| Robustness\*     | :star:                         | :star::star::star:             | :star::star::star::star::star: |
| Speed            | :star::star:                   | :star:                         | :star::star::star::star::star: |
| Space efficiency | :star::star::star::star::star: | :star::star::star::star::star: | :star::star::star::star:       |
| Augmented images | :x:                            | :heavy_check_mark:             | :heavy_check_mark:             |
| Separate files   | :heavy_check_mark:             | :x:                            | :heavy_check_mark:             |

\*Robustness against corruption of the dvdisaster-added ECC parts themselves

# :bulb: Rationale

Even if the optical media era is sunsetting now, and has been for a few years, it's still of some value for off-site backups. In any case, we still have media in our hands that we want to be able to repair, should it be damaged, during the next years/decades. Repairing is actually pretty much the very reason of dvdisaster existence (as long as parity data has been added, of course).
The idea of this unofficial version is to ensure dvdisaster doesn't get hard to find, use or compile, should upstream development never resume (we hope it does!).
This is also why precompiled Windows binaries and a precompiled static CLI-only Linux version are available here.

# :hammer: Compiling

See the [INSTALL](INSTALL) file. The [workflow file](.github/workflows/release.yml) that is used to automatically build binaries for each release can also help.

# :camera: Screenshots

### Scanning a damaged CD under Windows:

![dvdisaster_damaged_cd](https://user-images.githubusercontent.com/218502/123558682-e2cc3780-d797-11eb-8ad0-5247b2601656.PNG)

### Verifying the ECC correction data from the damaged image:

![dvdisaster_cd_verify](https://user-images.githubusercontent.com/218502/123558696-f6779e00-d797-11eb-861b-1e6eb9d201b3.PNG)

### Repairing the damaged image thanks to augmented data:

![dvdisaster_cd_repair](https://user-images.githubusercontent.com/218502/123558704-fe374280-d797-11eb-8f93-cd41848777d0.PNG)

### Verification of the image after correction:

![dvdisaster_cd_verify_ok](https://user-images.githubusercontent.com/218502/123558712-04c5ba00-d798-11eb-884f-bfd5443f036c.PNG)

### Scanning a healthy BD-R (single layer) with Linux GUI:

![dvdisaster_bdr_read](https://user-images.githubusercontent.com/218502/91436728-fbfb1380-e868-11ea-8444-04ebc60809d8.PNG)
