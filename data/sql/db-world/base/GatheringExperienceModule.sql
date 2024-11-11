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
    `enabled` TINYINT(1) NOT NULL DEFAULT 1,
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
-- Herbalism Data
-- ----------------------------------------

INSERT IGNORE INTO `gathering_experience` (item_id, base_xp, required_skill, profession, name) VALUES
-- Starter Herbs (1-50)
(2447, 25, 1, 2, 'Peacebloom'),
(765, 25, 1, 2, 'Silverleaf'),
(2449, 50, 1, 2, 'Earthroot'),

-- Low Level Herbs (50-125)
(785, 75, 50, 2, 'Mageroyal'),
(2450, 75, 50, 2, 'Briarthorn'),
(2452, 100, 75, 2, 'Swiftthistle'),
(2453, 100, 75, 2, 'Bruiseweed'),
(3355, 125, 100, 2, 'Wild Steelbloom'),
(3356, 125, 100, 2, 'Kingsblood'),
(3357, 150, 125, 2, 'Liferoot'),

-- Mid Level Herbs (125-225)
(3358, 175, 150, 2, 'Khadgar''s Whisker'),
(3369, 175, 150, 2, 'Grave Moss'),
(3818, 200, 175, 2, 'Fadeleaf'),
(3821, 200, 175, 2, 'Goldthorn'),
(3819, 225, 200, 2, 'Dragon''s Teeth'),
(3358, 225, 200, 2, 'Khadgar''s Whisker'),
(8836, 250, 225, 2, 'Arthas'' Tears'),
(8838, 250, 225, 2, 'Sungrass'),
(8839, 275, 225, 2, 'Blindweed'),
(8845, 275, 225, 2, 'Ghost Mushroom'),
(8846, 275, 225, 2, 'Gromsblood'),

-- High Level Herbs (225-300)
(13463, 300, 250, 2, 'Dreamfoil'),
(13464, 300, 250, 2, 'Golden Sansam'),
(13465, 325, 275, 2, 'Mountain Silversage'),
(13466, 325, 275, 2, 'Sorrowmoss'),
(13467, 350, 300, 2, 'Icecap'),
(13468, 350, 300, 2, 'Black Lotus'),

-- Outland Herbs (300-375)
(22785, 375, 300, 2, 'Felweed'),
(22786, 400, 325, 2, 'Dreaming Glory'),
(22787, 425, 350, 2, 'Ragveil'),
(22789, 450, 375, 2, 'Terocone'),
(22790, 475, 375, 2, 'Ancient Lichen'),
(22791, 475, 375, 2, 'Netherbloom'),
(22792, 475, 375, 2, 'Nightmare Vine'),
(22793, 500, 375, 2, 'Mana Thistle'),

-- Northrend Herbs (375-450)
(36901, 525, 375, 2, 'Goldclover'),
(36903, 550, 400, 2, 'Adder''s Tongue'),
(36904, 575, 425, 2, 'Tiger Lily'),
(36905, 600, 435, 2, 'Lichbloom'),
(36906, 625, 435, 2, 'Icethorn'),
(36907, 650, 450, 2, 'Talandra''s Rose'),
(36908, 675, 450, 2, 'Frost Lotus');


-- ----------------------------------------
-- Fishing Data
-- ----------------------------------------

INSERT IGNORE INTO `gathering_experience` (item_id, base_xp, required_skill, profession, name) VALUES
-- Starter Fish (1-25)
(6291, 50, 0, 4, 'Raw Brilliant Smallfish'),

-- Low Level Fish (25-150)
(6289, 100, 0, 4, 'Raw Longjaw Mud Snapper'),
(6317, 100, 0, 4, 'Raw Loch Frenzy'),
(6308, 125, 0, 4, 'Raw Bristle Whisker Catfish'),
(6358, 150, 0, 4, 'Oily Blackmouth'),
(12238, 150, 0, 4, 'Darkshore Grouper'),

-- Mid Level Fish (150-300)
(6359, 175, 0, 4, 'Firefin Snapper'),
(6361, 200, 0, 4, 'Raw Rainbow Fin Albacore'),
(6362, 225, 0, 4, 'Raw Rockscale Cod'),
(13422, 250, 0, 4, 'Stonescale Eel'),
(13756, 250, 0, 4, 'Raw Spotted Yellowtail'),
(21071, 300, 0, 4, 'Raw Sagefish'),
(21153, 350, 0, 4, 'Raw Greater Sagefish'),

-- Outland Fish (300-375)
(27422, 250, 0, 4, 'Barbed Gill Trout'),
(27425, 450, 0, 4, 'Spotted Feltail'),
(27429, 475, 0, 4, 'Zangarian Sporefish'),
(27435, 500, 0, 4, 'Figluster''s Mudfish'),
(27437, 525, 0, 4, 'Icefin Bluefish'),
(27438, 550, 0, 4, 'Golden Darter'),
(27439, 575, 0, 4, 'Furious Crawdad'),
(33823, 600, 0, 4, 'Bloodfin Catfish'),
(33824, 600, 0, 4, 'Crescent-Tail Skullfish'),

-- Northrend Fish (375-450)
(41800, 625, 0, 4, 'Deep Sea Monsterbelly'),
(41801, 650, 0, 4, 'Moonglow Cuttlefish'),
(41802, 675, 0, 4, 'Imperial Manta Ray'),
(41803, 700, 0, 4, 'Rockfin Grouper'),
(41805, 725, 0, 4, 'Borean Man O'' War'),
(41806, 750, 0, 4, 'Musselback Sculpin'),
(41807, 775, 0, 4, 'Dragonfin Angelfish'),
(41808, 800, 0, 4, 'Bonescale Snapper'),
(41809, 800, 0, 4, 'Glacial Salmon'),
(41810, 800, 0, 4, 'Fangtooth Herring'),
(41812, 800, 0, 4, 'Giant Darkwater Clam'),
(41813, 800, 0, 4, 'Nettlefish'),
(41814, 800, 0, 4, 'Glassfin Minnow'),

-- Special Fish
(43646, 300, 0, 4, 'Fountain Goldfish');


-- ----------------------------------------
-- Rarity Multipliers
-- ----------------------------------------

INSERT IGNORE INTO `gathering_experience_rarity` (item_id, multiplier) VALUES
-- Herbs
(13463, 1.5),  -- Dreamfoil
(8836, 1.5),   -- Arthas' Tears
(8838, 1.5),   -- Sungrass
(13467, 3.0),  -- Black Lotus
(8845, 2.5),   -- Ghost Mushroom

-- Mining
(7911, 1.5),   -- Truesilver Ore
(10620, 1.5),  -- Thorium Ore
(11382, 2.0),  -- Blood of the Mountain
(7910, 2.0),   -- Star Ruby
(12364, 2.0),  -- Huge Emerald
(12800, 2.0),  -- Azerothian Diamond
(12363, 2.5),  -- Arcane Crystal
(22203, 2.0),  -- Large Obsidian Shard

-- Skinning
(15417, 1.5),  -- Devilsaur Leather
(15416, 1.5),  -- Black Dragonscale
(44128, 1.5),  -- Arctic Fur
(15414, 1.5),  -- Red Dragonscale
(15419, 2.5),  -- Pristine Hide of the Beast
(15410, 3.0),  -- Scale of Onyxia
(15423, 2.5);  -- Brilliant Chromatic Scale


-- ----------------------------------------
-- Zone Multipliers
-- ----------------------------------------

INSERT IGNORE INTO `gathering_experience_zones` (zone_id, multiplier, name) VALUES
-- Starter Zones (1.0-1.2)
(1, 1.0, 'Dun Morogh'),
(12, 1.0, 'Elwynn Forest'),
(14, 1.0, 'Durotar'),
(85, 1.0, 'Tirisfal Glades'),
(141, 1.0, 'Teldrassil'),
(215, 1.0, 'Mulgore'),
(3430, 1.0, 'Eversong Woods'),

-- Low Level Zones (1.2-1.4)
(40, 1.2, 'Westfall'),
(44, 1.2, 'Redridge Mountains'),
(38, 1.3, 'Loch Modan'),
(10, 1.3, 'Duskwood'),
(11, 1.4, 'Wetlands'),

-- Mid Level Zones (1.4-1.6)
(33, 1.4, 'Stranglethorn Vale'),
(47, 1.5, 'The Hinterlands'),
(51, 1.5, 'Searing Gorge'),
(3, 1.6, 'Badlands'),

-- High Level Zones (1.6-1.8)
(139, 1.6, 'Eastern Plaguelands'),
(1377, 1.8, 'Silithus'),

-- Cities (1.2-1.5)
(1519, 1.2, 'Stormwind City'),
(1537, 1.2, 'Ironforge'),
(1657, 1.2, 'Darnassus'),
(1637, 1.2, 'Orgrimmar'),
(1638, 1.2, 'Thunder Bluff'),
(1497, 1.2, 'Undercity'),
(3557, 1.2, 'The Exodar'),
(3487, 1.2, 'Silvermoon City'),
(3703, 1.3, 'Shattrath City'),
(4395, 1.5, 'Dalaran'),  -- Slightly higher as it's a high-level city

-- Northrend zones (1.8-2.2)
(3537, 1.8, 'Borean Tundra'),
(65, 1.8, 'Dragonblight'),
(495, 1.9, 'Howling Fjord'),
(394, 1.9, 'Grizzly Hills'),
(66, 2.0, 'Zul''Drak'),
(2817, 2.0, 'Crystalsong Forest'),
(67, 2.1, 'Storm Peaks'),
(210, 2.1, 'Icecrown'),
(4197, 2.2, 'Wintergrasp');  -- Slightly higher as it's a PvP zone
