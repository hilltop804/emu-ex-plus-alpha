#!/bin/bash
#Install/update emulator packages

flatpak --if-not-exists --no-gpg-verify --user remote-add GBA ./GBA
flatpak --if-not-exists --no-gpg-verify --user remote-add GBC ./GBC
flatpak --if-not-exists --no-gpg-verify --user remote-add MD ./MD
flatpak --if-not-exists --no-gpg-verify --user remote-add NEO ./NEO
flatpak --if-not-exists --no-gpg-verify --user remote-add NES ./NES
flatpak --if-not-exists --no-gpg-verify --user remote-add NGP ./NGP
flatpak --if-not-exists --no-gpg-verify --user remote-add Snes9x ./Snes9x

flatpak install -y --or-update GBA org.ExPlusAlpha.GBAemu
flatpak install -y --or-update GBC org.ExPlusAlpha.GBCemu
flatpak install -y --or-update MD org.ExPlusAlpha.MDemu
flatpak install -y --or-update NEO org.ExPlusAlpha.NEOemu
flatpak install -y --or-update NES org.ExPlusAlpha.NESemu
flatpak install -y --or-update NGP org.ExPlusAlpha.NGPemu
flatpak install -y --or-update Snes9x org.ExPlusAlpha.Snes9x
