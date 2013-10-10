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

#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	char* hostname;
	char* username;
	char* usernameField;
	char* password;
	char* passwordField;
	char* formSubmitURL;
	char* httpRealm;
	char* guid;
} LoginInfo;
	
void GnomeKeyring_SetKeyringName( const char* _keyringName );
int32_t GnomeKeyring_CountLogins( const char* aHostname, const char* aActionURL, const char* aHttpRealm, uint32_t* _retval );
int32_t GnomeKeyring_GetAllLogins( uint32_t *aCount, LoginInfo **aLogins );
int32_t GnomeKeyring_FindLogins( uint32_t *count, const char* aHostname, const char* aActionURL, const char* aHttpRealm, LoginInfo **logins );
int32_t GnomeKeyring_FindLoginsWithGUID( uint32_t *count, const char* aGUID, LoginInfo **logins );
int32_t GnomeKeyring_AddLogin( const char* aUsername, const char* aUsernameField,
							const char* aPassword, const char* aPasswordField,
							const char* aActionURL, const char* aHttpRealm,
							const char* aHostname, const char* aGUID );
int32_t GnomeKeyring_RemoveLogin( const char* aUsername, const char* aActionURL, 
							   const char* aHttpRealm, const char* aHostname );
int32_t GnomeKeyring_RemoveAllLogins();
int32_t GnomeKeyring_GetAllDisabledHosts( uint32_t *aCount, char*** aHostnames );
int32_t GnomeKeyring_SetLoginSavingEnabled( const char* aHostname, const int32_t isEnabled );
int32_t GnomeKeyring_GetLoginSavingEnabled( const char* aHostname, int32_t* _isEnabled );

#ifdef __cplusplus
}
#endif
