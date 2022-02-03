
var port = null;
var reiceiving_data = "";
var tab = null;

function onMsg(msg) {
	if (msg.hasOwnProperty('data')) {
		reiceiving_data += msg["data"]
	} else if (msg.hasOwnProperty('status') && msg["status"] === 'DONE')  {
		// send the signed pdf
		chrome.tabs.sendMessage(tab,  { key: 'result', data: reiceiving_data}, function(response) {});
	} else if (msg.hasOwnProperty('error'))  {
		chrome.tabs.sendMessage(tab,  { key: 'error', data: msg.error}, function(response) {});  
	} else if (msg.hasOwnProperty('status'))  {
		chrome.tabs.sendMessage(tab,  { key: 'status', data: msg.status}, function(response) {});  
	}
}

function onDisconnect(msg) {
	if (chrome.runtime.lastError) {
		sendResponse( { key: 'test', result: "ERROR", message: "No native application installed!"});
	}
	port = null;
}

// [js app -> content] -> background -> [native app]
// run native application everytime a document is sent to be signed - 4 GB max
chrome.runtime.onMessage.addListener(function(message, sender, sendResponse) {
	if (message.key && message.key === 'test') {
		sendResponse( { key: 'test', result: "OK", message: "Extension OK!"});
	}
	else if (message.key && message.key === 'pdf') {		
		reiceiving_data = "";		
		if (port) {
			port.onMessage.removeListener(onMsg);
			port.onDisconnect.removeListener(onDisconnect);
			port.disconnect();
			port = null;
		}
		// run native app
		port = chrome.runtime.connectNative('com.pdfsigncc.nativehost');
		port.onDisconnect.addListener(onDisconnect);
		// save the tab of whoever send the message
		tab = sender.tab.id;
		// [native app] -> background -> [content js -> app]
		port.onMessage.addListener(onMsg);
	   	port.postMessage({ data: message.content.data, pos_x: message.content.pos_x, pos_y: message.content.pos_y, page: message.content.page });
		sendResponse( { key: 'test', result: "OK", message: "Extension OK!"});
	}
	return true; // prevents the Unchecked runtime.lastError: The message port closed before a response was received.
 });

