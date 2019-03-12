// Stop Watch Functionality

window.onload = main;

var el = {
	body: document.body,
	time: document.getElementById('time'),
	blank: "00:00:00",
	start: document.getElementById('start'),
	stop: document.getElementById('stop'),
	lap: document.getElementById('lap'),
	reset: document.getElementById('reset'),
	clear: document.getElementById('clear'),
	ul: document.getElementById('log')
};
var min = 0, sec = 0, mil = 0;

function watch(){
	this.style.display = "none";  
	el.stop.style.display = "inline-block";
	el.lap.style.display = "inline-block";
	var setI = setInterval(function(){
		var format = (min > 9 ? min : "0" + min)+':'+(sec > 9 ? sec : "0" + sec) + ':' + (mil > 9 ? mil : "0" + mil);
		el.time.innerHTML = format;
		mil++;
		if(mil > 99){sec++; mil = 0;} 
		if(sec > 59){min++; sec = 0;} 
		el.lap.onclick = function log(){
			var item = document.createElement('li');
			el.clear.style.display = "block";
			item.innerHTML = format;
			el.ul.prepend(item);
		}
			el.clear.onclick = function(){el.ul.innerHTML = "";this.style.display = "none";}
	} ,10);
	el.stop.onclick = function(){clearInterval(setI);this.style.display = "none"; el.start.style.display = "inline-block"; el.lap.style.display = "none";};
}

function reset(){
	min = 0;sec = 0;mil = 0;
	time.innerHTML = el.blank;
}

function main(e){
	el.start.onclick = watch;
	el.reset.onclick = reset;
};

// WebSocket Interface With NodeMCU 
var connection = new WebSocket('ws://'+location.hostname+':81/', ['arduino']);

connection.onopen = function () {
    connection.send('Connect ' + new Date());
};
connection.onerror = function (error) {
    console.log('WebSocket Error ', error);
};
connection.onmessage = function (e) {
    var message = JSON.parse(e.data);
    analyzeIncomingMessage(message);
};
connection.onclose = function(){
    console.log('WebSocket connection closed');
};

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

/* NodeMCU Sensor Functionality  */
function analyzeIncomingMessage(message) {
    if(message.button == "y") {
        console.log("Button is active");
        el.start.onclick();
    }
    else if(message.button == "n") {
        console.log("Button is not active");
        el.stop.onclick();
    }
}
