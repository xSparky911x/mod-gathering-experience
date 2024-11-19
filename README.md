# Gathering Experience Module

## Description
The Gathering Experience Module is a custom addition for AzerothCore that enhances the gameplay experience by rewarding players with experience points (XP) for gathering professions: Mining, Herbalism, Skinning, and Fishing. This module aims to make leveling through gathering more engaging and rewarding.

## Features

- Grants XP for gathering items from Mining, Herbalism, Skinning, and Fishing.
- Individual profession toggles that persist through server restarts
- Scales XP rewards based on player level, current skill level, and current zone.
- Implements diminishing returns to balance XP gains and prevent power leveling from low level characters in high level areas.
- Includes zone-based XP multipliers for fishing to encourage exploration.
- Configurable enable/disable option and announcement on player login.

## Installation

1. Clone this module into the `modules` directory of your AzerothCore source.
2. Re-build AzerothCore.

## Configuration

The module can be configured through the `GatheringExperience.conf` file:

- `GatheringExperience.Enable`: Enable or disable the module (default: enabled).
- `GatheringExperience.Announce`: Toggle login announcement (default: enabled).
- `GatheringExperience.Mining.Enable`: Enable or disable Mining XP (default: enabled).
- `GatheringExperience.Herbalism.Enable`: Enable or disable Herbalism XP (default: enabled).
- `GatheringExperience.Skinning.Enable`: Enable or disable Skinning XP (default: enabled).
- `GatheringExperience.Fishing.Enable`: Enable or disable Fishing XP (default: enabled).

## Usage

Once installed and enabled, the module works automatically. Players will receive XP when they gather items from supported professions. The basexp is stored in the database and can be adjusted on the fly. No need to recompile the server everytime. Once the appropriate command is used, the new value will be saved to the database and reloaded to memory making the changes live immediately

### Commands

All commands require GM level 2 access:

- `.gathering version`: Displays the current version of the module
- `.gathering reload`: Reloads all gathering data from the database
- `.gathering status`: Shows the current enabled/disabled state of each profession
- `.gathering toggle <profession>`: Toggles XP gains for the specified profession (mining, herbalism, skinning, fishing)
- `.gathering list [profession]`: Lists all gathering items for a specific profession
- `.gathering add <itemId> <baseXP> <reqSkill> <profession> <name>`: Adds a new gathering item
- `.gathering remove <itemId>`: Removes a gathering item
- `.gathering modify <itemId> <field> <value>`: Modifies an existing gathering item
  - Valid fields: basexp, reqskill, profession, name
  - For profession: Mining, Herbalism, Skinning, Fishing
  - For name: The name of the item to add has to be in quotes

- `.gathering zone <action> <zoneId> <multiplier>`: Manages zone multipliers
  - Valid actions: add, modify, remove
- `.gathering zone list`: Lists current zone multipliers
- `.gathering zone list zones`: Lists all available zones
- `.gathering currentzone`: Shows current zone information and its experience multiplier

Example commands:
- `.gathering toggle mining`: Toggles Mining XP on/off
- `.gathering toggle herbalism`: Toggles Herbalism XP on/off
- `.gathering toggle skinning`: Toggles Skinning XP on/off
- `.gathering toggle fishing`: Toggles Fishing XP on/off
- `.gathering status`: Shows current state of all professions
- `.gathering add 2447 360 1 Herbalism "Peacebloom"`
- `.gathering remove 2447`
- `.gathering modify 2447 basexp 360`
- `.gathering modify 2447 reqskill 1`
- `.gathering modify 2447 profession Herbalism`
- `.gathering modify 2447 name "Peacebloom"`
- `.gathering list Herbalism`
- `.gathering zone add 1 1.5`
- `.gathering zone modify 1 2.0`
- `.gathering zone remove 1`

## Credits

This module was created by xSparky911x and Thaxtin for AzerothCore.

## License

This module is released under the [GNU Affero General Public License v3.0](https://www.gnu.org/licenses/agpl-3.0.en.html).
