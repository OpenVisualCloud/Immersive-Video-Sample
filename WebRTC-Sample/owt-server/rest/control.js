#!/usr/bin/env node
// Copyright (C) <2019> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0
var initdb = require('./scripts/initdb');
var rest = require('./scripts/rest');

const serverl_url = 'https://localhost:3000';
const db_user_passwd = ''  // 'user:passwd@'
const db_url = `${db_user_passwd}localhost/owtdb`;

function RoomSetResolution(room, resolution) {
  var update = false;

  if (room.views[0].video.parameters.resolution.width != resolution.width ||
      room.views[0].video.parameters.resolution.height != resolution.height) {
    console.log(
        'Update room resolution',
        room.views[0].video.parameters.resolution.width, 'x',
        room.views[0].video.parameters.resolution.height, '=>',
        resolution.width, 'x', resolution.height);

    room.views[0].video.parameters.resolution = resolution;
    update = true;
  } else {
    console.log(
        'Room resolution', room.views[0].video.parameters.resolution.width, 'x',
        room.views[0].video.parameters.resolution.height);
  }

  return update;
}

function RoomSetVideoCodec(room, codec) {
  var update = false;

  // mediaIn
  {
    var find = false;
    for (var v of room.mediaIn.video) {
      if (v.codec === codec) {
        find = true;
        break;
      }
    }

    if (!find) {
      room.mediaIn.video.push({codec: codec});
      update |= true;

      console.log(`Update room mediaIn, ${codec}`);
    }
  }

  // mediaOut
  {
    var find = false;
    for (var v of room.mediaOut.video.format) {
      if (v.codec === codec) {
        find = true;
        break;
      }
    }

    if (!find) {
      room.mediaOut.video.format.push({codec: codec});
      update |= true;

      console.log(`Update room mediaOut, ${codec}`);
    }
  }

  /*
  room.mediaIn.video = [
    {'codec': 'h264'}, {'codec': 'vp8'}, {'codec': 'vp9'}, {'codec': 'h265'}
  ];
  room.mediaOut.video.format = [
    {'codec': 'vp8'}, {'profile': 'CB', 'codec': 'h264'}, {'codec': 'vp9'},
    {'codec': 'h265'}
  ];
  */

  return update;
}

async function UpdateRoom(resolution) {
  var service = await initdb.GetService(db_url);
  rest.API.init(
      service.sampleServiceId, service.sampleServiceKey, serverl_url, false);

  rest.API.getRooms(
      null,
      function(rooms) {
        for (var room of rooms) {
          console.log('Room', ':', room.name);
          const update = RoomSetResolution(room, resolution) ||
              RoomSetVideoCodec(room, 'h265');
          if (update) {
            const room_id = room._id;
            rest.API.updateRoom(
                room_id, room,
                function(res) {
                  console.log('Room', room_id, 'updated');
                },
                function(err) {
                  console.log('Error:', err);
                });
          }
        }
      },
      function(status, error) {
        // HTTP status and error
        console.log(status, error);
      });

  return;
}

async function AddStream(url) {
  var service = await initdb.GetService(db_url);
  rest.API.init(
      service.sampleServiceId, service.sampleServiceKey, serverl_url, false);

  rest.API.getRooms(
      null,
      function(rooms) {
        for (var room of rooms) {
          if (room.name === 'sampleRoom') {
            const room_id = room._id;

            var transport = {protocol: 'udp', bufferSize: 2048};
            var media = {audio: 'auto', video: true};

            rest.API.startStreamingIn(
                room_id, url, transport, media,
                function(stream) {
                  // console.log('Streaming-In:', stream);
                  var items = [
                    {'op': 'add', 'path': '/info/inViews', 'value': 'common'}
                  ]
                  rest.API.updateStream(
                      room_id, stream.id, items,
                      function(stream) {
                        console.log(`Stream ${stream.id} added`);
                      },
                      function(status, error) {
                        // HTTP status and error
                        console.log(status, error);
                      });
                },
                function(status, error) {
                  // HTTP status and error
                  console.log(status, error);
                });

            break;
          }
        }
      },
      function(status, error) {
        // HTTP status and error
        console.log(status, error);
      });
}

async function ClearStreams() {
  var service = await initdb.GetService(db_url);
  rest.API.init(
      service.sampleServiceId, service.sampleServiceKey, serverl_url, false);

  rest.API.getRooms(
      null,
      function(rooms) {
        for (var room of rooms) {
          if (room.name === 'sampleRoom') {
            const room_id = room._id;

            rest.API.getStreams(
                room_id,
                function(streams) {
                  // console.log ('This room has ', streams.length, 'streams');
                  for (var s of streams) {
                    // console.log(s);
                    if (s.type === 'forward' && s.info.type === 'streaming') {
                      const s_id = s.id;
                      rest.API.stopStreamingIn(
                          room_id, s_id,
                          function(result) {
                            console.log(
                                'External streaming-in:', s_id,
                                'in room:', room_id, 'stopped');
                          },
                          function(status, error) {
                            // HTTP status and error
                            console.log(status, error);
                          });
                    }
                  }
                },
                function(status, error) {
                  // HTTP status and error
                  console.log(status, error);
                });

            break;
          }
        }
      },
      function(status, error) {
        // HTTP status and error
        console.log(status, error);
      });
}

function Usage() {
  console.log(`\n\
Set owt server video encoding resolution, publish a stream, remove all streams.

Options:
    -s [4k|8k]      Set server video encoding resolution 3840x2048 or 7680x3840, default is 3840x2048
    -c url          Connect a FILE|RTSP|RTMP stream to server
    -d              Disconnect all streams from server"\
`)

  process.exit()
}

function ParseArgs(args) {
  var i = 0;
  var opts = {};

  while (i < args.length) {
    switch (args[i]) {
      case '-s':
        if (i + 2 != args.length)
          Usage();

        opts.room_resolution = args[i + 1];

        if (opts.room_resolution === '4k') {
          opts.room_resolution = {width: 3840, height: 2048};
        } else if (opts.room_resolution === '8k') {
          opts.room_resolution = {width: 7680, height: 3840};
        } else {
          Usage();
        }

        i += 2;
        break;
      case '-c':
        if (i + 2 != args.length)
          Usage();

        opts.stream_url = args[i + 1];
        i += 2;
        break;
      case '-d':
        if (i + 1 != args.length)
          Usage();

        opts.clear_steams = true;
        i += 1;
        break;
      default:
        Usage();
        break;
    }

    break;
  }

  if (Object.keys(opts).length === 0)
    Usage();

  return opts;
}

var options = ParseArgs(process.argv.slice(2));
console.log(options);

if (options.room_resolution)
  UpdateRoom(options.room_resolution);

if (options.stream_url)
  AddStream(options.stream_url);

if (options.clear_steams)
  ClearStreams();
