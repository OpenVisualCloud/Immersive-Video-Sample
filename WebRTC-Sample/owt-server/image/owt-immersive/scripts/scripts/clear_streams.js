var icsREST = require('./rest.js');
var config = require('./config.js')
icsREST.API.init('_service_ID_', '_service_KEY_', config.serverUrl, false);
var sampleRoomId;
icsREST.API.getRooms()
    .then((rooms) => {
        for (var i = 0; i <= rooms.length - 1; i++) {
            if (rooms[i].name === "sampleRoom") {
                sampleRoomId = rooms[i]._id;
            }
        }
        return icsREST.API.getStreams(sampleRoomId)
    })
    .then((streams) => {
        for (var i = 1; i < streams.length; i++) {
            if (streams[i] !== 'mixed') {
                console.log("clear stream:", streams[i].id)
                icsREST.API.stopStreamingIn(sampleRoomId, streams[i].id)
            }
        }
    })
