console.log("Connecting!");
const socket = new WebSocket('ws://localhost:8080');

function Send()
{
    socket.send(`Button pressed!`);
}

socket.onopen = function(event) {
    console.log("Connected!");
    socket.send(`Hi there!`);
};

socket.onclose = function(event) {
    console.log("Closed!");
};
