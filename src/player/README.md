RENDER README
=============

Render is an example application to play the OMAF based 360 video file on Linux platform.

# How to build & install

1. Type `cmake .` to create the configuration.

2. Then type `make` to build render example.

3. Type `make install` to install all binaries and libraries you built.

# How to run

1. run `export DISPLAY=:0.0`

2. run `export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH`

3. run `./render fileName`
