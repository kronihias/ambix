ambiX Ambisonic plug-in suite
==========

> cross-platform Ambisonic VST, LV2 plug-ins with variable order for use in Digital Audio Workstations like Reaper or Ardour or as Jack standalone applications.
> The plug-in suite use the ambiX *(1)* convention (ACN channel order, SN3D normalization, full periphony (3D)) except the sqrt(1/4pi) factor in equation 3.
> these plug-ins use a recursive implementation of the spherical harmonics, therefore the maximum Ambisonic order is defined at compile time. the practical maximum order is rather defined by the hosts maximum channel count (N+1)^2 or your CPU power

*(1) http://iem.kug.ac.at/fileadmin/media/iem/projects/2011/ambisonics11_nachbar_zotter_sontacchi_deleflie.pdf*


> This software uses the JUCE C++ framework, which is under GPL license.
More information: http://www.juce.com

> other libraries being used:
> Eigen (MPL2, http://eigen.tuxfamily.org), 
> libsoxr (LGPL, http://soxr.sourceforge.net)
> fftw under Linux and Windows (GPL, http://www.fftw.org)

this software package goes well together with the mcfx - multichannel cross plattform audio plug-in suite: http://www.matthiaskronlachner.com/?p=1910


plug-in reference:
----------
> some information about the software can be found here: http://lac.linuxaudio.org/2013/papers/51.pdf, http://iaem.at/Members/zotter/publications/2014_KronlachnerZotter_AmbiTransformationEnhancement_ICSA.pdf


* ambix_binaural - binaural decoder with various loudspeaker setups in real world studios/venues

* ambix_decoder - same as binaural decoder but without the convolution, the loudspeaker signals are sent directly to the outputs (single band decoding)
	* presets for ambix_binaural (including impulse responses) and ambix_decoder are located in the folder ambix_binaural/Presets (or available as separate download) and should be copied in following folders
		* Windows 7,8: C:\Users\username\AppData\Roaming\ambix\binaural_presets\
		* MacOS: ~/Library/ambix/binaural_presets/
		* Linux: ~/ambix/binaural_presets/

* ambix_converter - convert between different ambisonic standards on the fly (include different standards in one project), also between 2D/3D

* ambix_directional_loudness - amplify, attenuate or filter out certain parts of the spherical soundfielddf

* ambix_encoder - panning plug-in with different numbers of input channels - a width parameter spreads those channels equally along the azimuth

* ambix_maxre - apply or reciprocal apply spherical max_re weighting to suppress sidelobes according to Zotter, Frank - "All-Round Ambisonic Panning and Decoding"
 
* ambix_mirror - invert or mirror soundfield about x/y/z axis

* ambix_rotator_z - rotation around z axis

* ambix_rotator - rotation around xyz axis

* ambix_vmic - same principle as directional loudness but will not output ambisonics signal but the selected part of the soundfield - similar to the virtual microphone approach

* ambix_warp - warp soundfield towards equator, poles, front or back...* 

* ambix_widening - frequency dependant rotation around the z-axis, use this for source widening or creating diffuse early reflections, article: http://dx.doi.org/10.14279/depositonce-12, Created with help of Franz Zotter and Matthias Frank



prerequisites for building
--------------

- cmake, working build environment
- Steinberg VST 2.4 SDK
- fftw3, Eigen 3
- Standalone applications: ASIO SDK if you want ASIO under Windows

Install LINUX dependencies (Debian, Ubuntu):
--------------
*$ sudo apt-get install libasound-dev libfreetype6-dev libgl1-mesa-dev libx11-dev libxext-dev libxinerama-dev libxcursor-dev libeigen3-dev freeglut3-dev libxmu-dev libxi-dev libsoxr-dev libfftw3-dev*

Fedora:
--------------
*$ sudo dnf install alsa-lib-devel freetype-devel mesa-libGL-devel libX11-devel libXext-devel libXinerama-devel libXcursor-devel eigen3-devel freeglut-devel libXmu-devel libXi-devel soxr-devel fftw3-devel libXrandr-devel jack-audio-connection-kit-devel*

*$ sudo dnf groupinstall "Development Tools"*

*$ sudo dnf groupinstall "C Development Tools and Libraries"*


Install MacOSX dependencies (through MacPorts):
--------------

*$ sudo port install eigen3-devel soxr*


Install Windows dependencies
--------------
*getting the dependencies for Windows is a little bit more tricky. you have to get fftw3, libsoxr and Eigen.*


howto build ambix yourself:
--------------

- copy the Steinberg VST 2.4 SDK into the folder *mcfx/vstsdk2.4* (do to legal reasons those can not be included here)

- use cmake gui or cmake/ccmake from terminal:

- adjusts the various parameters to your needs, make sure the libraries are found

**TERMINAL:**

- create a folder in the *ambix* folder eg. *BUILD*

*ambix/BUILD> $ ccmake ..*

- adjust parameters 

then
*ambix/BUILD> $ make*

*(if you have a multicore processor you can speed up the make process by using make -j #CPUCORESx1.5)*

- find the binaries in the *ambix/BUILD/_bin* folder and copy to system VST folder

**VST installation folders:**


- MacOSX: /Library/Audio/Plug-Ins/VST or ~/Library/Audio/Plug-Ins/VST
- Windows: eg. C:\Programm Files\Steinberg\VstPlugins
- Linux: /usr/lib/lxvst or /usr/local/lib/lxvst

LV2 plug-in
-----------

- compile the plug-ins with the flag BUILD_LV2 ON

- go to the folder *lv2-ttl-generator* and **> make** for compiling the tool *lv2_ttl_generator*

- go to the _bin folder (eg. *ambix/BUILD/_bin* ) and execute the script *> ./../../lv2-ttl-generator/generate-ttl.sh*
this will generate all needed .tll files, afterwards you can copy all .lv2 folders from *ambix/BUILD/_bin/lv2* to /usr/lib/lv2

known problems
-----------
* documentation missing - for now you can check this: 
http://lac.linuxaudio.org/2013/papers/51.pdf

http://iaem.at/Members/zotter/publications/2014_KronlachnerZotter_AmbiTransformationEnhancement_ICSA.pdf

http://www.matthiaskronlachner.com/wp-content/uploads/2013/01/Kronlachner_Master_Spatial_Transformations_Mobile.pdf

http://www.matthiaskronlachner.com/wp-content/uploads/2013/01/kronlachner_aes_studentdesigncompetition_2014.pdf


* GUI less plug-ins can not be used as standalone - therefore GUI for some more of the plug-ins would be nice...

* Linux: LV2 plug-ins don't show the GUI, VST plug-ins GUI might crash - better just stick to the hosts' GUI or help me fix that. you can use the standalone version in any case and connect via Jack

* different orders require different plug-in instances: maybe VST 3 can fix that with it's dynamic input/output ports. but we'll have to wait for it beeing implemented in appropriate hosts.



changelog
-----------
* v0.2.6 (2016-04-08) converter: fixed scaling of "O" channel when converting from/to FuMa; binaural: convolution engine fix, preset dir save fix; vmic: gui fix

* v0.2.5 (2015-12-06) liblo dependencies removed
encoder: improved gui performance
decoder: fixed binaural swap l-r impulse responses bug, added volume control, indicate loaded preset in menu
directional_loudness: gui and different solo/window behavior (multiple filters are now passed through if soloed), 8 filters by default
rotator: gui, osc rewrite
vmic: gui, 8 outputs by default

* v0.2.4 (2015-07-19) improved efficiency for binaural decoder

* v0.2.3 (2014-12-27) multichannel encoder display actual source positions, compatibility with audiomulch for saving settings

* v0.2.2 (2014-08-18) encoder flickering fix, added osc settings, new control modes for encoder display: right mouse click for relative source movement, press shift to freeze elevation while moving and ctrl to freeze azimuth

* v0.2.1 (2014-04-17) fixed vst identifier for Plogue Bidule compatibility

* v0.2.0 (2014-03-30) added ambix_widening, JUCE update, encoder GUI panning fix

* (2014-03-15) fixed binaural decoder crash during configuration unloading

* (2014-02-19) warping curve 2 changed slightly (icsa paper), warping pre-emphasis added, encoder abs(elevation) > 90° was wrong!

* (2014-02-13) ambix_maxre added

* v0.1.0  (2014-01-24) - first release


thanks to
-----------
several people and institutions contributed to this software in one or another way, i would like to name them here without particular order: Institute of Electronic Music and Acoustics Graz, Franz Zotter, Winfried Ritsch, Martin Rumori, Florian Hollerweger, Peter Plessas, IOhannes Zmölnig, Thomas Musil, Gerriet K. Sharma, Matthias Frank, Fons Adriaensen, Jörn Nettingsmeier, Filipe Coelho (DISTRHO project), Music Innovation Studies Centre of the Lithuanian Academy of Music and Theatre, Ricardas Kabelis, Mantautas Krukauskas, Tadas Dailyda, Sebastian Grill, the surrsound and linux audio community - to be continued...


author
-----------
2013-2016 Matthias Kronlachner

m.kronlachner@gmail.com
www.matthiaskronlachner.com
