SpectralShift (Linux) Installation
==================================

Tested on Ubuntu 24.04. Run the installer from the extracted release folder that contains `CLAP/`, `VST3/`, and `Standalone/`.

Quick install (user)
--------------------
```bash
# the .sh files are executable by default, but you may need to run chmod anyway
# you may need to run these as sudo.
chmod +x install.sh 
./install.sh -y
```
- Installs VST3 to `~/.vst3`, CLAP to `~/.clap`, LV2 to `~/.lv2`, standalone to `~/.local/bin`.
- Use `--dry-run` to see actions without copying.

System-wide install
-------------------
```bash
sudo ./install.sh --system -y
```
- Installs to `/usr/lib/vst3`, `/usr/lib/clap`, `/usr/lib/lv2, `/usr/local/bin`.
- Prompts for `sudo` if not run as root.

Options
-------
- `--user` (default) or `--system`
- `--bin-dir DIR` override standalone destination (respects `BIN_DIR` env)
- `VST3_DIR`, `CLAP_DIR`, `LV2_DIR` env vars override user-mode plugin destinations
- `-y` replace existing files without prompting
- `--dry-run` print actions only

Manual install (if you prefer)
------------------------------
Copy these from the release folder:
- `VST3/SpectralShift.vst3` → `~/.vst3/` (or `/usr/lib/vst3/`)
- `CLAP/SpectralShift.clap` → `~/.clap/` (or `/usr/lib/clap/`)
- `Standalone/SpectralShift` → somewhere on `PATH` (e.g. `~/.local/bin/`), ensure executable (`chmod +x`).

After installing
----------------
- Re-scan plugins in your host (e.g. REAPER: Preferences → Plug-ins → VST → Re-scan).
- CLAP users: ensure the host points to `~/.clap` (REAPER does this already).
- Run the standalone from a terminal with `SpectralShift` to verify it launches.

Uninstall 
---------
```bash
# Again, this is executable by default, but you may need to run chmod anyway, and sudo if necessary.
chmod +x uninstall.sh
./uninstall.sh
```
