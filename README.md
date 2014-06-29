# Bipolar

Bipolar aims to be a convenient Windows GUI application for exporting activity
and exercise data from Polar's sync software.  Specifically, its intended to
provide a way to get your own data from Polar's new V800 watch, via Polar
FlowSync, but other devices (as well as Polar WebSync) may be supported too
eventually.

## Status

Bipolor consists of two parts:

1. A custom hook for extracting data from Polar's applications.
2. A GUI application for processing the extracted data (for conversion to other
   formats such as GPX and TCX).

The custom hook is pretty-much working now for Polar FlowSync 2.1.x.  However,
the GUI application has not been started yet.

The next step is to understand how to interpret the data exported by the custom
hook (it's encoded using Google Protocol Buffers).  Help with this would be
greatly appreciated!

See [Hooks](../../wiki/Hooks) for more information.
