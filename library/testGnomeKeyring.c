#include <dlfcn.h>
#include <stdio.h>
#include "GnomeKeyring.h"

int main() {
	int result;
	unsigned int count;
	const char *dlsym_error;
	LoginInfo* loginInfo;
	int i;

    const char *keyringName = "mozilla-test";

    printf( "Test GnomeKeyring\n" );

	printf( "*****************WARNING*******************\n" );
	printf( "This test is destructive to the keyring '%s'\n", keyringName );
	printf( "Backup your wallet first\n" );
	printf( "Continue (y/n) [n]? " );
	if( getchar() != 'y' ) {
		printf( "...bye\n" );
		return 1;
	}


    printf( "\nOpening libgnomekeyring.so...\n" );
    void* handle = dlopen("./libgnomekeyring.so", RTLD_LAZY);

    if (!handle) {
        printf( "Cannot open library: %s\n", dlerror() );
        return 1;
    }
// ***************************************************************************************************
    printf( "Loading symbol GnomeKeyring_SetKeyringName...\n" );
    typedef int (*libfunction_t)();

    dlerror();
    libfunction_t GnomeKeyring_SetKeyringName = (libfunction_t) dlsym(handle, "GnomeKeyring_SetKeyringName");
    dlsym_error = dlerror();
    if (dlsym_error) {
        printf( "Cannot load symbol 'GnomeKeyring_SetKeyringName': %s\n", dlsym_error );
        dlclose(handle);
        return 1;
    }

    printf( "Calling GnomeKeyring_SetKeyringName...\n" );
    GnomeKeyring_SetKeyringName(keyringName);
// ***************************************************************************************************
    printf( "Loading symbol GnomeKeyring_AddLogin...\n" );
    typedef int (*libfunction_t)();

    dlerror();
    libfunction_t GnomeKeyring_AddLogin = (libfunction_t) dlsym(handle, "GnomeKeyring_AddLogin");
    dlsym_error = dlerror();
    if (dlsym_error) {
        printf( "Cannot load symbol 'GnomeKeyring_AddLogin': %s\n", dlsym_error );
        dlclose(handle);
        return 1;
    }

    printf( "Calling GnomeKeyring_AddLogin...\n" );
    result = GnomeKeyring_AddLogin( "guillermo adrián", "user", "Kdsfñkijr", "password", "",
						"http://www.guillermomolina.com.ar/", "http://www.guillermomolina.com.ar/",
						"{900f8667-269a-4425-b1c0-2f804619f316}" );
	if( !result ) {
        printf( "Error calling 'GnomeKeyring_AddLogin'\n" );
        return 1;
	}
// ***************************************************************************************************
    printf( "Loading symbol GnomeKeyring_CountLogins...\n" );
    typedef int (*libfunction_t)();

    dlerror();
    libfunction_t GnomeKeyring_CountLogins = (libfunction_t) dlsym(handle, "GnomeKeyring_CountLogins");
    dlsym_error = dlerror();
    if (dlsym_error) {
        printf( "Cannot load symbol 'GnomeKeyring_CountLogins': %s\n", dlsym_error );
        dlclose(handle);
        return 1;
    }

    printf( "Calling GnomeKeyring_CountLogins...\n" );
    GnomeKeyring_CountLogins( "", "", "", &count );
// ***************************************************************************************************
    printf( "Loading symbol GnomeKeyring_GetAllLogins...\n" );
    typedef int (*libfunction_t)();

    dlerror();
    libfunction_t GnomeKeyring_GetAllLogins = (libfunction_t) dlsym(handle, "GnomeKeyring_GetAllLogins");
    dlsym_error = dlerror();
    if (dlsym_error) {
        printf( "Cannot load symbol 'GnomeKeyring_GetAllLogins': %s\n", dlsym_error );
        dlclose(handle);
        return 1;
    }

    printf( "Calling GnomeKeyring_GetAllLogins...\n" );
    GnomeKeyring_GetAllLogins( &count, &loginInfo );
	for( i = 0; i < count; i++ ) {
		printf( "Login[ %i ].hostname = %s\n", i, loginInfo[ i ].hostname );
		printf( "Login[ %i ].username = %s\n", i, loginInfo[ i ].username );
		printf( "Login[ %i ].usernameField = %s\n", i, loginInfo[ i ].usernameField );
		printf( "Login[ %i ].password = %s\n", i, loginInfo[ i ].password );
		printf( "Login[ %i ].passwordField = %s\n", i, loginInfo[ i ].passwordField );
		printf( "Login[ %i ].formSubmitURL = %s\n", i, loginInfo[ i ].formSubmitURL );
		printf( "Login[ %i ].httpRealm = %s\n", i, loginInfo[ i ].httpRealm );
		printf( "Login[ %i ].guid = %s\n", i, loginInfo[ i ].guid );
	}
// ***************************************************************************************************
   	printf( "Loading symbol GnomeKeyring_FindLogins...\n" );
    typedef int (*libfunction_t)();

    dlerror();
    libfunction_t GnomeKeyring_FindLogins = (libfunction_t) dlsym(handle, "GnomeKeyring_FindLogins");
    dlsym_error = dlerror();
    if (dlsym_error) {
        printf( "Cannot load symbol 'GnomeKeyring_FindLogins': %s\n", dlsym_error );
        dlclose(handle);
        return 1;
    }

    printf( "Calling GnomeKeyring_FindLogins...\n" );
    GnomeKeyring_FindLogins( &count, "http://www.guillermomolina.com.ar/", "",
    	"http://www.guillermomolina.com.ar/", &loginInfo );
	for( i = 0; i < count; i++ ) {
		printf( "Login[ %i ].hostname = %s\n", i, loginInfo[ i ].hostname );
		printf( "Login[ %i ].username = %s\n", i, loginInfo[ i ].username );
		printf( "Login[ %i ].usernameField = %s\n", i, loginInfo[ i ].usernameField );
		printf( "Login[ %i ].password = %s\n", i, loginInfo[ i ].password );
		printf( "Login[ %i ].passwordField = %s\n", i, loginInfo[ i ].passwordField );
		printf( "Login[ %i ].formSubmitURL = %s\n", i, loginInfo[ i ].formSubmitURL );
		printf( "Login[ %i ].httpRealm = %s\n", i, loginInfo[ i ].httpRealm );
		printf( "Login[ %i ].guid = %s\n", i, loginInfo[ i ].guid );
	}
// ***************************************************************************************************
    printf( "Loading symbol GnomeKeyring_FindLoginsWithGUID...\n" );
    typedef int (*libfunction_t)();

    dlerror();
    libfunction_t GnomeKeyring_FindLoginsWithGUID = (libfunction_t) dlsym(handle, "GnomeKeyring_FindLoginsWithGUID");
    dlsym_error = dlerror();
    if (dlsym_error) {
        printf( "Cannot load symbol 'GnomeKeyring_FindLoginsWithGUID': %s\n", dlsym_error );
        dlclose(handle);
        return 1;
    }

    printf( "Calling GnomeKeyring_FindLoginsWithGUID...\n" );
    GnomeKeyring_FindLoginsWithGUID( &count, "{900f8667-269a-4425-b1c0-2f804619f316}", &loginInfo );
	for( i = 0; i < count; i++ ) {
		printf( "Login[ %i ].hostname = %s\n", i, loginInfo[ i ].hostname );
		printf( "Login[ %i ].username = %s\n", i, loginInfo[ i ].username );
		printf( "Login[ %i ].usernameField = %s\n", i, loginInfo[ i ].usernameField );
		printf( "Login[ %i ].password = %s\n", i, loginInfo[ i ].password );
		printf( "Login[ %i ].passwordField = %s\n", i, loginInfo[ i ].passwordField );
		printf( "Login[ %i ].formSubmitURL = %s\n", i, loginInfo[ i ].formSubmitURL );
		printf( "Login[ %i ].httpRealm = %s\n", i, loginInfo[ i ].httpRealm );
		printf( "Login[ %i ].guid = %s\n", i, loginInfo[ i ].guid );
	}
// ***************************************************************************************************
    printf( "Loading symbol GnomeKeyring_RemoveLogin...\n" );
    typedef int (*libfunction_t)();

    dlerror();
    libfunction_t GnomeKeyring_RemoveLogin = (libfunction_t) dlsym(handle, "GnomeKeyring_RemoveLogin");
    dlsym_error = dlerror();
    if (dlsym_error) {
        printf( "Cannot load symbol 'GnomeKeyring_RemoveLogin': %s\n", dlsym_error );
        dlclose(handle);
        return 1;
    }

    printf( "Calling GnomeKeyring_RemoveLogin...\n" );
    GnomeKeyring_RemoveLogin( "guillermo adrián", "", "http://www.guillermomolina.com.ar/",
						"http://www.guillermomolina.com.ar/" );
// ***************************************************************************************************
    printf( "Loading symbol GnomeKeyring_RemoveAllLogins...\n" );
    typedef int (*libfunction_t)();

    dlerror();
    libfunction_t GnomeKeyring_RemoveAllLogins = (libfunction_t) dlsym(handle, "GnomeKeyring_RemoveAllLogins");
    dlsym_error = dlerror();
    if (dlsym_error) {
        printf( "Cannot load symbol 'GnomeKeyring_RemoveAllLogins': %s\n", dlsym_error );
        dlclose(handle);
        return 1;
    }

    printf( "Calling GnomeKeyring_RemoveAllLogins...\n" );
    GnomeKeyring_RemoveAllLogins();
// ***************************************************************************************************
   printf( "Loading symbol GnomeKeyring_SetLoginSavingEnabled...\n" );
    typedef int (*libfunction_t)();

    dlerror();
    libfunction_t GnomeKeyring_SetLoginSavingEnabled = (libfunction_t) dlsym(handle, "GnomeKeyring_SetLoginSavingEnabled");
    dlsym_error = dlerror();
    if (dlsym_error) {
        printf( "Cannot load symbol 'GnomeKeyring_SetLoginSavingEnabled': %s\n", dlsym_error );
        dlclose(handle);
        return 1;
    }

    printf( "Calling GnomeKeyring_SetLoginSavingEnabled...\n" );
    GnomeKeyring_SetLoginSavingEnabled( "http://www.guillermomolina.com.ar/", 0  );
// ***************************************************************************************************
    printf( "Loading symbol GnomeKeyring_GetLoginSavingEnabled...\n" );
    typedef int (*libfunction_t)();

    dlerror();
    libfunction_t GnomeKeyring_GetLoginSavingEnabled = (libfunction_t) dlsym(handle, "GnomeKeyring_GetLoginSavingEnabled");
    dlsym_error = dlerror();
    if (dlsym_error) {
        printf( "Cannot load symbol 'GnomeKeyring_GetLoginSavingEnabled': %s\n", dlsym_error );
        dlclose(handle);
        return 1;
    }

    printf( "Calling GnomeKeyring_GetLoginSavingEnabled...\n" );
	int32_t isEnabled;
    GnomeKeyring_GetLoginSavingEnabled( "http://www.guillermomolina.com.ar/", &isEnabled );
	printf( "Savings for http://www.guillermomolina.com.ar/ is " );
	if( isEnabled )
		printf( "enabled\n" );
	else
		printf( "disabled\n" );
// ***************************************************************************************************
    printf( "Loading symbol GnomeKeyring_GetAllDisabledHosts...\n" );
    typedef int (*libfunction_t)();

    dlerror();
    libfunction_t GnomeKeyring_GetAllDisabledHosts = (libfunction_t) dlsym(handle, "GnomeKeyring_GetAllDisabledHosts");
    dlsym_error = dlerror();
    if (dlsym_error) {
        printf( "Cannot load symbol 'GnomeKeyring_GetAllDisabledHosts': %s\n", dlsym_error );
        dlclose(handle);
        return 1;
    }

    printf( "Calling GnomeKeyring_GetAllDisabledHosts...\n" );
	char **hostnames;
    GnomeKeyring_GetAllDisabledHosts( &count, &hostnames );
	for( i = 0; i < count; i++ )
		printf( "DisabledHost[ %i ] = %s\n", i, hostnames[ i ] );
// ***************************************************************************************************
    printf( "Loading symbol GnomeKeyring_SetLoginSavingEnabled...\n" );
    typedef int (*libfunction_t)();

    dlerror();
    dlsym_error = dlerror();
    if (dlsym_error) {
        printf( "Cannot load symbol 'GnomeKeyring_SetLoginSavingEnabled': %s\n", dlsym_error );
        dlclose(handle);
        return 1;
    }

    printf( "Calling GnomeKeyring_SetLoginSavingEnabled...\n" );
    GnomeKeyring_SetLoginSavingEnabled( "http://www.guillermomolina.com.ar/", 1  );
// ***************************************************************************************************

    printf( "Closing library...\n" );
    dlclose(handle);
	return 0;
}
