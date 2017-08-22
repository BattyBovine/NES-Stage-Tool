# NES Stage Tool

**NES Stage Tool** allows homebrew NES developers to create stage layouts for their games using 16x16 pixel metatiles on 8x4 screens. Eventually it may be expanded to support more options, such as larger metatiles and differing screen layouts.



Using The Software
------------------

Tilesets can be loaded with the *File > Open CHR* option, or by dragging and dropping your CHR file onto the tileset window on the Tilesets tab. You can choose which banks to load by selecting the size of each bank and adjusting the bank numbers. Up to 7 tilesets are supported for each stage.

Metatiles and palettes are defined on the Metatiles tab. The palette colours can be changed by selecting a colour in one of the subpalettes at the bottom right, then choosing a new colour from the large colour selector below. The subpalette that contains the selection cursor is the subpalette that will be applied when you edit a metatile. Select a tileset on the upper right by using the scroll wheel, and select an 8x8 tile with the left mouse button. Place a tile in the metatile editor on the left with the left mouse button. The selected subpalette is automatically applied to a metatile while placing tiles, but you can also apply the selected subpalette with the right mouse button without otherwise changing the metatile.

Once you've defined a set of metatiles, you can place them in the stage editor on the *Stage* tab. Use the scroll wheel on the metatile selector on the right to choose the tileset you wish to use, then select a metatile with the left mouse button. Use the left mouse button on the stage editor to place a metatile in the stage, and the right mouse button to apply the current tileset to the selected screen. You can click and drag the left mouse button on the stage editor to paint a large area of tiles simultaneously. You can also define the label prefix for the output data in the *Label* textbox above the editor canvas. See the data format definition below for more info. You can navigate around the stage by clicking and holding the middle mouse button, or zoom into the canvas by scrolling the scroll wheel. Double-clicking the middle mouse button will reset the zoom to the default zoom level.

After editing the stage, you'll probably want to set up checkpoints and add some objects to the stage. You can do that by clicking the *Objects* tab on the right to switch to the Object editor. Double-clicking on the name of an object will allow you to edit its label, and double-clicking the blank image next to it allows you to add an image representing its sprite. After setting up an object, you can simply drag it to the stage editor to add it to the stage, and drag it around to where you want it to spawn in the game. Pressing the *Delete* key will remove it from the stage and free up a slot for another object. The current number of objects is capped at 64. Similarly, you can add a checkpoint by dragging it from the Checkpoint list into the stage editor and moving it to where the checkpoint should go. Up to seven checkpoints per stage can be added. Since there can only be one of each checkpoint, dragging a duplicate checkpoint number will replace the matching one if it already exists in the stage. Objects and checkpoints will only be editable or visible when the *Objects* tab is selected; to go back to editing the stage layout, click back onto the *Metatiles* tab.

You can save a stage file in ASM format using *File > Save Stage*. You can later reload this file by using *File > Open Stage*, or by dragging and dropping the saved file onto the stage editor canvas.



Data Format
-----------

The ASM format exported from the program is as follows:

### Screen tiles
Tiles are stored as a single stream of bytes per screen, from upper-left corner to lower right corner. The tiles are stored in columns, so that the metatile data can be easily retrieved and stored using the NES PPU's 32+ mode. The screens are arranged from left to right, then from top to bottom, such that the upper left screen is screen 0, the one to its right is screen 1, etc., and the last screen is screen 31 (0x1F) at the bottom right of the stage editor. Each screen has its own label based on the label name entered above the stage editor; for example, giving it the name *examplestage1* will make the first screen's label *examplestage1_metatiles_00*, the second screen's label *examplestage1_metatiles_01*, etc. The label numbers are in hexadecimal format, so screen 10 will be labeled *examplestage1_metatiles_0A*.

### Screen tilesets
Below the screen's metatile definitions is a compressed array of bytes representing the tileset data for each screen. Screens 0 and 1 are in byte 0, screens 2 and 3 are in byte 1, etc. The label is *examplestage1_tilesets*. The format looks like this:

```
0111 0111 - Byte for screen tilesets 0 and 1
 |||  |||
 |||  +++-- Screen 0's tileset
 +++------- Screen 1's tileset
```

### Screen properties
Below the tileset data is an array of bytes representing the other properties associated with each screen, namely the associated music, and whether the screen should scroll into the next screen as the player moves toward it. Each byte is associated with one screen, and there are as many bytes as there are screens. The label is *examplestage1_screenprops*. The format looks like this:

```
1000 1011 - Property byte for screen 0
|||| ||||
||++-++++-- Screen 0 music
|+--------- Screen 0 scroll block left
+---------- Screen 0 scroll block right

Note: "scroll block" refers to whether the screen scrolls continuously into the next screen, or if it should lock the camera into the current screen. When a scroll block is 1, the scrolling is blocked in that direction until the player crosses that boundary and the screen is transitioned into the next area.
```

### Metatile definitions
Metatiles are stored with each subtile in its own array. Top left is first, then top right, then bottom left, then bottom right. The labels for each are *examplestage1_subtiles_tl*, *examplestage1_subtiles_tr*, *examplestage1_subtiles_bl* and *examplestage1_subtiles_br*.

### Metatile subpalettes
Each metatile has its subpalette defined in this array, labelled *examplestage1_mtpalettes*. Each byte is compressed and contains the palettes for a group of four metatiles, like so:

```
0111 0100 - Subpalette byte for metatiles 0-3
|||| ||||
|||| ||++-- Metatile 0 subpalette
|||| ++---- Metatile 1 subpalette
||++------- Metatile 2 subpalette
++--------- Metatile 3 subpalette
```

### Metatile properties
Each metatile has a configurable "collision" value that affects how the player should interact with it. The value is a simple number, defined by the game developer. This can be interpreted in any way the developer finds most convenient but the simplest, and probably most sensible, solution is to use it as an entry point into a jump table, which then performs a particular action. The array is labelled *examplestage1_mtprops*, and each byte in the array represents one metatile.

### Checkpoints
Checkpoints can be used to start the player later in the level once they've passed a certain area. The checkpoint data is split into three separate arrays representing the screen, X position, and Y position. They are labelled *examplestage1_checkpoint_screen*, *examplestage1_checkpoint_posx*, and *examplestage1_checkpoint_posy*, respectively. Eight checkpoints are currently supported, with the first one intented to be the starting point for the level. Each of these arrays is eight bytes long, each byte representing the corresponding value. In other words, to load the player at checkpoint 2, read the second value in each array, and use the *screen* value to load the stage at that screen, and the *posx* and *posy* values to load the player at that pixel location within the screen.

### Objects
Objects are the interactive parts of a stage, such as enemies, items, destructible obstacles, moving platforms, etc. The object data is split into four separate arrays representing the object identifier, screen, X position, and Y position. They are labelled *examplestage1_object_id*, *examplestage1_object_screen*, *examplestage1_object_posx*, and *examplestage1_object_posy*, respectively. Up to 64 objects can be added to a single stage. Each of the object arrays is 64 bytes long, each byte representing the corresponding object's value. In other words, to load an object into the current screen, check which objects are meant to be loaded on the screen using the *screen* array, then add the object's *posx* value to the current camera position, and if the object is within camera boundaries, it can be spawned at the given *posx* and *posy* positions, using the *id* value to determine which object is meant to be loaded.

### Palette definitions
Each tileset's palette is defined here, with each byte in each array representing a colour from the NES's global palette. They are labelled *examplestage1_palette_00* for the first palette, *examplestage1_palette_01* for the second, etc. The bytes can be copied as-is from the array into the PPU.

### Tileset definitions
Tilesets are defined here, with each byte representing a bank number to swap to in the nametable memory. The length of the array is the same as the number of swappable banks (two for 2K, four for 1K, etc.). Arrays are labelled *examplestage1_tileset_00* for the first tileset, *examplestage1_tileset_01* for the second, etc.



Data Offsets
------------

For programming convenience, you can simply refer to a specific address or offset in a compiled binary of the stage data, rather than hard-coding label names into your interpreter. The offsets for a compiled binary stage file are as follows:

```
Start              = $0000

ScreenTiles        = $1800
ScreenProperties   = $1810

TilesTopLeft       = $1830
TilesBottomLeft    = $1930
TilesTopRight      = $1A30
TilesBottomRight   = $1B30
MetatilePalettes   = $1C30
MetatileProperties = $1C70

CheckpointScreen   = $1D70
CheckpointPosX     = $1D78
CheckpointPosY     = $1D80

ObjectId           = $1D88
ObjectScreen       = $1DC8
ObjectPosX         = $1E08
ObjectPosY         = $1F48

StagePalettes      = $1E88
StageTilesets      = $1F08
```
