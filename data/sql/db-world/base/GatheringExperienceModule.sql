-- ----------------------------------------
-- Table Creation
-- ----------------------------------------

-- Create profession table first (since it's referenced by gathering_experience)
CREATE TABLE IF NOT EXISTS `gathering_experience_professions` (
    `profession_id` TINYINT UNSIGNED NOT NULL,
    `name` VARCHAR(50) NOT NULL,
    `description` VARCHAR(255),
    PRIMARY KEY (`profession_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;


-- Create settings table
CREATE TABLE IF NOT EXISTS `gathering_experience_settings` (
    `profession` VARCHAR(32) NOT NULL,
    `enabled` TINYINT NOT NULL DEFAULT 1,
    `last_updated` TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    PRIMARY KEY (`profession`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;


-- Create main gathering experience table
CREATE TABLE IF NOT EXISTS `gathering_experience` (
    `item_id` INT UNSIGNED NOT NULL,
    `base_xp` INT UNSIGNED NOT NULL DEFAULT 0,
    `required_skill` INT UNSIGNED NOT NULL DEFAULT 0,
    `profession` TINYINT UNSIGNED NOT NULL,
    `name` VARCHAR(100) NOT NULL DEFAULT '' COMMENT 'Item name for reference',
    PRIMARY KEY (`item_id`),
    FOREIGN KEY (`profession`) REFERENCES `gathering_experience_professions` (`profession_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;


-- Create rarity multiplier table
CREATE TABLE IF NOT EXISTS `gathering_experience_rarity` (
    `item_id` INT UNSIGNED NOT NULL,
    `multiplier` FLOAT NOT NULL DEFAULT 1.0,
    PRIMARY KEY (`item_id`),
    FOREIGN KEY (`item_id`) REFERENCES `gathering_experience` (`item_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;


-- Create fishing zone multiplier table
CREATE TABLE IF NOT EXISTS `gathering_experience_zones` (
    `zone_id` INT UNSIGNED NOT NULL,
    `multiplier` FLOAT NOT NULL DEFAULT 1.0,
    `name` VARCHAR(100) NOT NULL DEFAULT '' COMMENT 'Zone name for reference',
    PRIMARY KEY (`zone_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;


-- ----------------------------------------
-- Initial Data Setup
-- ----------------------------------------

-- Insert professions first (since they're referenced)
INSERT IGNORE INTO `gathering_experience_professions` (profession_id, name, description) VALUES
(1, 'Mining', 'Gathering ore and minerals'),
(2, 'Herbalism', 'Gathering herbs and plants'),
(3, 'Skinning', 'Gathering leather and hides'),
(4, 'Fishing', 'Catching fish and other aquatic items');


-- Insert default enabled/disabled values
INSERT IGNORE INTO `gathering_experience_settings` (`profession`, `enabled`) VALUES
('mining', 1),
('herbalism', 1),
('skinning', 1),
('fishing', 1);


-- ----------------------------------------
-- Mining Data
-- ----------------------------------------

INSERT IGNORE INTO `gathering_experience` (item_id, base_xp, required_skill, profession, name) VALUES
-- Basic Ores
(2770, 50, 1, 1, 'Copper Ore'),
(2771, 100, 65, 1, 'Tin Ore'),
(2772, 200, 125, 1, 'Iron Ore'),
(3858, 400, 175, 1, 'Mithril Ore'),
(10620, 400, 230, 1, 'Thorium Ore'),
(23424, 425, 275, 1, 'Fel Iron Ore'),
(23425, 450, 325, 1, 'Adamantite Ore'),
(36909, 500, 350, 1, 'Cobalt Ore'),
(36912, 550, 400, 1, 'Saronite Ore'),
(36910, 525, 450, 1, 'Titanium Ore'),

-- Precious Metals
(2775, 150, 75, 1, 'Silver Ore'),
(2776, 250, 155, 1, 'Gold Ore'),
(7911, 350, 205, 1, 'Truesilver Ore'),
(23426, 475, 375, 1, 'Khorium Ore'),

-- Gems
(774, 50, 1, 1, 'Malachite'),
(818, 75, 1, 1, 'Tigerseye'),
(1705, 200, 65, 1, 'Lesser Moonstone'),
(1529, 175, 65, 1, 'Jade'),
(1210, 100, 65, 1, 'Shadowgem'),
(1206, 150, 65, 1, 'Moss Agate'),
(3864, 300, 125, 1, 'Citrine'),
(7909, 650, 155, 1, 'Aquamarine'),
(12800, 650, 230, 1, 'Azerothian Diamond'),
(12364, 700, 230, 1, 'Huge Emerald'),
(12363, 750, 230, 1, 'Arcane Crystal'),
(12799, 675, 230, 1, 'Large Opal'),
(12361, 700, 230, 1, 'Blue Sapphire'),
(7910, 625, 230, 1, 'Star Ruby'),
(11754, 600, 300, 1, 'Black Diamond'),

-- Stone
(2836, 50, 65, 1, 'Coarse Stone'),
(2838, 75, 125, 1, 'Heavy Stone'),
(7912, 125, 175, 1, 'Solid Stone'),
(12365, 100, 230, 1, 'Dense Stone'),

-- Special Items
(11370, 375, 230, 1, 'Dark Iron Ore'),
(19774, 675, 255, 1, 'Souldarite'),
(11382, 800, 300, 1, 'Blood of the Mountain'),
(22202, 575, 305, 1, 'Small Obsidian Shard'),
(22203, 625, 305, 1, 'Large Obsidian Shard'),

-- Crystallized Elements
(37705, 400, 350, 1, 'Crystallized Water'),
(37701, 300, 350, 1, 'Crystallized Earth'),
(37703, 350, 400, 1, 'Crystallized Shadow'),
(37702, 325, 450, 1, 'Crystallized Fire');


-- ----------------------------------------
-- Skinning Data
-- ----------------------------------------

INSERT IGNORE INTO `gathering_experience` (item_id, base_xp, required_skill, profession, name) VALUES
-- Basic Leather
(2934, 125, 1, 3, 'Ruined Leather Scraps'),
(2318, 250, 1, 3, 'Light Leather'),
(2319, 300, 100, 3, 'Medium Leather'),
(4234, 325, 150, 3, 'Heavy Leather'),
(4304, 375, 200, 3, 'Thick Leather'),
(8170, 425, 250, 3, 'Rugged Leather'),

-- Hides
(783, 200, 75, 3, 'Light Hide'),
(4232, 275, 125, 3, 'Medium Hide'),
(4235, 350, 175, 3, 'Heavy Hide'),
(8169, 400, 225, 3, 'Thick Hide'),
(8171, 450, 275, 3, 'Rugged Hide'),

-- Special Leather
(15417, 300, 300, 3, 'Devilsaur Leather'),
(21887, 350, 350, 3, 'Knothide Leather'),
(25649, 300, 300, 3, 'Knothide Leather Scraps'),
(33568, 425, 425, 3, 'Borean Leather'),
(33567, 400, 350, 3, 'Borean Leather Scraps'),
(38425, 450, 450, 3, 'Heavy Borean Leather'),
(44128, 475, 475, 3, 'Arctic Fur'),

-- Scales
(15415, 325, 325, 3, 'Blue Dragonscale'),
(15416, 325, 325, 3, 'Black Dragonscale'),
(15414, 350, 350, 3, 'Red Dragonscale'),
(15412, 375, 375, 3, 'Green Dragonscale'),
(25700, 375, 310, 3, 'Fel Scales'),
(6470, 250, 100, 3, 'Deviate Scale'),
(6471, 325, 150, 3, 'Perfect Deviate Scale'),
(5784, 275, 125, 3, 'Slimy Murloc Scale'),
(5785, 400, 225, 3, 'Thick Murloc Scale'),
(7286, 350, 175, 3, 'Black Whelp Scale'),
(7287, 375, 200, 3, 'Red Whelp Scale'),
(8154, 425, 250, 3, 'Scorpid Scale'),
(15408, 300, 300, 3, 'Heavy Scorpid Scale'),
(29539, 400, 400, 3, 'Cobra Scales'),
(29547, 400, 400, 3, 'Wind Scales'),

-- Special Items
(17012, 375, 375, 3, 'Core Leather'),
(25707, 400, 400, 3, 'Fel Hide'),
(15419, 475, 475, 3, 'Pristine Hide of the Beast'),
(15410, 500, 500, 3, 'Scale of Onyxia'),
(15423, 525, 525, 3, 'Brilliant Chromatic Scale');


-- ----------------------------------------
-- Herbalism Data
-- ----------------------------------------

INSERT IGNORE INTO `gathering_experience` (item_id, base_xp, required_skill, profession, name) VALUES
-- Starter Zones & Low Level
(765, 225, 1, 2, 'Silverleaf'),
(2447, 225, 1, 2, 'Peacebloom'),
(2449, 250, 1, 2, 'Earthroot'),

-- Low-Mid Level
(785, 300, 50, 2, 'Mageroyal'),
(2450, 300, 50, 2, 'Briarthorn'),
(2452, 325, 75, 2, 'Swiftthistle'),
(2453, 325, 75, 2, 'Bruiseweed'),
(3355, 350, 100, 2, 'Wild Steelbloom'),
(3356, 350, 100, 2, 'Kingsblood'),

-- Mid Level
(3357, 375, 125, 2, 'Liferoot'),
(3358, 400, 150, 2, 'Khadgar\'s Whisker'),
(3369, 400, 150, 2, 'Grave Moss'),
(3818, 425, 175, 2, 'Fadeleaf'),
(3819, 450, 200, 2, 'Dragon\'s Teeth'),
(3821, 425, 175, 2, 'Goldthorn'),

-- High Level Classic
(8836, 475, 225, 2, 'Arthas\' Tears'),
(8838, 475, 225, 2, 'Sungrass'),
(8839, 475, 225, 2, 'Blindweed'),
(8845, 475, 225, 2, 'Ghost Mushroom'),
(8846, 475, 225, 2, 'Gromsblood'),
(13463, 500, 250, 2, 'Dreamfoil'),
(13464, 500, 250, 2, 'Golden Sansam'),
(13465, 525, 275, 2, 'Mountain Silversage'),
(13466, 525, 275, 2, 'Sorrowmoss'),
(13467, 550, 300, 2, 'Icecap'),
(13468, 550, 300, 2, 'Black Lotus'),

-- Outland Herbs
(22785, 575, 300, 2, 'Felweed'),
(22786, 600, 325, 2, 'Dreaming Glory'),
(22787, 625, 350, 2, 'Ragveil'),
(22789, 650, 375, 2, 'Terocone'),
(22790, 675, 375, 2, 'Ancient Lichen'),
(22791, 675, 375, 2, 'Netherbloom'),
(22792, 675, 375, 2, 'Nightmare Vine'),
(22793, 700, 375, 2, 'Mana Thistle'),

-- Northrend Herbs
(36901, 725, 375, 2, 'Goldclover'),
(36903, 750, 400, 2, 'Adder\'s Tongue'),
(36904, 775, 425, 2, 'Tiger Lily'),
(36905, 800, 435, 2, 'Lichbloom'),
(36906, 825, 435, 2, 'Icethorn'),
(36907, 850, 450, 2, 'Talandra\'s Rose'),
(36908, 875, 450, 2, 'Frost Lotus');


-- ----------------------------------------
-- Fishing Data
-- ----------------------------------------

INSERT IGNORE INTO `gathering_experience` (item_id, base_xp, required_skill, profession, name) VALUES
-- Basic Fish
(6291, 50, 0, 4, 'Raw Brilliant Smallfish'),
(6289, 100, 0, 4, 'Raw Longjaw Mud Snapper'),
(6308, 125, 0, 4, 'Raw Bristle Whisker Catfish'),
(6358, 150, 0, 4, 'Oily Blackmouth'),
(6359, 175, 0, 4, 'Firefin Snapper'),
(6361, 200, 0, 4, 'Raw Rainbow Fin Albacore'),
(6362, 225, 0, 4, 'Raw Rockscale Cod'),
(4603, 300, 0, 4, 'Raw Spotted Yellowtail'),
(6317, 100, 0, 4, 'Raw Loch Frenzy'),

-- Mid Level Fish
(12238, 150, 0, 4, 'Darkshore Grouper'),
(13757, 375, 0, 4, 'Lightning Eel'),
(13888, 400, 0, 4, 'Darkclaw Lobster'),
(6522, 325, 0, 4, 'Deviate Fish'),
(8365, 250, 0, 4, 'Raw Mithril Head Trout'),

-- High Level Classic Fish
(13422, 350, 0, 4, 'Stonescale Eel'),

-- Outland Fish
(27422, 425, 0, 4, 'Barbed Gill Trout'),
(27425, 450, 0, 4, 'Spotted Feltail'),
(27429, 475, 0, 4, 'Zangarian Sporefish'),
(27435, 500, 0, 4, 'Figluster\'s Mudfish'),
(27437, 525, 0, 4, 'Icefin Bluefish'),
(27438, 550, 0, 4, 'Golden Darter'),
(27439, 575, 0, 4, 'Furious Crawdad'),

-- Northrend Fish
(41800, 600, 0, 4, 'Deep Sea Monsterbelly'),
(41801, 625, 0, 4, 'Moonglow Cuttlefish'),
(41802, 650, 0, 4, 'Imperial Manta Ray'),
(41803, 675, 0, 4, 'Musselback Sculpin'),
(41805, 700, 0, 4, 'Borean Man O\' War'),
(41806, 725, 0, 4, 'Dragonfin Angelfish'),
(41807, 750, 0, 4, 'Glacial Salmon'),
(41808, 775, 0, 4, 'Nettlefish'),
(41809, 800, 0, 4, 'Glassfin Minnow'),
(41810, 825, 0, 4, 'Fangtooth Herring'),
(41812, 850, 0, 4, 'Giant Darkwater Clam'),
(41813, 875, 0, 4, 'Succulent Orca Steak');


-- ----------------------------------------
-- Rarity Multipliers
-- ----------------------------------------

INSERT IGNORE INTO `gathering_experience_rarity` (item_id, multiplier, name) VALUES
(6522, 2, 'Deviate Fish'),
(7910, 2, 'Star Ruby'),
(7911, 1.5, 'Truesilver Ore'),
(8836, 1.5, 'Arthas\' Tears'),
(8838, 1.5, 'Sungrass'),
(8845, 2.5, 'Ghost Mushroom'),
(10620, 1.5, 'Thorium Ore'),
(11370, 2, 'Dark Iron Ore'),
(11382, 2, 'Blood of the Mountain'),
(12238, 1.5, 'Darkshore Grouper'),
(12363, 2.5, 'Arcane Crystal'),
(12364, 2, 'Huge Emerald'),
(12800, 2, 'Azerothian Diamond'),
(13463, 1.5, 'Dreamfoil'),
(13464, 1.5, 'Golden Sansam'),
(13467, 2, 'Icecap'),
(13468, 3, 'Black Lotus'),
(13757, 1.5, 'Lightning Eel'),
(13888, 1.5, 'Darkclaw Lobster'),
(15410, 3, 'Scale of Onyxia'),
(15412, 1.5, 'Green Dragonscale'),
(15414, 1.5, 'Red Dragonscale'),
(15415, 1.5, 'Blue Dragonscale'),
(15416, 1.5, 'Black Dragonscale'),
(15417, 1.5, 'Devilsaur Leather'),
(15419, 2.5, 'Pristine Hide of the Beast'),
(15423, 2.5, 'Brilliant Chromatic Scale'),
(17012, 1.5, 'Core Leather'),
(22203, 2, 'Large Obsidian Shard'),
(22790, 2, 'Ancient Lichen'),
(22793, 2, 'Mana Thistle'),
(23426, 2, 'Khorium Ore'),
(25700, 1.5, 'Fel Scales'),
(25707, 2, 'Fel Hide'),
(27438, 1.5, 'Golden Darter'),
(36908, 2.5, 'Frost Lotus'),
(36910, 2, 'Titanium Ore'),
(41802, 2, 'Imperial Manta Ray'),
(41807, 2, 'Glacial Salmon'),
(41812, 2, 'Giant Darkwater Clam'),
(41813, 2, 'Succulent Orca Steak'),
(44128, 1.5, 'Arctic Fur');


-- ----------------------------------------
-- Zone Multipliers
-- ----------------------------------------

INSERT IGNORE INTO `gathering_experience_zones` (zone_id, multiplier, name) VALUES
(1, 1, 'Dun Morogh'),
(3, 1.3, 'Badlands'),
(4, 1.4, 'Blasted Lands'),
(8, 1.3, 'Swamp of Sorrows'),
(10, 1.2, 'Ashenvale'),
(11, 1.5, 'Wetlands'),
(12, 1, 'Elwynn Forest'),
(14, 1, 'Durotar'),
(15, 1.3, 'Dustwallow Marsh'),
(16, 1.4, 'Azshara'),
(17, 1.1, 'The Barrens'),
(28, 1.4, 'Western Plaguelands'),
(33, 1.3, 'Stranglethorn Vale'),
(38, 1.2, 'Loch Modan'),
(40, 1, 'Westfall'),
(41, 1.5, 'Deadwind Pass'),
(44, 1.2, 'Redridge Mountains'),
(45, 1.5, 'Arathi Highlands'),
(46, 1.5, 'Burning Steppes'),
(47, 1.2, 'Duskwood'),
(51, 1.4, 'Searing Gorge'),
(66, 1.4, 'Dragonblight'),
(67, 1.5, 'The Storm Peaks'),
(85, 1, 'Tirisfal Glades'),
(130, 1.2, 'Silverpine Forest'),
(139, 1.4, 'Eastern Plaguelands'),
(141, 1, 'Teldrassil'),
(148, 1.1, 'Darkshore'),
(210, 1.5, 'Icecrown'),
(215, 1, 'Mulgore'),
(267, 1.2, 'Hillsbrad Foothills'),
(331, 1.3, 'Ashenvale'),
(357, 1.3, 'Feralas'),
(361, 1.3, 'Felwood'),
(394, 1.4, 'Grizzly Hills'),
(400, 1.3, 'Thousand Needles'),
(405, 1.3, 'Desolace'),
(406, 1.2, 'Stonetalon Mountains'),
(440, 1.3, 'Tanaris'),
(490, 1.4, 'Un\'Goro Crater'),
(495, 1.4, 'Howling Fjord'),
(618, 1.5, 'Winterspring'),
(1377, 1.4, 'Silithus'),
(1497, 1.25, 'Undercity'),
(1519, 1.25, 'Stormwind City'),
(1537, 1.25, 'Ironforge'),
(1637, 1.25, 'Orgrimmar'),
(1638, 1.25, 'Thunder Bluff'),
(1657, 1.25, 'Darnassus'),
(3430, 1, 'Eversong Woods'),
(3483, 1.3, 'Hellfire Peninsula'),
(3487, 1.25, 'Silvermoon City'),
(3518, 1.3, 'Nagrand'),
(3519, 1.4, 'Terokkar Forest'),
(3520, 1.4, 'Shadowmoon Valley'),
(3521, 1.4, 'Zangarmarsh'),
(3522, 1.5, 'Blade\'s Edge Mountains'),
(3524, 1, 'Azuremyst Isle'),
(3525, 1.5, 'Netherstorm'),
(3537, 1.3, 'Borean Tundra'),
(3557, 1.25, 'The Exodar'),
(3703, 1.25, 'Shattrath City'),
(3711, 1.4, 'Sholazar Basin'),
(4197, 1.5, 'Wintergrasp'),
(4395, 1.25, 'Dalaran'),
(4742, 1.5, 'Hrothgar\'s Landing');