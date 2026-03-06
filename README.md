# dox11cmd
    
!['dox11cmd'](https://github.com/markcapella/dox11cmd/blob/main/screenshot.png)

## Description

    Command line tool to perform X11 Window actions.
    
        list, raise, lower, map, & unmap.


## Installation

### Install Pre-reqs.

For Debian systems:

    sudo apt install git build-essential libglib2.0-dev libgtk-3-dev gettext automake libx11-dev libxft-dev libxpm-dev libxt-dev libxext-dev x11proto-dev libxinerama-dev libxtst-dev libxkbcommon-dev libgsl-dev appmenu-gtk3-module libncurses-dev

For Fedora systems:

    sudo dnf install git gcc gcc-c++ make glib2-devel gtk3-devel gdk-pixbuf2-modules-extra gettext automake libX11-devel libXft-devel libXpm-devel libXt-devel libXext-devel xorg-x11-proto-devel libXinerama-devel libXtst-devel libxkbcommon-devel gsl-devel unity-gtk3-module ncurses-devel

### Clone dox11cmd source folder.

    git clone https://github.com/markcapella/dox11cmd

### CD into source repo.

    cd dox11cmd


## Basic development.

    make
    make run

    sudo make install
    (test use)
    sudo make uninstall

    make clean


## Usage.

### Syntax.
    dox11cmd commmand windowName

    dox11cmd
    dox11cmd list
    
    dox11cmd raise bash
    dox11cmd lower Calculator
    
    dox11cmd map bash
    dox11cmd unmap Calc


## markcapella@twcny.rr.com Rocks !
    Yeah I do.
    