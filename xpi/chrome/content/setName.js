function initializeOverlay() {
	var pref = Components.classes["@mozilla.org/preferences-service;1"].
				getService(Components.interfaces.nsIPrefBranch);

	try {
		name = pref.getCharPref("extensions.mozilla-gnome-keyring.name");
	} catch(e) {}

	if (name == "Unknown" ) {
		var appInfo = Components.classes["@mozilla.org/xre/app-info;1"]
                        .getService(Components.interfaces.nsIXULAppInfo);
		pref.setCharPref("extensions.mozilla-gnome-keyring.name", appInfo.name );
	}
}

window.addEventListener("load", initializeOverlay, false);
