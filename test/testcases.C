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
#include "radio.h"
#include "source.h"
#include "utils.h"

#include <time.h>

// test cases
void testPoint() {
	cout << "== testPoint ==" << endl;

	Point* p = new Point(45.52391667, -111.2476944);
	Point* q = new Point(45.49677778, -111.2711389);

	cout << "distance:\tp->q=" << p->distance(q) << "\n\t\tq->p=" << q->distance(p) << endl;
	cout << "bearing:\tp->q=" << toDegrees(p->bearing(q)) << "\n\t\tq->p=" << toDegrees(q->bearing(p)) << endl;

	Point* r = p->project(q->bearing(p), p->distance(q) / 2);
	Point* s = p->project(p->bearing(q) + toRadians(45), p->distance(q) / 2);
	cout << "project:\tp, direction=q->p, distance=1/2 p->q\n\t\t" << *r << "\n\t\t" << *s << endl;

}

void testRegion() {
	cout << "== testRegion ==" << endl;

	Point* p = new Point(45.52391667, -111.2476944);
	Point* q = new Point(45.49677778, -111.2711389);

	RegionArea* normal = new RegionArea(q, p);
	cout << "normal:\t" << *normal << endl;

	RegionArea* add = new RegionArea();
	add->add(p);
	add->add(q);
	cout << "includ:\t" << *add << endl;

	RegionArea* radius = new RegionArea(q, 10);
	cout << "radius:\t" << *radius << endl;

	Point* out1 = p->project(toRadians(90), 0.050);
	Point* out2 = p->project(toRadians(45), 0.050);
	Point* out3 = p->project(toRadians(225), 10);
	Point* in1 = p->project(toRadians(270), 0.050);
	Point* in2 = p->project(toRadians(225), 0.050);

	cout << "out1:\t" << normal->contains(out1) << endl;
	cout << "out2:\t" << normal->contains(out2) << endl;
	cout << "out3:\t" << normal->contains(out3) << endl;
	cout << "in1:\t" << normal->contains(in1) << endl;
	cout << "in2:\t" << normal->contains(in2) << endl;

}

void testReading() {
	cout << "== testReading ==" << endl;

	//Point* p = new Point();
	//cin >> *p;
	//cout << *p;
}


void testFileLine() {
	cout << "== testFileLine ==" << endl;

	string file = "C:\\_JSharkey\\__Input\\ca199-raw.txt";
	RegionLine* line = new RegionLine(file);

	double resolution = 0.500;

	vector<Point*> list = line->discrete(resolution);
	double origlength = line->length(),
		thislength = list.size() * resolution;

	cout << "original=" << origlength << endl;
	cout << "n=" << list.size() << ", this=" << thislength << endl;

	// save each point out to file
	ofstream out("C:\\_JSharkey\\__Output\\test199.kml");
	out << "<?xml version=\"1.0\" encoding=\"Windows-1252\"?><kml xmlns=\"http://earth.google.com/kml/2.0\"><Document>";
	out.precision(8);

	vector<Point*>::iterator it;
	for(it = list.begin(); it != list.end(); it++) {
		out << "<Placemark><name /><Point><coordinates>" << (*it)->lon << "," << (*it)->lat << ",-1</coordinates></Point></Placemark>";
	}

	out << "</Document></kml>";
	out.close();


}






void testReadGF() {
	cout << "== testReadGF ==" << endl;

	Source* s;// = new SourceGridFloatDecimal("C:\\_JSharkey\\__Input\\hw191.80214271\\80214271.hdr", false, TYPE_ELEV);

	Point* p = new Point(45.52391667, -111.2476944);
	Point* q = new Point(45.49677778, -111.2711389);

	s->resolve(p);
	s->resolve(q);

	cout << "p.elev=" << p->elev << endl;
	cout << "q.elev=" << q->elev << endl;


}

void testFloat() {
	//char raw[] = {0x42, 0x7c, 0x00, 0x00};
	//double value = ieee_single(raw);

	//cout << "value=" << value << endl;
}



void testPath() {
	Point* p = new Point(45.52391667, -111.2476944);
	Point* q = new Point(45.530035, -111.235863);
	
	p->towerHeight = 10;
	q->towerHeight = 10;

	Source* s;// = new SourceGridFloatDecimal("C:\\_JSharkey\\__Input\\hw191.80214271\\80214271.hdr", false, TYPE_ELEV);
	SourceGroup* sg = new SourceGroup();
	sg->add(s);

	// should be field strength=-56.6443697139135
//	double loss = pathLoss(p, q, sg, 0.010);
//	cout << "found loss=" << loss << endl;

}

void testBulk() {
	// time running a set of bulk points

	time_t start,end;

	//Source* s = new SourceGridFloatMemory("C:\\_JSharkey\\__Input\\hw191.80214271\\80214271.hdr");
	Source* s;// = new SourceGridFloatDecimal("C:\\_JSharkey\\__Input\\hw191.80214271\\80214271.hdr", false, TYPE_ELEV);
	SourceGroup* sg = new SourceGroup();
	sg->add(s);

	time(&start);

	Point* p = new Point(45.52391667, -111.2476944);
	RegionArea* a = new RegionArea(p, 1);
	vector<Point*> list = a->discrete(0.100);

	cout << "total points=" << list.size() << endl;

	p->towerHeight = 10;

	// run a path loss to each point
	vector<Point*>::iterator it;
	for(it = list.begin(); it != list.end(); it++) {
		Point* q = *it;
		q->towerHeight = 10;

//		double loss = pathLoss(p, q, sg, 0.010);

	}

	time(&end);
	double dif = difftime(end, start);

	cout << "seconds=" << dif << endl;

	cout << "finished!" << endl;

}

void testAlbers() {
	cout << "== testAlbers ==" << endl;

//	prepareAlbers();

	Point* p = new Point(45.52391667, -111.2476944);
/*	PointAlbers* q = new PointAlbers(p);

	cout.precision(12);
	cout << " middleLon=" << middleLon;
	cout << " bigC=" << bigC << endl;
	cout << " coneConst=" << coneConst;
	cout << " r0=" << r0 << endl;

	cout << "lat=" << p->lat << " lon=" << p->lon << endl;
	cout << "albers x=" << q->x << " y=" << q->y << endl;

*/
}

void testVegetation() {
	cout << "== testVegetation ==" << endl;

	Point* p = new Point(45.52391667, -111.2476944);
	Point* q = new Point(45.535590, -111.237847);  // fs=-117.811987241978
	Point* r = new Point(45.515451, -111.249752); // fs=-59.430352054876

	p->towerHeight = 10;
	q->towerHeight = 10;
	r->towerHeight = 10;

	Convert* norm = new Convert();
	Convert* albers = new ConvertAlbers();

	Source* s = new SourceGridFloat(norm, TYPE_ELEV, "mt191/80214271.elev/80214271.hdr", false);
	Source* t = new SourceGridFloat(albers, TYPE_VEGHEIGHT, "mt191/22276103.height/22276103.hdr", false);
	Source* u = new SourceGridFloat(albers, TYPE_VEGTYPE, "mt191/22273282.type/22273282.hdr", false);

	SourceGroup* sg = new SourceGroup();
	sg->add(s);
	sg->add(t);
	sg->add(u);

	sg->resolve(p);
	sg->resolve(q);
	sg->resolve(r);

	cout << "p\telev=" << p->elev << "\tvegtype=" << p->vegType << "\tvegheight=" << p->vegHeight << "\tvegcover=" << p->vegCover << endl;
	cout << "q\telev=" << q->elev << "\tvegtype=" << q->vegType << "\tvegheight=" << q->vegHeight << "\tvegcover=" << q->vegCover << endl;
	cout << "r\telev=" << r->elev << "\tvegtype=" << r->vegType << "\tvegheight=" << r->vegHeight << "\tvegcover=" << r->vegCover << endl;

//	double losspq = pathLoss(p, q, sg, 0.010);
//	double losspr = pathLoss(p, r, sg, 0.010);
	cout.precision(8);
//	cout << "loss pq=" << losspq << endl;
//	cout << "loss pr=" << losspr << endl;

}


void testInteger() {

// load an integer data source and try reading various points

	Convert* albers = new ConvertAlbers();
	Source* s = new SourceInteger(albers, TYPE_LAND, "mt191/99981370.land/99981370.hdr", false);

	Point* p = new Point(45.52391667, -111.2476944);

ofstream out("results/landcover.txt");
out.precision(8);

RegionArea* area = new RegionArea(p, 1);
vector<Point*> list = area->discrete(0.050);

vector<Point*>::iterator it;
for(it = list.begin(); it != list.end(); it++) {
	Point* q = *it;
	s->resolve(q);
	out << q->lon << "\t" << q->lat << "\t" << q->landType << endl;
}

out.close();





return;

	Point* q = new Point(45.535590, -111.237847);
	Point* r = new Point(45.515451, -111.249752);

	s->resolve(p);
	s->resolve(q);
	s->resolve(r);

	cout << "p\ttype=" << p->landType << endl;
	cout << "q\ttype=" << q->landType << endl;
	cout << "r\ttype=" << r->landType << endl;



}



void testTypePath() {

// test the new viewshed with and without path information

SourceGroup* sg = new SourceGroup();
Convert* norm = new Convert();
Convert* albers = new ConvertAlbers();
bool cache = false;

// mt191
sg->add(new SourceGridFloat(norm, TYPE_ELEV, "mt191/80214271.elev/80214271.hdr", cache));
sg->add(new SourceGridFloat(albers, TYPE_VEGHEIGHT, "mt191/22276103.height/22276103.hdr", cache));
sg->add(new SourceGridFloat(albers, TYPE_VEGTYPE, "mt191/22273282.type/22273282.hdr", cache));

Point* p = new Point(45.52391667, -111.2476944);
Point* q = new Point(45.535590, -111.237847);
Point* r = new Point(45.515451, -111.249752);

p->towerHeight = 10;
q->towerHeight = 10;
r->towerHeight = 10;

double pq, pq2, pr, pr2;

pq = pathLoss(p, q, sg, 0.010, 4000, 0, 900);
pr = pathLoss(p, r, sg, 0.010, 4000, 0, 900);

sg->add(new SourceInteger(albers, TYPE_LAND, "mt191/99981370.land/99981370.hdr", cache));

pq2 = pathLoss(p, q, sg, 0.010, 4000, 0, 900);
pr2 = pathLoss(p, r, sg, 0.010, 4000, 0, 900);

cout << "\np->q\tbefore=\t" << pq << "\n\tafter=\t" << pq2 << endl;
cout << "\np->r\tbefore=\t" << pr << "\n\tafter=\t" << pr2 << endl;

}



