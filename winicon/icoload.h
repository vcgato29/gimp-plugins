/* The GIMP -- an image manipulation program
 * Copyright (C) 1995-1997 Spencer Kimball and Peter Mattis
 *
 * GIMP Plug-in for Windows Icon files.
 * Copyright (C) 2002 Christian Kreibich <christian@whoop.org>.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef __ICO_LOAD_H__
#define __ICO_LOAD_H__


gint   ico_get_bit_from_data    (guint8 *data, gint line_width, gint bit);
gint   ico_get_nibble_from_data (guint8 *data, gint line_width, gint nibble);
gint   ico_get_byte_from_data   (guint8 *data, gint line_width, gint byte);

gint32 LoadICO                  (const gchar *file_name);


#endif /* __ICO_LOAD_H__ */
