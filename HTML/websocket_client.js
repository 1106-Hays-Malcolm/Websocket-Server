console.log("Connecting!");
const socket = new WebSocket('ws://localhost:8080');

// function Connect()
// {
//     console.log("Connecting!");
// }

socket.onopen = function(event) {
    console.log("Connected!");
};
