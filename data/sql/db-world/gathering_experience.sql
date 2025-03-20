SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0;
SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT;
SET @OLD_SQL_MODE=@@SQL_MODE;

CREATE TABLE IF NOT EXISTS `gathering_experience_professions` (
    `profession_id` TINYINT UNSIGNED NOT NULL,
    `name` VARCHAR(50) NOT NULL,
    `description` VARCHAR(255),
    PRIMARY KEY (`profession_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE IF NOT EXISTS `gathering_experience_settings` (
    `profession` VARCHAR(32) NOT NULL,
    `enabled` TINYINT NOT NULL DEFAULT 1,
    `last_updated` TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    PRIMARY KEY (`profession`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE IF NOT EXISTS `gathering_experience` (
    `item_id` INT UNSIGNED NOT NULL,
    `base_xp` INT UNSIGNED NOT NULL DEFAULT 0,
    `required_skill` INT UNSIGNED NOT NULL DEFAULT 0,
    `profession` TINYINT UNSIGNED NOT NULL,
    `name` VARCHAR(100) NOT NULL DEFAULT '' COMMENT 'Item name for reference',
    PRIMARY KEY (`item_id`),
    FOREIGN KEY (`profession`) REFERENCES `gathering_experience_professions` (`profession_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE IF NOT EXISTS `gathering_experience_zones` (
    `zone_id` INT UNSIGNED NOT NULL,
    `multiplier` FLOAT NOT NULL DEFAULT 1.0,
    `name` VARCHAR(100) NOT NULL DEFAULT '' COMMENT 'Zone name for reference',
    PRIMARY KEY (`zone_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE IF NOT EXISTS `gathering_experience_rarity` (
    `item_id` INT UNSIGNED NOT NULL,
    `multiplier` FLOAT NOT NULL DEFAULT '1',
    PRIMARY KEY (`item_id`),
    FOREIGN KEY (`item_id`) REFERENCES `gathering_experience` (`item_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- ----------------------------------------
-- Initial Data Setup
-- ----------------------------------------

INSERT IGNORE INTO `gathering_experience_professions` (profession_id, name, description) VALUES
(1, 'Mining', 'Gathering ore and minerals'),
(2, 'Herbalism', 'Gathering herbs and plants'),
(3, 'Skinning', 'Gathering leather and hides'),
(4, 'Fishing', 'Catching fish and other aquatic items');

INSERT IGNORE INTO `gathering_experience_settings` (`profession`, `enabled`) VALUES
('mining', 1),
('herbalism', 1),
('skinning', 1),
('fishing', 1);

-- ----------------------------------------
-- Mining Data (Profession ID: 1)
-- ----------------------------------------

INSERT IGNORE INTO `gathering_experience` (item_id, base_xp, required_skill, profession, name) VALUES
-- Basic Ores
(2770, 100, 1, 1, 'Copper Ore'),
(2771, 200, 65, 1, 'Tin Ore'),
(2772, 350, 125, 1, 'Iron Ore'),
(3858, 500, 175, 1, 'Mithril Ore'),
(10620, 600, 230, 1, 'Thorium Ore'),
(23424, 600, 275, 1, 'Fel Iron Ore'),
(23425, 650, 325, 1, 'Adamantite Ore'),
(36909, 700, 350, 1, 'Cobalt Ore'),
(36912, 750, 400, 1, 'Saronite Ore'),
(36910, 800, 450, 1, 'Titanium Ore'),

-- Precious Metals
(2775, 250, 75, 1, 'Silver Ore'),
(2776, 400, 155, 1, 'Gold Ore'),
(7911, 500, 205, 1, 'Truesilver Ore'),
(23426, 700, 375, 1, 'Khorium Ore'),

-- Special Items
(11370, 375, 230, 1, 'Dark Iron Ore'),
(22202, 575, 305, 1, 'Small Obsidian Shard'),
(22203, 625, 305, 1, 'Large Obsidian Shard');

-- ----------------------------------------
-- Herbalism Data (Profession ID: 2)
-- ----------------------------------------

INSERT IGNORE INTO `gathering_experience` (item_id, base_xp, required_skill, profession, name) VALUES
(765, 50, 1, 2, 'Silverleaf'),
(785, 100, 50, 2, 'Mageroyal'),
(2447, 50, 1, 2, 'Peacebloom'),
(2449, 100, 15, 2, 'Earthroot'),
(2450, 150, 70, 2, 'Briarthorn'),
(2452, 150, 50, 2, 'Swiftthistle'),
(2453, 200, 100, 2, 'Bruiseweed'),
(3355, 220, 115, 2, 'Wild Steelbloom'),
(3356, 240, 125, 2, 'Kingsblood'),
(3357, 300, 150, 2, 'Liferoot'),
(3358, 370, 185, 2, 'Khadgar\'s Whisker'),
(3369, 220, 120, 2, 'Grave Moss'),
(3818, 320, 160, 2, 'Fadeleaf'),
(3819, 390, 195, 2, 'Wintersbite'),
(3820, 200, 85, 2, 'Stranglekelp'),
(3821, 340, 170, 2, 'Goldthorn'),
(4625, 410, 205, 2, 'Firebloom'),
(8831, 420, 210, 2, 'Purple Lotus'),
(8836, 440, 220, 2, 'Arthas\' Tears'),
(8838, 460, 230, 2, 'Sungrass'),
(8839, 470, 235, 2, 'Blindweed'),
(8845, 470, 245, 2, 'Ghost Mushroom'),
(8846, 500, 250, 2, 'Gromsblood'),
(13463, 540, 250, 2, 'Dreamfoil'),
(13464, 520, 260, 2, 'Golden Sansam'),
(13465, 560, 280, 2, 'Mountain Silversage'),
(13466, 570, 285, 2, 'Plaguebloom'),
(13467, 600, 300, 2, 'Icecap'),
(13468, 600, 300, 2, 'Black Lotus'),
(22785, 600, 300, 2, 'Felweed'),
(22786, 600, 315, 2, 'Dreaming Glory'),
(22787, 650, 325, 2, 'Ragveil'),
(22789, 600, 325, 2, 'Terocone'),
(22790, 680, 340, 2, 'Ancient Lichen'),
(22791, 700, 350, 2, 'Netherbloom'),
(22792, 700, 365, 2, 'Nightmare Vine'),
(22793, 700, 375, 2, 'Mana Thistle'),
(36901, 720, 350, 2, 'Goldclover'),
(36903, 770, 400, 2, 'Adder\'s Tongue'),
(36904, 720, 375, 2, 'Tiger Lily'),
(36905, 800, 425, 2, 'Lichbloom'),
(36906, 800, 435, 2, 'Icethorn'),
(36907, 720, 385, 2, 'Talandra\'s Rose'),
(36908, 800, 450, 2, 'Frost Lotus');

-- ----------------------------------------
-- Skinning Data (Profession ID: 3)
-- ----------------------------------------

INSERT IGNORE INTO `gathering_experience` (item_id, base_xp, required_skill, profession, name) VALUES
(783, 150, 75, 3, 'Light Hide'),
(2318, 100, 1, 3, 'Light Leather'),
(2319, 200, 100, 3, 'Medium Leather'),
(2934, 75, 1, 3, 'Ruined Leather Scraps'),
(4232, 250, 125, 3, 'Medium Hide'),
(4234, 300, 150, 3, 'Heavy Leather'),
(4235, 350, 175, 3, 'Heavy Hide'),
(4304, 400, 200, 3, 'Thick Leather'),
(5784, 250, 125, 3, 'Slimy Murloc Scale'),
(5785, 450, 225, 3, 'Thick Murloc Scale'),
(6470, 200, 100, 3, 'Deviate Scale'),
(6471, 300, 150, 3, 'Perfect Deviate Scale'),
(7286, 350, 175, 3, 'Black Whelp Scale'),
(7287, 400, 200, 3, 'Red Whelp Scale'),
(8154, 500, 250, 3, 'Scorpid Scale'),
(8169, 450, 225, 3, 'Thick Hide'),
(8170, 500, 250, 3, 'Rugged Leather'),
(8171, 550, 275, 3, 'Rugged Hide'),
(15408, 600, 300, 3, 'Heavy Scorpid Scale'),
(15410, 800, 500, 3, 'Scale of Onyxia'),
(15412, 675, 375, 3, 'Green Dragonscale'),
(15414, 650, 350, 3, 'Red Dragonscale'),
(15415, 625, 325, 3, 'Blue Dragonscale'),
(15416, 625, 325, 3, 'Black Dragonscale'),
(15417, 600, 300, 3, 'Devilsaur Leather'),
(15419, 775, 475, 3, 'Pristine Hide of the Beast'),
(15423, 825, 525, 3, 'Brilliant Chromatic Scale'),
(17012, 675, 375, 3, 'Core Leather'),
(21887, 650, 350, 3, 'Knothide Leather'),
(25649, 600, 300, 3, 'Knothide Leather Scraps'),
(25700, 675, 310, 3, 'Fel Scales'),
(25707, 700, 400, 3, 'Fel Hide'),
(29539, 700, 400, 3, 'Cobra Scales'),
(29547, 700, 400, 3, 'Wind Scales'),
(33567, 650, 350, 3, 'Borean Leather Scraps'),
(33568, 700, 425, 3, 'Borean Leather'),
(38425, 750, 450, 3, 'Heavy Borean Leather'),
(44128, 800, 475, 3, 'Arctic Fur');

-- ----------------------------------------
-- Fishing Data (Profession ID: 4)
-- ----------------------------------------

INSERT IGNORE INTO `gathering_experience` (item_id, base_xp, required_skill, profession, name) VALUES
(4603, 600, 0, 4, 'Raw Spotted Yellowtail'),
(6289, 200, 0, 4, 'Raw Longjaw Mud Snapper'),
(6291, 100, 0, 4, 'Raw Brilliant Smallfish'),
(6292, 150, 0, 4, '10 Pound Mud Snapper'),
(6294, 150, 0, 4, '12 Pound Mud Snapper'),
(6295, 150, 0, 4, '15 Pound Mud Snapper'),
(6308, 250, 0, 4, 'Raw Bristle Whisker Catfish'),
(6309, 300, 0, 4, '17 Pound Catfish'),
(6310, 300, 0, 4, '19 Pound Catfish'),
(6311, 325, 0, 4, '22 Pound Catfish'),
(6317, 200, 0, 4, 'Raw Loch Frenzy'),
(6358, 300, 0, 4, 'Oily Blackmouth'),
(6359, 350, 0, 4, 'Firefin Snapper'),
(6361, 400, 0, 4, 'Raw Rainbow Fin Albacore'),
(6362, 450, 0, 4, 'Raw Rockscale Cod'),
(6363, 325, 0, 4, '26 Pound Catfish'),
(6364, 350, 0, 4, '32 Pound Catfish'),
(6522, 625, 0, 4, 'Deviate Fish'),
(12238, 300, 0, 4, 'Darkshore Grouper'),
(13422, 650, 0, 4, 'Stonescale Eel'),
(13757, 675, 0, 4, 'Lightning Eel'),
(13888, 700, 0, 4, 'Darkclaw Lobster'),
(21071, 500, 0, 4, 'Raw Sagefish'),
(21153, 550, 0, 4, 'Raw Greater Sagefish'),
(27422, 725, 0, 4, 'Barbed Gill Trout'),
(27425, 750, 0, 4, 'Spotted Feltail'),
(27429, 775, 0, 4, 'Zangarian Sporefish'),
(27435, 800, 0, 4, 'Figluster\'s Mudfish'),
(27437, 825, 0, 4, 'Icefin Bluefish'),
(27438, 850, 0, 4, 'Golden Darter'),
(27439, 875, 0, 4, 'Furious Crawdad'),
(41800, 900, 0, 4, 'Deep Sea Monsterbelly'),
(41801, 925, 0, 4, 'Moonglow Cuttlefish'),
(41802, 950, 0, 4, 'Imperial Manta Ray'),
(41803, 975, 0, 4, 'Musselback Sculpin'),
(41805, 1000, 0, 4, 'Borean Man O\' War'),
(41806, 1025, 0, 4, 'Dragonfin Angelfish'),
(41807, 1050, 0, 4, 'Glacial Salmon'),
(41808, 1075, 0, 4, 'Nettlefish'),
(41809, 1100, 0, 4, 'Glassfin Minnow'),
(41810, 1125, 0, 4, 'Fangtooth Herring'),
(41812, 1150, 0, 4, 'Giant Darkwater Clam'),
(41813, 1175, 0, 4, 'Succulent Orca Steak');
-- ----------------------------------------
-- Zone Multipliers
-- ----------------------------------------

INSERT IGNORE INTO `gathering_experience_zones` (zone_id, multiplier, name) VALUES
(1, 1.0, 'Dun Morogh'),
(3, 1.3, 'Badlands'),
(4, 1.4, 'Blasted Lands'),
(8, 1.3, 'Swamp of Sorrows'),
(10, 1.2, 'Duskwood'),
(11, 1.5, 'Wetlands'),
(12, 1.0, 'Elwynn Forest'),
(14, 1.0, 'Durotar'),
(15, 1.3, 'Dustwallow Marsh'),
(16, 1.4, 'Azshara'),
(17, 1.1, 'The Barrens'),
(28, 1.4, 'Western Plaguelands'),
(33, 1.3, 'Stranglethorn Vale'),
(38, 1.2, 'Loch Modan'),
(40, 1.0, 'Westfall'),
(41, 1.5, 'Deadwind Pass'),
(44, 1.2, 'Redridge Mountains'),
(45, 1.5, 'Arathi Highlands'),
(46, 1.5, 'Burning Steppes'),
(47, 1.3, 'The Hinterlands'),
(51, 1.4, 'Searing Gorge'),
(85, 1.0, 'Tirisfal Glades'),
(130, 1.2, 'Silverpine Forest'),
(139, 1.4, 'Eastern Plaguelands'),
(141, 1.0, 'Teldrassil'),
(148, 1.1, 'Darkshore'),
(215, 1.0, 'Mulgore'),
(267, 1.2, 'Hillsbrad Foothills'),
(331, 1.3, 'Ashenvale'),
(357, 1.3, 'Feralas'),
(361, 1.3, 'Felwood'),
(400, 1.3, 'Thousand Needles'),
(405, 1.3, 'Desolace'),
(406, 1.2, 'Stonetalon Mountains'),
(440, 1.3, 'Tanaris'),
(490, 1.4, 'Un''Goro Crater'),
(491, 1.4, 'Dragonblight'),
(493, 1.4, 'Moonglade'),
(495, 1.4, 'Howling Fjord'),
(618, 1.5, 'Winterspring'),
(1377, 1.4, 'Silithus'),
(1497, 0.5, 'Undercity'),
(1519, 0.5, 'Stormwind City'),
(1537, 0.5, 'Ironforge'),
(1637, 0.5, 'Orgrimmar'),
(1638, 0.5, 'Thunder Bluff'),
(1657, 0.5, 'Darnassus'),
(3430, 1.0, 'Eversong Woods'),
(3433, 1.0, 'Ghostlands'),
(3483, 1.3, 'Hellfire Peninsula'),
(3487, 0.5, 'Silvermoon City'),
(3518, 1.3, 'Nagrand'),
(3519, 1.4, 'Terokkar Forest'),
(3520, 1.4, 'Shadowmoon Valley'),
(3521, 1.4, 'Zangarmarsh'),
(3522, 1.5, 'Blade''s Edge Mountains'),
(3523, 1.0, 'Netherstorm'),
(3524, 1.0, 'Azuremyst Isle'),
(3525, 1.5, 'Bloodmyst Isle'),
(3537, 1.3, 'Borean Tundra'),
(3557, 0.5, 'The Exodar'),
(3703, 0.5, 'Shattrath City'),
(3711, 1.4, 'Sholazar Basin'),
(4080, 1.4, 'Isle of Quel''Danas'),
(4197, 1.5, 'Wintergrasp'),
(4395, 0.5, 'Dalaran'),
(4742, 1.5, 'Hrothgar''s Landing'),
(4812, 1.5, 'Icecrown'),
(4813, 1.4, 'Grizzly Hills'),
(4815, 1.5, 'The Storm Peaks'),
(4820, 1.4, 'Zul''Drak'),
(4922, 1.4, 'Crystalsong Forest'),
(4298, 1.0, 'Plaguelands: The Scarlet Enclave');

-- ----------------------------------------
-- Rarity Data
-- ----------------------------------------

INSERT IGNORE INTO `gathering_experience_rarity` (`item_id`, `multiplier`) VALUES
(6522, 2),      -- Deviate Fish
(7910, 2),      -- Mithril Deposit (Uncommon)
(7911, 1.5),    -- Truesilver Ore
(8836, 1.5),    -- Arthas' Tears
(8838, 1.5),    -- Sungrass
(8845, 2.5),    -- Ghost Mushroom
(10620, 1.5),   -- Thorium Ore
(11382, 2),     -- Blood of the Mountain
(12363, 2.5),   -- Arcane Crystal
(12364, 2),     -- Huge Emerald
(12800, 2),     -- Azerothian Diamond
(13463, 1.5),   -- Dreamfoil
(13467, 3),     -- Icecap
(13468, 3),     -- Black Lotus
(13757, 1.5),   -- Lightning Eel
(13888, 1.5),   -- Darkclaw Lobster
(15410, 3),     -- Scale of Onyxia
(15414, 1.5),   -- Red Dragonscale
(15416, 1.5),   -- Black Dragonscale
(15417, 1.5),   -- Devilsaur Leather
(15419, 2.5),   -- Pristine Hide of the Beast
(15423, 2.5),   -- Brilliant Chromatic Scale
(22203, 2),     -- Large Obsidian Shard
(22793, 2),     -- Mana Thistle
(23426, 2),     -- Khorium Ore
(25707, 2),     -- Fel Hide
(36908, 2.5),   -- Frost Lotus
(36910, 2),     -- Titanium Ore
(41802, 2),     -- Imperial Manta Ray
(41807, 2),     -- Glacial Salmon
(41813, 2),     -- Succulent Orca Steak
(44128, 1.5);   -- Arctic Fur


SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS;
SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT;
SET SQL_MODE=@OLD_SQL_MODE;