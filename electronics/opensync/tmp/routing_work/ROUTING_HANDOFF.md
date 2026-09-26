# OpenSync routing handoff

Saved project: `opensync.kicad_pcb` in this KiCad project directory.

## Final verification

- Native KiCad DRC, including all track errors, zone refill, and schematic parity: **0 errors, 0 unconnected items, 0 schematic parity issues**.
- **63 pre-existing silkscreen/text warnings remain**: 29 overlaps, 25 silk-over-copper, 4 silk-to-edge, and 5 text-height warnings. Their item identities match the original board's warnings; they were not excluded or downgraded.
- Independently compared all 678 schematic-assigned PCB pads against the updated hierarchical schematic netlist: no mismatches.
- Preserved all 221 footprints' pad geometry, pad numbering, drills, layers, shapes, relative pad positions, component values, fields, and 3D models. The four USB pad-net updates below are the only electrical pad-assignment changes.
- Board outline and MCU position unchanged. Existing non-USB copper net assignments, widths, and layers preserved.
- ERC is unchanged: one pre-existing D14 library-symbol mismatch warning, no new ERC findings.

This completes the requested copper routing, not fabrication sign-off. The remaining silkscreen warnings still require review. No field-solver certification, electrical simulation, or hardware validation was performed.

## Completed work

- Replicated the channel-5 local VOUT routing into the other seven output channels, respecting the differing selector/driver placement offsets. Preserved the existing output signal paths.
- Finished power mux, 5 V converter, 3.3 V converter, MCU supply/feedback/USB, input supply, boot/reset, all eleven LED signals, and run/stop connections.
- Added compact regulator power/ground pours, direct bypass connections, and local ground returns. Connected the crystal ground island and corrected the existing unassigned MCU LX zone net.
- Kept LED routing away from the projected MCU switching-node/inductor region. Added 19 ground-return stitching vias after checking existing power-plane boundaries.
- Removed six existing open-ended trace stubs and one redundant LED_G breakout via; pruned two temporary LED-routing hairpins. All original data is recoverable from the baseline board below.
- Fixed the two channel-5 paired-via spacings using 0.1 mm translations and connected their top-layer copper; no via drill/diameter changes.
- Applied **1.5 mm copper-to-edge clearance at front/rear**, with **3 mm four-layer keepouts at left/right enclosure guides**. LED row position preserved.
- Adjusted the broad 3.3 V zone's clearance to 0.20 mm and its thermal gap/spoke width to 0.20/0.25 mm. Used a bounded local solid connection for MCU C1. Removed isolated copper during fills.

The only whole-footprint transformations were R79 and R65 rotated 180 degrees, and L2 translated 0.7 mm left to (48.6, 81.3) mm. Reference label placement was preserved. No footprint was deformed or rebuilt.

## Authorized USB schematic change

Only USB-interface connectivity around L4/D14 was edited in `opensync_interface_usb.kicad_sch`. All schematic symbol properties and placements were preserved. The equivalent choke/ESD paths are swapped; connector polarity is not reversed:

| Signal | Final path |
| --- | --- |
| USB D+ | J13.3 → D14.3 → L4.2–1 → R10 → U1.67 |
| USB D− | J13.2 → D14.4 → L4.3–4 → R11 → U1.66 |

Corresponding PCB net assignments changed on J13 pads 2/3 and L4 pads 1/4, plus one existing J13 stub. The USB data pair stays on F.Cu with no signal transition vias. No other schematic circuits were edited.

## Backups and evidence

All paths below are relative to the KiCad project directory:

- Original board: `tmp/routing_work/baseline.kicad_pcb`
- Original project settings: `tmp/routing_work/baseline.kicad_pro`
- Original USB sheet: `tmp/routing_digital_review/opensync_interface_usb.before_channel_swap.kicad_sch`
- Final DRC: `tmp/routing_work/final-drc.json`
- Geometry/metadata audit: `tmp/routing_work/invariants.json`
- Independent pin-net audit: `tmp/routing_work/netlist-check.json`
- Before/after schematic netlists and ERC: `tmp/routing_digital_review/`
- Routing preview: `tmp/routing_work/final-preview.png`

All implementation scripts, scratch boards, reports, and backups are contained inside this KiCad project. They are diagnostic artifacts; the delivered board is the main `opensync.kicad_pcb`.
