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

#include "radio.h"

#include "geom.h"
#include "source.h"
#include "utils.h"

//class LongleyWrapper;
#include "longley.C"


double knifeEdgeLoss(double startY, double endY, double pointY, double distX, double dist1X, double dist2X, double lambda, double sight) {
	// calculate loss along a given path, assuming a knife-edge model
	// http://wireless.ictp.trieste.it/school_2002/labo/linkloss/index.html
	double d = sqrt(pow(endY - startY, 2) + pow(distX, 2)),
		d1 = sqrt(pow(dist1X, 2) + pow(startY - pointY, 2)),
		d2 = sqrt(pow(dist2X, 2) + pow(endY - pointY, 2));

	double deltaD = d1 + d2 - d,
		v = 2 * sqrt(deltaD / lambda);
	double loss = 6.9 + 20 * log10(sqrt(pow(v, 2) + 1) + v);
	return loss;
}


double calcFresnelLoss(double ground, double sight, double fresnel) {
	double partial = fresnel*0.6;
	double margin = sight - ground;
	if(margin > 0.6 * fresnel) return 0;
	return 6 * (margin / partial);
}



double calcLandLoss(int type, double distance, double freq) {
	double loss = 0;
	// equations below are coming from TIA TR8 using an curve fit through their loss values
	// we assume their values are over 200-meter grids, so we treat accordingly
	switch(type) {
		case LAND_FOREST:
			loss = -9.484 + 2.776 * log(freq); break;
		case LAND_RESIDENTIAL:
			loss = -9.735 + 3.196 * log(freq); break;
		case LAND_COMMERCIAL:
			loss = -10.31 + 3.616 * log(freq); break;
	}
//cout << "about to use loss=" << loss << "/200m with total dist=" << distance << endl;
	distance /= 200;
	return loss * distance;
}



double pathLoss(Point* p, Point* q, SourceGroup* s, double resolution, double txPower, double antenna, double freq) {
// lilys studies are 4000, 0, 900
//	double txPower = 1000, // mW
//		antenna = 0, // dB/dBi
//		freq = 900; // MHz

	// perform radio conversions
	double lambda = SPEED_LIGHT / (freq * 1000000);
	double txPowerDbm = 10 * log10(txPower);

	double dist = p->distance(q) * 1000;
	s->resolve(p);
	s->resolve(q);

	// check if outside of radio horizon
	// http://en.wikipedia.org/wiki/Radio_horizon
	double elevStart = p->elev + p->towerHeight,
		elevEnd = q->elev + q->towerHeight;
	double horizon = (3.569 * sqrt(elevStart)) * 1000;
	if(dist > horizon)
		return DENIED;

	// correct ending elevation for earth curvature
	double curve = (pow(dist / 1000, 2) / (2 * RADIUS)) * 1000; // http://mathforum.org/library/drmath/view/54904.html
	elevEnd -= curve;

	// gather data along signal path
	RegionLine* line = new RegionLine(p, q);
	vector<Point*> path = line->discrete(resolution);
	s->resolveList(path);

	double vegDepth = 0;
	double forestDepth = 0, residentialDepth = 0, commercialDepth = 0;
	double worstFresnel = 0, freeSpace = 0, vegLoss = 0, landLoss = 0;
	bool lineDead = false;

	// walk along entire path
	for(unsigned int i = 0; i < path.size(); i++) {
		Point* r = path[i];

		double fraction = (double)i / (double)path.size();
		double d1 = fraction * dist,
			d2 = (1 - fraction) * dist;

		double ground = r->elev;
		double sight = ((elevEnd - elevStart) * fraction) + elevStart;
		double fresnel = sqrt((lambda * d1 * d2) / (d1 + d2)); // http://en.wikipedia.org/wiki/Fresnel_zone
		double curve = (pow(d1 / 1000, 2) / (2 * RADIUS)) * 1000; // http://mathforum.org/library/drmath/view/54904.html
		double veg = r->vegHeight;

		//cout << "walking sight=" << sight << "\tground=" << ground << endl;

		// assert good elevation data and correct for earth curve
//		assert(ground != -1);
		ground -= curve;

		// check for line-of-sight
		if(sight < ground) {
			lineDead = true;
			break;
		}

		// check for fresnel violation
		if(sight - fresnel < ground) {
/**/
			double startY = elevStart, pointY = ground,
				endY = elevEnd - ((pow(dist / 1000, 2) / (2 * RADIUS)) * 1000);
			double distX = dist, dist1X = d1, dist2X = d2;

			double fresLoss = knifeEdgeLoss(startY, endY, pointY, distX, dist1X, dist2X, lambda, sight);
/**/
//			double fresLoss = calcFresnelLoss(ground, sight, fresnel);
			if(fresLoss > worstFresnel)
				worstFresnel = fresLoss;

		}

		// check for vegetation
		if(sight < ground + veg) {
			vegDepth += resolution;
		}

		// measure all land use depths
//if(sight - fresnel < ground) {
		switch(r->landType) {
			case LAND_FOREST: forestDepth += resolution; break;
			case LAND_RESIDENTIAL: residentialDepth += resolution; break;
			case LAND_COMMERCIAL: commercialDepth += resolution; break;
		}
//}
	}

	freeSpace = 32.4 + 20 * log10(freq) + 20 * log10(dist / 1000);

	if(CONSIDER_VEG) {
		//cout << "considering vegetation" << endl;
		vegDepth *= 1000;
		if(vegDepth < 14) {
			vegLoss = 0.45 * pow(freq / 1000, 0.284) * vegDepth;
		} else if(vegDepth < 400) {
			vegLoss = 1.33 * pow(freq / 1000, 0.284) * pow(vegDepth, 0.588);
		} else {
			vegLoss = LARGE_LOSS;
		}
	}

	if(CONSIDER_LAND) {
/**/
		forestDepth *= 1000;
		residentialDepth *= 1000;
		commercialDepth *= 1000;

		landLoss += calcLandLoss(LAND_FOREST, forestDepth, freq);
		landLoss += calcLandLoss(LAND_RESIDENTIAL, residentialDepth, freq);
		landLoss += calcLandLoss(LAND_COMMERCIAL, commercialDepth, freq);
/**/
/** /
// consder land use at destination point
switch(q->landType) {
	case LAND_FOREST:
		landLoss += -9.484 + 2.776 * log(freq); break;
	case LAND_RESIDENTIAL:
		landLoss += -9.735 + 3.196 * log(freq); break;
	case LAND_COMMERCIAL:
		landLoss += -10.31 + 3.616 * log(freq); break;
}

switch(p->landType) {
	case LAND_FOREST:
		landLoss += -9.484 + 2.776 * log(freq); break;
	case LAND_RESIDENTIAL:
		landLoss += -9.735 + 3.196 * log(freq); break;
	case LAND_COMMERCIAL:
		landLoss += -10.31 + 3.616 * log(freq); break;
}

/**/


	}

//cout << "\n========\nfreeSpace=" << freeSpace; cout << "\nworstFresnel=" << worstFresnel; cout << "\nvegLoss=" << vegLoss; cout << "\nlandLoss=" << landLoss;

	double system = txPowerDbm + antenna;
	double totalLoss = freeSpace + worstFresnel + vegLoss + landLoss;

	// clean up discrete path that we built
	delete line;
	while(!path.empty()) {
		delete path.back();
		path.pop_back();
	}

	if(lineDead)
		return DENIED;
	else
		return system - totalLoss;

}



double pathLossLongley(Point* p, Point* q, SourceGroup* s, double resolution, double txPower, double antenna, double freq) {

	// perform radio conversions
	double lambda = SPEED_LIGHT / (freq * 1000000);
	double txPowerDbm = 10 * log10(txPower);

	double dist = p->distance(q) * 1000;
	s->resolve(p);
	s->resolve(q);

	// gather data along signal path
	RegionLine* line = new RegionLine(p, q);
	vector<Point*> path = line->discrete(resolution);
	s->resolveList(path);

if(path.size() == 0) return txPowerDbm + antenna;
//cout << "pathLossLongley: path.size()=" << path.size() << endl; cout.flush();

	// form elev[] array to pass to longley rice algorithm
	double* elev = new double[path.size() + 2];
	elev[0] = path.size()-1;
	elev[1] = resolution*1000;
	for(unsigned int i = 0; i < path.size(); i++) {
		Point* r = path[i];
		assert(r->elev != -1);
		elev[i+2] = r->elev;
	}

	// clean up discrete path that we built
	delete line;
	while(!path.empty()) {
		delete path.back();
		path.pop_back();
	}

	// set variables for longley rice algorithm
	// some default values used from http://www.softwright.com/faq/engineering/prop_longley_rice.html
	// horizontal versus vertical polarization http://www.tpub.com/neets/book10/42c.htm
	double tht_m = p->towerHeight; // transmitter height (meters)
	double rht_m = q->towerHeight; // receiver height (meters)

	double eps_dielect = 15; // dielectric constant, set for average ground
	double sgm_conductivity = 0.005; // conductivity constant, set for average ground
	double eno_ns_surfref = 301; // Surface refractivity of the atmosphere, set for average

	double frq_mhz = freq; // radio frequency (mhz)
	int radio_climate = 5; // climate, set for continental temperate

	int pol = 0; // polarization, set for horizontal
	double conf = 0.9; // time variability, set to 50%
	double rel = 0.9; // situation (confidence) variability, set to 50%

	double dbloss = -1; // calculated loss in dbm
	char strmode[128]; // string describing mode used
	int errnum = -1; // resulting error code

	// run actual longley rice calculation
	point_to_point(elev, tht_m, rht_m, eps_dielect, sgm_conductivity, eno_ns_surfref, frq_mhz, radio_climate, pol, conf, rel, dbloss, strmode, errnum);

	// delete elevation data
	delete[] elev;

	// check for any error codes
	if(errnum != 0) return DENIED;

/** /
	//assert(errnum == 0);
	cout << "pathLossLongley: debug strmode=" << strmode << endl;
	switch(errnum) {
		case 0: break;
		case 1:
			cout << "pathLossLongley: Warning: Some parameters are nearly out of range. Results should be used with caution." << endl;
			return DENIED; break;
		case 2:
			cout << "pathLossLongley: Note: Default parameters have been substituted for impossible ones." << endl;
			return DENIED; break;
		case 3:
			cout << "pathLossLongley: Warning: A combination of parameters is out of range. Results are probably invalid." << endl;
			return DENIED; break;
		default:
			cout << "pathLossLongley: errnum=" << errnum << " Warning: Some parameters are out of range. Results are probably invalid." << endl;
			return DENIED; break;
	}
/**/

	double system = txPowerDbm + antenna;
	double totalLoss = dbloss;

	return system - totalLoss;

}


