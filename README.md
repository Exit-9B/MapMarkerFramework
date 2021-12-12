# Custom Map Marker Framework
This SKSE plugin provides a framework for mods to add new map marker icons without being subject to
the limitations that normally apply when adding markers without SKSE. This means:
- No conflicts with mods that edit the UI
- No conflicts between mods that add markers
- Customizable discovery music
- Far more than 256 icons

The canonical implementation of this framework, which it was created for, is
[CoMAP - Common Marker Addon Project](https://www.nexusmods.com/skyrimspecialedition/mods/56123) by
Jelidity.

## Usage
CMMF reads config files from the `Data\MapMarkers\` directory in order to apply its changes. Each
active plugin in your load order can have a config file associated with it, using the same stem
filename and a .json extension. For example, the config file corresponding to the Skyrim.esm plugin
is `Data\MapMarkers\Skyrim.json`. Conflicts between these config files are resolved based on the
plugin load order, just like plugin contents, .bsa files, and .ini files.

### Adding New Markers
In order to add new icons, you first need to create a .swf file that exports 2 sprites, one for
the discovered marker and one for the undiscovered marker. See the
[CoMAP Dev Kit Guide](https://docs.google.com/document/d/1MNDihv3ew4MoghlMQNZhYmYi12yHz0_ZMUJQ2w_uOUE/edit)
for information on how to create the icons and .swf files.

Here is an example of what a config that assigns a new marker to Riverwood could look like:
```jsonc
{
  "iconDefinitions": [
    {
      "name": "RiverwoodMarker",
      "source": {
        "path": "resources\\mymarkerart.swf",
        "exportNames": ["RiverwoodMarker", "RiverwoodMarkerUndiscovered"]
      },
      "scale": 1.333333,
      "discoveryMusic": "MUSDiscoveryTown"
    }
  ],
  "mapMarkers": [
    {
      "refID": "Skyrim.esm|162A4",
      "iconName": "RiverwoodMarker"
    }
  ]
}
```
The `name` property is a name that you provide for each icon so that it can be assigned to via the
`iconName` property. It is recommended to add a mod-specific prefix to your names to avoid
conflicts, just like with editor IDs, scripts, and asset paths.

The `source` property contains information about the .swf file where markers should be read from.
It is recommended to use the `resources\` subdirectory for your resources for organizational
purposes. The export names are defined within the .swf file. You should provide exactly 2 names,
corresponding to the discovered and undiscovered marker.

The `scale` property configures how large the icon should appear on the map. Recommended values are
`1.333333` for towns and cities, and `0.666667` for minor locations.

The `discoveryMusic` property configures the music that plays when the player first discovers the
location. This property accepts the Editor ID of a Music Type form. The tracks provided by the
vanilla game are `MUSDiscoveryCity`, `MUSDiscoveryTown`, `MUSDiscoveryDungeon`, and
`MUSDiscoveryGeneric`.

The `hideFromHUD` property can also be added and set to `true` for markers that should not appear on
the HUD compass. This is useful for icons such as arrows which would not make sense in the HUD.

### Editing Existing Markers
It is also possible to make some edits to existing markers within the config.

Here is an example of some possible edits:
```jsonc
{
  "iconDefinitions": [
    {
      "index": 15, // Mine
      "discoveryMusic": "MUSDiscoveryGeneric"
    },
    {
      "index": 57, // DLC02ToSkyrim
      "hideFromHUD": true
    },
    {
      "index": 58, // DLC02ToSolstheim
      "hideFromHUD": true
    },
  ]
}
```
The `index` property identifies the icon by index. These can be found by decompiling the relevant
.swf files or in the
[CommonLibSSE source code](https://github.com/Ryan-rsm-McKenzie/CommonLibSSE/blob/master/include/RE/E/ExtraMapMarker.h).

The `discoveryMusic` and `hideFromHUD` properties are the same as for new markers. Note that the
`scale` property is NOT supported when editing existing icons.

### Local Map Markers
You can customize the markers that appear for doors on the local map.

Here is an example of a possible local map config:
```jsonc
{
  "mapMarkers": [
    {
      "locType": "LocTypeInn",
      "iconName": "InnMarker"
    },
    {
      "vendorList": "Skyrim.esm|937A0", // VendorItemsTailor
      "iconName": "TailorMarker"
    },
    {
      "location": "Skyrim.esm|2263B", // Riften Fishery
      "iconName": "FishMarker"
    }
  ]
}
```
The `location` property specifies a specific location to assign an icon to.

The `vendorList` property specifies a vendor's buy/sell list. The specified icon will be used for
any location containing a vendor that uses that list.

The `locType` property specifies a location keyword, by name. The specified icon will be used for
any location that uses that keyword.

Each of the above properties take priority from least to most specific. `location` has the highest
priority, followed by `vendorList`, and then `locType`.

## Build Instructions
### Requirements
* [CMake](https://cmake.org/)
	* Add this to your `PATH`
* [PowerShell](https://github.com/PowerShell/PowerShell/releases/latest)
* [Vcpkg](https://github.com/microsoft/vcpkg)
	* Add the environment variable `VCPKG_ROOT` with the value as the path to the folder containing vcpkg
* [Visual Studio Community 2019](https://visualstudio.microsoft.com/)
	* Desktop development with C++

### Register Visual Studio as a Generator
* Open `x64 Native Tools Command Prompt`
* Run `cmake`
* Close the cmd window

### Building
```
git clone https://github.com/Exit-9B/MapMarkerFramework
cd MapMarkerFramework
git submodule update --init --recursive
cmake -B build -S .
```
