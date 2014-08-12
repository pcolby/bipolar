# [![PMDA++](qrc/icon/bipolar30.png)](#) Bipolar
[![Build Status](http://api.travis-ci.org/pcolby/bipolar.svg)](https://travis-ci.org/pcolby/bipolar)
[![Github Release](http://img.shields.io/github/release/pcolby/bipolar.svg)](https://github.com/pcolby/bipolar/releases/latest)
[![GPLv3 License](http://img.shields.io/badge/license-GPLv3-blue.svg)](https://www.gnu.org/copyleft/gpl.html)

Bipolar aims to be a convenient GUI application for exporting activity
and exercise data from Polar's sync software.  Specifically, its intended to
provide a way to get your own data from Polar's new V800 watch, via Polar
FlowSync, but works well for the Polar Loop too.

## Status

The release plans is roughly as follows:

- [x] 0.0.1 Working hook DLL for extracting data from Polar FlowSync on Windows.
- [x] 0.1.0 Export to both GPX and TCX, in a headless mode (no GUI yet).
- [x] 0.2.0 Mac OS support (both hook library, and headless app).
- [x] 0.2.x Minor features and bug fixes.
- [ ] 0.3.0 Basic GUI.

Note, while the current (0.2.x) release includes a lot of behind-the-scenes code
for parsing training session data and exporting to GPX and TCX, the GUI has
been intentionally left ultra-bare at this stage. This, of course, is to allow
people (like myself) to begin capturing and exporting data as soon as possible,
while the GUI bells and whistles can wait for a later release (currently planned
for 0.3.0).

## How it Works

### Windows

When Bipolar is installed, it places a custom DLL into the Polar FlowSync
application directory. This custom DLL then writes all training session data it
sees to binary files (in `%LOCALAPPDATA%\Polar\PolarFlowSync\export`).

The installer also installs a separate Bipolar application, which, when exectued
looks through all training session data exported above, and, for any that don't
already have corresponding `*.gpx` and/or `*.tcx` files, converts those sessions
to GPX and/or TCX accordingly.

### OSX

For OSX, Bipolar is provided as a standard disk image (`*.dmg`) file. This file
contains both the Bipolar application (which can run completely standalone), and
the hook library.  However, the hook library is not automatically installed for
OSX. Instead, the disk image includes a simple hook install script to run to
install the hook (which you will want to do, unless using the direct USB option
mentioned below).

### Direct USB Interface

The separate [v800_downloader](https://github.com/profanum429/v800_downloader)
project provides a way of extracting training session data from the V800
directly, without the need for the hook mentioned above. That project extracts
data in a format that is compatible with this project's Bipolar application, so
can be used as a hook alternative if desired.

Of course, that is particularly exciting for Linux users, who otherwise could
not make any significant use of this project, nor the Polar V800 in general.

## Contact

The [Bipolar Google Group](http://groups.google.com/d/forum/bipolar-app) is
available for general discussion, questions, comments, suggestions,
announcements etc.  Email bipolar-app@googlegroups.com to subscribe.

Of course, the [issue tracker](https://github.com/pcolby/bipolar/issues?state=open)
is also available for reporting issues, asking questions, making comments and
suggestions, etc.

There's also a bit of Bipolar-related chatter on the
[Polar forums](http://forum.polar.fi/showthread.php?t=29092).