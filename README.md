UNO IRC bot
======
**UNO** is written with the Qt Framework.

## Usage
Qt5 is required to build it
# Linux
```
$ git clone https://github.com/TheShayy/UNO.git && cd UNO
$ ./configure
$ ./buildUNO
```
Once built, you can copy UNO and its environment by copying the UNO/build directory

# Windows
```
> git clone https://github.com/TheShayy/UNO.git && cd UNO
> call configure.bat
> mingw32-make
```
Once built, UNO.exe will be available inside the src\release directory

### Infos
- Latest update date is in src/commit_date.h
- To see which version you are using, you can send it a CTCP VERSION or launch it with --version or -v argument

### External library
- libcommuni (https://github.com/communi/libcommuni)

## Contact
* IRC: irc.freenode.net #unobot
* Mail: shayy.public [at] tuxange.org
