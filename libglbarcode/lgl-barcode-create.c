/*
 *  lgl-barcode-create.c
 *  Copyright (C) 2010  Jim Evins <evins@snaught.com>.
 *
 *  This file is part of gLabels.
 *
 *  gLabels is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  gLabels is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with gLabels.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <config.h>

#include "lgl-barcode-create.h"

#include "lgl-barcode-postnet.h"
#include "lgl-barcode-onecode.h"
#include "lgl-barcode-code39.h"


/*===========================================*/
/* Private macros and constants.             */
/*===========================================*/


/*===========================================*/
/* Private types                             */
/*===========================================*/

typedef lglBarcode *(*BarcodeNewFunc) (lglBarcodeType  type,
                                       gboolean        text_flag,
                                       gboolean        checksum_flag,
                                       gdouble         w,
                                       gdouble         h,
                                       const gchar    *data);


/*===========================================*/
/* Private globals                           */
/*===========================================*/

BarcodeNewFunc create_func[LGL_BARCODE_N_TYPES] = {

        /* LGL_BARCODE_TYPE_POSTNET    */ lgl_barcode_postnet_new,
        /* LGL_BARCODE_TYPE_POSTNET_5  */ lgl_barcode_postnet_new,
        /* LGL_BARCODE_TYPE_POSTNET_9  */ lgl_barcode_postnet_new,
        /* LGL_BARCODE_TYPE_POSTNET_11 */ lgl_barcode_postnet_new,
        /* LGL_BARCODE_TYPE_CEPNET     */ lgl_barcode_postnet_new,

        /* LGL_BARCODE_TYPE_ONECODE    */ lgl_barcode_onecode_new,

        /* LGL_BARCODE_TYPE_CODE39     */ lgl_barcode_code39_new,
        /* LGL_BARCODE_TYPE_CODE39_EXT */ lgl_barcode_code39_new,
};

/*===========================================*/
/* Local function prototypes                 */
/*===========================================*/


/****************************************************************************/
/* Lookup barcode type and create appropriate barcode.                      */
/****************************************************************************/
lglBarcode *
lgl_barcode_create (lglBarcodeType     type,
                    gboolean           text_flag,
                    gboolean           checksum_flag,
                    gdouble            w,
                    gdouble            h,
                    const gchar       *data)
{
        if ( (type < 0) || (type >= LGL_BARCODE_N_TYPES) )
        {
                g_message ("Invalid barcode type.");
                return NULL;
        }

        return create_func[type] (type, text_flag, checksum_flag, w, h, data);
}



/*
 * Local Variables:       -- emacs
 * mode: C                -- emacs
 * c-basic-offset: 8      -- emacs
 * tab-width: 8           -- emacs
 * indent-tabs-mode: nil  -- emacs
 * End:                   -- emacs
 */