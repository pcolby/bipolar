# [![PMDA++](qrc/icon/bipolar30.png)](#) Bipolar
[![Build Status](http://api.travis-ci.org/pcolby/bipolar.svg)](https://travis-ci.org/pcolby/bipolar)
[![Github Release](http://img.shields.io/github/release/pcolby/bipolar.svg)](https://github.com/pcolby/bipolar/releases/latest)
[![GPLv3 License](http://img.shields.io/badge/license-GPLv3-blue.svg)](https://www.gnu.org/copyleft/gpl.html)

Bipolar aims to be a convenient GUI application for exporting activity
and exercise data from Polar's sync software.  Specifically, its intended to
provide a way to get your own data from Polar's new V800 watch, via Polar
FlowSync, but other devices (as well as Polar WebSync) may be supported too
eventually.

## Status

The release plans is roughly as follows:

- [x] 0.0.1 Working hook DLL for extracting data from Polar FlowSync on Windows.
- [x] 0.1.0 Export to both GPX and TCX, in a headless mode (no GUI yet).
- [ ] 0.2.0 Mac OS support (both hook library, and headless app).
- [ ] 0.3.0 Basic GUI.

Note, while the current (0.1.0) release includes a lot of behind-the-scenes code
for parsing training session data and exporting to GPX and TCX, the GUI has
been intentionally left ultra-bare at this stage. This, of course, is to allow
people (like myself) to begin capturing and uploading data as soon as possible,
while the GUI bells and whistles can follow later (slated for release 0.3.0).

## Contact

The [Bipolar Google Group](http://groups.google.com/d/forum/bipolar-app) is
available for general discussion, questions, comments, suggestions,
announcements etc.  Email bipolar-app@googlegroups.com to subscribe.
