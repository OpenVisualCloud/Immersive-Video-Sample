#!/bin/sh

sudo yum install doxygen.x86_64 -y

cd ../360SCVP
doxygen Doxyfile
cd -

cd ../OmafDashAccess
doxygen Doxyfile
cd -

cd ../VROmafPacking
doxygen Doxyfile
cd -

cd ../player
doxygen Doxyfile
cd -
