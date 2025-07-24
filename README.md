<img src="https://github.com/PoliEcho/pupes_releases_tracker/raw/refs/heads/master/resources/img/icon.svg" alt="Project logo" width="128"></img> [![Build Application](https://github.com/PoliEcho/pupes_releases_tracker/actions/workflows/build.yml/badge.svg)](https://github.com/PoliEcho/pupes_releases_tracker/actions/workflows/build.yml)
# Pupes releases tracker
#### minimal gtk4(gtkmm4) app writen in C++ for tracking when your favorite stuff comes out  
> [!IMPORTANT]  
> this app works only on linux thanks to libappindicator3  
> i did not try but i think it will not work on wsl  

### Demo  
<details>
<summary>Demo</summary>

<video src="https://github.com/user-attachments/assets/97d539da-26fd-4219-a0e4-523c2bb5b370" controls></video>

</details>

## Goals
> minimal simple to use UI  
> small memory and cpu footprint  
> ease of use  
## Roadmap
> - [x] design UI   
> - [x] data input posible   
> - [x] automatic update of Release In  
> - [x] load data from disk   
> - [x] minimize to systray   
> - [x] notifications   


## Dependencies  
> GTK3  
> GTKmm4  
> libappindicator3
> #### Build only  
> nlohmann-json  
> g++ (C++23 compatable)

### Install Dependencies  
#### Arch Linux  
```Bash
sudo pacman -S --needed gcc gtkmm-4.0 gtk3 libappindicator-gtk3 nlohmann-json
```
#### Ubuntu/Kali(building on kali is broken so download the dependencies and download the release)     
```Bash
sudo apt-get update && \
sudo apt-get install -y build-essential && \
sudo apt-get install -y libgtk-3-dev libgtkmm-4.0-dev libayatana-appindicator3-dev && \
sudo apt-get install -y nlohmann-json3-dev && \
sudo apt-get install -y gcc-13 g++-13 && \
sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-13 100 && \
sudo update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-13 100
```
> [!CAUTION]  
> <span style="color:red;">if the 3. command trows error try libayatana-appindicator3-dev insted of libappindicator3-dev</span>

## Download
[Latest Release](https://github.com/PoliEcho/pupes_releases_tracker/releases)    
## Build  
```Bash
make -j$(nproc)
```

## QaA
> is it just calendar with less features?
######       
well, yes, but not really this can help you separate what you need to do and what you want to do +  
UNIX filozofy says:   
"Write programs that do one thing and do it well." - <cite>Peter H. Salus, A Quarter-Century of Unix</cite>   
