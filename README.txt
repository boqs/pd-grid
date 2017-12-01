monome aleph's grid operators ported to puredata.  Currently supports
only 128 grids, but should be fairly easy to extend to other monome
grids

build dependencies are pd vanilla & liblo for OSC support.  You will
also need serialosc & a monome https://github.com/monome/serialosc (or
grid device with serialosc-compatible controller e.g
https://github.com/lazzarello/monome-apc40 )

type make to build several externals:

- grid.pd_linux
- kria.pd_linux
- mp.pd_linux
- step.pd_linux
- ww.pd_linux

if the build doesn't work you might need to edit the Makefile set
PDINCLUDEDIR to wherever pd is installed on your system My machine is
arch linux, so manually compiled programs end up in /usr/local.

copy these .pd_linux files (or .pd_whatever if you're on a different
OS) to your pd-externals directory.

the included patch pd-grid-examples shows how all these externals can
be used in pd patches.  Documentation for the grid apps themselves can
be found here:

https://monome.org/docs/modular/whitewhale/
https://monome.org/docs/modular/meadowphysics/
https://llllllll.co/t/kria-0-3-initial-release/2409
https://monome.org/docs/aleph/ops/ (see documentation for 'step')

there's no documentation for 'grid' - it's just buttons 'n lights!
You can get the idea by following along with the 'grid studies'
tutorial here: https://monome.org/docs/grid-studies/pd/

all the grid ops in this 'ecosystem' respond to the message 'focus' by
grabbing focus of a connected monome grid.  'unfocus' gives focus back
to the focus manager, showing a diagonal gradient on the grid.

known bugs:

None at present.  If you find one, drop me a line!

bugs & feature requests to sasquatch@rickvenn.com
