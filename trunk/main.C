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
#include "geom.h"
#include "radio.h"
#include "utils.h"
//#include "test/testcases.h"

#include <iostream>
#include <fstream>

using namespace std;


// this example will test an area around a tower for radio coverage
int main(int argc, char** argv) {
	
	Convert* norm = new Convert();
	Convert* albers = new ConvertAlbers();
	bool cache = false;
	
	// load all data sources needed
	SourceGroup* sg = new SourceGroup();
	sg->add(new SourceGridFloat(norm, TYPE_ELEV, "data/80214271.elev/80214271.hdr", cache));
	//sg->add(new SourceGridFloat(albers, TYPE_VEGHEIGHT, "data/22276103.height/22276103.hdr", cache));
	//sg->add(new SourceGridFloat(albers, TYPE_VEGTYPE, "data/22273282.type/22273282.hdr", cache));
	//sg->add(new SourceInteger(albers, TYPE_LAND, "data/99981370.land/99981370.hdr", cache));
	
	// create central tower point and set height
	Point* tower = new Point(45.52391667, -111.2476944);
	tower->towerHeight = 10;
	vector<Point*> list;
	
	// define our area of interest as 5 kilometers around tower and turn it into list of discrete points using a 75 meter grid
	RegionArea* area = new RegionArea(tower, 5);
	list = area->discrete(0.075);
	
	// define our area of interest as an entire roadway, split into discrete points every 25 meters
	//RegionLine* road = new RegionLine("data/mt199-highway.txt");
	//list = road->discrete(0.025);
	
	// show progress display to user
	TimeRemaining *t = new TimeRemaining(list.size(), 256);
	
	// save results to file
	ofstream out("data/predicted.txt");
	out.precision(8);
	
	// run a path loss to each point
	vector<Point*>::iterator it;
	for(it = list.begin(); it != list.end(); it++) {
		Point* r = *it;
		r->towerHeight = 10;
		
		// test path loss if stepping along path in 10 meter increments, using 4 watt transmitter, no antennas, and 900MHz radio system
		double loss = pathLoss(tower, r, sg, 0.010, 4000, 0, 900);
		double longleyLoss = pathLossLongley(tower, r, sg, 0.010, 4000, 0, 900);
		t->increment();
		
		// only output if we can actually cover the point being tested
		if(loss == DENIED) continue;
		out << r->lat << "\t" << r->lon << "\t" << loss << "\t" << longleyLoss << endl;
		
	}
	
	out.close();
	
}




