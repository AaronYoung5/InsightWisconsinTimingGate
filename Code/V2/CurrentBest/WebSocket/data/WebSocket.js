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
        console.log("E");
        connection.send("E");
        document.getElementById('enable').style.backgroundColor = '#00878F';
        document.getElementById('enable').innerHTML = "Disable Timing Gate";
    } else {
        console.log("D")
        connection.send("D");
        document.getElementById('enable').style.backgroundColor = '#999';
        document.getElementById('enable').innerHTML = "Enable Timing Gate";
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
