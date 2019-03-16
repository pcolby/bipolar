## Bipolar Changelog

### 0.5.11 (_unreleased_)
???

### 0.5.10 (2019-03-16)
Features:
- fallback to sport types when session names are absent ([#91](../../issues/91))

Bugs:
- use x86-64 hook on OSX ([#92](../../issues/92))

Special thanks to @turhake for contributing test data for this release.

### 0.5.9 (2018-10-29)
Features:
- compatible with FlowSync 3 ([#89](../../issues/89))

### 0.5.8 (2018-10-21)
Bugs:
- check for speed (not cadence) for TCX speed output ([#86](../../pull/86))
- correct detection of speed and altitude for HRM output ([#86](../../pull/86))

Special thanks to @mz-pdm for contributing to this release.

### 0.5.7 (2017-12-17)
Features:
- Les Mills activity types ([b68cccd](../../commit/b68cccdf5990a7ef5ccdb35bcacaaaec565781a4))

Bugs:
- sanitise power samples ([#81](../../issues/81))

Other:
- new test data: laps with power ([#46](../../issues/46))
- new test data: swimming ([#75](../../issues/75))

Special thanks to @remifreiwald for contributing to this release.

### 0.5.6 (2016-12-17)
Features:
- fallback to exercise HRV data ([#42](../../issues/42))

Bugs:
- use metres-per-second for TCX TPX Speed values ([#74](../../issues/74))

### 0.5.5 (2016-10-19)
Bugs:
- use metres-per-second for TCX speeds ([#74](../../issues/74))

### 0.5.4 (2016-09-25)
Features:
- numerous Windows installer improvements ([ecffb9b](../../commit/ecffb9be6c202e028a5760d6d942b5c5c4bb9825))

### 0.5.3 (2016-08-23)
Features:
- Qt 5.7 support ([5e2b270](../../commit/5e2b2704015a05784f37ac0bbe69d68677d2ed49),
  [8a09bfb](../../commit/8a09bfb2979d1a4845cf2d6c68f07eb7c61e578b))
- new Polar sport types ([a5f67bd](../../commit/a5f67bd15e5b54250037132c397b2ca4339135aa))
- faster unit tests ([#71](../../issues/71))

Bugs:
- preserve TotalTimeSeconds precision ([4cc2ac9](../../commit/4cc2ac9de031e93ba70d2487a449978c2afab9b8))
- locale independent GPX lat/lon ([af7e80d](../../commit/af7e80db712a23c3323f03e1e3c43dccda0acd3b))

### 0.5.2 (2016-03-29)
Bugs:
- Qt 5.6 / C++11 support ([3bb2f60](../../commit/3bb2f60e693ecc84ba4dd6fed1638543a7b88d99))

### 0.5.1 (2016-02-25)
Bugs:
- improved FlowSync hook detection on Windows ([#63](../../issues/63))

### 0.5.0 (2015-12-16)
Features:
- additional Polar sport types ([#50](../../issues/50))
- Enhanced output precision ([70822af](../../commit/70822af249429f5e67ead43e36cde6367651b205))
- Stages Power meter support ([#57](../../issues/57))
- upgrade to Qt 5.5.1 (from 5.3.1, 5.3.2)
- Windows only: hook for Qt 5.5.1 / FlowSync 2.6.2+ ([#60](../../issues/60))
- numerous build system improvements, including support for out-of-source builds
- automated builds / releases via AppVeyor (Windows) and Travis CI (OS X)

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
