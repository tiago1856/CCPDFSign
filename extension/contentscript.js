
//console.log("SCRIPT INJECTED");

const ID = 'agmijmlopfbbgdbliclncpnddhhdmodo';
//const ID = 'mgmgfopdehofnnhmkgnepkmbajmkonpo';

localStorage.setItem('has_sign_extension',true);

// check if extension exists
function extensionExists() {
	try {
	  chrome.runtime.sendMessage(ID,
		{key:"test"}, 
		function(message) {
			localStorage.setItem('has_sign_extension',true);
			//console.warn("exists", message);
			if (chrome.runtime.lastError) {
				// ERROR --- no response: most likely due to wrong ID		
			}
			return true;
		}
	  );
	} catch (e) {
	  //console.warn('No extension');
	  localStorage.setItem('has_sign_extension',false);
	}
}

extensionExists();


// from app
window.addEventListener("message", (event) => {
	// We only accept messages from ourselves
	if (event.source != window) {
	  return;
	}
	if (event.data.origin && (event.data.origin === "APP")) {
		//console.log("sending to background...", event.data);
		// send to background		
		try {
		  chrome.runtime.sendMessage(ID,
			{ key: 'pdf', content: event.data.content}, 
			function(message) {
				localStorage.setItem('has_sign_extension',true);
				// error => no native application
				if (message.result && message.result === 'ERROR') {
					console.error(message.message);
					// not from native, but whathever
					window.postMessage({ origin: "NATIVE", content: message }, "*");
				}
				return true;
			}
		  );
		} catch (e) {
		  console.warn('No extension');		  
		  localStorage.setItem('has_sign_extension',false);
		}

	}
	return true;
}, false);


// send to js app
chrome.runtime.onMessage.addListener((message, sender, sendResponse) => {
  window.postMessage({ origin: "NATIVE", content: message }, "*");
  return true;
});

