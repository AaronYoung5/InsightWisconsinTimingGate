var timingGateEnabled = false;
var connection = new WebSocket('ws://'+location.hostname+':81/', ['arduino']);
connection.onopen = function () {
    connection.send('Connect ' + new Date());
};
connection.onerror = function (error) {
    console.log('WebSocket Error ', error);
};
connection.onmessage = function (e) {
    var message = JSON.parse(e.data);
    addRow(message.IP, message.time);
    console.log('Server: ', e.data);
};
connection.onclose = function(){
    console.log('WebSocket connection closed');
};

function timingGateStart(){
    timingGateEnabled = ! timingGateEnabled;
    if(timingGateEnabled){
        if(isServerConnected()) {
            console.log("E");
            connection.send("E");
            document.getElementById('enable').style.backgroundColor = '#00878F';
            document.getElementById('enable').innerHTML = "Disable Timing Gate";
        }
        else {
            updateServerStatus(false);
            console.log("Server is not connected");
        }
    } else {
        if(isServerConnected()) {
            console.log("D")
            connection.send("D");
            document.getElementById('enable').style.backgroundColor = '#999';
            document.getElementById('enable').innerHTML = "Enable Timing Gate";
        }
        else {
            updateServerStatus(false);
            console.log("Server is not connected");
        }
    }  
}

function addRow(ip, time) {
    var table = document.getElementById("table");
    var row = table.insertRow();
    var cell1 = row.insertCell(0);
    var cell2 = row.insertCell(1);
    cell1.innerHTML = ip;
    cell2.innerHTML = time;
}

function isServerConnected() {
    try {
        state = connection.readyState;
        updateServerStatus(state);
        return state == 1;
     }
    catch(err) {
        return false;
    }
}

function updateServerStatus(serverStatus) {
    var state = "";
    
    switch(serverStatus) {
        case 0:
            serverStatus = false;
            state = "Connecting";
            break;
        case 1:
            serverStatus = true;
            state = "Open";
            break;
        case 2:
            serverStatus = false;
            state = "Closing";
            break;
        case 3:
            serverStatus = false;
            state = "Closed";
            break;
        default:
            serverStatus = false;
            state = "Error";
    }
    
    if(serverStatus) {
        document.getElementById("status").innerHTML = state;
        document.getElementById("status").className = state.toLowerCase() + " server-status";
    }
    else {
        document.getElementById("status").className = state.toLowerCase() + " server-status";
        document.getElementById("status").innerHTML = state;       
    }
}
