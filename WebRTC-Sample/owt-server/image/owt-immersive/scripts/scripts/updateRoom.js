var icsREST = require('./rest.js');
var config = require('./config.js');
icsREST.API.init('_service_ID_', '_service_KEY_', config.serverUrl, false);
var sampleRoomId;
icsREST.API.getRooms()
    .then((rooms) => {
        for (var i = 0; i <= rooms.length - 1; i++) {
            if (rooms[i].name === "sampleRoom") {
                sampleRoomId = rooms[i]._id;
            }
        }
        return icsREST.API.updateRoom(sampleRoomId)
    })
    .then((room) => {
        console.log(`updateRoom success => resolution : ${room.views[0].video.parameters.resolution.width} x ${room.views[0].video.parameters.resolution.height}`)
    })
