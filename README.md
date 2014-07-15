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

Bipolor consists (or _will_ consist) of two parts:

1. A custom hook for extracting data from Polar's applications.
2. A GUI application for processing the extracted data (for conversion to other
   formats such as GPX and TCX).

The custom hook is pretty-much working now for Polar FlowSync 2.1.x.  However,
the GUI application has not been started yet.

The next step is to understand how to interpret the data exported by the custom
hook (it's encoded using Google Protocol Buffers).  Help with this would be
greatly appreciated!

See [Hooks](../../wiki/Hooks) for more information.

### Update

I've now worked out the latitude, longitude, and altitude encoding! Work on the
conversion application can begin now :D

## How to Help

Right now, I need help with deciphering the Google Protocol Buffers. If you
already know what that means, then please go ahead and install the
[FlowSync hook](../../wiki/Hooks), capture some session data, and start
mapping - either add the results to the [Polar API V2](../../wiki/Polar API V2)
wiki page, or to a new issue.

If you still want to help, but aren't sure how, just create a new issue asking
how to help, and we can all discuss some options :D
