"use strict";
// Client-side interactions with the browser.

// Make connection to server when web page is fully loaded.
var socket = io.connect();
$(document).ready(function() {

	window.setInterval(function() {sendRequest('uptime')}, 1000); //for proc/uptime
	
	//TODO: have this return device info in a json object
	// Then, update the UI with up-to-date device info
	window.setInterval(function() {sendCommandViaUDP("getInfo");}, 1000); 

	$('#btnTerminate').click(function(){
		sendCommandViaUDP("terminate");
	});

	$('#btnUpload1').click(function() {
		var file = document.getElementById('fileUpload').files[0];

		if(file) {
			var fileReader = new FileReader();
			fileReader.onload = function(event) {
				var fileData = event.target.result;
				sendFileViaUDP("cl1 ", fileData);
			};
			fileReader.readAsArrayBuffer(file);
		}
	});

	$('#btnUpload2').click(function() {
		var file = document.getElementById('fileUpload').files[0];

		if(file) {
			var fileReader = new FileReader();
			fileReader.onload = function(event) {
				var fileData = event.target.result;
				sendFileViaUDP("cl2 ", fileData);
			};
			fileReader.readAsArrayBuffer(file);
		}
	});

	document.getElementById("msgForm").addEventListener("submit", function(event) {
        event.preventDefault();

        var newMessage = document.getElementById("new-message").value;

        sendCommandViaUDP("espeak " + newMessage);
    });
	
});

// Add event listener for file input change
$('#fileUpload').on('change', function() {
	var btnUpload = $('#btnUpload1');
	btnUpload.prop('disabled', !this.files[0]); // Disable the button if no file is selected
	btnUpload = $('#btnUpload2');
	btnUpload.prop('disabled', !this.files[0]); // Disable the button if no file is selected
});

function sendFileViaUDP(lang, fileData) {    
    // Need to convert to base64, I can't seem to get the buffer to send correctly without it
	var base64Data = btoa(String.fromCharCode.apply(null, new Uint8Array(fileData)));
	socket.emit('fileUpload', lang + base64Data);
	
	// Dont really need to have a timeout here, but can uncomment and work on if we want
	// (we wont really be able to tell if a message is succesfully translated from here anyway)
	// var timeout = setTimeout(function() { 
	// 	var errMsg = "No response from back-end. Is NodeJS running on the target?";
	// 	$('#error-message').html(errMsg);
	// 	$('#error-box').css("display", "block");
	// }, 20000);

    // socket.on('commandReply', function(result) {
	// 	clearTimeout(timeout);
    // })
};

function sendCommandViaUDP(message) {
	socket.emit('udpCommand', message);

	var timeout = setTimeout(function() { 
		var errMsg = "No response from back-end. Is NodeJS running on the target?";
		$('#error-message').html(errMsg);
		$('#error-box').css("display", "block");
	}, 10000);

	socket.on('commandReply', function(result) {
		clearTimeout(timeout);
		try {
			// see if reply is in json
			var jsonObject = JSON.parse(result);
			console.log(jsonObject);
		} catch (error) {
			console.log(result);
		}
		var splitRes = result.split(";");
		
		switch(splitRes[0]) {
			case "currentMessage":
				$('#current-message').html(splitRes[1]);
				break;
			default:
				console.log("Unknown command: " + splitRes[0]);
				break;
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

function sendRequest(file) {
	socket.emit('proc', file);

	var timeout = setTimeout(function() {
		var errMsg = "No response from back-end. Is NodeJS running on the target?";
		$('#device-status').html("Offline")
		$('#error-message').html(errMsg);
		$('#error-box').css("display", "block");
	}, 2000);

	socket.on('fileContents', function(result) {
		clearTimeout(timeout);
		var fileName = result.fileName;
		var contents = result.contents;
		if (fileName != 'uptime'){
			console.log("Unknown DOM object: " + fileName);
			return;
		}
		var uptimeVals = contents.split(" ");
		var bbgUptime = parseInt(uptimeVals[0]);
		var seconds = bbgUptime % 60;
		var minutes = Math.floor(bbgUptime / 60);
		var hours = Math.floor(minutes / 60);
		minutes = minutes % 60;
		$('#device-status').html("Device is up for: " + hours + " hours, " + minutes + " minutes, " + seconds + " seconds.");
	});
}