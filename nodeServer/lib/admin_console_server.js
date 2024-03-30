"use strict";
/*
 * Respond to commands over a websocket to relay UDP commands to a local program
 */

// Large portion adapted from udpServer example

var fs   = require('fs');
var socketio = require('socket.io');
var io;

var dgram = require('dgram');

exports.listen = function(server) {
	io = socketio.listen(server);
	io.set('log level 1');

	io.sockets.on('connection', function(socket) {
		handleCommand(socket);
	});
};

const MAX_PACKET_SIZE = 65000;
function handleCommand(socket) {
	// Passed file to relay
	socket.on('fileUpload', function(fileData) {
		var PORT = 12345;
		var HOST = '127.0.0.1';

		var client = dgram.createSocket('udp4');
		console.log('Received data:', fileData);

		var base64Data = fileData.substring(4); // Remove the 'cl1 ' prefix
		var binaryData = Buffer.from(base64Data, 'base64');
		var buffer = Buffer.from(binaryData);

		var prefix = fileData.substring(0, 4);
		let numPackets = Math.ceil(buffer.length / MAX_PACKET_SIZE);
		var startBuffer = Buffer.from(prefix + " " + numPackets);

		client.send(startBuffer, 0, startBuffer.length, PORT, HOST, function(err, bytes) {
			if (err) {
				throw err;
			}
		});

		console.log("handleCommand data byteLength = " + buffer.length + " bytes\n");
		let start = 0;
		while (start < buffer.length) {
			let end = Math.min(start + MAX_PACKET_SIZE, buffer.length);
			let packet = buffer.slice(start, end);
			client.send(packet, 0, packet.length, PORT, HOST, function(err, bytes) {
				if (err) {
					throw err;
				}
			});
			start = end;
		}

		console.log("Sent file");

	});
	// Pased string of comamnd to relay
	socket.on('udpCommand', function(data) {

		// Info for connecting to the local process via UDP
		var PORT = 12345;
		var HOST = '127.0.0.1';
		var buffer = new Buffer(data);

		var client = dgram.createSocket('udp4');

		var timeout = setTimeout(function() {
			var errMsg = "No response from ByteSpeak application. Is it running?"
			socket.emit('errorReply', errMsg);
			client.close();
		}, 1000);

		client.send(buffer, 0, buffer.length, PORT, HOST, function(err, bytes) {
			if (err) 
				throw err;
		});

		client.on('listening', function () {
			var address = client.address();
		});
		// Handle an incoming message over the UDP from the local application.
		client.on('message', function (message, remote) {

			var reply = message.toString('utf8')
			socket.emit('commandReply', reply);
			clearTimeout(timeout);
			client.close();

		});
		client.on("UDP Client: close", function() {
			console.log("closed");
			clearTimeout(timeout);
		});
		client.on("UDP Client: error", function(err) {
			clearTimeout(timeout);
			console.log("error: ",err);
		});
	});

	// proc/uptime request from A3
	// Implement if able to set up securely

	// socket.on('proc', function(fileName) {
	// 	// NOTE: Very unsafe? Why?
	// 	// Hint: think of ../
	// 	var absPath = "/proc/" + fileName;
		
	// 	fs.exists(absPath, function(exists) {
	// 		if (exists) {
	// 			// Can use 2nd param: 'utf8', 
	// 			fs.readFile(absPath, function(err, fileData) {
	// 				if (err) {
	// 					emitSocketData(socket, fileName, 
	// 							"ERROR: Unable to read file " + absPath);
	// 				} else {
	// 					emitSocketData(socket, fileName, 
	// 							fileData.toString('utf8'));
	// 				}
	// 			});
	// 		} else {
	// 			emitSocketData(socket, fileName, 
	// 					"ERROR: File " + absPath + " not found.");
	// 		}
	// 	});
	// });
};

// helper for proc/uptime

// function emitSocketData(socket, fileName, contents) {
// 	var result = {
// 			fileName: fileName,
// 			contents: contents
// 	}
// 	socket.emit('fileContents', result);	
// }