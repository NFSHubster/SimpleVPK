# SimpleVPK

SimpleVPK is a homebrew app based on [easyVPK](https://github.com/Electric1447/EasyVPK) that's designed for the PlayStation®Vita that allows users to easily download & install VPKs & Data Files from [VitaDB](https://www.rinnegatamante.eu/vitadb/#/)

# Current Goals in Progress 

- Utilize easyVPK's base to build a data files downloader for [PS Vita Data Files Repository](https://www.vita.unaux.com)
- Use PSARCS format whenever possible 


# Changes From easyVPK

- Updated VitaDB's URL to make the app usable again (from rinnegatamante.it/vitadb to rinnegatamante.eu/vitadb)

- Updated CMakeLists.txt & updated it to use the new VitaSDK

- Changed LiveArea Assets

# Building

- Prerequisites:
  [VitaSDK](https://vitasdk.org/)

- Compiling:

```sh
mkdir build
cd build

cmake ..
make
```


# Credit & Legal

- [Electric1447](https://github.com/Electric1447) For creating the original source code of easyVPK.

- trademarked69/Desoxyn on Discord for patching critical errors & helping aith the troubleshooting process

- The PS Vita Homebrew Community for their continued support.
