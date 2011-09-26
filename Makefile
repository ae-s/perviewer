previewer: previewer.c previewer.xml
	gcc previewer.c -o previewer `pkg-config --cflags --libs gtk+-2.0`

previewer.xml: previewer.glade
	gtk-builder-convert previewer.glade previewer.xml
