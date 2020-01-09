CC = gcc
CXX = g++
CC_VERSION = $(shell gcc -dumpversion)
PKG_CONFIG_PATH = "/usr/local/lib/pkgconfig:/usr/lib/pkgconfig"

all : server client test

server :
	@ echo "$(CC_VERSION)"

	@ echo "Start build server on $(SYSTEM) ..."
	@ mkdir -p build/server/360SCVP
	@ mkdir -p build/server/distributed_encoder
	@ mkdir -p build/server/ffmpeg
	@ mkdir -p build/server/VROmafPacking

	@ # Install 360SCVP
	@ cd build/server/360SCVP && \
 	  cmake ../../../360SCVP && \
 	  make -j `nproc` && \
	  sudo make install \

	@ # Install distributed_encoder
	@ cd build/server/distributed_encoder && \
	  cmake ../../../distributed_encoder && \
	  make -j `nproc` && \
	  sudo make install \

	@ # Install VROMAFPACKING
	@ export PKG_CONFIG_PATH=${PKG_CONFIG_PATH} && \
	  cd external && \
	  cd ../build/server/VROmafPacking && \
	  cmake ../../../VROmafPacking && \
	  make -j `nproc` && \
	  sudo make install

	@ # Install ffmpeg
	@ export PKG_CONFIG_PATH=${PKG_CONFIG_PATH} && \
	  cd build/server/ffmpeg && \
	  ../../../ffmpeg/configure \
	      --prefix=/usr --libdir=/usr/lib --enable-static --disable-shared \
	      --enable-gpl --enable-nonfree --disable-optimizations --disable-vaapi \
              --enable-libDistributedEncoder --enable-libVROmafPacking && \
	  make -j `nproc`

client :
	@ echo "Start build client on $(SYSTEM) ..."

	@ mkdir -p build/client/360SCVP
	@ mkdir -p build/client/OmafDashAccess
	@ mkdir -p build/client/player
	@ mkdir -p build/client/ffmpeg

	@ # Install 360SCVP
	@ cd build/client/360SCVP && \
          cmake ../../../360SCVP && \
          make -j `nproc` && \
          sudo make install

	@ # Install OmafDashAccess
	@ cd build/client/OmafDashAccess && \
	  cmake ../../../OmafDashAccess && \
	  make -j `nproc` && \
	  sudo make install

	@ # Install Player
	@ cd player && cp ../utils/tinyxml2.h ./ && \
	  cp ../utils/tinyxml2.cpp ./ && \
          cd ../build/client/player && \
	  cmake -DUSE_OMAF=ON -DUSE_WEBRTC=OFF  ../../../player  && \
	  make -j `nproc`

	@ # Install ffmpeg
	@ export PKG_CONFIG_PATH=${PKG_CONFIG_PATH} && \
	  cd build/client/ffmpeg && \
	  ../../../ffmpeg/configure \
	      --prefix=/usr --libdir=/usr/lib --enable-static --disable-shared \
	      --enable-gpl --enable-nonfree --disable-optimizations \
              --enable-libOmafDashAccess --enable-ffplay && \
	  make -j `nproc`

test:
	@ echo "Compiling and running unit test ..."
	@ mkdir -p build/test/360SCVP
	@ mkdir -p build/test/distributed_encoder
	@ mkdir -p build/test/VROmafPacking
	@ mkdir -p build/test/OmafDashAccess

	@ # Compile 360SCVP test
	@ cd build/test/360SCVP && \
            g++ -I../../../google_test -std=c++11 -I../util/ -g -c \
            ../../../360SCVP/test/testI360SCVP.cpp \
            -D_GLIBCXX_USE_CXX11_ABI=0 && \
          g++ -L/usr/local/lib testI360SCVP.o \
            ../googletest/googletest/build/libgtest.a -o \
            testI360SCVP -I/usr/local/include/ -l360SCVP \
            -lstdc++ -lpthread -lm -L/usr/local/lib

	@ # Compile distributed_encoder test
	@ cd build/test/distributed_encoder && \
          g++ -I../../../google_test -std=c++11 \
            -I../../../distributed_encoder/util/ -g -c \
            ../../../distributed_encoder/test/testMainEncoder.cpp \
            -D_GLIBCXX_USE_CXX11_ABI=0 && \
          g++ -I../../../google_test -std=c++11 \
            -I../../../distributed_encoder/util/ -g -c \
            ../../../distributed_encoder/test/testWorkSession.cpp \
            -D_GLIBCXX_USE_CXX11_ABI=0 && \
          g++ -I../../../google_test -std=c++11 \
            -I../../../distributed_encoder/util/ -g -c \
            ../../../distributed_encoder/test/testDecoder.cpp \
            -D_GLIBCXX_USE_CXX11_ABI=0 && \
          g++ -I../../../google_test -std=c++11 \
            -I../../../distributed_encoder/util/ -g -c \
            ../../../distributed_encoder/test/testSubEncoder.cpp \
            -D_GLIBCXX_USE_CXX11_ABI=0 -I/usr/local/include/svt-hevc && \
          g++ -I../../../google_test -std=c++11 \
            -I../../../distributed_encoder/util/ -g -c \
            ../../../distributed_encoder/test/testSubEncoderManager.cpp \
            -D_GLIBCXX_USE_CXX11_ABI=0 && \
          g++ -I../../../google_test -std=c++11 \
            -I../../../distributed_encoder/util/ -g -c \
            ../../../distributed_encoder/test/testEncoder.cpp \
            -D_GLIBCXX_USE_CXX11_ABI=0 && \
          g++ -L/usr/local/lib testMainEncoder.o \
            ../googletest/googletest/build/libgtest.a -o testMainEncoder \
            -I/usr/local/include/thrift -I/usr/local/include/svt-hevc \
            -lDistributedEncoder -lEncoder -lstdc++ -lpthread -lthrift \
            -lSvtHevcEnc -lopenhevc -lthriftnb -levent -lglog -pthread \
            -lavdevice -lxcb -lxcb-shm -lxcb-shape -lxcb-xfixes -lavfilter \
            -lswscale -lavformat -lavcodec -llzma -lz -lswresample -lavutil \
            -lva-drm -lva-x11 -lm -lva -lXv -lX11 -lXext -l360SCVP \
            -L/usr/local/lib && \
          g++ -L/usr/local/lib testWorkSession.o \
            ../googletest/googletest/build/libgtest.a -o testWorkSession \
            -I/usr/local/include/thrift -I/usr/local/include/svt-hevc \
            -lDistributedEncoder -lEncoder -lstdc++ -lpthread -lthrift \
            -lSvtHevcEnc -lopenhevc -lthriftnb -levent -lglog -pthread \
            -lavdevice -lxcb -lxcb-shm -lxcb-shape -lxcb-xfixes -lavfilter \
            -lswscale -lavformat -lavcodec -llzma -lz -lswresample -lavutil \
            -lva-drm -lva-x11 -lm -lva -lXv -lX11 -lXext -l360SCVP \
            -L/usr/local/lib && \
          g++ -L/usr/local/lib testDecoder.o \
            ../googletest/googletest/build/libgtest.a -o testDecoder \
            -I/usr/local/include/thrift -I/usr/local/include/svt-hevc \
            -lDistributedEncoder -lEncoder -lstdc++ -lpthread -lthrift \
            -lSvtHevcEnc -lopenhevc -lthriftnb -levent -lglog -pthread \
            -lavdevice -lxcb -lxcb-shm -lxcb-shape -lxcb-xfixes -lavfilter \
            -lswscale -lavformat -lavcodec -llzma -lz -lswresample -lavutil \
            -lva-drm -lva-x11 -lm -lva -lXv -lX11 -lXext -l360SCVP \
            -L/usr/local/lib && \
          g++ -L/usr/local/lib testSubEncoder.o \
            ../googletest/googletest/build/libgtest.a -o testSubEncoder \
            -I/usr/local/include/thrift -I/usr/local/include/svt-hevc \
            -lDistributedEncoder -lEncoder -lstdc++ -lpthread -lthrift \
            -lSvtHevcEnc -lopenhevc -lthriftnb -levent -lglog -pthread \
            -lavdevice -lxcb -lxcb-shm -lxcb-shape -lxcb-xfixes -lavfilter \
            -lswscale -lavformat -lavcodec -llzma -lz -lswresample -lavutil \
            -lva-drm -lva-x11 -lm -lva -lXv -lX11 -lXext -l360SCVP \
            -L/usr/local/lib && \
          g++ -L/usr/local/lib testEncoder.o \
            ../googletest/googletest/build/libgtest.a -o testEncoder \
            -I/usr/local/include/thrift -I/usr/local/include/svt-hevc \
            -lDistributedEncoder -lEncoder -lstdc++ -lpthread -lthrift \
            -lSvtHevcEnc -lopenhevc -lthriftnb -levent -lglog -pthread \
            -lavdevice -lxcb -lxcb-shm -lxcb-shape -lxcb-xfixes -lavfilter \
            -lswscale -lavformat -lavcodec -llzma -lz -lswresample -lavutil \
            -lva-drm -lva-x11 -lm -lva -lXv -lX11 -lXext -l360SCVP \
            -L/usr/local/lib && \
          g++ -L/usr/local/lib testSubEncoderManager.o \
            ../googletest/googletest/build/libgtest.a -o testSubEncoderManager \
            -I/usr/local/include/thrift -I/usr/local/include/svt-hevc \
            -lDistributedEncoder -lEncoder -lstdc++ -lpthread -lthrift \
            -lSvtHevcEnc -lopenhevc -lthriftnb -levent -lglog -pthread \
            -lavdevice -lxcb -lxcb-shm -lxcb-shape -lxcb-xfixes -lavfilter \
            -lswscale -lavformat -lavcodec -llzma -lz -lswresample -lavutil \
            -lva-drm -lva-x11 -lm -lva -lXv -lX11 -lXext -l360SCVP \
            -L/usr/local/lib

	@ # Compile OmafDashAccess test
	@ cd build/test/OmafDashAccess && \
          g++ -I../../../google_test -std=c++11 -I../util/ -g -c \
            ../../../OmafDashAccess/test/testMediaSource.cpp \
            -I../../../utils -D_GLIBCXX_USE_CXX11_ABI=0 && \
          g++ -I../../../google_test -std=c++11 -I../util/ -g -c \
            ../../../OmafDashAccess/test/testMPDParser.cpp \
            -I../../../utils -D_GLIBCXX_USE_CXX11_ABI=0 && \
          g++ -I../../../google_test -std=c++11 -I../util/ -g -c \
            ../../../OmafDashAccess/test/testOmafReader.cpp \
            -I../../../utils -D_GLIBCXX_USE_CXX11_ABI=0 && \
          g++ -I../../../google_test -std=c++11 -I../util/ -g -c \
            ../../../OmafDashAccess/test/testOmafReaderManager.cpp \
            -I../../../utils -D_GLIBCXX_USE_CXX11_ABI=0 && \
          g++ -L/usr/local/lib testMediaSource.o \
            ../googletest/googletest/build/libgtest.a -o \
            testMediaSource -I/usr/local/include/ -lOmafDashAccess \
            -lstdc++ -lpthread -lglog -l360SCVP -lm -L/usr/local/lib && \
          g++ -L/usr/local/lib testMPDParser.o \
            ../googletest/googletest/build/libgtest.a -o \
            testMPDParser -I/usr/local/include/ -lOmafDashAccess \
            -lstdc++ -lpthread -lglog -l360SCVP -lm -L/usr/local/lib && \
          g++ -L/usr/local/lib testOmafReader.o \
            ../googletest/googletest/build/libgtest.a -o \
            testOmafReader -I/usr/local/include/ -lOmafDashAccess \
            -lstdc++ -lpthread -lglog -l360SCVP -lm -L/usr/local/lib && \
          g++ -L/usr/local/lib testOmafReaderManager.o \
            ../googletest/googletest/build/libgtest.a -o \
            testOmafReaderManager -I/usr/local/include/ -lOmafDashAccess\
            -lstdc++ -lpthread -lglog -l360SCVP -lm -L/usr/local/lib

	@ # Compile VROmafPacking test
	@ cd build/test/VROmafPacking && \
          g++ -I../../../google_test -std=c++11 -g -c \
            ../../../VROmafPacking/test/testHevcNaluParser.cpp \
            -D_GLIBCXX_USE_CXX11_ABI=0 && \
          g++ -I../../../google_test -std=c++11 -g -c \
            ../../../VROmafPacking/test/testVideoStream.cpp \
            -D_GLIBCXX_USE_CXX11_ABI=0 && \
          g++ -I../../../google_test -std=c++11 -g -c \
            ../../../VROmafPacking/test/testExtractorTrack.cpp \
            -D_GLIBCXX_USE_CXX11_ABI=0 && \
          g++ -I../../../google_test -std=c++11 -g -c \
            ../../../VROmafPacking/test/testDefaultSegmentation.cpp \
            -D_GLIBCXX_USE_CXX11_ABI=0 && \
          g++ -L/usr/local/lib testHevcNaluParser.o \
            ../googletest/googletest/build/libgtest.a -o \
            testHevcNaluParser -I/usr/local/lib -lVROmafPacking \
            -l360SCVP -lstdc++ -lpthread -lm -L/usr/local/lib && \
          g++ -L/usr/local/lib testVideoStream.o \
            ../googletest/googletest/build/libgtest.a -o \
            testVideoStream -I/usr/local/lib -lVROmafPacking \
            -l360SCVP -lstdc++ -lpthread -lm -L/usr/local/lib && \
          g++ -L/usr/local/lib testExtractorTrack.o \
            ../googletest/googletest/build/libgtest.a -o \
            testExtractorTrack -I/usr/local/lib -lVROmafPacking \
            -l360SCVP -lstdc++ -lpthread -lm -L/usr/local/lib && \
          g++ -L/usr/local/lib testDefaultSegmentation.o \
            ../googletest/googletest/build/libgtest.a -o \
            testDefaultSegmentation -I/usr/local/lib -lVROmafPacking \
            -l360SCVP -lstdc++ -lpthread -lm -L/usr/local/lib

clean :
	@ rm -rf ./build


.PHONY : server client test clean
