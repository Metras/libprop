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

#include "geom.h"
#include "utils.h"

#include <ios>
#include <iostream>
#include <fstream>
#include <vector>

using namespace std;



ostream& operator<<(ostream& output, const Point& p) {
	output.precision(8);
	output << "(" << p.lat << "," << p.lon << ")";
	return output;
}

ofstream& operator<<(ofstream& output, const Point& p) {
	output.precision(8);
	output << p.lat << "\t" << p.lon << endl;
	return output;
}

ifstream& operator>>(ifstream& input, Point& p) {
	input >> p.lat >> p.lon;
	return input;
}

Point::Point() : lat(-1), lon(-1), elev(-1), towerHeight(0), vegHeight(0), vegType(-1), vegCover(-1), landType(-1) {
}

Point::Point(double _lat, double _lon) : lat(_lat), lon(_lon), elev(-1), towerHeight(0), vegHeight(0), vegType(-1), vegCover(-1), landType(-1) {
}

Point::Point(double _lat, double _lon, double _towerHeight) : lat(_lat), lon(_lon), elev(-1), towerHeight(_towerHeight), vegHeight(0), vegType(-1), vegCover(-1), landType(-1) {
}

double Point::distance(Point* p) {
	// calculate distance from here to point in km
	double lat1 = toRadians(lat), lon1 = toRadians(lon),
		lat2 = toRadians(p->lat), lon2 = toRadians(p->lon);

	return acos(sin(lat1) * sin(lat2) +
		cos(lat1) * cos(lat2) * cos(lon2 - lon1)) * RADIUS;
}

double Point::bearing(Point* p) {
	// calculate bearing from here to point in radians
	double lat1 = toRadians(lat), lon1 = toRadians(lon),
		lat2 = toRadians(p->lat), lon2 = toRadians(p->lon);

	return atan2(sin(lon2 - lon1) * cos(lat2),
		cos(lat1) * sin(lat2) -
		sin(lat1) * cos(lat2) * cos(lon2 - lon1));
}

Point* Point::project(double bearing, double distance) {
	// project from here at bearing (radians) for distance (km)
	double lat1 = toRadians(lat), lon1 = toRadians(lon);
	double ratio = distance / RADIUS;

	double lat2 = asin(sin(lat1) * cos(ratio) +
		cos(lat1) * sin(ratio) * cos(bearing));
	double lon2 = lon1 + atan2(sin(bearing) * sin(ratio) * cos(lat1),
		cos(ratio) - sin(lat1) * sin(lat2));

	return new Point(toDegrees(lat2), toDegrees(lon2));
}







ostream& operator<<(ostream& output, const RegionArea& r) {
	output << "[" << *(r.bottomLeft) << "," << *(r.topRight) << "]";
	return output;
}

RegionArea::RegionArea() {
	bottomLeft = new Point(1024, 1024);
	topRight = new Point(0, -1024);
}

RegionArea::~RegionArea() {
	delete bottomLeft;
	delete topRight;
}

RegionArea::RegionArea(Point* bottomLeft, Point* topRight) {
	this->bottomLeft = bottomLeft;
	this->topRight = topRight;
}

RegionArea::RegionArea(Point* center, double radius) {
	topRight = center->project(toRadians(45), radius);
	bottomLeft = center->project(toRadians(225), radius);
}

vector<Point*> RegionArea::discrete(double resolution) {
	// step through entire region making a grid of discrete points
	// convert km to degree resolution
	Point* q = bottomLeft->project(0, resolution);
	double degRes = q->lat - bottomLeft->lat;

	vector<Point*> list;
	for(double lat = bottomLeft->lat; lat < topRight->lat; lat += degRes) {
		for(double lon = bottomLeft->lon; lon < topRight->lon; lon += degRes) {
			list.push_back(new Point(lat, lon));
		}
	}
	return list;
}

bool RegionArea::contains(Point* p) {
	// check if given point is inside this area
	return (p->lat <= topRight->lat && p->lat >= bottomLeft->lat &&
		p->lon <= topRight->lon && p->lon >= bottomLeft->lon);
}

void RegionArea::add(Point* p) {
	// force the area to enlarge to include given point
	bottomLeft->lat = min(bottomLeft->lat, p->lat);
	bottomLeft->lon = min(bottomLeft->lon, p->lon);
	topRight->lat = max(topRight->lat, p->lat);
	topRight->lon = max(topRight->lon, p->lon);
}








ofstream& operator<<(ofstream& output, const RegionLine& r) {
	vector<Point*> list = r.list;
	vector<Point*>::iterator it;
	for(it = list.begin(); it != list.end(); it++) {
		cout << *(*it);
	}
	return output;
}

RegionLine::RegionLine() {
}

RegionLine::RegionLine(vector<Point*> list) {
	this->list = list;
}

RegionLine::RegionLine(Point* p, Point* q) {
	list.push_back(p);
	list.push_back(q);
}

RegionLine::RegionLine(string filename) {
	// read in file of points to create line
	ifstream in(filename.c_str(), ifstream::in);
	while(in.good()) {
		Point* p = new Point();
		in >> *p;
		if(p->lat == -1 || p->lon == -1) continue;
		list.push_back(p);
	}
	in.close();
	
}

RegionLine::~RegionLine() {
	// not deallocated, what if we still need these points later?
	//vector<Point*>::iterator it;
	//for(it = list.begin(); it != list.end(); it++) {
	//	delete *it;
	//}
}

double RegionLine::length() {
	vector<Point*>::iterator it;
	double length = 0;
	Point* last = list[0];
	for(it = list.begin(), it++; it != list.end(); it++) {
		length += last->distance(*it);
		last = *it;
	}
	return length;
}

vector<Point*> RegionLine::discrete(double resolution) {
	// turn path into list of discrete points
	vector<Point*> list;
	double length = this->length();
	int i = 0;

	Point* last = NULL;
	Point* next = this->list[i++];
	double lastExact = 0,
		bearing = 0,
		distance = 0;
	
//cout << "entering next=" << next << endl; fflush(stdout);
//cout << "expected length=" << length << endl;

	for(double here = 0; here < length; here += resolution) {
		// check if we need to start into the next line segment
		while(here - lastExact >= distance) {
			last = next;
			next = this->list[i++];
//cout << "\tnewline, last=" << last << "\tnext=" << next << "\tdistance=" << distance << endl; fflush(stdout);
			lastExact += distance;
			bearing = last->bearing(next);
			distance = last->distance(next);
		}

		// extend from last point
		Point* p = last->project(bearing, here - lastExact);
		list.push_back(p);
	}

	return list;
}

bool RegionLine::contains(Point* p) {
	return false;
}

void RegionLine::add(Point* p) {
	list.push_back(p);
}

