#!/bin/bash
#Install/update emulator packages

flatpak --if-not-exists remote-add GBA GBA
flatpak --if-not-exists remote-add GBC GBC
flatpak --if-not-exists remote-add MD MD
flatpak --if-not-exists remote-add NEO NEO
flatpak --if-not-exists remote-add NES NES
flatpak --if-not-exists remote-add NGP NGP
flatpak --if-not-exists remote-add Snes9x Snes9x

flatpak install --or-update GBA org.ExPlusAlpha.GBAemu
flatpak install --or-update GBC org.ExPlusAlpha.GBCemu
flatpak install --or-update MD org.ExPlusAlpha.MDemu
flatpak install --or-update NEO org.ExPlusAlpha.NEOemu
flatpak install --or-update NES org.ExPlusAlpha.NESemu
flatpak install --or-update NGP org.ExPlusAlpha.NGPemu
flatpak install --or-update Snes9x org.ExPlusAlpha.Snes9x
