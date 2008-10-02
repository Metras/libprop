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

#include <vector>

using namespace std;

#include "utils.h"


/// Describe a specific point on the Earth surface.  It can also store details about that point, such as its elevation and vegetation.  Provides functions such as distance between two points, and projection along a bearing.
class Point {
	/// Output point in "(lat,long)" format
	friend ostream& operator<<(ostream& output, const Point& p);
	/// Output point in "lat\tlong" format
	friend ofstream& operator<<(ofstream& output, const Point& p);
	/// Read in point in "lat\tlong" format
	friend ifstream& operator>>(ifstream& input, Point& p);
	
public:
	/// Actual latitude of this point
	double lat;
	/// Actual longitude of this point
	double lon;
	/// Optional elevation of given point, usually filled by a Source object
	double elev;
	/// Optional height of a tower at this point
	double towerHeight;
	/// Optional height of vegetation above ground elevation at this point, usually filled by a Source object
	double vegHeight;
	/// Optional raw vegetation type, usually filled by vegetation Source object
	int vegType;
	/// Optional percentage (0-100) of vegetation cover as read from vegetation Source object
	int vegCover;
	/// Optional raw land use type, usually filled by a land-use Source object
	int landType;
	
	Point();
	
	/// Create new Point object with given coordinates.
	/// @param _lat Latitude to assign to new point (range -90 to 90)
	/// @param _lon Longitude to assign to new point (range -180 to 180)
	Point(double _lat, double _lon);
	
	/// Create new Point object with given coordinates.  Also set the height of a tower at this point to the height given in meters.
	/// @param _lat Latitude to assign to new point (range -90 to 90)
	/// @param _lon Longitude to assign to new point (range -180 to 180)
	/// @param _towerHeight Height to assign to tower at this point (in meters)
	Point(double _lat, double _lon, double _towerHeight);
	
	/// Calculate the distance between this point and the second given point.
	/// @param p Second point to compare this object against
	/// @return Distance between the two given points in kilometers
	double distance(Point* p);
	
	/// Calculate the compass bearing from this point towards the second given point.  If we walk from this point using the returned bearing, we will reach the second point.
	/// @param p Second point to compare this object against.
	/// @return Bearing from this point towards second given point (value returned in radians (range 0 to 2\pi)
	double bearing(Point* p);
	
	/// Project this point along the given bearing line for the given distance and return the new point.
	/// @param bearing Bearing to follow from this point in radians (range 0 to 2\pi)
	/// @param distance Distance to walk along bearing line in kilometers
	/// @return New point object after walking given distance along the bearing line.
	Point* project(double bearing, double distance);

};



/// Define a generic region on the Earth.  Can turn its defined region into a series of discrete points, and check if a given point is inside the region.
class Region {
public:
	/// Turn the defined region into a discrete set of points, using resolution to describe the level of detail.
	/// @param resolution Level of spacing (detail) between the discrete points to be created.  Value in kilometers.
	/// @return List of points that describe this region using the level of resolution requested
	virtual vector<Point*> discrete(double resolution) = 0;
	
	/// Check if this region contains the given point.
	/// @param p Point to check against
	/// @return True if point is inside this region, false otherwise
	virtual bool contains(Point* p) = 0;
	
	/// Add the given Point to this Region, expanding the region as needed so that contains(p) returns true.
	/// @param p The point to include in this region
	virtual void add(Point* p) = 0;
};


/// Define a square region across the surface of the Earth, defined using two corner points.  Can turn its defined region into a series of discrete points, and check if a given point is inside the region.
class RegionArea : Region {
	/// Output region in "[(lat,long),(lat,long]" format
	friend ostream& operator<<(ostream& output, const RegionArea& r);
private:
public:
	/// Describe the bottom left corner of this square region
	Point* bottomLeft;
	/// Describe the top right corner of this square region
	Point* topRight;
	
	RegionArea();
	
	/// Create new square region using the given points as the bottom-left and top-right corners.
	/// @param bottomLeft Bottom-left corner point of the new region
	/// @param topRight Top-right corner point of the new region
	RegionArea(Point* bottomLeft, Point* topRight);
	
	/// Create new square region using the given point as the region center and distance of radius to each corner.
	/// @param center Point to center the square region around
	/// @param radius Radius in kilometers from the center point to each corner of the square region
	RegionArea(Point* center, double radius);
	
	~RegionArea();
	
	/// Turn the defined square region into a discrete set of points, using resolution to describe the level of detail.  Effectively builds a grid across the square with resolution spacing.
	/// @param resolution Level of spacing (detail) between the discrete points to be created.  Value in kilometers.
	/// @return List of points that describe this region using the level of resolution requested
	vector<Point*> discrete(double resolution);
	
	/// Check if this region contains the given point.
	/// @param p Point to check against
	/// @return True if point is inside this region, false otherwise
	bool contains(Point* p);
	
	/// Add the given Point to this Region, expanding the region as needed so that contains(p) returns true.  Will expand the nearest square corner so that this region covers the given point.
	/// @param p The point to include in this region
	void add(Point* p);

};


/// Define a linear region across the surface of the Earth, defined as a series of points.  Can turn its defined region into a series of discrete points, and check if a given point is inside the region.
class RegionLine : Region {
	/// Output region in "lat\tlon\n..." format
	friend ofstream& operator<<(ofstream& output, const RegionLine& r);
private:
	/// List of all points contained in this linear region
	vector<Point*> list;
public:
	RegionLine();
	
	/// Create new linear region that will contain the given list of points.
	/// @param list List of points to include in the new linear region
	RegionLine(vector<Point*> list);
	
	/// Create new linear region that contains only the two given points.  Additional points can still be added later if needed.
	/// @param p Starting point for new line
	/// @param q Ending point for new line
	RegionLine(Point* p, Point* q);
	
	/// Create a new linear region by reading a list of points from a file.  Usually used for reading in description of a roadway.  Expecting file format to be "lat\tlon\n...-1\t-1\n".
	/// @param filename Filename to open and read list of points from
	RegionLine(string filename);
	
	~RegionLine();
	
	/// Calcuate the total length of this linear region
	/// @return Length of linear region in kilometers
	double length();
	
	/// Turn the defined linear region into a discrete set of points, using resolution to describe the level of detail.  Effectively steps along the line defined in resolution-length steps.
	/// @param resolution Level of spacing (detail) between the discrete points to be created.  Value in kilometers.
	/// @return List of points that describe this region using the level of resolution requested
	vector<Point*> discrete(double resolution);
	
	/// Check if this region contains the given point.  Always returns false for line segments.
	/// @param p Point to check against
	/// @return Always returns false.
	bool contains(Point* p);
	
	/// Add the given Point to this Region.  Will append given point to the end of this line segment.
	/// @param p The point to include in this region
	void add(Point* p);
	
};


