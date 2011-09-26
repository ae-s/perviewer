/* TIFF image perviewer, for scans.
 */

#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>

GtkImage *recto, *verso;
GtkViewport *rectoview, *versoview;

GdkPixbuf *verso_p;
GdkPixbuf *recto_p;

void fill_image(GdkPixbuf *original, GtkImage *target, int t_width, int t_height);
void changesize_recto(GtkWidget *widget, GtkAllocation *allo, gpointer data);
void changesize_verso(GtkWidget *widget, GtkAllocation *allo, gpointer data);

int main(int argc, char **argv)
{
	GtkBuilder *foo = NULL;
	GtkWidget *window;
	int i;

	gtk_init(&argc, &argv);


	foo = gtk_builder_new();
	i = gtk_builder_add_from_file(foo, "previewer.xml", NULL);
	printf("==== %d\n", i);

	window = GTK_WIDGET(gtk_builder_get_object(foo, "imagebrowser"));
	recto = GTK_WIDGET(gtk_builder_get_object(foo, "recto"));
	rectoview = GTK_WIDGET(gtk_builder_get_object(foo, "rectoview"));
	verso = GTK_WIDGET(gtk_builder_get_object(foo, "verso"));
	versoview = GTK_WIDGET(gtk_builder_get_object(foo, "versoview"));
	g_object_unref(G_OBJECT(foo));
	foo = NULL;
	gtk_widget_show(window);

	recto_p = gdk_pixbuf_new_from_file(argv[2], NULL);
	verso_p = gdk_pixbuf_new_from_file(argv[1], NULL);

	g_signal_connect(G_OBJECT(rectoview), "size-allocate", G_CALLBACK(changesize_recto), NULL);
	g_signal_connect(G_OBJECT(versoview), "size-allocate", G_CALLBACK(changesize_verso), NULL);

	gtk_main();
	exit(0);
}

void changesize_recto(GtkWidget *widget, GtkAllocation *allo, gpointer data)
{
	fill_image(recto_p, recto, allo->width, allo->height);
}

void changesize_verso(GtkWidget *widget, GtkAllocation *allo, gpointer data)
{
	fill_image(verso_p, verso, allo->width, allo->height);
}

/* Dump the given image into the given GtkImage, scaling it to the
 * provided sizes.  Enforces aspect ratio.
 */
void fill_image(GdkPixbuf *original, GtkImage *target, int t_width, int t_height)
{
	GdkPixbuf *img;
	GtkImage *oimg;
	int width, height;
	double t_ratio, ratio;

	width = gdk_pixbuf_get_width(original);
	height = gdk_pixbuf_get_height(original);

	ratio = (double)height/(double)width;
	t_ratio = (double)t_height/(double)t_width;
	printf("%d %d -> %d %d\n", width, height, t_width, t_height);
	printf("%lf and %lf\n", ratio, t_ratio);

	if (ratio > t_ratio) {
		// wide frame, relative to image
		printf("wide frame: ");
		width = t_height / ratio;
		height = t_height;
		printf("h = %d, w = %d\n", height, width);
	} else {
		// narrow frame, relative to image
		printf("narrow frame: ");
		height = t_width * ratio;
		width = t_width;
	}

	oimg = gtk_image_get_pixbuf(target);
	if (width == gdk_pixbuf_get_width(oimg) &&
	    height == gdk_pixbuf_get_height(oimg)) {
		// image is already sized appropriately
		return;
	}

	img = gdk_pixbuf_scale_simple(original,
				      width, height,
				      GDK_INTERP_BILINEAR);

	printf("img is %s\n", G_OBJECT_TYPE_NAME(img));
	gtk_image_set_from_pixbuf(target, img);
	g_object_unref(img);
	return;
}
