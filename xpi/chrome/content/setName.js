function initializeOverlay() {
	var pref = Components.classes["@mozilla.org/preferences-service;1"].
				getService(Components.interfaces.nsIPrefBranch);

	try {
		name = pref.getCharPref("extensions.gnomekeyring.name");
	} catch(e) {}

	if (name == "Unknown" ) {
		var appInfo = Components.classes["@mozilla.org/xre/app-info;1"]
                        .getService(Components.interfaces.nsIXULAppInfo);
		pref.setCharPref("extensions.gnomekeyring.name", appInfo.name );
	}
}

window.addEventListener("load", initializeOverlay, false);
