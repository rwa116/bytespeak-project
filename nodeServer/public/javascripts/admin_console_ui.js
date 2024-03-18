"use strict";
// Client-side interactions with the browser.

// Make connection to server when web page is fully loaded.
var socket = io.connect();
$(document).ready(function() {

	window.setInterval(function() {sendRequest('uptime')}, 1000);
	window.setInterval(function() {sendCommandViaUDP("getInfo");}, 1000);

	$('#btnTerminate').click(function(){
		sendCommandViaUDP("stop");
	});
	
});

function sendCommandViaUDP(message) {
	socket.emit('udpCommand', message);

	var timeout = setTimeout(function() { 
		var errMsg = "No response from back-end. Is NodeJS running on the target?";
		$('#error-message').html(errMsg);
		$('#error-box').css("display", "block");
	}, 1000);

	socket.on('commandReply', function(result) {
		clearTimeout(timeout);
		try {
			// see if reply is in json
			var jsonObject = JSON.parse(result);
			console.log(jsonObject);
		} catch (error) {
			console.log(result);
		}
		$('#error-box').css("display", "none");
	});

	socket.on('errorReply', function(result) {
		clearTimeout(timeout);
		$('#error-message').html(result);
		$('#error-box').css("display", "block");
	});
};

// A3 code for requesting bbg uptime
// implement only if able to be secure

// function sendRequest(file) {
// 	socket.emit('proc', file);

// 	var timeout = setTimeout(function() {
// 		var errMsg = "No response from back-end. Is NodeJS running on the target?";
// 		$('#error-message').html(errMsg);
// 		$('#error-box').css("display", "block");
// 	}, 1000);

// 	socket.on('fileContents', function(result) {
// 		clearTimeout(timeout);
// 		var fileName = result.fileName;
// 		var contents = result.contents;
// 		if (fileName != 'uptime'){
// 			console.log("Unknown DOM object: " + fileName);
// 			return;
// 		}
// 		var uptimeVals = contents.split(" ");
// 		var bbgUptime = parseInt(uptimeVals[0]);
// 		var seconds = bbgUptime % 60;
// 		var minutes = Math.floor(bbgUptime / 60);
// 		var hours = Math.floor(minutes / 60);
// 		minutes = minutes % 60;
// 		$('#hours').html(hours);
// 		$('#minutes').html(minutes);
// 		$('#seconds').html(seconds);
// 	});
// }