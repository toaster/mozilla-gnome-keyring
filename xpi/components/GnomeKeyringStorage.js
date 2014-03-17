Components.utils.import("resource://gre/modules/XPCOMUtils.jsm");
Components.utils.import("resource://gre/modules/Services.jsm");
Components.utils.import("resource://mozilla-gnome-keyring/modules/GnomeKeyringLibrary.jsm");

function GnomeKeyringStorage() {
    XPCOMUtils.defineLazyGetter(this, "_lib", function() new GnomeKeyringLibrary());
}

GnomeKeyringStorage.prototype = {
    classDescription: "GnomeKeyring Login Manager Storage",
    contractID:       "@pruetz.net/mozilla-gnome-keyring;1",
    classID:          Components.ID("{3fea2231-2801-45c0-8bdd-84b82bd6c62c}"),
    QueryInterface:   XPCOMUtils.generateQI([Components.interfaces.nsISupports,
                                             Components.interfaces.nsILoginManagerStorage]),
    uiBusy: false, // XXX seems to be needed in <=ff4.0b7

    /*
     * log
     *
     * Internal function for logging debug messages to the Error Console.
     */
    log: function(m) {
		if (!this._lib._debug)
			return;
        Services.console.logStringMessage("GnomeKeyringStorage: " + m);
    },

    /*
     * init
     *
     * Initialize this storage component.
     */
    init: function () {
		this.log( "init() Start" );
		this._lib.init();
    },

    initWithFile: function (inFile, outFile) {
		this.log( "initWithFile() Unimplemented function" );
    },

    addLogin: function (login) {
		this.log( "addLogin() Start" );
		login.QueryInterface(Components.interfaces.nsILoginMetaInfo);
		if( !login.guid || login.guid == "" ) {
			let g = Components.classes['@mozilla.org/uuid-generator;1'].getService(Components.interfaces.nsIUUIDGenerator);
			login.guid = login.username + "@" + login.hostname + "-" + g.generateUUID();
			this.log( "addLogin() New guid:" + login.guid );
		}
		this._lib.addLogin( login.username, login.usernameField, login.password, login.passwordField,
						login.formSubmitURL, login.httpRealm, login.hostname, login.guid );
    },

    removeLogin: function (login) {
		this.log( "removeLogin() Start" );
		this._lib.removeLogin( login.username, login.formSubmitURL, login.httpRealm, login.hostname );
    },

    modifyLogin: function (oldLogin, newLoginData) {
		this.log( "modifyLogin() Start" );
        let newLogin;
        let needsUpdate = false;
        newLogin = oldLogin.clone().QueryInterface(Components.interfaces.nsILoginMetaInfo);
        if (newLoginData instanceof Components.interfaces.nsILoginInfo) {
            newLogin.init(newLoginData.hostname,
                    newLoginData.formSubmitURL, newLoginData.httpRealm,
                    newLoginData.username, newLoginData.password,
                    newLoginData.usernameField, newLoginData.passwordField);
            newLogin.QueryInterface(Components.interfaces.nsILoginMetaInfo);

            if (newLogin.username != oldLogin.username) {
                this.log("Updating username");
                needsUpdate = true;
            }
            if (newLogin.password != oldLogin.password) {
                this.log("Updating password");
                needsUpdate = true;
            }
        }  else if (newLoginData instanceof Components.interfaces.nsIPropertyBag) {
            let propEnum = newLoginData.enumerator;
            while (propEnum.hasMoreElements()) {
                let prop = propEnum.getNext().QueryInterface(Components.interfaces.nsIProperty);
                switch (prop.name) {
                    // nsILoginInfo properties...
                    //
                    // only care about these 4 for updating
                    case "hostname":
                    case "username":
                    case "password":
                    case "formSubmitURL":
                        needsUpdate = true;
                        this.log("updating field: " + prop.name);
                    case "usernameField":
                    case "passwordField":
                    case "httpRealm":
                    // nsILoginMetaInfo properties...
                    case "guid":
                    case "timeCreated":
                    case "timeLastUsed":
                    case "timePasswordChanged":
                    case "timesUsed":
                        if (prop.name == "guid") {
                            this.log("Guid is changing?!  Not supported");
                            break;
                        }
                        newLogin[prop.name] = prop.value;
                        break;

                    // Fake property, allows easy incrementing.
                    case "timesUsedIncrement":
                        newLogin.timesUsed += prop.value;
                        break;

                    // Fail if caller requests setting an unknown property.
                    default:
                        throw "Unexpected propertybag item: " + prop.name;
                }
            }
        } else {
            throw "newLoginData needs an expected interface!";
        }
        if (needsUpdate) {
			this.removeLogin(oldLogin);
            this.addLogin(newLogin);
        }
    },

	_array2NsILoginInfo: function( outCount, entries ) {
 		this.log( "_array2NsILoginInfo() Start" );
		let logins = [];
        for (let i = 0; i < entries.length; i++) {
			let l = Components.classes['@mozilla.org/login-manager/loginInfo;1']
                    .createInstance(Components.interfaces.nsILoginInfo);
			if( entries[i].formSubmitURL != "" )
				l.formSubmitURL = entries[i].formSubmitURL;
			if( entries[i].httpRealm != "" )
				l.httpRealm = entries[i].httpRealm;
			if( entries[i].hostname != "" )
				l.hostname = entries[i].hostname;
			if( entries[i].username != "" )
				l.username = entries[i].username;
			if( entries[i].password != "" )
				l.password = entries[i].password;
			if( entries[i].usernameField != "" )
				l.usernameField = entries[i].usernameField;
			if( entries[i].passwordField != "" )
				l.passwordField = entries[i].passwordField;
			l.QueryInterface(Components.interfaces.nsILoginMetaInfo);
			if( entries[i].guid != "" )
				l.guid = entries[i].guid;
            logins.push(l);
        }
		this.log( "_array2NsILoginInfo() Count: " + logins.length );
		outCount.value = logins.length;
        return logins;
	},

    getAllLogins: function (outCount) {
		this.log( "getAllLogins() Start" );
        let entries = this._lib.getAllLogins();
		return this._array2NsILoginInfo( outCount, entries );
    },

    getAllEncryptedLogins: function (outCount) {
		this.log( "getAllEncryptedLogins() Start" );
        return this.getAllLogins(outCount);
    },

    searchLogins: function (outCount, matchData) {
		this.log( "searchLogins() Start" );
		let propEnum = matchData.enumerator;
		var guid;
		while (propEnum.hasMoreElements()) {
			let prop = propEnum.getNext().QueryInterface(Components.interfaces.nsIProperty);
			switch (prop.name) {
				case "guid":
					guid = prop.value;
					break;
/*
				case "hostname":
				case "username":
				case "password":
				case "formSubmitURL":
				case "usernameField":
				case "passwordField":
				case "httpRealm":
				// nsILoginMetaInfo properties...
//				case "guid":
				case "timeCreated":
				case "timeLastUsed":
				case "timePasswordChanged":
				case "timesUsed":
				case "timesUsedIncrement":*/
				// Fail if caller requests setting an unknown property.
				default:
					throw "Unexpected propertybag item: " + prop.name;
			}
		}
        let entries = this._lib.findLoginsWithGuid( guid );
 		return this._array2NsILoginInfo( outCount, entries );
    },

    removeAllLogins: function() {
		this.log( "removeAllLogins() Start" );
    },

    getAllDisabledHosts: function(count) {
		this.log( "getAllDisabledHosts() Start" );
        var result = this._lib.getAllDisabledHosts();
		count.value = result.length;
		return result;
    },

    getLoginSavingEnabled: function(hostname) {
		this.log( "getLoginSavingEnabled() Start" );
		return this._lib.getLoginSavingEnabled( hostname );
    },

    setLoginSavingEnabled: function(hostname, enabled) {
		this.log( "setLoginSavingEnabled() Start" );
		this._lib.setLoginSavingEnabled( hostname, enabled );
    },

    findLogins: function (outCount, hostname, submitURL, realm) {
		this.log( "findLogins() Start" );
        let entries = this._lib.findLogins( hostname, submitURL, realm );
		return this._array2NsILoginInfo( outCount, entries );
    },

    countLogins: function countLogins(hostname, submitURL, realm) {
		this.log( "countLogins() Start" );
		var count = this._lib.countLogins(hostname, submitURL, realm);
		this.log( "countLogins() counted = " + count );
		return count;
    },
};

const NSGetFactory = XPCOMUtils.generateNSGetFactory([GnomeKeyringStorage]);

