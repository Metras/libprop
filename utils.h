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

#include <math.h>
#include <float.h>
#include <assert.h>

#include <iostream>
#include <time.h>

using namespace std;


#define PI 3.14159
#define RADIUS 6378.2064
 
/// Convert given degree value into radians
double toRadians(double d);

/// Convert given radians value into degrees
double toDegrees(double r);

/// Convert the given character string into a single value, interpreting it using the IEEE single standard.
/// @param data2 Value to be converted, requires four bytes
/// @return Interpreted value expressed as a double
double ieee_single(char *data2);



/// Helpful class to express remaining time to a user.  Displays a progress bar with a remaining time in minutes and seconds.
class TimeRemaining {
private:
	/// Current increment value
	int value;
	/// Number of every increment() calls we update display
	int display;
	/// Total number of increment() calls we expect to process
	int max;
	/// Width of progress bar to output
	int wide;
	/// Marked starting time of first increment() call
	time_t start;

public:
	/// Create a new progress bar with user output.
	/// @param _max The total number of increment() calls that we expect to process
	/// @param _display Update the user display after every "display" number of increment() calls
	TimeRemaining(int _max, int _display);
	
	/// Increment our internal counter by one task.  Will update the user display if needed.
	void increment();
};

