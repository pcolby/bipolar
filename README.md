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

Note, while the current (0.1.x) release includes a lot of behind-the-scenes code
for parsing training session data and exporting to GPX and TCX, the GUI has
been intentionally left ultra-bare at this stage. This, of course, is to allow
people (like myself) to begin capturing and exporting data as soon as possible,
while the GUI bells and whistles can wait for a later release (currently planned
for 0.3.0).

## How it Works

When Bipolar is installed, it places a custom DLL into the Polar FlowSync
application directory. This custom DLL then writes all training session data it
sees to binary files (in `%LOCALAPPDATA%\Polar\PolarFlowSync\export`).

The installer also installs a separate Bipolar application, which, when exectued
looks through all training session data exported above, and, for any that don't
already have corresponding `*.gpx` and/or `*.tcx` files, converts those sessions
to GPX and/or TCX accordingly.

For the current release (0.1.x) this Bipolar application is extremely simple in
user interface - it basically just displays a log of its activity.  The UI will
be made more featureful sometime in the future, but it already does the most
valuable thing - converting all training sessions to GPX and TCX.

## Contact

The [Bipolar Google Group](http://groups.google.com/d/forum/bipolar-app) is
available for general discussion, questions, comments, suggestions,
announcements etc.  Email bipolar-app@googlegroups.com to subscribe.

Of course, the [issue tracker](https://github.com/pcolby/bipolar/issues?state=open)
is also available for reporting issues, asking questions, making comments and
suggestions, etc.
