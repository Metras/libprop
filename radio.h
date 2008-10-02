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

#include "geom.h"
#include "source.h"
#include "utils.h"


#define SPEED_LIGHT 299792458 // meters/second
#define DENIED 1024
#define LARGE_LOSS -16777216

#define CONSIDER_VEG true
#define CONSIDER_LAND true

/// Calculate the knife-edge loss given the situation variables.
/// @param startY Signal origin elevation point, in meters
/// @param endY Signal end elevation point, in meters
/// @param pointY Elevation where we are currently calculating knife-edge loss, in meters
/// @param distX Distance along the path between the origin and end points, in meters
/// @param dist1X Fraction along the path between origin and end point, from origin point
/// @param dist2X Fraction along the path between origin and end point, remaining towards end point
/// @param lambda Lambda value calculated using signal frequency
/// @param sight Imaginary elevation of line-of-sight between origin and end point, in meters
/// @return Calculated loss as provided by knife-edge diffraction model, in dBm
double knifeEdgeLoss(double startY, double endY, double pointY, double distX, double dist1X, double dist2X, double lambda, double sight);

/// Calculate the fresnel loss given the situation variables.  This model was found in literature, but seems less reliable.
/// @param ground Elevation of ground at current point, in meters
/// @param sight Imaginary elevation of line-of-sight between origin and end point, in meters
/// @param fresnel Low-point elevation of first-fresnel zone at current point, in meters
/// @return Calculated loss, in dBm
double calcFresnelLoss(double ground, double sight, double fresnel);

/// Calculate the land-use attentuation loss assuming we pass through a specific distance of given type.  This equation seems very unreliable, and has not been verified.
/// @param type Enumeration value of type of land-use we are passing through
/// @param distance Distance of this land-use type that our signal passes through, in meters
/// @param freq Frequency of our transmitted signal, in MHz
/// @return Calculated loss, in dBm
double calcLandLoss(int type, double distance, double freq);

//double pathLoss(Point* p, Point* q, SourceGroup* s, double resolution);

/// Calculate the loss if we follow a given path between two radio towers.
/// @param p Signal origin point, with towerHeight set
/// @param q Signal destination point, with towerHeight set
/// @param s SourceGroup to provide elevation and vegetaion data as required
/// @param resolution Detail used to step along the line-of-sight path, in kilometers
/// @param txPower Transmitter power, in mW
/// @param antenna Total antenna gain of both receiver and transmitter, in dB
/// @param freq Frequency that radios operate at, in MHz
/// @return Calculated loss along given path, in dBm
double pathLoss(Point* p, Point* q, SourceGroup* s, double resolution, double txPower, double antenna, double freq);


/// Calculate the loss if we follow a given path between two radio towers.  Uses Longley-Rice propagation model to calculate attenuation.
/// @param p Signal origin point, with towerHeight set
/// @param q Signal destination point, with towerHeight set
/// @param s SourceGroup to provide elevation and vegetaion data as required
/// @param resolution Detail used to step along the line-of-sight path, in kilometers
/// @param txPower Transmitter power, in mW
/// @param antenna Total antenna gain of both receiver and transmitter, in dB
/// @param freq Frequency that radios operate at, in MHz
/// @return Calculated loss along given path, in dBm
double pathLossLongley(Point* p, Point* q, SourceGroup* s, double resolution, double txPower, double antenna, double freq);
