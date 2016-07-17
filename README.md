### Intro

This is what I've learned about FFmpeg 3.1.1 (and SDL2 2.0.4, too). It's not exactly same as the tutorial of Dranger (see Reference) in which I've learned. Feel free to use it to upgrade version of the tutorial of Dranger, with danger of yourself :). I'm almost c/cpp/FFmpeg/SDL newbie, the source code is only runnable, not trustable, so send me pull requests if you want to correct it (Thanks anyway).

### Require

* gcc & blabla for building

 * Ubuntu based: `sudo apt-get install build-essential`

 * Redhat/CentOS based: `sudo yum installgroup "Development Tools"`

* FFmpeg

 * `git clone git@github.com:FFmpeg/FFmpeg.git && cd FFmpeg && git checkout refs/tags/n3.1.1`

 * `./configure && make && make check && sudo make install`

* SDL

 * [Suggested solution](http://www.gamedev.net/topic/646010-sdl2-mixer-no-such-audio-device-solved/) for no such audio device problem: `sudo apt-get install libasound2-dev libpulse-dev`

 * `file=SDL2-2.0.4 && wget https://www.libsdl.org/release/${file}.tar.gz && tar -zxvf ${file}.tar.gz && cd ${file}`

 * `./configure && make && sudo make install`

### Build

I only known how to run ./configure, make, make check, make install and I don't want to complicate my pour knowledge with Make/CMake, I use only command line gcc for compile.


* Compile: `FILE=Blabla gcc $FILE.c -o $FILE -lavformat -lavcodec -lavutil -lswscale -lswresample -lm -lz -lpthread $(sdl2-config --cflags --libs) `

* Run: `./FILE path_to_video more_param_blabla`

Note that if you enter problem with any \*.so.\*, remember to include all directories contains required \*.so.\* files in `LD_LIBRARY_PATH`

* Debug: `gdb --agrs ./FILE path_to_video more_param_blabla`, and hit "run"

### Reference

* [An Ffmpeg and SDL tutorial](http://dranger.com/ffmpeg/ffmpeg.html) of Dranger
* [Above tutorial source code](https://github.com/mpenkov/ffmpeg-tutorial) by mpenkov(of course, Dranger has his own source code, but what I read was mpenkov source code)
