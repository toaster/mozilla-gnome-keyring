/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is KDE Wallet password manager storage.
 *
 * The Initial Developer of the Original Code is
 * Guillermo Molina <guillermoadrianmolina@hotmail.com>
 * Portions created by the Initial Developer are Copyright (C) 2009
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */


#include "GnomeKeyring.h"
#include <glib.h>
#include <gnome-keyring.h>
#include <gnome-keyring-memory.h>
#include <string.h>
// TODO should use profile identifier instead of a constant
#define UNIQUE_PROFILE_ID "v2"

/** magic signature that a keyring item represents login information */
const char *kLoginInfoMagicAttrName = "mozLoginInfoMagic";
const char *kLoginInfoMagicAttrValue = "loginInfoMagic" UNIQUE_PROFILE_ID;

const char *kHostnameAttr = "hostname";
const char *kFormSubmitURLAttr = "formSubmitURL";
const char *kHttpRealmAttr = "httpRealm";
const char *kUsernameFieldAttr = "usernameField";
const char *kPasswordFieldAttr = "passwordField";
const char *kUsernameAttr = "username";
const char *kPasswordAttr = "password";
const char *kGuidAttr = "guid";

/** magic signature that a keyring item represents login information */
const char *kHostInfoMagicAttrName = "mozHostInfoMagic";
const char *kHostInfoMagicAttrValue = "hostInfoMagic" UNIQUE_PROFILE_ID;

char *keyringName = NULL;

uint32_t loginInfoBufferCount = 0;
LoginInfo* loginInfoBuffer = NULL;

uint32_t disabledHostsBufferCount = 0;
char** disabledHostsBuffer = NULL;

void freeLoginInfoBuffer() {
	if( !loginInfoBuffer && !loginInfoBufferCount )
		return;
	if( loginInfoBuffer && loginInfoBufferCount ) {
		int i;
		for( i = 0; i < loginInfoBufferCount; i++ ) {
			gnome_keyring_memory_free( loginInfoBuffer[ i ].password );
			g_free( loginInfoBuffer[ i ].hostname );
			g_free( loginInfoBuffer[ i ].username );
			g_free( loginInfoBuffer[ i ].usernameField );
			g_free( loginInfoBuffer[ i ].passwordField );
			g_free( loginInfoBuffer[ i ].formSubmitURL );
			g_free( loginInfoBuffer[ i ].httpRealm );
			g_free( loginInfoBuffer[ i ].guid );
		}
		g_free( loginInfoBuffer );
		loginInfoBufferCount = 0;
		return;
	}
	g_critical( "freeLoginInfoBuffer: Memory buffer and count differ" );
}

gboolean allocLoginInfoBuffer( uint32_t count ) {
	if( loginInfoBufferCount || loginInfoBuffer )
		freeLoginInfoBuffer();
	loginInfoBufferCount = count;
	loginInfoBuffer = g_new0( LoginInfo, count );
	if( !loginInfoBuffer ) {
		g_critical( "allocLoginInfoBuffer: Could not alloc memory buffer" );
		return FALSE;
	}
	return TRUE;
}

void freeDisabledHostsBuffer() {
	if( !disabledHostsBuffer && !disabledHostsBufferCount )
		return;
	if( disabledHostsBuffer && disabledHostsBufferCount ) {
		int i;
		for( i = 0; i < disabledHostsBufferCount; i++ )
			g_free( disabledHostsBuffer[ i ] );
		g_free( disabledHostsBuffer );
		disabledHostsBufferCount = 0;
		return;
	}
	g_critical( "freeDisabledHostsBuffer: Memory buffer and count differ" );
}

gboolean allocDisabledHostsBuffer( uint32_t count ) {
	if( disabledHostsBuffer || disabledHostsBufferCount )
		freeDisabledHostsBuffer();
	disabledHostsBufferCount = count;
	disabledHostsBuffer = g_new0( char*, count );
	if( !disabledHostsBuffer ) {
		g_critical( "allocDisabledHostsBuffer: Could not alloc memory buffer" );
		return FALSE;
	}
	return TRUE;
}

/**
* Filter out items based on a user-supplied function and data. Code
* adapted from GLib's g_list_remove_all.
*
* @filter: gpointer -> bool. false to remove from the array. should
* also free the memory used by the data, if appropriate.
*/
GList* _g_list_remove_all_custom(GList* list,
                          gboolean (*filter)(gconstpointer, gpointer),
                          gconstpointer user_data) {
	GList *tmp = list;

	while (tmp) {
		if (filter(user_data, tmp->data))
			tmp = tmp->next;
		else {
			GList *next = tmp->next;

			if (tmp->prev)
				tmp->prev->next = next;
			else
				list = next;
			if (next)
				next->prev = tmp->prev;

			g_slice_free(GList, tmp);
			tmp = next;
		}
	}
	return list;
}

gboolean matchKeyring(gconstpointer user_data, gpointer data) {
	GnomeKeyringFound* found = (GnomeKeyringFound*) data;
	const char* keyring = (const char*) user_data;
	gboolean keep = !g_strcmp0(found->keyring, keyring);
	if (!keep)
    	gnome_keyring_found_free(found);
	return keep;
}

gboolean matchGUID(gconstpointer user_data, gpointer data) {
	GnomeKeyringFound* found = (GnomeKeyringFound*) data;
	const char* guid = (const char*) user_data;

	int j;
	GnomeKeyringAttribute* attributeArray = (GnomeKeyringAttribute*) found->attributes->data;
	gboolean keep = FALSE;
	for( j = 0; j < found->attributes->len; j++ ) {
		const char *attributeName = attributeArray[j].name;
		const char *attributeValue = attributeArray[j].value.string;

		if (!g_strcmp0(attributeName, kGuidAttr) && !g_strcmp0(attributeValue, guid))
			keep = TRUE;
	}

	if (!keep)
    	gnome_keyring_found_free(found);
	return keep;
}

void GnomeKeyring_SetKeyringName( const char* _keyringName  ) {
	g_debug( "Set keyringName to %s", _keyringName );
	if( keyringName )
		g_free( keyringName );
	keyringName =  g_ascii_strdown( _keyringName, -1 );
	g_debug( "KeyringName is %s", keyringName );
}

int32_t checkKeyRing() {
	if( keyringName == NULL )
		keyringName = g_strdup( "firefox" );

	GnomeKeyringResult result = gnome_keyring_unlock_sync( keyringName, NULL );
	if( result == GNOME_KEYRING_RESULT_OK )
		return TRUE;

	result = gnome_keyring_create_sync( keyringName, NULL );
	if( result != GNOME_KEYRING_RESULT_OK ) {
		g_critical( "GnomeKeyring_Init: %s", gnome_keyring_result_to_message( result ) );
		return FALSE;
	}

	return TRUE;
}

int32_t GnomeKeyring_AddLogin( const char* aUsername, const char* aUsernameField,
							const char* aPassword, const char* aPasswordField,
							const char* aActionURL, const char* aHttpRealm,
							const char* aHostname, const char* aGUID ) {
	if( !checkKeyRing() )
		return FALSE;

	GnomeKeyringAttributeList* attributes = gnome_keyring_attribute_list_new();
	gnome_keyring_attribute_list_append_string( attributes, kLoginInfoMagicAttrName, kLoginInfoMagicAttrValue );
	gnome_keyring_attribute_list_append_string( attributes, kUsernameAttr, aUsername );
	gnome_keyring_attribute_list_append_string( attributes, kUsernameFieldAttr, aUsernameField );
	gnome_keyring_attribute_list_append_string( attributes, kPasswordFieldAttr, aPasswordField );
	gnome_keyring_attribute_list_append_string( attributes, kFormSubmitURLAttr, aActionURL );
	gnome_keyring_attribute_list_append_string( attributes, kHttpRealmAttr, aHttpRealm );
	gnome_keyring_attribute_list_append_string( attributes, kHostnameAttr, aHostname );
	gnome_keyring_attribute_list_append_string( attributes, kGuidAttr, aGUID );

	g_debug("Add guid = %s", aGUID);

	//TODO: Verify the guid is not already inside de DB !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

	guint32 item_id;
	GnomeKeyringResult result = gnome_keyring_item_create_sync(
		keyringName,
		GNOME_KEYRING_ITEM_GENERIC_SECRET,
		aGUID,
		attributes,
		aPassword,
		TRUE,
		&item_id
	);
	gnome_keyring_attribute_list_free( attributes );

	if( result != GNOME_KEYRING_RESULT_OK ) {
		g_critical( "GnomeKeyring_AddLogin: %s", gnome_keyring_result_to_message( result ) );
		return FALSE;
	}
	return TRUE;
}

gboolean findLogins( const char* aUsername,
				const char* aHostname,
				const char* aActionURL,
				const char* aHttpRealm,
				GList **found_list) {
	if( !checkKeyRing() )
		return FALSE;

	GnomeKeyringAttributeList* attributes = gnome_keyring_attribute_list_new();
	gnome_keyring_attribute_list_append_string( attributes, kLoginInfoMagicAttrName, kLoginInfoMagicAttrValue );

	g_debug("Find logins that have aActionURL = %s,aHttpRealm = %s,aHostname = %s", aActionURL, aHttpRealm, aHostname);

	if( aUsername && strlen(aUsername) )
		gnome_keyring_attribute_list_append_string( attributes, kUsernameAttr, aUsername );
	if( aActionURL && strlen(aActionURL) )
		gnome_keyring_attribute_list_append_string( attributes, kFormSubmitURLAttr, aActionURL );
	if( aHttpRealm && strlen(aHttpRealm) )
		gnome_keyring_attribute_list_append_string( attributes, kHttpRealmAttr, aHttpRealm );
	if( aHostname && strlen(aHostname) )
		gnome_keyring_attribute_list_append_string( attributes, kHostnameAttr, aHostname );

	g_debug("Find logins that have aActionURL = %s,aHttpRealm = %s, aHostname = %s", aActionURL, aHttpRealm, aHostname);

	GnomeKeyringResult result = gnome_keyring_find_items_sync(
		GNOME_KEYRING_ITEM_GENERIC_SECRET,
		attributes,
		found_list
	);

	if( result == GNOME_KEYRING_RESULT_NO_MATCH )
		return TRUE;

	if( result != GNOME_KEYRING_RESULT_OK  ) {
		g_debug( "findLogins: %s", gnome_keyring_result_to_message( result ) );
		return FALSE;
	}

	*found_list = _g_list_remove_all_custom(*found_list, matchKeyring, keyringName);

	return TRUE;
}

gboolean keyringEntryToLoginInfo( GList* found_list,
							 uint32_t *count,
							 LoginInfo **logins ) {
	*count = g_list_length( found_list );

	g_debug( "keyringEntryToLoginInfo: Found %d items", *count );
	if( *count == 0 )
		return TRUE;

	if( !allocLoginInfoBuffer( *count ) ) {
		g_critical( "keyringEntryToLoginInfo: Could not alloc buffer for logins" );
		return FALSE;
	}

	int i = 0;
	GList *iterator = found_list;
	while( iterator != NULL ) {
		GnomeKeyringFound* item = (GnomeKeyringFound*)iterator->data;
		int j;
		GnomeKeyringAttribute* attributeArray = (GnomeKeyringAttribute*) item->attributes->data;
		for( j = 0; j < item->attributes->len; j++ ) {
			if (attributeArray[j].type != GNOME_KEYRING_ATTRIBUTE_TYPE_STRING)
			  continue;

			const char *attributeName = attributeArray[j].name;
			const char *attributeValue = attributeArray[j].value.string;

			g_debug( "keyringEntryToLoginInfo: attribute name = %s", attributeName );
			if (!g_strcmp0(attributeName, kHostnameAttr))
				loginInfoBuffer[i].hostname = g_strdup( attributeValue );
			else if (!g_strcmp0(attributeName, kUsernameAttr))
				loginInfoBuffer[i].username = g_strdup( attributeValue );
			else if (!g_strcmp0(attributeName, kUsernameFieldAttr))
				loginInfoBuffer[i].usernameField = g_strdup( attributeValue );
			else if (!g_strcmp0(attributeName, kPasswordFieldAttr))
				loginInfoBuffer[i].passwordField = g_strdup( attributeValue );
			else if (!g_strcmp0(attributeName, kFormSubmitURLAttr))
				loginInfoBuffer[i].formSubmitURL = g_strdup( attributeValue );
			else if (!g_strcmp0(attributeName, kHttpRealmAttr))
				loginInfoBuffer[i].httpRealm = g_strdup( attributeValue );
			else if (!g_strcmp0(attributeName, kGuidAttr))
				loginInfoBuffer[i].guid = g_strdup( attributeValue );
			else if (g_strcmp0(attributeName, kLoginInfoMagicAttrName)){
				g_debug("keyringEntryToLoginInfo: Unknown attribute %s = %s", attributeName, attributeValue);
			}
		}
		loginInfoBuffer[i].password = gnome_keyring_memory_strdup( item->secret );
		i++;
		iterator = g_list_next( iterator );
	}
	gnome_keyring_found_list_free( found_list );
	if( i != loginInfoBufferCount || i != *count ) {
		g_critical("keyringEntryToLoginInfo: loginInfoBuffer is not ok");
		return FALSE;
	}
	*logins = loginInfoBuffer;

	return TRUE;
}

int32_t GnomeKeyring_FindLogins( uint32_t *count,
			const char* aHostname,
			const char* aActionURL,
			const char* aHttpRealm,
			LoginInfo **logins ) {
	*count = 0;
	GList *found_list;

	if( !findLogins( "", aHostname, aActionURL, aHttpRealm, &found_list ) )
		return FALSE;

	return keyringEntryToLoginInfo( found_list, count, logins );
}

int32_t GnomeKeyring_GetAllLogins( uint32_t *aCount, LoginInfo **aLogins ) {
	return GnomeKeyring_FindLogins( aCount, "", "", "", aLogins);
}

int32_t GnomeKeyring_CountLogins( const char* aHostname, const char* aActionURL, const char* aHttpRealm, uint32_t* _retval ) {
	g_debug( "GnomeKeyring_CountLogins: aHostname = %s, aActionURL = %s, aHttpRealm = %s", aHostname, aActionURL, aHttpRealm );
	*_retval = 0;

	if( !checkKeyRing() )
		return FALSE;

	GList *found_list;
	if( !findLogins( "", aHostname, aActionURL, aHttpRealm, &found_list ) )
		return FALSE;

	*_retval = g_list_length( found_list );
	return TRUE;
}

int32_t GnomeKeyring_FindLoginsWithGUID( uint32_t *count,
								const char* aGUID,
								LoginInfo **logins ) {
	*count = 0;

	if( !checkKeyRing() )
		return FALSE;

	GList *found_list;
	if( !findLogins( "", "", "", "", &found_list ) )
		return FALSE;

	found_list = _g_list_remove_all_custom(found_list, matchGUID, aGUID);

	return keyringEntryToLoginInfo( found_list, count, logins );
}

int32_t GnomeKeyring_RemoveLogin( const char* aUsername, const char* aActionURL,
							   const char* aHttpRealm, const char* aHostname ) {

	if( !checkKeyRing() )
		return FALSE;

	g_debug("GnomeKeyring_RemoveLogin: Remove username = %s", aUsername );

	GList *found_list;
	if( !findLogins( aUsername, aHostname, aActionURL, aHttpRealm, &found_list ) )
		return FALSE;

	int count = g_list_length( found_list );

	if( count == 0 )
		return FALSE;
/*
	if( count > 1 )
		g_warning( "GnomeKeyring_RemoveLogin: removing more than one login" );
*/
	GList *iterator = found_list;
	while( iterator != NULL ) {
		GnomeKeyringFound* item = (GnomeKeyringFound*)iterator->data;

		GnomeKeyringResult result = gnome_keyring_item_delete_sync(item->keyring, item->item_id);
		if( result != GNOME_KEYRING_RESULT_OK  )
			g_debug( "GnomeKeyring_RemoveLogin: %s", gnome_keyring_result_to_message( result ) );

		iterator = g_list_next( iterator );
	}

	return TRUE;
}

int32_t GnomeKeyring_RemoveAllLogins() {
	return GnomeKeyring_RemoveLogin( "", "", "", "" );
}

gboolean findDisabledHosts( const char* aHostname,
				GList **found_list) {
	if( !checkKeyRing() )
		return FALSE;

	GnomeKeyringAttributeList* attributes = gnome_keyring_attribute_list_new();
	gnome_keyring_attribute_list_append_string( attributes, kHostInfoMagicAttrName, kHostInfoMagicAttrValue );

	g_debug("findDisabledHosts: Find disabled host that have aHostname = %s", aHostname);

	if( aHostname && strlen(aHostname) )
		gnome_keyring_attribute_list_append_string( attributes, kHostnameAttr, aHostname );

	GnomeKeyringResult result = gnome_keyring_find_items_sync(
		GNOME_KEYRING_ITEM_NOTE,
		attributes,
		found_list
	);

	if( result == GNOME_KEYRING_RESULT_NO_MATCH )
		return TRUE;

	if( result != GNOME_KEYRING_RESULT_OK  ) {
		g_debug( "findDisabledHosts: %s", gnome_keyring_result_to_message( result ) );
		return FALSE;
	}

	*found_list = _g_list_remove_all_custom(*found_list, matchKeyring, keyringName);

	return TRUE;
}

int32_t GnomeKeyring_SetLoginSavingEnabled( const char* aHostname, const int32_t isEnabled ) {
	if( !checkKeyRing() )
		return FALSE;

	g_debug("GnomeKeyring_SetLoginSavingEnabled: host = %s", aHostname );

  	if( isEnabled ) {
		GList *found_list;
		if( !findDisabledHosts( aHostname, &found_list ) )
			return FALSE;

		int count = g_list_length( found_list );

		if( count == 0 )
			return TRUE;

		GList *iterator = found_list;
		while( iterator != NULL ) {
			GnomeKeyringFound* item = (GnomeKeyringFound*)iterator->data;

			GnomeKeyringResult result = gnome_keyring_item_delete_sync(item->keyring, item->item_id);
			if( result != GNOME_KEYRING_RESULT_OK  )
				g_debug( "GnomeKeyring_SetLoginSavingEnabled: %s", gnome_keyring_result_to_message( result ) );

			iterator = g_list_next( iterator );
		}

		return TRUE;
	}

	GnomeKeyringAttributeList* attributes = gnome_keyring_attribute_list_new();
	gnome_keyring_attribute_list_append_string( attributes, kHostInfoMagicAttrName, kHostInfoMagicAttrValue );
	gnome_keyring_attribute_list_append_string( attributes, kHostnameAttr, aHostname );

	char name[1024] = "Disabled host ";
	g_strlcat( name, aHostname, 1024 );

	//TODO: Verify the host is not already inside de DB !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	guint32 item_id;
	GnomeKeyringResult result = gnome_keyring_item_create_sync(
		keyringName,
		GNOME_KEYRING_ITEM_NOTE,
		name,
		attributes,
		"", // no secret
		TRUE,
		&item_id
	);
	gnome_keyring_attribute_list_free( attributes );

	if( result != GNOME_KEYRING_RESULT_OK ) {
		g_critical( "GnomeKeyring_SetLoginSavingEnabled: %s", gnome_keyring_result_to_message( result ) );
		return FALSE;
	}
	return TRUE;
}

int32_t GnomeKeyring_GetLoginSavingEnabled( const char* aHostname, int32_t* _isEnabled ) {
	if( !checkKeyRing() )
		return FALSE;

	*_isEnabled = 1;
	g_debug("GnomeKeyring_SetLoginSavingEnabled: host = %s", aHostname );

	GList *found_list;
	if( !findDisabledHosts( aHostname, &found_list ) )
		return FALSE;

	if( g_list_length( found_list ) != 0 )
		*_isEnabled = 0;

	return TRUE;
}

int32_t GnomeKeyring_GetAllDisabledHosts( uint32_t *count, char*** aHostnames ) {
	*count = 0;

	if( !checkKeyRing() )
		return FALSE;

	GList *found_list;
	if( !findDisabledHosts( "", &found_list ) )
		return FALSE;

	*count = g_list_length( found_list );

	g_debug( "GnomeKeyring_GetAllDisabledHosts: Found %d items", *count );
	if( *count == 0 )
		return TRUE;

	if( !allocDisabledHostsBuffer( *count ) ) {
		g_critical( "GnomeKeyring_GetAllDisabledHosts: Could not alloc buffer for logins" );
		return FALSE;
	}

	int i = 0;
	GList *iterator = found_list;
	while( iterator != NULL ) {
		GnomeKeyringFound* item = (GnomeKeyringFound*)iterator->data;
		int j;
		GnomeKeyringAttribute* attributeArray = (GnomeKeyringAttribute*) item->attributes->data;
		for( j = 0; j < item->attributes->len; j++ ) {
			if (attributeArray[j].type != GNOME_KEYRING_ATTRIBUTE_TYPE_STRING)
			  continue;

			const char *attributeName = attributeArray[j].name;
			const char *attributeValue = attributeArray[j].value.string;

			g_debug( "GnomeKeyring_GetAllDisabledHosts: attribute name = %s", attributeName );
			if (!g_strcmp0(attributeName, kHostnameAttr))
				disabledHostsBuffer[i] = g_strdup( attributeValue );
			else if (g_strcmp0(attributeName, kHostInfoMagicAttrName)){
				g_debug("GnomeKeyring_GetAllDisabledHosts: Unknown attribute %s = %s", attributeName, attributeValue);
			}
		}
		i++;
		iterator = g_list_next( iterator );
	}
	gnome_keyring_found_list_free( found_list );
	if( i != disabledHostsBufferCount || i != *count ) {
		g_critical("GnomeKeyring_GetAllDisabledHosts: loginInfoBuffer is not ok");
		return FALSE;
	}
	*aHostnames = disabledHostsBuffer;

	return TRUE;
}

