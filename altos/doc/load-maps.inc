=== Load Maps

	.Load Maps Window
	image::load-maps.png[width=500]

	Before heading out to a new launch site, you can use
	this to load satellite images in case you don't have
	internet connectivity at the site.  Try not to wait
	until the last minute, though, particularly if you're
	heading to a major launch.  If too many people are 
	all trying to download map data at once, Google may
	limit access until the next day.

	There's a drop-down menu of launch sites we know
	about; if your favorites aren't there, please let us
	know the lat/lon and name of the site. The contents of
	this list are actually downloaded from our server at
	run-time, so as new sites are sent in, they'll get
	automatically added to this list.  If the launch site
	isn't in the list, you can manually enter the lat/lon
	values

	There are four different kinds of maps you can view;
	you can select which to download by selecting as many
	as you like from the available types:

	Hybrid::
	A combination of satellite imagery and road data. This
	is the default view.

	Satellite::
	Just the satellite imagery without any annotation.

	Roadmap::
	Roads, political boundaries and a few geographic
	features.

	Terrain::
	Contour intervals and shading that show hills and
	valleys.

	You can specify the range of zoom levels to download;
	smaller numbers show more area with less
	resolution. The default level, 0, shows about
	3m/pixel. One zoom level change doubles or halves that
	number. Larger zoom levels show more detail, smaller
	zoom levels less.

	The Map Radius value sets how large an area around the
	center point to download. Select a value large enough
	to cover any plausible flight from that site. Be aware
	that loading a large area with a high maximum zoom
	level can attempt to download a lot of data. Loading
	hybrid maps with a 10km radius at a minimum zoom of -2
	and a maximum zoom of 2 consumes about 120MB of
	space. Terrain and road maps consume about 1/10 as
	much space as satellite or hybrid maps.

	Clicking the 'Load Map' button will fetch images from
	Google Maps; note that Google limits how many images
	you can fetch at once, so if you load more than one
	launch site, you may get some gray areas in the map
	which indicate that Google is tired of sending data to
	you. Try again later.
