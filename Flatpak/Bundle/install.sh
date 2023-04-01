#!/bin/bash
#Install/update emulator packages

flatpak --if-not-exists --no-gpg-verify --user remote-add GBA ./Emu/GBA
flatpak --if-not-exists --no-gpg-verify --user remote-add GBC ./Emu/GBC
flatpak --if-not-exists --no-gpg-verify --user remote-add MD ./Emu/MD
flatpak --if-not-exists --no-gpg-verify --user remote-add NEO ./Emu/NEO
flatpak --if-not-exists --no-gpg-verify --user remote-add NES ./Emu/NES
flatpak --if-not-exists --no-gpg-verify --user remote-add NGP ./Emu/NGP
flatpak --if-not-exists --no-gpg-verify --user remote-add Snes9x ./Emu/Snes9x

flatpak install --or-update GBA org.ExPlusAlpha.GBAemu
flatpak install --or-update GBC org.ExPlusAlpha.GBCemu
flatpak install --or-update MD org.ExPlusAlpha.MDemu
flatpak install --or-update NEO org.ExPlusAlpha.NEOemu
flatpak install --or-update NES org.ExPlusAlpha.NESemu
flatpak install --or-update NGP org.ExPlusAlpha.NGPemu
flatpak install --or-update Snes9x org.ExPlusAlpha.Snes9x

if [ ! -d /home/deck/.local/share/icons/exemu ]; then mkdir /home/deck/.local/share/icons/exemu; fi
cp ./Icons/* /home/deck/.local/share/icons/exemu

if [ ! -d /home/deck/.local/share/applications ]; then mkdir /home/deck/.local/share/applications; fi
cp ./Entries/* /home/deck/.local/share/applications
