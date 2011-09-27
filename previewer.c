/* TIFF image perviewer, for scans.
 */

#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>

GtkImage *recto, *verso;
GtkViewport *rectoview, *versoview;
GtkTreeView *pagelist;

GdkPixbuf *verso_p;
GdkPixbuf *recto_p;

void fill_image(GdkPixbuf *original, GtkImage *target, int t_width, int t_height, int force);
void changesize_recto(GtkWidget *widget, GtkAllocation *allo, gpointer data);
void changesize_verso(GtkWidget *widget, GtkAllocation *allo, gpointer data);
GtkListStore *load_pages(char *files[]);
GtkTreeModel *pages_setup(GtkTreeView *container, char *files[]);

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
	pagelist = GTK_WIDGET(gtk_builder_get_object(foo, "pagelist"));

	pages_setup(pagelist, argv + 1);

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

/* Callback for resizing the recto imageview
 */
void changesize_recto(GtkWidget *widget, GtkAllocation *allo, gpointer data)
{
	fill_image(recto_p, recto, allo->width, allo->height, 0);
}

/* Callback for resizing the verso imageview
 *
 * TODO: fold into changesize_recto
 */
void changesize_verso(GtkWidget *widget, GtkAllocation *allo, gpointer data)
{
	fill_image(verso_p, verso, allo->width, allo->height, 0);
}

void advance_spread(GtkWidget *pages, GtkImage *verso, GtkImage *recto)
{

}

GtkTreeModel *pages_setup(GtkTreeView *container, char *files[])
{
	GtkTreeViewColumn *col;
	GtkCellRenderer *rend;
	GtkTreeModel *model;
	GtkListStore *pages;

	pages = load_pages(files);
	puts("made it");

	gtk_tree_view_set_headers_visible(pagelist, FALSE);

	/* -- column 1 - number -- */
	col = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(col, "Spread");
	gtk_tree_view_append_column(GTK_TREE_VIEW(container), col);

	rend = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start(col, rend, TRUE);
	gtk_tree_view_column_add_attribute(col, rend, "text", 0);

	/* -- column 2 - verso -- */
	col = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(col, "Verso");
	gtk_tree_view_append_column(GTK_TREE_VIEW(container), col);

	rend = gtk_cell_renderer_pixbuf_new();
	gtk_tree_view_column_pack_start(col, rend, TRUE);
	gtk_tree_view_column_add_attribute(col, rend, "pixbuf", 1);

	/* -- column 3 - recto -- */
	col = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(col, "Recto");
	gtk_tree_view_append_column(GTK_TREE_VIEW(container), col);

	rend = gtk_cell_renderer_pixbuf_new();
	gtk_tree_view_column_pack_start(col, rend, TRUE);
	gtk_tree_view_column_add_attribute(col, rend, "pixbuf", 2);


	model = pages;
	gtk_tree_view_set_model(container, model);
	return model;
}


GtkListStore *load_pages(char *files[])
{
	GtkTreeIter it;
	GtkListStore *pages;
	GdkPixbuf *buf;
	GdkPixbuf *buf_sm_v, *buf_sm_r;

	pages = gtk_list_store_new(3, G_TYPE_STRING,
				   GDK_TYPE_PIXBUF,
				   GDK_TYPE_PIXBUF);

	printf("pages is %s\n", G_OBJECT_TYPE_NAME(pages));

	if (files == NULL)
		return pages;

	while (*files != NULL) {
		/* verso */
		buf = gdk_pixbuf_new_from_file(*files++, NULL);
		printf("adding %p (%s)\n", buf, G_OBJECT_TYPE_NAME(buf));
		buf_sm_v = gdk_pixbuf_scale_simple(buf, 50, 50, GDK_INTERP_BILINEAR);
		g_object_unref(buf);

		/* recto */
		buf = gdk_pixbuf_new_from_file(*files++, NULL);
		printf("adding %p (%s)\n", buf, G_OBJECT_TYPE_NAME(buf));
		buf_sm_r = gdk_pixbuf_scale_simple(buf, 50, 50, GDK_INTERP_BILINEAR);
		g_object_unref(buf);
		gtk_list_store_append(pages, &it);

		gtk_list_store_set(pages, &it,
				   0, "foo:",
				   1, buf_sm_v,
				   2, buf_sm_r,
				   -1);

		g_object_unref(buf_sm_v);
		g_object_unref(buf_sm_r);
	}

	return pages;
}

GdkPixbuf *scale_image(GdkPixbuf *original, int t_width, int t_height)
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

	img = gdk_pixbuf_scale_simple(original,
				      width, height,
				      GDK_INTERP_BILINEAR);
	return img;
}

/* Dump the given image into the given GtkImage, scaling it to the
 * provided sizes.  Enforces aspect ratio.
 */
void fill_image(GdkPixbuf *original, GtkImage *target, int t_width, int t_height, int force)
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
	    height == gdk_pixbuf_get_height(oimg) && !force) {
		// image is already sized appropriately
		return;
	}

	img = gdk_pixbuf_scale_simple(original,
				      width, height,
				      GDK_INTERP_BILINEAR);

	oimg = gtk_image_get_pixbuf(target);
	if (width == gdk_pixbuf_get_width(oimg) &&
	    height == gdk_pixbuf_get_height(oimg) && !force) {
		// image is already sized appropriately
		return;
	}


	gtk_image_set_from_pixbuf(target, img);
	g_object_unref(img);
	return;
}
