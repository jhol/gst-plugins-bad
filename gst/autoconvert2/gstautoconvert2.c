/* GStreamer
 *
 *  Copyright 2018-2020 VCA Technology Ltd.
 *   @author: Joel Holdsworth <joel.holdsworth@vcatechnology.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
/**
 * SECTION:element-autoconvert2
 * @title: autoconvert2
 * @short_description: Constructs a graph of elements based on the caps.
 *
 * The #autoconvert2 element has sink and source request pads. The element will
 * attempt to construct a graph of conversion elements that will convert from
 * the input caps to the output caps in the most efficient manner possible. The
 * incoming streams fed into the sink pads are assumed to represent related
 * input data but represented in different forms e.g. a video stream where the
 * frames are available in different frame sizes.
 *
 * If the caps change, the element will replace the network with another that
 * will convert to the new caps.
 */


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "gstautoconvert2.h"

GST_DEBUG_CATEGORY (autoconvert2_debug);
#define GST_CAT_DEFAULT (autoconvert2_debug)

#define gst_auto_convert2_parent_class parent_class
G_DEFINE_TYPE (GstAutoConvert2, gst_auto_convert2, GST_TYPE_BIN);

static void
gst_auto_convert2_class_init (GstAutoConvert2Class * klass)
{
  GstElementClass *const gstelement_class = (GstElementClass *) klass;

  GST_DEBUG_CATEGORY_INIT (autoconvert2_debug, "autoconvert2", 0,
      "autoconvert2 element");

  gst_element_class_set_static_metadata (gstelement_class,
      "Selects conversion elements based on caps", "Generic/Bin",
      "Creates a graph of transform elements based on the caps",
      "Joel Holdsworth <joel.holdsworth@vcatechnology.com>");
}

static void
gst_auto_convert2_init (GstAutoConvert2 * autoconvert2)
{
}
