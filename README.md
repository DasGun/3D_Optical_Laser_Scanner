#3D Optical Laser Scanner
===========
 
University of Queensland 2015 Semester 1 Thesis Project

Dasun Gunasinghe
 
## Project Overview ##
 
The aim of the thesis was to research and design an optical 3D laser desktop scanner. This was conducted to allow prototyping engineers an easily accessible method to scan objects in a research laboratory setting. The system was designed in phases, initially based off a simple triangulation laser line design applying basic calibration procedures and a dimensional acquisition algorithm. Following this base outline, the design was able to be built up and improved using techniques such as planar homography - through camera calibration - and sub-pixel detection of points.
 
## Code Description ##
 
This code is for a 3D optical laser scanner using a dual laser and a camera setup. The data acquisition is accomplished using planar homography, with convolution using a Gaussian kernel done per image row for sub-pixel detection of the laser line(s).
 
## Dependencies ##
 
    OpenCV [http://sourceforge.net/projects/opencvlibrary/]
    QT Framework [https://www.qt.io/download/]
    QExtSerialPort [https://code.google.com/p/qextserialport/]
    
   
## Compiling ##
 
The code can be compiled in the QT program. To do so, just click the 'run' button on the bottom left, and this will build and run the code.
 
## Usage ##
 
Running the program will bring up an interface for the scanner. To begin a scan, first click 'save mesh' to save the output mesh file to a directory of your choosing. Then, once the 'begin scan' button appears, click it to start the scan. The interface will show a progress bar of the scanning process, and will revert to the initial state on completion. 
 
## Safety ##

Always with the use of any lasers, the classes must be known. If you intend to build your own, make sure you adhere to the laser class system and safety in use. 

See here for more information: https://en.wikipedia.org/wiki/Laser_safety
 
## License ##
 
This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program.  If not, see <http://www.gnu.org/licenses/>.
