# Editable visual resources

Run `python source/asset_tools/regenerate_meshes.py` from the mod folder.
This writes only to `build/asset-preview`, never the live asset paths.
The default outputs match the three supplied visual meshes.
Only the Python standard library is required.

The rock tool reads the adapted original Earth Spike mesh already in the
mod's assets directory, then adds deterministic branches. The wind tool reads
`source/asset_sources/whirlwind-before-fire.mesh` and changes its height.
These original source meshes remain credited to juubi23; GPL licensing of
the tools does not change ownership of the original models.

The three generated-image originals are in `source/asset_sources/`:
fire.png, ember.png and earth-rock-v2.png. The runtime uses smaller DDS copies;
these PNGs are supplied as editable source rather than loaded by the game.
Runtime material/shader/particle scripts are already editable text in the
normal resource directories. AEV_FireNeutralNormal.dds is a constant flat
normal texture; the original Arcane normal map is separately credited.

The generator scripts use Ogre's native mesh format and the same coordinate
conversions as the reviewed release. They do not change spell logic.
