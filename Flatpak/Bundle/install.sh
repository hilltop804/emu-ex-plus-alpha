#!/bin/bash
#Install/update emulator packages

flatpak --if-not-exists --no-gpg-verify --user remote-add EMU ./Emu

flatpak install -y --or-update EMU org.ExPlusAlpha.GBAemu
flatpak install -y --or-update EMU org.ExPlusAlpha.GBCemu
flatpak install -y --or-update EMU org.ExPlusAlpha.MDemu
flatpak install -y --or-update EMU org.ExPlusAlpha.NEOemu
flatpak install -y --or-update EMU org.ExPlusAlpha.NESemu
flatpak install -y --or-update EMU org.ExPlusAlpha.NGPemu
flatpak install -y --or-update EMU org.ExPlusAlpha.Snes9x

if [ ! -d /home/deck/.local/share/icons/exemu ]; then mkdir /home/deck/.local/share/icons/exemu; fi
cp ./Icons/* /home/deck/.local/share/icons/exemu

if [ ! -d /home/deck/.local/share/applications ]; then mkdir /home/deck/.local/share/applications; fi
cp ./Entries/* /home/deck/.local/share/applications
