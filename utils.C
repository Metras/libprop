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

#include "utils.h"

#include <time.h>
#include <iostream>

using namespace std;

double toRadians(double d) {
	return d * PI / 180;
}

double toDegrees(double r) {
	return r * 180 / PI;
}

// taken from http://www.thescripts.com/forum/thread59795.html
// requires input in reversed byte order
double ieee_single(char *data2) {
	//const unsigned char *data = v;
	unsigned char data[4];
	data[0] = data2[3];
	data[1] = data2[2];
	data[2] = data2[1];
	data[3] = data2[0];
	int s, e;
	unsigned long src;
	//long f;
	double f;
	double value;

	src = ((unsigned long)data[0] << 24) +
		((unsigned long)data[1] << 16) +
		((unsigned long)data[2] << 8) +
		((unsigned long)data[3]);

	s = (src & 0x80000000UL) >> 31;
	e = (src & 0x7F800000UL) >> 23;
	f = (src & 0x007FFFFFUL);

	if (e == 255 && f != 0) {
		/* NaN - Not a number */
		value = DBL_MAX;
	} else if (e == 255 && f == 0 && s == 1) {
		/* Negative infinity */
		value = -DBL_MAX;
	} else if (e == 255 && f == 0 && s == 0) {
		/* Positive infinity */
		value = DBL_MAX;
	} else if (e > 0 && e < 255) {
		/* Normal number */
		f += 0x00800000UL;
		if (s) f = -f;
		value = ldexp(f, (int)(e - 127 - 23));
	} else if (e == 0 && f != 0) {
		/* Denormal number */
		if (s) f = -f;
		value = ldexp(f, -126 - 23);
	} else if (e == 0 && f == 0 && s == 1) {
		/* Negative zero */
		value = 0;
	} else if (e == 0 && f == 0 && s == 0) {
		/* Positive zero */
		value = 0;
	} else {
		/* Never happens */
		//printf("s = %d, e = %d, f = %lu\n", s, e, f);
		//assert(!"Oops, unhandled case in ieee_single()");
	}
	return value;
}

TimeRemaining::TimeRemaining(int _max, int _display) : max(_max), display(_display) {
	value = 0;
	wide = 50;
	time(&start);
}

void TimeRemaining::increment() {
	value++;
	if(value % display != 0 && value != max) return;

	time_t end; time(&end);
	double diff = difftime(end, start);
	int sec = (int)((double)diff / (double)value * (double)(max - value));
	int prog = (int)((double)value * (double)wide / (double)max);
	int pct = (int)((double)value * 100 / (double)max);

	if(prog > wide) return;

	string a(prog, '*');
	string b(wide - prog, ' ');

	int min = sec / 60;
	sec -= min * 60;

	cout << "\r[" << a << b << "] " << pct << "%, " << min << " min " << sec << " sec      ";
	fflush(stdout);		
}
