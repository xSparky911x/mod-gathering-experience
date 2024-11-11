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
(2934, 25, 1, 3, 'Ruined Leather Scraps'),
(2318, 50, 1, 3, 'Light Leather'),
(2319, 100, 100, 3, 'Medium Leather'),
(4234, 150, 150, 3, 'Heavy Leather'),
(4304, 200, 200, 3, 'Thick Leather'),
(8170, 250, 250, 3, 'Rugged Leather'),

-- Hides
(783, 150, 75, 3, 'Light Hide'),
(4232, 125, 125, 3, 'Medium Hide'),
(4235, 175, 175, 3, 'Heavy Hide'),
(8169, 225, 225, 3, 'Thick Hide'),
(8171, 275, 275, 3, 'Rugged Hide'),

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
(6470, 100, 100, 3, 'Deviate Scale'),
(6471, 150, 150, 3, 'Perfect Deviate Scale'),
(5784, 125, 125, 3, 'Slimy Murloc Scale'),
(5785, 225, 225, 3, 'Thick Murloc Scale'),
(7286, 175, 175, 3, 'Black Whelp Scale'),
(7287, 200, 200, 3, 'Red Whelp Scale'),
(8154, 250, 250, 3, 'Scorpid Scale'),
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
(6359, 175, 0, 4, 'Firefin Snapper'),
(6361, 200, 0, 4, 'Raw Rainbow Fin Albacore'),
(6362, 225, 0, 4, 'Raw Rockscale Cod'),

-- Mid Level Fish (150-300)
(21071, 250, 0, 4, 'Raw Sagefish'),
(21153, 275, 0, 4, 'Raw Greater Sagefish'),
(4603, 300, 0, 4, 'Raw Spotted Yellowtail'),
(6522, 325, 0, 4, 'Deviate Fish'),
(13422, 350, 0, 4, 'Stonescale Eel'),
(13757, 375, 0, 4, 'Lightning Eel'),
(13888, 400, 0, 4, 'Darkclaw Lobster'),

-- High Level Fish (300-375)
(27422, 425, 0, 4, 'Barbed Gill Trout'),
(27425, 450, 0, 4, 'Spotted Feltail'),
(27429, 475, 0, 4, 'Zangarian Sporefish'),
(27435, 500, 0, 4, 'Figluster''s Mudfish'),
(27437, 525, 0, 4, 'Icefin Bluefish'),
(27438, 550, 0, 4, 'Golden Darter'),
(27439, 575, 0, 4, 'Furious Crawdad'),

-- Northrend Fish (375-450)
(41800, 600, 0, 4, 'Deep Sea Monsterbelly'),
(41801, 625, 0, 4, 'Moonglow Cuttlefish'),
(41802, 650, 0, 4, 'Imperial Manta Ray'),
(41803, 675, 0, 4, 'Musselback Sculpin'),
(41805, 700, 0, 4, 'Borean Man O'' War'),
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

INSERT IGNORE INTO `gathering_experience_rarity` (item_id, multiplier) VALUES
-- Mining Rares
(7911, 1.5),   -- Truesilver Ore
(10620, 1.5),  -- Thorium Ore
(11382, 2.0),  -- Blood of the Mountain
(7910, 2.0),   -- Star Ruby
(12364, 2.0),  -- Huge Emerald
(12800, 2.0),  -- Azerothian Diamond
(12363, 2.5),  -- Arcane Crystal
(22203, 2.0),  -- Large Obsidian Shard
(23426, 2.0),  -- Khorium Ore
(36910, 2.0),  -- Titanium Ore

-- Herbalism Rares
(13463, 1.5),  -- Dreamfoil
(8836, 1.5),   -- Arthas' Tears
(8838, 1.5),   -- Sungrass
(13467, 2.0),  -- Icecap
(8845, 2.5),   -- Ghost Mushroom
(13468, 3.0),  -- Black Lotus
(22793, 2.0),  -- Mana Thistle
(36908, 2.5),  -- Frost Lotus

-- Skinning Rares
(15417, 1.5),  -- Devilsaur Leather
(15416, 1.5),  -- Black Dragonscale
(44128, 1.5),  -- Arctic Fur
(15414, 1.5),  -- Red Dragonscale
(15419, 2.5),  -- Pristine Hide of the Beast
(15410, 3.0),  -- Scale of Onyxia
(15423, 2.5),  -- Brilliant Chromatic Scale
(25707, 2.0),  -- Fel Hide

-- Fishing Rares
(13757, 1.5),  -- Lightning Eel
(13888, 1.5),  -- Darkclaw Lobster
(41802, 2.0),  -- Imperial Manta Ray
(41807, 2.0),  -- Glacial Salmon
(41813, 2.0),  -- Succulent Orca Steak
(6522, 2.0);   -- Deviate Fish


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

-- Outland Zones (1.6-1.8)
(3483, 1.6, 'Hellfire Peninsula'),
(3519, 1.6, 'Terokkar Forest'),
(3520, 1.7, 'Shadowmoon Valley'),
(3522, 1.7, 'Blade''s Edge Mountains'),
(3525, 1.8, 'Netherstorm'),

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
