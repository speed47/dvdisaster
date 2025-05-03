# :cd: What is dvdisaster?

**dvdisaster** provides additional [ECC](https://en.m.wikipedia.org/wiki/Error_correction_code) protection for optical media.
If a medium gets damaged, dvdisaster can recover it as long as the amount of damage is smaller than the amount of ECC data you added to protect it.

It can loosely be compared to [.par2](https://en.m.wikipedia.org/wiki/Parchive) files, but the protection works at the *iso* level instead of working at the file level.
This way, even if metadata from the optical medium filesystem is damaged, dvdisaster can still work flawlessly.

Please refer to the [PDF manual](documentation/user-manual/manual.pdf) for more information.

# :wrench: Unofficial version

The last upstream version dates back to 2021, and can be found on the [official website](https://dvdisaster.jcea.es).

This version is built on top of the latest upstream version, with the following notable enhancements:

- Added pre-defined sizes for BD-R Triple Layer (100GB), BD-R Quadruple Layer (128GB)
- Added an option to use more space for ECC on BD-R when using RS03 (`--no-bdr-defect-management`)
- Re-enabled adaptive reading for RS01 and RS02, and optionally for RS03 on user request (more on that below)
- Ability to strip an augmented image from its additional ECC-data
- A new CLI-only version, not depending on gtk (`./configure --with-gui=no && make clean && make -j4`)
- GUI codebase ported from gtk2 to gtk3 to ensure future-proofness
- Windows and macOS builds are supported again, those were dropped upstream a few versions back
- Non-regression tests on each code change, along with prebuilt binaries for Linux64, Windows32/64 and macOS, for both CLI and GUI versions
- Fixed a bunch of other (minor) quirks, a few (minor) bugs, added a couple other (minor) features

Please refer to the [CHANGELOG](CHANGELOG) for all the details.

This version will never break compatibility with upstream versions,
the goal is to ensure an optical media protected by upstream dvdisaster will still be able to be repaired
with this version, decades from now. Regression tests are here to ensure this is the case.

# :twisted_rightwards_arrows: 3 available protection modes ("codecs")

For a more detailed explanation of the algorithms, please refer to the [codecs specification PDF](documentation/codecs.pdf).

:arrow_forward: **RS01** creates error correction files which are stored separately from the image they belong to.
The artefact is an **ecc** file, which must be stored on another media than the one we're protecting.

:arrow_forward: **RS02** creates error correction data which is added to the medium to protect, we call this *augmenting* the image we're protecting.
Damaged sectors in the error correction information reduce the data recovering capacity,
but do not make recovery impossible - a second medium for keeping or protecting the error correction information is not required.
Intelligent adaptive reading is also available when reading a damaged RS02-protected disc: dvdisaster will only attempt
to read the minimum amount of required sectors to be able to rebuild the image, using a so-called "divide and conquer"
seeking mechanism, cutting off up to 90% of the time required to read and recover a damaged media.

:arrow_forward: **RS03** is a further development of RS01 and RS02. It can create both error correction files and
augmented images, with the following added features:

- It can use multiple CPU cores and is therefore **WAY** faster than RS01/RS02 on modern hardware.
- RS03 augmented images and error correction files are - contrary to RS01, and to a lesser extent RS02 - robust against
  damage of the dvdisaster-added recovery data itself

There are, however, a few cons that must be noted for RS03:

- In image mode, the RS03 augmented image file size will be picked up from a predefined list of well-known medium sizes,
  while the size of augmented images can be freely chosen in RS02. This is the "price to pay" for the added robustness
  of the correction data.
- In image mode, intelligent adaptive reading is not available for RS03-protected images. The "divide and conquer"
  algorithm will still be used, but dvdisaster will not stop as soon as enough sectors have been recovered to rebuild
  the image: it'll attempt to read them all until you stop it, or until it tried to read all the sectors. You can still
  stop it manually and attempt a "verify" of the resulting image file, to see if enough data has been read for recovery,
  otherwise resuming the adaptive reading until this is the case.
- The changes for parallel computation and higher robustness make RS03 a tiny bit less space efficient, e.g. RS03 error
  correction data has slighly less (around -3%) error correction capacity than RS02 on images with equal size. This is
  usually considered a cheap price to pay for the added robustness against corruption.

# :mag: Comparison table

This attempts to summarize the differences, pros and cons of each codec:

| Codecs                               | RS01 (separate file, obsolete) | RS02 (augmented image)         | RS03 (in separate file mode)   | RS03 (in augmented image mode) |
|--------------------------------------|--------------------------------|--------------------------------|--------------------------------|--------------------------------|
| Robustness :one:                     | :star:                         | :star::star::star:             | :star::star::star::star::star: | :star::star::star::star::star: |
| Space efficiency                     | :star::star:                   | :star::star::star::star::star: | :star::star::star::star:       | :star::star::star::star:       |
| Computational generation speed :two: | :star::star:                   | :star::star:                   | :star::star::star::star::star: | :star::star::star::star::star: |
| Computational repair speed :two:     | :star::star::star:             | :star::star::star:             | :star::star::star:             | :star::star::star:             |
| Damaged media recovery speed :three: | :star:                         | :star::star::star:             | :star:                         | :star:                         |
| Supports customizing redundancy size | :heavy_check_mark:             | :heavy_check_mark:             | :heavy_check_mark:             | :x: :four:                     |

:one: Here we're talking about the robustness against corruption of the dvdisaster-added ECC parts _themselves_. The higher the ranking, the less it is likely than a few badly located damaged sectors render the whole correction impossible because they affect dvdisaster metadata on-disc. For example, corruption of the first dozens of sectors of an image can make RS02 entirely unusable regardless of the redundancy data originally stored on it.

:two: When algorithm is CPU-bound, i.e. generating or repairing an image stored on a SSD/NVMe drive where the storage i/o speed is not an issue.

:three: Using adaptive reading when supported (RS02), limiting the number of damaged sectors that need to be read to what is strictly necessary for repair. Using linear reading otherwise (RS03 and separate file codecs), assuming a badly damaged media, taking into account the time the drive takes to try to read damaged sectors.

:four: The robustness of RS03 comes at the cost of having to augment images strictly to well-known media sizes, as explained in the previous section above. This usually doesn't make much difference as long as you intend to burn the augmented image to a classic medium (CD-R, DVD-R, BD-R, ...).

# :bulb: Rationale

Even if the peak of the optical media era is well behind us, optical media is still of some value for specific use cases such as off-site backups.
In any case, we still have media in our hands that we want to be able to repair, should it be damaged, during the next years/decades.
Repairing is actually pretty much the very reason of dvdisaster existence (as long as parity data has been added, of course).
The main purpose of this unofficial version is to ensure dvdisaster doesn't get hard to find, use or compile on recent systems.
To this effect, prebuilt binaries are available for the 3 main categories of operating systems, and on top of that we've also fixed a few
bugs and added a few tiny features.

# :hammer: Compiling

See the [INSTALL](INSTALL) file. The [workflow file](.github/workflows/release.yml) that is used to automatically build binaries for each release can also help.

# :camera: Screenshots

### Reading a damaged CD under Windows:

![dvdisaster_damaged_cd](https://user-images.githubusercontent.com/218502/123558682-e2cc3780-d797-11eb-8ad0-5247b2601656.PNG)

### Verifying the ECC correction data from the damaged image:

![dvdisaster_cd_verify](https://user-images.githubusercontent.com/218502/123558696-f6779e00-d797-11eb-861b-1e6eb9d201b3.PNG)

### Repairing the damaged image thanks to augmented data:

![dvdisaster_cd_repair](https://user-images.githubusercontent.com/218502/123558704-fe374280-d797-11eb-8f93-cd41848777d0.PNG)

### Verification of the image after correction:

![dvdisaster_cd_verify_ok](https://user-images.githubusercontent.com/218502/123558712-04c5ba00-d798-11eb-884f-bfd5443f036c.PNG)

### Reading a quad-layer multi-session BDXL with Linux GUI:

![dvdisaster_bdxl_read](https://user-images.githubusercontent.com/218502/124361434-8baee280-dc2f-11eb-892e-27a9e738b41c.png)

Note that the disc still has some room for more sessions (capacity is 128 GB).
