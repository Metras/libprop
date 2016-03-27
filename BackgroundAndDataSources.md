# Background #

Radio propagation analysis describes how radio signals move through a given area.  Given a set of known radio equipment and two specific locations, we can model the environmental conditions between those points to determine if successful radio communication is possible.

libprop was written by [Jeffrey Sharkey](http://jsharkey.org/) while a Graduate Research Assistant at the [Western Transportation Institute](http://www.coe.montana.edu/wti/) at [Montana State University](http://montana.edu/).  His work was sponsored by the Western Transportation Institute, and was guided by advisor Doug Galarus. Other valuable guidance was provided by Dr. Bill Jameson and Gary Schoep.

# Data Sources #

The USGS is an excellent source of digitized terrain data, and they continue to improve data quality and resolution as technology allows.  Currently they are offering 10-meter resolution data for the entire United States, which is what libprop usually expects.

  * [USGS National Map Seamless Server](http://seamless.usgs.gov/)

libprop can accept data in either the GridFloat or BIL (integer) formats from USGS, and it understands native lat/long mapping and Albers projection.  For terrain data we recommend using "[National Elevation Dataset (NED) 1/9 Arc Second](http://seamless.usgs.gov/products/9arc.php)" data source, which provides 10-meter resolution.  We also recommend the "[Landfire Existing Vegetation Cover](http://www.landfire.gov/NationalProductDescriptions23.php)" and "[Landfire Existing Vegetation Height](http://www.landfire.gov/NationalProductDescriptions22.php)" data sources which provides vegetation type and height information.  Optionally, we can accept the "[National Land Cover Dataset 2001](http://seamless.usgs.gov/products/nlcd01.php)" data source which provides land-use classifications.  Our algorithm doesn't use this data by default.