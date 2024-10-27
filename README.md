# Gathering Experience Module

## Description
The Gathering Experience Module is a custom addition for AzerothCore that enhances the gameplay experience by rewarding players with experience points (XP) for gathering professions: Mining, Herbalism, Skinning, and Fishing. This module aims to make leveling through gathering more engaging and rewarding.

## Features

- Grants XP for gathering items from Mining, Herbalism, Skinning, and Fishing.
- Scales XP rewards based on player level, current skill level, and item rarity.
- Implements diminishing returns to balance XP gains.
- Provides extra XP bonuses for rare and super rare items.
- Includes zone-based XP multipliers for fishing to encourage exploration.
- Configurable enable/disable option and player announcement on login.

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

- `.gathering version`: Displays the current version of the Gathering Experience module.

## Customization

The module includes predefined XP values and required skill levels for various gathering items. These can be adjusted in the `GatheringExperienceModule` class constructor if needed.

## Credits

This module was created by Thaxtin for AzerothCore.

## License

This module is released under the [GNU Affero General Public License v3.0](https://www.gnu.org/licenses/agpl-3.0.en.html).
