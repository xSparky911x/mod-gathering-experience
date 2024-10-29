# Gathering Experience Module

## Description
The Gathering Experience Module is a custom addition for AzerothCore that enhances the gameplay experience by rewarding players with experience points (XP) for gathering professions: Mining, Herbalism, Skinning, and Fishing. This module aims to make leveling through gathering more engaging and rewarding.

## Features

- Grants XP for gathering items from Mining, Herbalism, Skinning, and Fishing.
- Scales XP rewards based on player level, current skill level, and item rarity.
- Implements diminishing returns to balance XP gains.
- Provides extra XP bonuses for rare and super rare items.
- Includes zone-based XP multipliers for fishing to encourage exploration.
- Configurable enable/disable option and announcement on player login.

## Installation

1. Clone this module into the `modules` directory of your AzerothCore source.
2. Re-run cmake and rebuild AzerothCore.

## Configuration

The module can be configured through the `GatheringExperience.conf` file:

- `GatheringExperience.Enable`: Enable or disable the module (default: enabled).
- `GatheringExperience.Announce`: Toggle login announcement (default: enabled).

## Usage

Once installed and enabled, the module works automatically. Players will receive XP when they gather items from supported professions.

### Commands

All commands require GM level access:

- `.gathering version`: Displays the current version of the module
- `.gathering reload`: Reloads all gathering data from the database
- `.gathering list [profession]`: Lists all gathering items for a specific profession
- `.gathering add <itemId> <baseXP> <reqSkill> <profession> <name>`: Adds a new gathering item
- `.gathering remove <itemId>`: Removes a gathering item
- `.gathering modify <itemId> <field> <value>`: Modifies an existing gathering item
  - Valid fields: basexp, reqskill, profession, name
  - For profession: Mining, Herbalism, Skinning, Fishing
  - For name: The name of the item to add in quotes
- `.gathering zone <action> <zoneId> <multiplier>`: Manages zone multipliers
  - Valid actions: add, modify, remove
- `.gathering zone list`: Lists current zone multipliers
- `.gathering zone list zones`: Lists all available zones

Example commands:
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

This module was created by Thaxtin for AzerothCore.

## License

This module is released under the [GNU Affero General Public License v3.0](https://www.gnu.org/licenses/agpl-3.0.en.html).
