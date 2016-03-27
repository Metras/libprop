libprop uses high-resolution USGS terrain data to accurately predict radio propagation. It's a set of C++ classes that you can easily incorporate into an existing project.

  * It uses 10-meter USGS terrain data to walk along the line-of-sight propagation path
  * Takes in radio frequency, transmitter power, and antenna gains, and outputs estimated signal strength at destination. This strength can then be compared against a receiver sensitivity.
  * Models first Fresnel zone violations as knife-edge diffractions
  * Handles vegetation penetration using Weissberger's Model
  * Can also calculate Longley-Rice point-to-point path loss

All libprop materials are released under GPLv3 or GFDL.

The original code was developed by Jeffrey Sharkey as part of his thesis work at Montana State University. His work was sponsored by the Western Transportation Institute, and was guided by advisor Doug Galarus. Other valuable guidance was provided by Dr. Bill Jameson and Gary Schoep.

This code was used as part of Jeffrey's thesis research which uses Artificial Intelligence to automate radio network design.  Code from the AI project can be found at:  http://code.google.com/p/aco-netdesign/