# z64rom

### For questions and feature requests [join z64tools discord](https://discord.gg/52DgAggYAT)!

### [Download z64rom here!](https://github.com/z64tools/z64rom/releases)

### Read the documentation [here](https://github.com/z64tools/z64rom/wiki)!

# Credits

**Documentation:** <br>
DezZival <br>
sklitte22

**Testers:** <br>
logdebug <br>
Nokaubure <br>
Skawo <br>
sklitte22 <br>
Zeldaboy14 <br>
zfg

**Special Thanks:** <br>
Dragorn421 <br>
Sauraen <br>
Tharo <br>
zel

**Tools:** <br>
[z64convert](https://github.com/z64me/z64convert) by [z64me](https://z64.me) <br>
[novl](https://github.com/z64me/nOvl) by [z64me](https://z64.me) <br>
[seq64](https://github.com/sauraen/seq64) by [Sauraen](https://github.com/sauraen/) <br>
Sequence Assembler (seqas) by [Zelda Reverse Engineering Team](https://zelda64.dev/) <br>
[z64audio](https://github.com/z64tools/z64audio) by [rankaisija](https://github.com/rankaisija64) <br>

# Development

## Cloning the repo

```bash
git clone --recurse-submodules https://github.com/z64utils/z64rom.git
```

## Compiling for Win32

To compile for Win32, you need MXE running in WSL or native Linux: https://mxe.cc/

When building MXE, run this command:

```bash
make gcc glib
```

Now you can build for Windows by running:

```
make win32 -j
```

You can then find the Win32 build in `app_win32/z64rom.exe`

## Compiling for Linux

Make sure you have the dependencies:

```bash
sudo apt install build-essential git gcc g++ glib2.0-dev libelf-dev libcurl4-gnutls-dev
```

Now you can build for Linux by running:

```
make linux -j
```

This will output a Linux build into `app_linux/z64rom`
