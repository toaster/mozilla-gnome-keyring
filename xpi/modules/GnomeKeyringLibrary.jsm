Components.utils.import("resource://gre/modules/XPCOMUtils.jsm");
Components.utils.import("resource://gre/modules/Services.jsm");
Components.utils.import("resource://gre/modules/ctypes.jsm");

var EXPORTED_SYMBOLS = [ "GnomeKeyringLibrary" ];

function GnomeKeyringLibrary() {
    this.log("GnomeKeyringLibrary() Start");
    this._prefBranch = Services.prefs.getBranch("extensions.gnomekeyring.");
    let gkr = this;
    const ioService = Components.classes["@mozilla.org/network/io-service;1"].getService(Components.interfaces.nsIIOService);
    var uri = ioService.newURI( "resource://libgnomekeyring", null, null);
    if( uri instanceof Components.interfaces.nsIFileURL ) {
		gkr.gnomeKeyring = ctypes.open(uri.file.path);
// void GnomeKeyring_SetKeyringName( const char* _keyringName );
		gkr._SetKeyringName = this.gnomeKeyring.declare("GnomeKeyring_SetKeyringName", ctypes.default_abi, ctypes.void_t, ctypes.char.ptr);
/*typedef struct {
	char* hostname;
	char* username;
	char* usernameField;
	char* password;
	char* passwordField;
	char* formSubmitURL;
	char* httpRealm;
	char* guid;
} LoginInfo;*/
		gkr._LoginInfo = ctypes.StructType( "LoginInfo", [
			{ hostname			: ctypes.char.ptr	},
			{ username			: ctypes.char.ptr	},
			{ usernameField		: ctypes.char.ptr	},
			{ password			: ctypes.char.ptr	},
			{ passwordField		: ctypes.char.ptr	},
			{ formSubmitURL		: ctypes.char.ptr	},
			{ httpRealm			: ctypes.char.ptr	},
			{ guid				: ctypes.char.ptr	}
		]);
//int32_t GnomeKeyring_GetAllLogins( uint32_t *aCount, LoginInfo **aLogins )
		gkr._GetAllLogins = this.gnomeKeyring.declare("GnomeKeyring_GetAllLogins", ctypes.default_abi, ctypes.int32_t, ctypes.uint32_t.ptr, gkr._LoginInfo.ptr.ptr );
// int32_t GnomeKeyring_FindLogins( uint32_t *count, const char* aHostname, const char* aActionURL, const char* aHttpRealm, LoginInfo **logins )
		gkr._FindLogins = this.gnomeKeyring.declare("GnomeKeyring_FindLogins", ctypes.default_abi, ctypes.int32_t, ctypes.uint32_t.ptr, ctypes.char.ptr, ctypes.char.ptr, ctypes.char.ptr, gkr._LoginInfo.ptr.ptr );
// int32_t GnomeKeyring_FindLoginsWithGUID( uint32_t *count, const char* guid, LoginInfo **logins )
		gkr._FindLoginsWithGUID = this.gnomeKeyring.declare("GnomeKeyring_FindLoginsWithGUID", ctypes.default_abi, ctypes.int32_t, ctypes.uint32_t.ptr, ctypes.char.ptr, gkr._LoginInfo.ptr.ptr );
// int32_t GnomeKeyring_CountLogins( const char* aHostname, const char* aActionURL, const char* aHttpRealm, uint32_t* _retval );
		gkr._CountLogins = this.gnomeKeyring.declare("GnomeKeyring_CountLogins", ctypes.default_abi, ctypes.int32_t, ctypes.char.ptr, ctypes.char.ptr, ctypes.char.ptr, ctypes.uint32_t.ptr );
//int32_t GnomeKeyring_AddLogin( const char* aUsername, const char* aUsernameField,
//							const char* aPassword, const char* aPasswordField,
//							const char* aActionURL, const char* aHttpRealm,
//							const char* aHostname, const char* aGUID );
		gkr._AddLogin = this.gnomeKeyring.declare("GnomeKeyring_AddLogin", ctypes.default_abi, ctypes.int32_t, ctypes.char.ptr, ctypes.char.ptr, ctypes.char.ptr,ctypes.char.ptr, ctypes.char.ptr, ctypes.char.ptr, ctypes.char.ptr, ctypes.char.ptr );
//int32_t GnomeKeyring_RemoveLogin( const char* aUsername, const char* aActionURL, 
//							   const char* aHttpRealm, const char* aHostname )
		gkr._RemoveLogin = this.gnomeKeyring.declare("GnomeKeyring_RemoveLogin", ctypes.default_abi, ctypes.int32_t, ctypes.char.ptr, ctypes.char.ptr, ctypes.char.ptr, ctypes.char.ptr );
//int32_t GnomeKeyring_GetLoginSavingEnabled( const char* aHostname, int32_t* _isEnabled )
		gkr._GetLoginSavingEnabled = this.gnomeKeyring.declare("GnomeKeyring_GetLoginSavingEnabled", ctypes.default_abi, ctypes.int32_t, ctypes.char.ptr, ctypes.uint32_t.ptr );
//int32_t GnomeKeyring_SetLoginSavingEnabled( const char* aHostname, const int32_t isEnabled )
		gkr._SetLoginSavingEnabled = this.gnomeKeyring.declare("GnomeKeyring_SetLoginSavingEnabled", ctypes.default_abi, ctypes.int32_t, ctypes.char.ptr, ctypes.uint32_t );
//int32_t GnomeKeyring_GetAllDisabledHosts( uint32_t *aCount, char*** aHostnames )
		gkr._GetAllDisabledHosts = this.gnomeKeyring.declare("GnomeKeyring_GetAllDisabledHosts", ctypes.default_abi, ctypes.int32_t, ctypes.uint32_t.ptr, ctypes.char.ptr.ptr.ptr );
    }
    this._signonPrefBranch = Services.prefs.getBranch("signon.");
    this._debugObserver = {
        QueryInterface: XPCOMUtils.generateQI([Components.interfaces.nsIObserver,
                                               Components.interfaces.nsISupportsWeakReference]),
        observe: function(subject, topic, data) {
            gkr._debug = gkr._signonPrefBranch.getBoolPref("debug");
            gkr.log("_debugObserver debug preference updated: " + gkr._debug);
        }
    };
    this._signonPrefBranch.QueryInterface(Components.interfaces.nsIPrefBranch);
    this._signonPrefBranch.addObserver("debug", this._debugObserver, false);
    this._debug = this._signonPrefBranch.getBoolPref("debug");
            
    this._keyringNameObserver = {
        QueryInterface: XPCOMUtils.generateQI([Components.interfaces.nsIObserver,
                                               Components.interfaces.nsISupportsWeakReference]),
        observe: function(subject, topic, data) {
            gkr._keyringName = gkr._prefBranch.getCharPref("name");
            gkr.log("_keyringNameObserver Name preference updated: " + gkr._keyringName);
	    gkr._setKeyringName();
        }
    };
    this._prefBranch.QueryInterface(Components.interfaces.nsIPrefBranch);
    this._prefBranch.addObserver("name", this._keyringNameObserver, false);
    this._keyringName = this._prefBranch.getCharPref("name");
    this.log("GnomeKeyringLibrary() name = " + this._keyringName );
}

GnomeKeyringLibrary.reload = function() {
	Services.console.logStringMessage("GnomeKeyringLibrary: Reloading GnomeKeyringLibrary module");
	let l = Components.classes['@mozilla.org/moz/jssubscript-loader;1'].getService(Components.interfaces.mozIJSSubScriptLoader);
	l.loadSubScript("resource://GnomeKeyring/modules/GnomeKeyringLibrary.jsm");
};

GnomeKeyringLibrary.prototype = {
		log: function(m) {
		if (this._debug)
			Services.console.logStringMessage("GnomeKeyringLibrary: " + m);
    },

    init: function() {
		this.log( "init() Start" );
		this._setKeyringName();
    },

    _setKeyringName: function() {
		this.log( "_setKeyringName() Start" );
		this._SetKeyringName( this._keyringName );
    },
    
    countLogins: function(aHostname, aActionURL, aHttpRealm) {
		this.log( "countLogins( " + aHostname + ", " + aActionURL + ", " + aHttpRealm + " ) Start" );
		let count = new ctypes.uint32_t; 
		this._CountLogins( aHostname, aActionURL, aHttpRealm, count.address() );
		this.log( "countLogins() logins counted = " + count.value );
		return count.value;
    },
    
	_loginInfo2Array: function( count, loginInfo ) {
		this.log( "_loginInfo2Array() Start loginInfo: " + loginInfo.toString() );
		var loginArray = [];
		for(var i = 0; i < count; i++ ) {
			var r = {
				hostname			: loginInfo.contents[i].hostname.readString(),
				username			: loginInfo.contents[i].username.readString(),
				usernameField		: loginInfo.contents[i].usernameField.readString(),
				password			: loginInfo.contents[i].password.readString(),
				passwordField		: loginInfo.contents[i].passwordField.readString(),
				formSubmitURL		: loginInfo.contents[i].formSubmitURL.readString(),
				httpRealm			: loginInfo.contents[i].httpRealm.readString(),
				guid				: loginInfo.contents[i].guid.readString()
			};
			loginArray.push( r );
			this.log( "_loginInfo2Array() Adding login for: " + r.hostname + ", username: " + r.username );
		}
		this.log( "_loginInfo2Array() Count: " + loginArray.length );
		return loginArray;
	},
    
    getAllLogins: function() {
		this.log( "getAllLogins() Start" );
		var count = new ctypes.uint32_t; 
		var loginInfo = new this._LoginInfo.ptr;
		this._GetAllLogins( count.address(), loginInfo.address() );
		loginInfoAsArray = ctypes.cast( loginInfo, this._LoginInfo.array( count.value ).ptr );
		return this._loginInfo2Array( count.value, loginInfoAsArray );
    },
    
    findLogins: function( aHostname, aActionURL, aHttpRealm) {
		this.log( "findLogins( " + aHostname + ", " + aActionURL + ", " + aHttpRealm + " ) Start" );
		let count = new ctypes.uint32_t; 
		let loginInfo = new this._LoginInfo.ptr;
		this._FindLogins( count.address(), aHostname, aActionURL, aHttpRealm, loginInfo.address() );
		loginInfoAsArray = ctypes.cast( loginInfo, this._LoginInfo.array( count.value ).ptr );
		return this._loginInfo2Array( count.value, loginInfoAsArray );
    },
    
    findLoginsWithGuid: function( aGuid ) {
		this.log( "findLoginsWithGuid( " + aGuid + " ) Start" );
		let count = new ctypes.uint32_t; 
		let loginInfo = new this._LoginInfo.ptr;
		this._FindLoginsWithGUID( count.address(), aGuid, loginInfo.address() );
		loginInfoAsArray = ctypes.cast( loginInfo, this._LoginInfo.array( count.value ).ptr );
		return this._loginInfo2Array( count.value, loginInfoAsArray );
    },
    
    removeLogin: function(aUsername, aActionURL, aHttpRealm, aHostname) {
		this.log( "removeLogin( " + aUsername + ", " + aHostname + ", " + aActionURL + ", " + aHttpRealm + " ) Start" );
		this._RemoveLogin(aUsername, aActionURL, aHttpRealm, aHostname);
    },
    
	addLogin: function( aUsername, aUsernameField, aPassword, aPasswordField,
						aActionURL, aHttpRealm, aHostname, aGUID ) {
		this.log( "addLogin( " + aUsername + ", " + aHostname + ", " + aGUID + " ) Start" );
		return this._AddLogin( aUsername, aUsernameField, aPassword, aPasswordField,
						aActionURL, aHttpRealm, aHostname, aGUID );
	},
    
    getLoginSavingEnabled: function(aHostname) {
		this.log( "getLoginSavingEnabled( " + aHostname + " ) Start" );
		let isEnabled = new ctypes.uint32_t; 
		this._GetLoginSavingEnabled( aHostname, isEnabled.address() );
		this.log( "countLogins() logins counted = " + isEnabled.value != 0 );
		return isEnabled.value != 0;
    },
    
    setLoginSavingEnabled: function(aHostname, isEnabled) {
		this.log( "setLoginSavingEnabled( " + aHostname + "," + isEnabled + " ) Start" );
		this._SetLoginSavingEnabled( aHostname, isEnabled );
    },
    
    getAllDisabledHosts: function() {
		this.log( "getAllDisabledHosts() Start" );
		let count = new ctypes.uint32_t; 
		let disabledHosts = new ctypes.char.ptr.ptr;
		this._GetAllDisabledHosts( count.address(), disabledHosts.address() );
		disabledHostsAsArray = ctypes.cast( disabledHosts, ctypes.char.ptr.array( count.value ).ptr );
		var disabledHostsArray = [];
		for(var i = 0; i < count.value; i++ ) {
			disabledHostsArray.push( disabledHostsAsArray.contents[i].readString() );
			this.log( "getAllDisabledHosts() Adding host: " + disabledHostsAsArray.contents[i].readString() );
		}
		this.log( "getAllDisabledHosts() Count: " + disabledHostsArray.length );
		return disabledHostsArray;
    }
};
