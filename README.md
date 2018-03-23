# Welcome to the ogon Qt platform plugin

This is a Qt5 platform plugin to export applications through ogon without modifying
the original code. It is part of the ogon project.

## What is the ogon project?

The ogon project is an open source driven collection of services and tools mainly written in C/C++
that provide graphical remote access to Linux desktop sessions using the Remote Desktop Protocol
(RDP). It supports most modern RDP protocol extensions, bitmap compression codecs, dis- and
reconnection to sessions and device redirections.
ogon is compatible with virtually any existing RDP Client.

Any X11 destkop, weston or qt application can be used as session. Due to it's modular
design it's easily possible to extend or add features or add new backends.

# tl;dr - too long; didn't read - I just want ...

* .. to report [a BUG][bugs]
* .. to build it - have a look at our [documentation][documentation]
* .. help - have a look to our [SUPPORT.md document][support]
* .. to get in touch - have a look to our [SUPPORT.md document][support]
* .. to contribute - have a look to [CONTRIBUTING.md][contribute]

# License

Most components of the ogon-project are licensed under the GNU AFFERO GENERAL PUBLIC LICENSE version 3.
See LICENSE file of the respective repository.

# Application details

## Requirements

* Qt5
* FreeRDP - pkg-config: winpr, freerdp
* ogon - pkg-config: ogon-backend

## Build

To build just run:

```qmake && make && make install```

It's important to run make and make install in
**two distinctive steps** otherwise strange things will happen (like build dependency failures).

qogon supports the following qmake options:

* PREFIX - Installation prefix.
* ADDITIONAL_RPATHS - Set additional library runtime paths to the binaries (rpath). List of space separated directories.  Same as format as for QMAKE_RPATHDIR.
* SBP_PROTO_PREFIX - Set directory where SBP.proto can be found. Default is to use the same prefix as ogon-backend (pkg-config) or QOGON_PREFIX as fallback.
* LOCAL_TRANSLATIONS - Use source directory as TRANSLATIONS_DIR otherwise path to installed. This is useful when working on translations.

Example:

```
qmake ADDITIONAL_RPATHS=/opt/ogon/lib PREFIX=/opt/ogon
```

If PREFIX is used it might be necessary to copy/link the platform libraries and translations into
qt's platform plugin directory or add the path to the environment variable
QT_QPA_PLATFORM_PLUGIN_PATH or run the application with -platformpluginpath.

[support]: https://github.com/ogon-project/ogon-project/SUPPORT.md
[bugs]: https://github.com/ogon-project/ogon-project/SUPPORT.md#bugs
[documentation]: https://github.com/ogon-project/ogon-project/SUPPORT.md#documentation
[contribute]: https://github.com/ogon-project/ogon-project/CONTRIBUTING.md
