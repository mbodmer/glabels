/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  merge.c:  document merge module
 *
 *  Copyright (C) 2001-2002  Jim Evins <evins@snaught.com>.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 */
#include <config.h>

#include <gnome.h>
#include <string.h>

#include "merge.h"

#include "debug.h"

/*========================================================*/
/* Private types.                                         */
/*========================================================*/

struct _glMergePrivate {
	gchar             *name;
	gchar             *description;
	gchar             *src;
	glMergeSrcType     src_type;
};

enum {
	LAST_SIGNAL
};

typedef struct {

	GType              type;
	gchar             *name;
	gchar             *description;
	glMergeSrcType     src_type;

	const gchar       *first_arg_name;
	va_list            args;

} Backend;

/*========================================================*/
/* Private globals.                                       */
/*========================================================*/

static GObjectClass *parent_class = NULL;

static guint signals[LAST_SIGNAL] = {0};

static GList *backends = NULL;

/*========================================================*/
/* Private function prototypes.                           */
/*========================================================*/

static void gl_merge_class_init    (glMergeClass *klass);
static void gl_merge_instance_init (glMerge      *object);
static void gl_merge_finalize      (GObject      *object);



/*****************************************************************************/
/* Register a new merge backend.                                             */
/*****************************************************************************/
void
gl_merge_register_backend (GType              type,
			   gchar             *name,
			   gchar             *description,
			   glMergeSrcType     src_type,
			   const gchar       *first_arg_name,
			   ...)
{
	Backend *backend;
	va_list  args;
	gchar   *a_nam;
	gpointer a_val;

	backend = g_new0 (Backend, 1);

	backend->type = type;
	backend->name = g_strdup (name);
	backend->description = g_strdup (description);

	backend->first_arg_name = first_arg_name;
	va_start (args, first_arg_name);
	G_VA_COPY (backend->args, args);
	va_end (args);

	backends = g_list_append (backends, backend);

}

/*****************************************************************************/
/* Get list of registered backend descriptions.                              */
/*****************************************************************************/
GList *
gl_merge_get_descriptions (void)
{
	GList   *descriptions = NULL;
	GList   *p;
	Backend *backend;

	descriptions = g_list_append (descriptions, g_strdup(_("None")));

	for ( p=backends; p!=NULL; p=p->next) {
		backend = (Backend *)p->data;
		descriptions = g_list_append (descriptions,
					      g_strdup(backend->description));
	}

	return descriptions;
}

/*****************************************************************************/
/* Free list of descriptions.                                                */
/*****************************************************************************/
void
gl_merge_free_descriptions (GList **descriptions)
{
	GList *p;

	gl_debug (DEBUG_MERGE, "START");

	for (p = *descriptions; p != NULL; p = p->next) {
		g_free (p->data);
		p->data = NULL;
	}

	g_list_free (*descriptions);
	*descriptions = NULL;

	gl_debug (DEBUG_MERGE, "END");
}

/*****************************************************************************/
/* Lookup name of backend from description.                                  */
/*****************************************************************************/
gchar *
gl_merge_description_to_name (gchar *description)
{
	GList   *p;
	Backend *backend;

	if (g_strcasecmp(description, _("None")) == 0) {
		return g_strdup("None");
	}

	for ( p=backends; p!=NULL; p=p->next) {
		backend = (Backend *)p->data;
		if (g_strcasecmp(description, backend->description) == 0) {
			return g_strdup(backend->name);
		}
	}

	return g_strdup("None");
}

/*****************************************************************************/
/* Boilerplate object stuff.                                                 */
/*****************************************************************************/
GType
gl_merge_get_type (void)
{
	static GType type = 0;

	if (!type) {
		GTypeInfo info = {
			sizeof (glMergeClass),
			NULL,
			NULL,
			(GClassInitFunc) gl_merge_class_init,
			NULL,
			NULL,
			sizeof (glMerge),
			0,
			(GInstanceInitFunc) gl_merge_instance_init,
		};

		type = g_type_register_static (G_TYPE_OBJECT,
					       "glMerge", &info, 0);
	}

	return type;
}

static void
gl_merge_class_init (glMergeClass *klass)
{
	GObjectClass *object_class = (GObjectClass *) klass;

	gl_debug (DEBUG_MERGE, "START");

	parent_class = g_type_class_peek_parent (klass);

	object_class->finalize = gl_merge_finalize;

	gl_debug (DEBUG_MERGE, "END");
}

static void
gl_merge_instance_init (glMerge *merge)
{
	gl_debug (DEBUG_MERGE, "START");

	merge->private = g_new0 (glMergePrivate, 1);

	gl_debug (DEBUG_MERGE, "END");
}

static void
gl_merge_finalize (GObject *object)
{
	gl_debug (DEBUG_MERGE, "START");

	g_return_if_fail (object && GL_IS_MERGE (object));

	g_free (GL_MERGE(object)->private->name);
	g_free (GL_MERGE(object)->private->description);
	g_free (GL_MERGE(object)->private->src);
	g_free (GL_MERGE(object)->private);

	G_OBJECT_CLASS (parent_class)->finalize (object);

	gl_debug (DEBUG_MERGE, "END");
}

/*****************************************************************************/
/* New merge object.                                                         */
/*****************************************************************************/
glMerge *
gl_merge_new (gchar *name)
{
	glMerge *merge = NULL;
	GList   *p;
	Backend *backend;

	gl_debug (DEBUG_MERGE, "START");

	for (p=backends; p!=NULL; p=p->next) {
		backend = (Backend *)p->data;

		if (g_strcasecmp(name, backend->name) == 0) {

			merge = GL_MERGE (g_object_new_valist (backend->type,
							       backend->first_arg_name,
							       backend->args));

			merge->private->name        = name;
			merge->private->description = backend->description;
			merge->private->src_type    = backend->src_type;

			break;
		}
	}

	if ( (merge == NULL) && (g_strcasecmp (name, "None") != 0)) {
		g_warning ("Unknown merge backend \"%s\"", name);
	}

	gl_debug (DEBUG_MERGE, "END");

	return merge;
}

/*****************************************************************************/
/* Duplicate merge.                                                         */
/*****************************************************************************/
glMerge *
gl_merge_dup (glMerge *src_merge)
{
	glMerge    *dst_merge;

	gl_debug (DEBUG_MERGE, "START");

	if (src_merge == NULL) {
		gl_debug (DEBUG_MERGE, "END (NULL)");
		return NULL;
	}

	g_return_val_if_fail (GL_IS_MERGE (src_merge), NULL);

	dst_merge = g_object_new (G_OBJECT_TYPE(src_merge), NULL);
	dst_merge->private->name        = g_strdup (src_merge->private->name);
	dst_merge->private->description = g_strdup (src_merge->private->description);
	dst_merge->private->src         = g_strdup (src_merge->private->src);

	if ( GL_MERGE_GET_CLASS(src_merge)->copy != NULL ) {

		/* We have an object specific method, use it */
		GL_MERGE_GET_CLASS(src_merge)->copy (dst_merge, src_merge);

	}

	gl_debug (DEBUG_MERGE, "END");

	return dst_merge;
}

/*****************************************************************************/
/* Get name of merge.                                                        */
/*****************************************************************************/
gchar *
gl_merge_get_name (glMerge *merge)
{
	gl_debug (DEBUG_MERGE, "");

	if (merge == NULL) {
		return g_strdup("None");
	}

	g_return_val_if_fail (GL_IS_MERGE (merge), g_strdup("None"));

	return g_strdup(merge->private->name);
}

/*****************************************************************************/
/* Get description of merge.                                                 */
/*****************************************************************************/
gchar *
gl_merge_get_description (glMerge *merge)
{
	gl_debug (DEBUG_MERGE, "");

	if (merge == NULL) {
		return g_strdup(_("None"));
	}

	g_return_val_if_fail (GL_IS_MERGE (merge), g_strdup(_("None")));

	return g_strdup(merge->private->description);
}

/*****************************************************************************/
/* Get source type of merge.                                                 */
/*****************************************************************************/
glMergeSrcType
gl_merge_get_src_type (glMerge *merge)
{
	gl_debug (DEBUG_MERGE, "");

	if (merge == NULL) {
		return GL_MERGE_SRC_IS_FIXED;
	}

	g_return_val_if_fail (GL_IS_MERGE (merge), GL_MERGE_SRC_IS_FIXED);

	return merge->private->src_type;
}

/*****************************************************************************/
/* Set src of merge.                                                         */
/*****************************************************************************/
void
gl_merge_set_src (glMerge *merge,
		  gchar   *src)
{
	gl_debug (DEBUG_MERGE, "START");

	g_return_if_fail (merge && GL_IS_MERGE (merge));

	g_free(merge->private->src);
	merge->private->src = src;

	gl_debug (DEBUG_MERGE, "END");
}

/*****************************************************************************/
/* Get src of merge.                                                         */
/*****************************************************************************/
gchar *
gl_merge_get_src (glMerge *merge)
{
	gl_debug (DEBUG_MERGE, "");

	if (merge == NULL) {
		return NULL;
	}

	g_return_val_if_fail (GL_IS_MERGE (merge), NULL);

	return g_strdup(merge->private->src);
}

/*****************************************************************************/
/* Get Key List.                                                             */
/*****************************************************************************/
GList *
gl_merge_get_key_list (glMerge *merge)
{
	GList *key_list = NULL;

	gl_debug (DEBUG_MERGE, "START");

	if (merge == NULL) {
		return NULL;
	}

	g_return_val_if_fail (GL_IS_MERGE (merge), NULL);

	if ( GL_MERGE_GET_CLASS(merge)->get_key_list != NULL ) {

		key_list = GL_MERGE_GET_CLASS(merge)->get_key_list (merge);

	}

	gl_debug (DEBUG_MERGE, "END");

	return key_list;
}

/*****************************************************************************/
/* Free a list of keys.                                                      */
/*****************************************************************************/
void
gl_merge_free_key_list (GList **key_list)
{
	GList *p;

	gl_debug (DEBUG_MERGE, "START");

	for (p = *key_list; p != NULL; p = p->next) {
		g_free (p->data);
		p->data = NULL;
	}

	g_list_free (*key_list);
	*key_list = NULL;

	gl_debug (DEBUG_MERGE, "END");
}

/*****************************************************************************/
/* Open merge source.                                                        */
/*****************************************************************************/
void
gl_merge_open (glMerge *merge)
{
	gl_debug (DEBUG_MERGE, "START");

	g_return_if_fail (merge && GL_IS_MERGE (merge));

	if ( GL_MERGE_GET_CLASS(merge)->open != NULL ) {

		GL_MERGE_GET_CLASS(merge)->open (merge);

	}

	gl_debug (DEBUG_MERGE, "END");
}

/*****************************************************************************/
/* Close merge source.                                                       */
/*****************************************************************************/
void
gl_merge_close (glMerge *merge)
{
	gl_debug (DEBUG_MERGE, "START");

	g_return_if_fail (merge && GL_IS_MERGE (merge));

	if ( GL_MERGE_GET_CLASS(merge)->close != NULL ) {

		GL_MERGE_GET_CLASS(merge)->close (merge);

	}

	gl_debug (DEBUG_MERGE, "END");
}

/*****************************************************************************/
/* Get next record (list of fields) from opened merge source.                */
/*****************************************************************************/
glMergeRecord *
gl_merge_get_record (glMerge *merge)
{
	glMergeRecord *record = NULL;

	gl_debug (DEBUG_MERGE, "START");

	g_return_val_if_fail (merge && GL_IS_MERGE (merge), NULL);

	if ( GL_MERGE_GET_CLASS(merge)->get_record != NULL ) {

		record = GL_MERGE_GET_CLASS(merge)->get_record (merge);

	}

	gl_debug (DEBUG_MERGE, "END");

	return record;
}

/*****************************************************************************/
/* Free a merge record (list of fields)                                      */
/*****************************************************************************/
void
gl_merge_free_record (glMergeRecord **record)
{
	GList *p;
	glMergeField *field;

	gl_debug (DEBUG_MERGE, "START");

	for (p = (*record)->field_list; p != NULL; p = p->next) {
		field = (glMergeField *) p->data;

		g_free (field->key);
		field->key = NULL;
		g_free (field->value);
		field->value = NULL;

		g_free (p->data);
		p->data = NULL;

	}
	g_list_free ((*record)->field_list);
	(*record)->field_list = NULL;

	g_free (*record);
	*record = NULL;

	gl_debug (DEBUG_MERGE, "END");
}

/*****************************************************************************/
/* Find key in given record and evaluate.                                    */
/*****************************************************************************/
extern gchar *
gl_merge_eval_key (glMergeRecord *record,
		   gchar         *key)
		   
{
	GList        *p;
	glMergeField *field;
	gchar        *val = NULL;

	gl_debug (DEBUG_MERGE, "START");

	if ( (record != NULL) && record->select_flag  ) {
		for (p = record->field_list; p != NULL; p = p->next) {
			field = (glMergeField *) p->data;

			if (strcmp (key, field->key) == 0) {
				val = g_strdup (field->value);
			}

		}
	}

	gl_debug (DEBUG_MERGE, "END");

	return val;
}

/*****************************************************************************/
/* Read all records from merge source.                                       */
/*****************************************************************************/
GList *
gl_merge_read_record_list (glMerge *merge)
{
	glMergeRecord *record;
	GList *record_list = NULL;

	gl_debug (DEBUG_MERGE, "START");

	gl_merge_open (merge);
	while ( (record = gl_merge_get_record (merge)) != NULL ) {
		record_list = g_list_append( record_list, record );
	}
	gl_merge_close (merge);
	      
	gl_debug (DEBUG_MERGE, "END");

	return record_list;
}

/*****************************************************************************/
/* Free a list of records.                                                   */
/*****************************************************************************/
void
gl_merge_free_record_list (GList **record_list)
{
	GList *p;
	glMergeRecord *record;

	gl_debug (DEBUG_MERGE, "START");

	for (p = *record_list; p != NULL; p = p->next) {
		record = (glMergeRecord *) p->data;

		gl_merge_free_record( &record );

	}

	g_list_free (*record_list);
	*record_list = NULL;

	gl_debug (DEBUG_MERGE, "END");
}

/*****************************************************************************/
/* Count selected records.                                                   */
/*****************************************************************************/
gint
gl_merge_count_records (GList *record_list)
{
	GList *p;
	glMergeRecord *record;
	gint count;

	gl_debug (DEBUG_MERGE, "START");

	count = 0;
	for ( p=record_list; p!=NULL; p=p->next ) {
		record = (glMergeRecord *)p->data;

		if ( record->select_flag ) count ++;
	}

	gl_debug (DEBUG_MERGE, "END");

	return count;
}


