"use strict";
// Client-side interactions with the browser.

// Make connection to server when web page is fully loaded.
var socket = io.connect();
$(document).ready(function() {
	// Populate the voice dropdown based on the selected language
	populateVoiceDropdown();
	sendCommandViaUDP("getInfo"); 

	window.setInterval(function() {sendRequest('uptime')}, 1000); //for proc/uptime
	
	// Buttons
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

	$('#updateVoice').click(function() {
		var languageDropdown = document.getElementById("language-dropdown");
		var voiceDropdown = document.getElementById("voice-dropdown");
		var selectedLang = languageDropdown.value;
		var selectedVoice = voiceDropdown.value;

		document.getElementById("updating-voice").classList.remove('hidden');
		sendCommandViaUDP("setVoice " + selectedLang + " " + selectedVoice);
	});

	document.getElementById("msgForm").addEventListener("submit", function(event) {
        event.preventDefault();

        var newMessage = document.getElementById("new-message").value;

		document.getElementById("updating-msg").classList.remove('hidden');
        sendCommandViaUDP("espeak " + newMessage);
    });
	
});

function populateVoiceDropdown() {
	var languageDropdown = document.getElementById("language-dropdown");
	var voiceDropdown = document.getElementById("voice-dropdown");
	var selectedLang = languageDropdown.value;

	// Clear the voice dropdown
	voiceDropdown.innerHTML = "";

	// Populate the voice dropdown
	switch(selectedLang) {
		case "en-US":
			var mOption = document.createElement("option");
			mOption.text = "Danny";
			mOption.value = "m";
			voiceDropdown.add(mOption);
			var fOption = document.createElement("option");
			fOption.text = "Amy";
			fOption.value = "f";
			voiceDropdown.add(fOption);
			break;
		case "es-ES":
			var mOption = document.createElement("option");
			mOption.text = "Carlos";
			mOption.value = "m";
			voiceDropdown.add(mOption);
			var fOption = document.createElement("option");
			fOption.text = "Maya";
			fOption.value = "f";
			voiceDropdown.add(fOption);
			break;
		case "fr-FR":
			var mOption = document.createElement("option");
			mOption.text = "Gilles";
			mOption.value = "m";
			voiceDropdown.add(mOption);
			var fOption = document.createElement("option");
			fOption.text = "Siwis";
			fOption.value = "f";
			voiceDropdown.add(fOption);
			break;
		case "de-DE":
			var fOption = document.createElement("option");
			fOption.text = "Eva";
			fOption.value = "f";
			voiceDropdown.add(fOption);
			var mOption = document.createElement("option");
			mOption.text = "Karlsson";
			mOption.value = "m";
			voiceDropdown.add(mOption);
			break;
		case "zh-CN":
			var fOption = document.createElement("option");
			fOption.text = "Huayan";
			fOption.value = "f";
			voiceDropdown.add(fOption);
			break;
	}
}

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

	// var timeout = setTimeout(function() { 
	// 	var errMsg = "No response from back-end. Is NodeJS running on the target?";
	// 	$('#error-message').html(errMsg);
	// 	$('#error-box').css("display", "block");
	// }, 10000);

	socket.on('commandReply', function(result) {
		// clearTimeout(timeout);
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
				document.getElementById("updating-msg").classList.add('hidden');
				break;
			case "setVoice":
				document.getElementById("updating-voice").classList.add('hidden');
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
		$('#device-status').html("Offline")
	}, 5000);

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