## Bipolar Changelog

### 0.4.1 (_unreleased_)
Features:
- additional Polar sport types ([#50](../../issues/50))
- Enhanced output precision ([70822af](../../commit/70822af249429f5e67ead43e36cde6367651b205))
- Stages Power meter support ([#57](../../issues/57))
- upgrade to Qt 5.5.1 (from 5.3.1, 5.3.2)
- Windows only: hook for Qt 5.5.1 / FlowSync 2.6.2+ ([#60](../../issues/60))

### 0.4.0 (2015-01-10)
Features:
- hook debug logging ([d5e970f](../../commit/d5e970fcb0b66446fde8a28670483ab5ac43bc79))
- Cluetrust GPXData Extension ([#29](../../issues/29))
- Garmin (GPX) Acceleration Extension ([#32](../../issues/32))
- Garmin (GPX) Trackpoint Extension ([#30](../../issues/30))
- Garmin (TCX) Activity Extension ([#31](../../issues/31))
- Garmin (TCX) Course Extension ([#47](../../issues/47))
- power data support ([#43](../../issues/43))
- Windows only: upgrade to Qt 5.3.2 (from 5.3.1)

Special thanks to @profanum429 and @trimathias for contributing to this release.

### 0.3.1 (2014-09-06)
Features:
- option to enable/disable R-R HRM output.
- option to enable/disable the HRM "LapNames" extension.
- option to use UTC timestamps in TCX output ([#36](../../issues/36))

### 0.3.0 (2014-08-24)
Features:
- wizard-based GUI ([#6](../../issues/6))

### 0.2.3 (2014-08-13)
Features:
- separate HRM files for R-R data ([#28](../../issues/28))

### 0.2.2 (2014-08-12)
Bugs:
- preserve trailing laps in TCX output ([#27](../../issues/27))

### 0.2.1 (2014-08-09)
Features:
- support for HRM export ([#14](../../issues/14))
- lap data in TCX output ([#18](../../issues/18))
- lap data in GPX output ([#22](../../issues/22))

### 0.2.0 (2014-08-03)
Features:
- OSX support.
- additional TCX elements ([#16](../../issues/16))
- preview (incomplete) HRM export ([#14](../../issues/14))
- upgrade to Qt 5.3.1 (from 5.3.0)

### 0.1.1 (2014-07-27)
Features:
- sport type autodetection ([#12](../../issues/12))

### 0.1.0 (2014-07-26)
Features:
- export to both GPX and TCX file formats.
- Windows installer, including installation of the Polar FlowSync hook.
- absolute minimum UI.

### 0.0.2 (2014-07-18)
Bugs:
- preserve "create" request files within the export hook.

### 0.0.1 (2014-06-29)
- initial release of the first export hook, supporting Polar FlowSync 2.1 on
  Windows only.
