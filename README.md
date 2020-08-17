# veins_python #

Sample combination of Veins and Python

## Supported program versions ##

- Veins 5.0 (see <http://veins.car2x.org/>)
- OMNeT++ 5.5.1 (see <https://omnetpp.org/>)
- Python 3.6 (see <https://www.python.org/>)

## Supported platforms ##

- Windows 10
- Debian 10
- macOS 10.15

## Setup ##

First, make sure that you are using compatible program versions (see above) on a compatible platform (see above).

Make sure that Veins is set up and working from the command line by opening an OMNeT++ shell, navigating to the Veins directory, and executing `./configure` followed by `make`.
Change to the `examples/veins` subdirectory and run `./run` to confirm Veins is working.
Use the same command line window for entering any of the following commands.

In the command line window, change to the `veins_python` directory.

Configure the build system by running
```
./configure
```

- If you receive errors about `Python.h` not being found, make sure that you have installed Python along with its headers (e.g., on Debian Linux you might need to install `libpython3.7-dev` or similar)
- The configure script adopts settings from the Python interpreter it is running under. If you want to use a different Python interpreter, execute the script as, e.g., `python3.8 ./configure`.

Build `veins_python` by running
```
make
```

Try out `veins_python` by running

```
cd examples/veins_python
./run -u Cmdenv
```

You should see the following output (amidst other OMNeT++ output):

```
[INFO] General:0@t=1 Scenario.manager(veins_python/ApplicationLayerTest.cc:48): Veins2py testfun returning 42
Veins2py testfun returned 42
[INFO] General:0@t=1 Scenario.manager(veins_python/ApplicationLayerTest.cc:148): Py2veins funtest returned 24
```

## Bugs ##

- Probably many


## License ##

Veins is composed of many parts. See the version control log for a full list of
contributors and modifications. Each part is protected by its own, individual
copyright(s), but can be redistributed and/or modified under an open source
license. License terms are available at the top of each file. Parts that do not
explicitly include license text shall be assumed to be governed by the "GNU
General Public License" as published by the Free Software Foundation -- either
version 2 of the License, or (at your option) any later version
(SPDX-License-Identifier: GPL-2.0-or-later). Parts that are not source code and
do not include license text shall be assumed to allow the Creative Commons
"Attribution-ShareAlike 4.0 International License" as an additional option
(SPDX-License-Identifier: GPL-2.0-or-later OR CC-BY-SA-4.0). Full license texts
are available with the source distribution.

