/* gpapasecretkey.c  -	The GNU Privacy Assistant Pipe Access
 *	  Copyright (C) 2000 Free Software Foundation, Inc.
 *
 * This file is part of GPAPA
 *
 * GPAPA is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * GPAPA is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */

#include <config.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include "gpapa.h"

void gpapa_secret_key_export (
  GpapaSecretKey *key, gchar *targetFileID, GpapaArmor Armor,
  GpapaCallbackFunc callback, gpointer calldata
) {
  if ( ! key )
    callback ( GPAPA_ACTION_ERROR, "no valid public key specified", calldata );
  if ( ! targetFileID )
    callback ( GPAPA_ACTION_ERROR, "target file not specified", calldata );
  if ( key && targetFileID )
    {
      gchar *full_keyID;
      char *gpgargv [ 7 ];
      int i = 0;
      full_keyID = xstrcat2 ( "0x", key -> key -> KeyID );
      gpgargv [ i++ ] = "-o";
      gpgargv [ i++ ] = targetFileID;
      gpgargv [ i++ ] = "--yes";  /* overwrite the file */
      if ( Armor == GPAPA_ARMOR )
	gpgargv [ i++ ] = "--armor";
      gpgargv [ i++ ] = "--export-secret-key";
      gpgargv [ i++ ] = full_keyID;
      gpgargv [ i ] = NULL;
      gpapa_call_gnupg
	(
	  gpgargv, TRUE, NULL, NULL,
	  gpapa_linecallback_dummy, NULL,
	  callback, calldata
	);
      free ( full_keyID );
    }
} /* gpapa_secret_key_export */

/* Due to gpg's security features, this currently does not work.
 */
void gpapa_secret_key_delete (
  GpapaSecretKey *key, GpapaCallbackFunc callback, gpointer calldata
) {
  if ( ! key )
    callback ( GPAPA_ACTION_ERROR, "no valid secret key specified", calldata );
  else
    {
      gchar *full_keyID;
      char *gpgargv [ 3 ];
      full_keyID = xstrcat2 ( "0x", key -> key -> KeyID );
      gpgargv [ 0 ] = "--delete-secret-key";
      gpgargv [ 1 ] = full_keyID;
      gpgargv [ 2 ] = NULL;
      gpapa_call_gnupg
	(
	  gpgargv, TRUE, NULL, NULL,
	  gpapa_linecallback_dummy, NULL,
	  callback, calldata
	);
      free ( full_keyID );
      gpapa_refresh_secret_keyring ( callback, calldata );
    }
} /* gpapa_secret_key_delete */;

/* Due to gpg's security features, this currently does not work.
 */
void gpapa_secret_key_create_revocation (
  GpapaSecretKey *key, GpapaCallbackFunc callback, gpointer calldata
) {
} /* gpapa_secret_key_create_revocation */
