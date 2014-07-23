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
- [ ] 0.1.0 Export to both GPX and TCX, in a headless mode (no GUI yet).
- [ ] 0.2.0 Mac OS support (both hook library, and headless app).
- [ ] 0.3.0 Basic GUI.

The initial hook for Windows has already been released, and is working quite
well.  All of the protobuf data parsing working (Polar uses Google Protocol
Buffers).  Rudimentary GPX and TCX conversion is also working.

Things remaining for the next release (0.1.0) are:
- a Windows installer.

## Contact

The [Bipolar Google Group](http://groups.google.com/d/forum/bipolar-app) is
available for general discussion, questions, comments, suggestions,
announcements etc.  Email bipolar-app@googlegroups.com to subscribe.
