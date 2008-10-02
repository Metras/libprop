/*
	libprop -- library to simulate and measure radio signal propagation
	
	Copyright (C) 2007 Jeffrey Sharkey, jsharkey.org
	
	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.
	
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
	
	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include <iostream>
#include <fstream>
#include <string>

#include <math.h>

using namespace std;

#include "geom.h"
#include "utils.h"

#define TYPE_ELEV 2
#define TYPE_VEGTYPE 3
#define TYPE_VEGHEIGHT 4
#define TYPE_LAND 5

#define LAND_NONE 0
#define LAND_FOREST 1
#define LAND_RESIDENTIAL 2
#define LAND_COMMERCIAL 3

#define EC 0.082271854
#define EC2 0.006768658


/// Interface to convert a Point into another coordinate system that could be used to reference into a Source.
class Convert {
public:
	/// Convert the given Point into another coordinate system.
	/// @param p Point to be converted
	/// @param x Output x coordinate after conversion
	/// @param y Output y coordinate after conversion
	virtual void convert(Point* p, double* x, double* y);
};


/// Interface to convert Point objects into default Albers projection coordinate system.
class ConvertAlbers : public Convert {
private:
	double middleLon, bigC, coneConst, r0;
	
	double calcQ(double lat);
	double calcMsq(double lat);
	
public:
	ConvertAlbers();
	
	/// Convert the given Point into default Albers projection coordinate system.
	/// @param p Point to be converted
	/// @param x Output x coordinate after conversion
	/// @param y Output y coordinate after conversion
	void convert(Point* p, double* x, double* y);
	
};


/// Interface to a data source.  Sources can be asked to fill Point objects with any available data.
class Source {
protected:
	/// Conversion to apply to each incoming Point
	Convert* convert;
	/// Enumeration describing what type of data this object provides
	int type;
	int ncols, nrows;
	double top, left, bottom, right;
	/// Optional internal value describing the size of each data cell
	double cellsize;
	/// Filename pointing at actual source datafile
	string rawfilename;
	/// Open stream to source datafile, if already opened
	ifstream raw;
	
	Source();
	
	/// Create a new data source.
	/// @param _convert The conversion to apply to all incoming points
	/// @param _type Enumeration value describing the type of data we provide
	Source(Convert* _convert, int _type);
	
public:
	/// Resolve a given Point by filling it with any new data this source can provide.  Will ignore given point if this source can't provide data.
	/// @param p The point to try filling with data
	virtual void resolve(Point* p) = 0;
	
	/// Resolve the entire list of Point objects by calling resolve() on each of them.
	/// @param list List of Point objects to try resolving
	void resolveList(vector<Point*> list);
	
	/// Check if this source provides data about the given point.
	/// @param p Point to check against
	/// @return True if this source provides data about given point, otherwise false.
	virtual bool contains(Point* p);
	
};



/// Interface to a data source.  Sources can be asked to fill Point objects with any available data.  Specifically handles data files in integer format, usually ending with ".bil" extension.
class SourceInteger : public Source {
protected:
	/// Internal cache of entire data file, if requested
	int* cache;
	
	/// Retrieve a specific value, either from raw source file, or from cache if it exists.
	/// @param offset Offset into the data file to read
	/// @return Value at offset location
	int value(int offset);
	
public:
	
	/// Create a new integer data source.
	/// @param convert The conversion to apply to all incoming points
	/// @param type Enumeration value describing the type of data we provide
	/// @param filename Filename pointing to ".hdr" file for this data source
	/// @param cache Cache entire data file in memory if true, otherwise read directly from file when requested
	SourceInteger(Convert* convert, int type, string filename, bool cache);
	
	~SourceInteger();
	
	/// Resolve a given Point by filling it with any new data this source can provide.  Will ignore given point if this source can't provide data.
	/// @param p The point to try filling with data
	void resolve(Point* p);
};



/// Interface to a data source.  Sources can be asked to fill Point objects with any available data.  Specifically handles data files in grid float format, usually ending with ".flt" extension.
class SourceGridFloat : public Source {
protected:
	/// Internal cache of entire data file, if requested
	double* cache;
	
	/// Retrieve a specific value, either from raw source file, or from cache if it exists.
	/// @param offset Offset into the data file to read
	/// @return Value at offset location
	double value(int offset);
	
public:
	
	/// Create a new grid float data source.
	/// @param convert The conversion to apply to all incoming points
	/// @param type Enumeration value describing the type of data we provide
	/// @param filename Filename pointing to ".hdr" file for this data source
	/// @param cache Cache entire data file in memory if true, otherwise read directly from file when requested
	SourceGridFloat(Convert* convert, int type, string filename, bool cache);
	
	/// Create a new grid float data source.  This constructor is specifically used to bypass reading the ".hdr" header file, which can save time when using large numbers of data source tiles.
	/// @param convert The conversion to apply to all incoming points
	/// @param type Enumeration value describing the type of data we provide
	/// @param filename Filename pointing to ".hdr" file for this data source
	/// @param _ncols Number of columns in the data file
	/// @param _nrows Number of rows in the data file
	/// @param _left Left longitude of the data file
	/// @param _bottom Bottom latitude of the data file
	/// @param _cellsize Size of each cell in the data file
	SourceGridFloat(Convert* convert, int type, string filename, int _ncols, int _nrows, double _left, double _bottom, double _cellsize);
	
	~SourceGridFloat();
	
	/// Resolve a given Point by filling it with any new data this source can provide.  Will ignore given point if this source can't provide data.
	/// @param p The point to try filling with data
	void resolve(Point* p);
	
};



/// Interface to a collection of data sources.  This can quickly fill Point objects with available data from a large set of source files.
class SourceGroup : public Source {
private:
	/// Internal list of all Source objects we know about
	vector<Source*> list;
public:
	~SourceGroup();
	
	/// Add the given Source to this list of sources.  Will be used in any resolve() calls in the future.
	/// @param s Source object to add to our list
	void add(Source* s);
	
	/// Resolve a given Point by filling it with any new data this source can provide.  Will ignore given point if this source can't provide data.
	/// @param p The point to try filling with data
	void resolve(Point* p);
	
	/// Check if any sources we know about can provide data about the given point.
	/// @param p Point to check against
	/// @return True if any source provides data about given point, otherwise false.
	bool contains(Point* p);
	
};






