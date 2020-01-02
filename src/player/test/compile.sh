#!/bin/bash -e

cp ../../google_test/libgtest.a .

LD_FLAGS="-lavfilter -lavformat -lavcodec -lavdevice -lavutil -lswscale -lswresample -lpostproc -lglfw -lGL -lGLU -lX11 -l360SCVP -ldash -lOmafDashAccess -lpthread -ldl -g -lz -llzma -lva -lva-x11 -lva-drm -lglog -lEGL -lGLESv2"

g++ -Wall -g -fPIC -lglog -std=c++11 -fpermissive -c ../*.cpp

g++ -I. -I../../google_test -DGPAC_HAVE_CONFIG_H -std=c++11 -g  -c testMediaSource.cpp -D_GLIBCXX_USE_CXX11_ABI=0
g++ -I. -I../../google_test -DGPAC_HAVE_CONFIG_H -std=c++11 -g  -c testRenderSource.cpp -D_GLIBCXX_USE_CXX11_ABI=0
g++ -I. -I../../google_test -DGPAC_HAVE_CONFIG_H -std=c++11 -g  -c testRenderManager.cpp -D_GLIBCXX_USE_CXX11_ABI=0

g++ -g -I../../google_test MediaSource.o testMediaSource.o FFmpegMediaSource.o libgtest.a -o testMediaSource ${LD_FLAGS}
g++ -g -I../../google_test Mesh.o Render2TextureMesh.o RenderBackend.o FFmpegMediaSource.o MediaSource.o VideoShader.o SWRenderSource.o RenderSource.o testRenderSource.o libgtest.a -o testRenderSource ${LD_FLAGS}
g++ -g -I../../google_test ViewPortManager.o RenderBackend.o RenderTarget.o SurfaceRender.o ERPRender.o CubeMapRender.o Mesh.o ERPMesh.o Render2TextureMesh.o CubeMapMesh.o DashMediaSource.o FFmpegMediaSource.o MediaSource.o HWRenderSource.o SWRenderSource.o DMABufferRenderSource.o RenderContext.o EGLRenderContext.o GLFWRenderContext.o RenderSource.o VideoShader.o RenderManager.o testRenderManager.o libgtest.a -o testRenderManager ${LD_FLAGS}

./testMediaSource
./testRenderSource
./testRenderManager
