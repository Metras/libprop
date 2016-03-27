# Examples #

You should consider including a 10dBm link margin to account for other interference.

## Path loss between two points ##

Here is a quick example of how to find the path loss between two points:

```
// open elevation data source
SourceGroup* sg = new SourceGroup();
sg->add(new SourceGridFloat(new Convert(), TYPE_ELEV, "data/80214271.elev/80214271.hdr", false));

// define points to test, both with 5-meter towers
Point* start = new Point(45.52391667, -111.2476944, 5);
Point* end = new Point(45.516652, -111.254980, 5);

// assume 10-meter detail, 4-watt transmitter,
// 0dB antennas, and a 900MHz system
// returned value is signal strength in dBm
double loss = pathLoss(start, end, sg, 0.010, 4000, 0, 900);

// link is possible if loss is above the receiver sensitivity
bool possible = (loss > -85);
```


## Testing a roadway ##

Here is another example of testing all points along an entire roadway loaded from a file.  The input file is a series of lat/long tab-separated pairs which could be derived from GIS files provided by a state DOT.  We correctly space the discrete points, regardless of spacing used in input file.

```
// open elevation data source
SourceGroup* sg = new SourceGroup();
sg->add(new SourceGridFloat(new Convert(), TYPE_ELEV, "data/80214271.elev/80214271.hdr", true));

// define tower point we are interested in with 10 meter height
Point* tower = new Point(45.52391667, -111.2476944, 10);

// load roadway from input file
// split into discrete points every 50 meters
RegionLine* road = new RegionLine("data/mt199-highway.txt");
vector<Point*> list = road->discrete(0.050);

// run a path loss from tower to each road point
// assume road point antennas are all 3 meters high
vector<Point*>::iterator it;
for(it = list.begin(); it != list.end(); it++) {
    Point* r = *it;
    r->towerHeight = 3;

    // assume 25-meter detail, 1-watt transmitter,
    // 10dB antennas on both ends, and a 2.4GHz system
    // returned value is signal strength in dBm
    double loss = pathLoss(tower, r, sg, 0.025, 1000, 10+10, 2400);

    // output any signal strength if not completely blocked
    if(loss == DENIED) continue;
    cout << r->lat << "\t" << r->lon << "\t" << loss << endl;
}
```


## Testing a grid region with vegetation ##

Here is another example of testing all points in a grid around a given tower, while also adding vegetation information.

```
// open elevation data source, along with vegetation information
SourceGroup* sg = new SourceGroup();
sg->add(new SourceGridFloat(new Convert(), TYPE_ELEV, "data/80214271.elev/80214271.hdr", true));
sg->add(new SourceGridFloat(new ConvertAlbers(), TYPE_VEGHEIGHT, "data/22276103.height/22276103.hdr", true));
sg->add(new SourceGridFloat(new ConvertAlbers(), TYPE_VEGTYPE, "data/22273282.type/22273282.hdr", true));

// define tower point we are interested in with 10 meter height
Point* tower = new Point(45.52391667, -111.2476944, 10);

// define area with 5-km radius from tower
// split into discrete points every 100 meters
RegionArea* area = new RegionArea(tower, 5);
vector<Point*> list = area->discrete(0.100);

// test loss between points as shown above
// ...
```
