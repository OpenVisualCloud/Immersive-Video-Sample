# How to set up decklink plugin with 8K resolution

- Recommend to build the server components in `docker image`.

- Assuming all dependencies are pre-installed

## Apply patch for decklink plugin

```bash
cd /path/to/tids_oss/src/external && ./install_FFmpeg.sh
cd ../FFmpeg/libavdevice && patch -p1 < ../../ffmpeg/decklink_8k/decklink_8k.patch
```

## Modify FFmpeg configure options

```bash
vim /path/to/tids_oss/src/CMakeLists.txt
# Add "--enable-decklink" at line 58
```

## Build server components

```bash
cd /path/to/tids_oss/src/external
./install_FFmpeg.sh server
./build.sh server n
```

## Using script to run ffmpeg

```bash
./run_decklink.sh <options>
# select from <null, copy, rawvideo, 264, de>
```
