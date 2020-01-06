var icsREST = require('./rest.js');
var config = require('./config.js')
icsREST.API.init('_service_ID_', '_service_KEY_', config.serverUrl, false);
var sampleRoomId;
var streamid;
icsREST.API.getRooms()
    .then((rooms) => {
        for (var i = 0; i <= rooms.length - 1; i++) {
            if (rooms[i].name === "sampleRoom") {
                sampleRoomId = rooms[i]._id;
            }
        }
        return icsREST.API.startStreamingIn(sampleRoomId)
    })
    .then((stream) => {
        stream = JSON.parse(stream)
        streamid = stream.id;
        return icsREST.API.mixStream(sampleRoomId, streamid)
    })
    .then(() => {
        console.log(`startStreamingIn sucess => streaminId :${streamid}`)
    })

