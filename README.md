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
> fftw under Linux and Windows (GPL, http://www.fftw.org)
> libzita-convolver3 optional for Linux (GPL v3)

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
- fftw3, Eigen 3
- Standalone applications: ASIO SDK if you want ASIO under Windows

Install LINUX dependencies (Debian, Ubuntu):
--------------
```sudo apt install libasound2-dev libjack-jackd2-dev \
    ladspa-sdk \
    libcurl4-openssl-dev  \
    libfreetype6-dev \
    libx11-dev libxcomposite-dev libxcursor-dev libxcursor-dev libxext-dev libxinerama-dev libxrandr-dev libxrender-dev \
    libwebkit2gtk-4.0-dev \
    libglu1-mesa-dev mesa-common-dev \
	libfftw3-dev \
	libzita-convolver3 \
	libzita-convolver-dev
```

Install MacOSX dependencies (through MacPorts):
--------------

```sudo port install eigen3-devel```


Install Windows dependencies
--------------
*getting the dependencies for Windows is a little bit more tricky. you have to get fftw3 and Eigen.*


howto build ambix yourself:
--------------

- use cmake gui or cmake/ccmake from terminal:

- adjusts the various parameters to your needs, make sure the libraries are found

**TERMINAL:**

- create a folder in the `ambix` folder eg. `BUILD`

*ambix/BUILD> $ ccmake ..*

- adjust parameters

then
*ambix/BUILD> $ make*

*(if you have a multicore processor you can speed up the make process by using `make -j #CPUCORESx1.5`)*

- find the binaries in the *ambix/BUILD/_bin* folder and copy to system VST folder

**VST installation folders:**


- MacOSX: `/Library/Audio/Plug-Ins/VST or ~/Library/Audio/Plug-Ins/VST`
- Windows: eg. `C:\Programm Files\Steinberg\VstPlugins`
- Linux: `/usr/lib/lxvst` or `/usr/local/lib/lxvst`

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
* v0.3.0 (2022-04-16)
new builds optimized for Apple Silicon and 64 bit Intel Mac; Win 64 bit; Update to JUCE 7, removed soxr dependency to simplify build

* v0.2.10 (2020-02-06)
amibx_binaural: fix dropouts/artifacts for hosts that send incomplete block sizes (eg. Adobe, Steinberg); option to store preset within the project -> allows to exchange a DAW (eg. Reaper) project without need to provide the preset files extra, allow to export stored preset as .zip file for recovering it from the project

* v0.2.9 (2019-06-12)
amibx_warp: fixed crash

* v0.2.8 (2017-05-20)
ambix_binaural: fftw threadsafty improved: fix crash during startup if other plugin uses fftw
ambix_rotator: add quaternion input

* v0.2.7 (2017-03-22)
ambix_binaural: improved performance, various bugfixes
ambix_rotator: improved performance
ambix_encoder: adjustable custom id for remote control

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
2013-2022 Matthias Kronlachner

m.kronlachner@gmail.com
www.matthiaskronlachner.com
