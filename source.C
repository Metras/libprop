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

#include "source.h"

#include <iostream>
#include <fstream>
#include <string>

#include <math.h>

using namespace std;

#include "geom.h"
#include "utils.h"



void Convert::convert(Point* p, double* x, double* y) {
	*x = p->lon;
	*y = p->lat;
}




ConvertAlbers::ConvertAlbers() {
	double northLat = toRadians(29.500000),
		southLat = toRadians(45.500000),
		originLat = toRadians(23.000000),
		originLon = toRadians(-96.000000);

	middleLon = originLon;

	double q1 = calcQ(southLat),
		q2 = calcQ(northLat),
		q0 = calcQ(originLat);

	double m1sq = calcMsq(southLat),
		m2sq = calcMsq(northLat);

	coneConst = (m1sq - m2sq) / (q2 - q1);
	bigC = m1sq + coneConst * q1;
	r0 = (RADIUS * 1000) * sqrt(bigC - coneConst * q0) / coneConst;

}

double ConvertAlbers::calcQ(double lat) {
	double s = sin(lat),
		es = s * EC;
	return (1.0 - EC2) * ((s / (1 - es * es)) -
		(1 / (2 * EC)) * log((1 - es) / (1 + es)));
}

double ConvertAlbers::calcMsq(double lat) {
	double c = cos(lat),
		es = sin(lat) * EC;
	return c * c / (1 - es * es);
}

void ConvertAlbers::convert(Point* p, double* x, double* y) {
	double lat = toRadians(p->lat),
		lon = toRadians(p->lon);

	double q = calcQ(lat),
		theta = coneConst * (lon - middleLon),
		r = (RADIUS * 1000) * sqrt(bigC - coneConst * q) / coneConst;

	*x = (r * sin(theta) * 1) + 80;
	*y = ((r0 - r * cos(theta)) * 1) + 80;
}






Source::Source() {
}

Source::Source(Convert* _convert, int _type) : convert(_convert), type(_type) {
}

void Source::resolveList(vector<Point*> list) {
	vector<Point*>::iterator it;
	for(it = list.begin(); it != list.end(); it++) {
		resolve(*it);
	}
}

bool Source::contains(Point* p) {
	double x, y;
	convert->convert(p, &x, &y);
	return (x > left && x < right &&
		y > bottom && y < top);
}








int SourceInteger::value(int offset) {
	if(cache == NULL) {
		char data[1];
		raw.seekg(offset);
		raw.read(data, 1);
		int value = (int)*data;
		return value;
	} else {
		return cache[offset];
	}
}

SourceInteger::SourceInteger(Convert* convert, int type, string filename, bool cache) : Source(convert, type) {
	// read in header information
	ifstream in(filename.c_str(), ifstream::in);
	string name; double value;
	while(in.good()) {
		in >> name;
		if(name == "NCOLS") in >> ncols;
		if(name == "NROWS") in >> nrows;
	}
	in.close();

	filename.replace(filename.end() - 3, filename.end(), "blw");
	ifstream in2(filename.c_str(), ifstream::in);
	in2 >> cellsize >> value >> value >> value >> left >> top;
	in2.close();

	bottom = top - (nrows * cellsize);
	right = left + (ncols * cellsize);

//cout << fixed << " cell=" << cellsize; cout << " rows=" << nrows; cout << " cols=" << ncols; cout << endl;
//cout << " top=" << top; cout << " bot=" << bottom; cout << " lef=" << left; cout << " rig=" << right; cout << endl;

	// open data source
	filename.replace(filename.end() - 3, filename.end(), "bil");
	raw.open(filename.c_str(), ifstream::in | ifstream::binary);

	if(cache) {
		// create a cache for entire data file
		long size = nrows * ncols;
		this->cache = new int[size];

		cout << "SourceInteger: loading data file..."; fflush(stdout);
		char data[1];
		for(int i = 0; i < size; i++) {
			raw.read(data, 1);
			this->cache[i] = (int)*data;
		}
		cout << "done" << endl;
	} else {
		this->cache = NULL;
	}

}

SourceInteger::~SourceInteger() {
	raw.close();
	if(cache != NULL)
		delete[] cache;
}

void SourceInteger::resolve(Point* p) {
	double x, y;
	convert->convert(p, &x, &y);

	// find the approximate cell location
	int row = (int)((y - bottom) / cellsize),
		col = (int)((x - left) / cellsize);

	row = nrows - max(0, min(row, nrows)) - 1;
	col = max(0, min(col, ncols));

	long offset = ((row * ncols) + col);
//cout << fixed << "about to use x=" << x << "\ty=" << y << endl;
//cout << "about to use row=" << row << "\tcol=" << col << endl;
//cout << "about to use offset=" << offset << endl;
	int iv = value(offset);
	if(type == TYPE_LAND) {
		switch(iv) {
			case 41: case 42: case 43: case 90: case 91: case 93:
				p->landType = LAND_FOREST; break;
			case 22:
				p->landType = LAND_RESIDENTIAL; break;
			case 23: case 24:
				p->landType = LAND_COMMERCIAL; break;
			default:
				p->landType = LAND_NONE; break;
		}
	}
//cout << "FOUND value=" << iv << endl;

}








double SourceGridFloat::value(int offset) {
//cout << "trying to run offset=" << offset << endl; fflush(stdout);
	if(cache == NULL) {
		char data[4];
		raw.seekg(offset * 4);
		raw.read(data, 4);
		return ieee_single(data);
	} else {
		return cache[offset];
	}
}

SourceGridFloat::SourceGridFloat(Convert* convert, int type, string filename, bool cache) : Source(convert, type) {
	// read in header information
	ifstream in(filename.c_str(), ifstream::in);
	while(in.good()) {
		string name;
		double value;
		in >> name >> value;
		if(name == "ncols") ncols = (int)value;
		if(name == "nrows") nrows = (int)value;
		if(name == "xllcorner") left = value;
		if(name == "yllcorner") bottom = value;
		if(name == "cellsize") cellsize = value;
	}
	in.close();

	top = bottom + (nrows * cellsize);
	right = left + (ncols * cellsize);

	// open data source
	filename.replace(filename.end() - 3, filename.end(), "flt");
	raw.open(filename.c_str(), ifstream::in | ifstream::binary);

	if(cache) {
		// create a cache for entire data file
		long size = nrows * ncols;
		this->cache = new double[size];

		cout << "SourceGridFloat: loading data file..."; fflush(stdout);
		char data[4];
		for(int i = 0; i < size; i++) {
			raw.read(data, 4);
			this->cache[i] = ieee_single(data);
		}
		cout << "done" << endl;
	} else {
		this->cache = NULL;
	}
}

SourceGridFloat::SourceGridFloat(Convert* convert, int type, string filename, int _ncols, int _nrows, double _left, double _bottom, double _cellsize) : Source(convert, type) {
	// set explicit header file values
	nrows = _nrows;
	ncols = _ncols;
	left = _left;
	bottom = _bottom;
	cellsize = _cellsize;

	top = bottom + (nrows * cellsize);
	right = left + (ncols * cellsize);

	// save data source filename for later opening if needed
	filename.replace(filename.end() - 3, filename.end(), "flt");
	rawfilename = filename;

	this->cache = NULL;
}


SourceGridFloat::~SourceGridFloat() {
	if(raw != NULL)
		raw.close();
	if(cache != NULL)
		delete[] cache;
}

void SourceGridFloat::resolve(Point* p) {
	double x, y;
	convert->convert(p, &x, &y);

	// open data source if needed
	if(!raw.is_open())
		raw.open(rawfilename.c_str(), ifstream::in | ifstream::binary);


	// find the approximate cell location
	int row = (int)((y - bottom) / cellsize),
		col = (int)((x - left) / cellsize);

	row = nrows - max(0, min(row, nrows)) - 1;
	col = max(0, min(col, ncols));

	long offset = ((row * ncols) + col);
//cout << "about to use x=" << x << "\ty=" << y << endl;
//cout << "about to use row=" << row << "\tcol=" << col << endl;
//cout << "about to use offset=" << offset << endl;
	double dv = value(offset);
	int iv = (int)dv;
	if(type == TYPE_ELEV) {
		p->elev = dv;
	} else if(type == TYPE_VEGTYPE) {
		p->vegType = iv;
		// save the vegetation density/cover as the average (half) percent
		switch(iv) {
			case 101: case 111: case 121: p->vegCover = 15; break;
			case 102: case 112: case 122: p->vegCover = 25; break;
			case 103: case 113: case 123: p->vegCover = 35; break;
			case 104: case 114: case 124: p->vegCover = 45; break;
			case 105: case 115: case 125: p->vegCover = 55; break;
			case 106: case 116: case 126: p->vegCover = 65; break;
			case 107: case 117: case 127: p->vegCover = 75; break;
			case 108: case 118: case 128: p->vegCover = 85; break;
			case 109: case 119: case 129: p->vegCover = 95; break;
		}
	} else if(type == TYPE_VEGHEIGHT) {
		// save the vegetation height as the average (half) in meters
		switch(iv) {
			case 101: p->vegHeight = 0.25; break;
			case 102: p->vegHeight = 0.5; break;
			case 103: p->vegHeight = 1; break;

			case 104: p->vegHeight = 0.25; break;
			case 105: p->vegHeight = 0.5; break;
			case 106: p->vegHeight = 1.5; break;
			case 107: p->vegHeight = 3; break;

			case 108: p->vegHeight = 2.5; break;
			case 109: p->vegHeight = 5; break;
			case 110: p->vegHeight = 12.5; break;
			case 111: p->vegHeight = 25; break;
			case 112: p->vegHeight = 50; break;
		}
	}

}







SourceGroup::~SourceGroup() {
	while(!list.empty()) {
		delete list.back();
		list.pop_back();
	}
}

void SourceGroup::resolve(Point* p) {
	vector<Source*>::iterator it;
	for(it = list.begin(); it != list.end(); it++) {
		if((*it)->contains(p))
			(*it)->resolve(p);
	}
}

bool SourceGroup::contains(Point* p) {
	vector<Source*>::iterator it;
	for(it = list.begin(); it != list.end(); it++) {
		if((*it)->contains(p)) return true;
	}
	return false;
}

void SourceGroup::add(Source* s) {
	list.push_back(s);
}

