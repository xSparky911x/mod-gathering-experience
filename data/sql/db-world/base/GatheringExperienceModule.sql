-- Drop tables in correct order (reverse dependency order)
DROP TABLE IF EXISTS `gathering_experience`;
DROP TABLE IF EXISTS `gathering_experience_rarity`;
DROP TABLE IF EXISTS `gathering_experience_zones`;
DROP TABLE IF EXISTS `gathering_experience_professions`;

-- Create profession table first (since it's referenced by gathering_experience)
CREATE TABLE IF NOT EXISTS `gathering_experience_professions` (
    `profession_id` TINYINT UNSIGNED NOT NULL,
    `name` VARCHAR(50) NOT NULL,
    `description` VARCHAR(255),
    PRIMARY KEY (`profession_id`)
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

-- Insert professions first (since they're referenced)
INSERT INTO `gathering_experience_professions` (profession_id, name, description) VALUES
(1, 'Mining', 'Gathering ore and minerals'),
(2, 'Herbalism', 'Gathering herbs and plants'),
(3, 'Skinning', 'Gathering leather and hides'),
(4, 'Fishing', 'Catching fish and other aquatic items');

-- Skinning Data
INSERT INTO `gathering_experience` (item_id, base_xp, required_skill, profession, name) VALUES
(2934, 25, 1, 3, 'Ruined Leather Scraps'),
(2318, 50, 1, 3, 'Light Leather'),
(783, 150, 75, 3, 'Light Hide'),
(2319, 100, 100, 3, 'Medium Leather'),
(4234, 150, 150, 3, 'Heavy Leather'),
(4304, 200, 200, 3, 'Thick Leather'),
(8170, 250, 250, 3, 'Rugged Leather'),
(15417, 300, 300, 3, 'Devilsaur Leather'),
(15415, 325, 325, 3, 'Blue Dragonscale'),
(15416, 325, 325, 3, 'Black Dragonscale'),
(21887, 350, 350, 3, 'Knothide Leather'),
(25700, 375, 310, 3, 'Fel Scales'),
(25707, 400, 400, 3, 'Fel Hide'),
(33568, 425, 425, 3, 'Borean Leather'),
(38425, 450, 450, 3, 'Heavy Borean Leather'),
(44128, 475, 475, 3, 'Arctic Fur'),
(17012, 375, 375, 3, 'Core Leather'),
(29539, 400, 400, 3, 'Cobra Scales'),
(29547, 400, 400, 3, 'Wind Scales'),
(4232, 125, 125, 3, 'Medium Hide'),
(4235, 175, 175, 3, 'Heavy Hide'),
(8169, 225, 225, 3, 'Thick Hide'),
(8171, 275, 275, 3, 'Rugged Hide'),
(6470, 100, 100, 3, 'Deviate Scale'),
(6471, 150, 150, 3, 'Perfect Deviate Scale'),
(5784, 125, 125, 3, 'Slimy Murloc Scale'),
(7286, 175, 175, 3, 'Black Whelp Scale'),
(7287, 200, 200, 3, 'Red Whelp Scale'),
(5785, 225, 225, 3, 'Thick Murloc Scale'),
(8154, 250, 250, 3, 'Scorpid Scale'),
(15408, 300, 300, 3, 'Heavy Scorpid Scale'),
(25649, 300, 300, 3, 'Knothide Leather Scraps'),
(33567, 400, 350, 3, 'Borean Leather Scraps'),
(15414, 350, 350, 3, 'Red Dragonscale'),
(15412, 375, 375, 3, 'Green Dragonscale'),
(15419, 475, 475, 3, 'Pristine Hide of the Beast'),
(15410, 500, 500, 3, 'Scale of Onyxia'),
(15423, 525, 525, 3, 'Brilliant Chromatic Scale');

-- Mining Data
INSERT INTO `gathering_experience` (item_id, base_xp, required_skill, profession, name) VALUES
(2770, 50, 1, 1, 'Copper Ore'),
(774, 50, 1, 1, 'Malachite'),
(818, 75, 1, 1, 'Tigerseye'),
(2771, 100, 65, 1, 'Tin Ore'),
(1705, 200, 65, 1, 'Lesser Moonstone'),
(1529, 175, 65, 1, 'Jade'),
(1210, 100, 65, 1, 'Shadowgem'),
(1206, 150, 65, 1, 'Moss Agate'),
(2836, 50, 65, 1, 'Coarse Stone'),
(2775, 150, 75, 1, 'Silver Ore'),
(2772, 200, 125, 1, 'Iron Ore'),
(3864, 300, 125, 1, 'Citrine'),
(2838, 75, 125, 1, 'Heavy Stone'),
(2776, 250, 155, 1, 'Gold Ore'),
(7909, 650, 155, 1, 'Aquamarine'),
(3858, 400, 175, 1, 'Mithril Ore'),
(7912, 125, 175, 1, 'Solid Stone'),
(7911, 350, 205, 1, 'Truesilver Ore'),
(10620, 400, 230, 1, 'Thorium Ore'),
(12800, 650, 230, 1, 'Azerothian Diamond'),
(12364, 700, 230, 1, 'Huge Emerald'),
(12363, 750, 230, 1, 'Arcane Crystal'),
(12799, 675, 230, 1, 'Large Opal'),
(12361, 700, 230, 1, 'Blue Sapphire'),
(7910, 625, 230, 1, 'Star Ruby'),
(12365, 100, 230, 1, 'Dense Stone'),
(11370, 375, 230, 1, 'Dark Iron Ore'),
(19774, 675, 255, 1, 'Souldarite'),
(23424, 425, 275, 1, 'Fel Iron Ore'),
(11754, 600, 300, 1, 'Black Diamond'),
(11382, 800, 300, 1, 'Blood of the Mountain'),
(22202, 575, 305, 1, 'Small Obsidian Shard'),
(22203, 625, 305, 1, 'Large Obsidian Shard'),
(23425, 450, 325, 1, 'Adamantite Ore'),
(37705, 400, 350, 1, 'Crystallized Water'),
(37701, 300, 350, 1, 'Crystallized Earth'),
(36909, 500, 350, 1, 'Cobalt Ore'),
(23426, 475, 375, 1, 'Khorium Ore'),
(36912, 550, 400, 1, 'Saronite Ore'),
(37703, 350, 400, 1, 'Crystallized Shadow'),
(36910, 525, 450, 1, 'Titanium Ore'),
(37702, 325, 450, 1, 'Crystallized Fire');

-- Herbalism Data
INSERT INTO `gathering_experience` (item_id, base_xp, required_skill, profession, name) VALUES
(765, 360, 1, 2, 'Silverleaf'),
(2447, 360, 1, 2, 'Peacebloom'),
(2449, 540, 15, 2, 'Earthroot'),
(785, 720, 50, 2, 'Mageroyal'),
(2450, 900, 70, 2, 'Briarthorn'),
(3820, 1440, 85, 2, 'Stranglekelp'),
(2453, 1260, 100, 2, 'Bruiseweed'),
(2452, 1080, 115, 2, 'Swiftthistle'),
(3355, 1620, 115, 2, 'Wild Steelbloom'),
(3369, 2160, 120, 2, 'Grave Moss'),
(3356, 1800, 125, 2, 'Kingsblood'),
(3357, 1980, 150, 2, 'Liferoot'),
(3818, 2340, 160, 2, 'Fadeleaf'),
(3821, 2700, 170, 2, 'Goldthorn'),
(3358, 2880, 185, 2, 'Khadgars Whisker'),
(3819, 2520, 195, 2, 'Wintersbite'),
(4625, 3240, 205, 2, 'Firebloom'),
(8831, 3420, 210, 2, 'Purple Lotus'),
(8836, 3600, 220, 2, 'Arthas Tears'),
(8838, 3780, 230, 2, 'Sungrass'),
(8839, 3960, 235, 2, 'Blindweed'),
(8845, 4140, 245, 2, 'Ghost Mushroom'),
(8846, 4320, 250, 2, 'Gromsblood'),
(13464, 4860, 260, 2, 'Golden Sansam'),
(13463, 4680, 270, 2, 'Dreamfoil'),
(13465, 5040, 280, 2, 'Mountain Silversage'),
(13466, 5220, 285, 2, 'Plaguebloom'),
(13467, 5400, 300, 2, 'Black Lotus'),
(22785, 5760, 300, 2, 'Felweed'),
(22786, 5940, 315, 2, 'Dreaming Glory'),
(22787, 6120, 325, 2, 'Ragveil'),
(22789, 6300, 325, 2, 'Terocone'),
(22790, 6480, 340, 2, 'Ancient Lichen'),
(22791, 6660, 350, 2, 'Netherbloom'),
(36901, 7200, 350, 2, 'Goldclover'),
(22792, 6840, 365, 2, 'Nightmare Vine'),
(22793, 7020, 375, 2, 'Mana Thistle'),
(36904, 7560, 375, 2, 'Tiger Lily'),
(22710, 7560, 375, 2, 'Blood Thistle'),
(36903, 7380, 400, 2, 'Adders Tongue'),
(36907, 8100, 400, 2, 'Talandras Rose'),
(36905, 7740, 425, 2, 'Lichbloom'),
(36906, 7920, 435, 2, 'Icethorn');

-- Fishing Data
INSERT INTO `gathering_experience` (item_id, base_xp, required_skill, profession, name) VALUES
(6291, 25, 0, 4, 'Raw Brilliant Smallfish'),
(6317, 100, 0, 4, 'Raw Loch Frenzy'),
(6308, 150, 0, 4, 'Raw Bristle Whisker Catfish'),
(6358, 200, 0, 4, 'Oily Blackmouth'),
(12238, 350, 0, 4, 'Darkshore Grouper'),
(6359, 300, 0, 4, 'Firefin Snapper'),
(6361, 400, 0, 4, 'Raw Rainbow Fin Albacore'),
(6362, 500, 0, 4, 'Raw Rockscale Cod'),
(6289, 250, 0, 4, 'Raw Longjaw Mud Snapper'),
(13422, 450, 0, 4, 'Stonescale Eel'),
(13756, 205, 0, 4, 'Raw Spotted Yellowtail'),
(21071, 600, 0, 4, 'Raw Sagefish'),
(21153, 700, 0, 4, 'Raw Greater Sagefish'),
(27422, 400, 0, 4, 'Barbed Gill Trout'),
(27425, 450, 0, 4, 'Spotted Feltail'),
(27429, 500, 0, 4, 'Zangarian Sporefish'),
(27435, 550, 0, 4, 'Figluster''s Mudfish'),
(27437, 600, 0, 4, 'Icefin Bluefish'),
(27438, 650, 0, 4, 'Golden Darter'),
(27439, 700, 0, 4, 'Furious Crawdad'),
(33823, 750, 0, 4, 'Bloodfin Catfish'),
(33824, 800, 0, 4, 'Crescent-Tail Skullfish'),
(41800, 850, 0, 4, 'Deep Sea Monsterbelly'),
(41801, 900, 0, 4, 'Moonglow Cuttlefish'),
(41802, 950, 0, 4, 'Imperial Manta Ray'),
(41803, 1000, 0, 4, 'Rockfin Grouper'),
(41805, 1050, 0, 4, 'Borean Man O'' War'),
(41806, 1100, 0, 4, 'Musselback Sculpin'),
(41807, 1150, 0, 4, 'Dragonfin Angelfish'),
(41808, 1200, 0, 4, 'Bonescale Snapper'),
(41809, 1250, 0, 4, 'Glacial Salmon'),
(41810, 1300, 0, 4, 'Fangtooth Herring'),
(41812, 1350, 0, 4, 'Giant Darkwater Clam'),
(41813, 1400, 0, 4, 'Nettlefish'),
(41814, 1450, 0, 4, 'Glassfin Minnow');

-- Insert rarity multipliers
INSERT INTO `gathering_experience_rarity` (item_id, multiplier) VALUES
(13463, 1.5),  -- Dreamfoil
(8836, 1.5),   -- Arthas' Tears
(8838, 1.5),   -- Sungrass
(15417, 1.5),  -- Devilsaur Leather
(15416, 1.5),  -- Black Dragonscale
(7911, 1.5),   -- Truesilver Ore
(10620, 1.5),  -- Thorium Ore
(44128, 1.5),  -- Arctic Fur
(15414, 1.5),  -- Red Dragonscale
(13467, 3.0),  -- Black Lotus
(8845, 2.5),   -- Ghost Mushroom
(11382, 2.0),  -- Blood of the Mountain
(7910, 2.0),   -- Star Ruby
(12364, 2.0),  -- Huge Emerald
(12800, 2.0),  -- Azerothian Diamond
(12363, 2.5),  -- Arcane Crystal
(22203, 2.0),  -- Large Obsidian Shard
(15419, 2.5),  -- Pristine Hide of the Beast
(15410, 3.0),  -- Scale of Onyxia
(15423, 2.5);  -- Brilliant Chromatic Scale

-- Insert zone multipliers
INSERT INTO `gathering_experience_zones` (zone_id, multiplier, name) VALUES
(1, 1.0, 'Dun Morogh'),
(12, 1.2, 'Elwynn Forest'),
(14, 1.5, 'Durotar'),
(85, 1.3, 'Tirisfal Glades'),
(141, 1.4, 'Teldrassil'),
(215, 1.6, 'Mulgore'),
(3, 1.1, 'Badlands'),
(10, 1.7, 'Duskwood'),
(11, 1.8, 'Wetlands'),
(33, 1.9, 'Stranglethorn Vale'),
(38, 2.0, 'Loch Modan'),
(40, 1.6, 'Westfall'),
(44, 1.7, 'Redridge Mountains'),
(47, 1.8, 'The Hinterlands'),
(51, 1.9, 'Searing Gorge'),
(139, 2.0, 'Eastern Plaguelands'),
(1377, 2.1, 'Silithus'),
(1519, 2.2, 'Stormwind City'),
(1637, 2.3, 'Orgrimmar'),
(3430, 2.4, 'Eversong Woods'),
-- Northrend zones (higher multipliers for higher level content)
(3537, 2.5, 'Borean Tundra'),
(65, 2.5, 'Dragonblight'),
(66, 2.6, 'Zul''Drak'),
(394, 2.6, 'Grizzly Hills'),
(495, 2.7, 'Howling Fjord'),
(2817, 2.7, 'Crystalsong Forest'),
(4395, 2.8, 'Dalaran'),
(67, 2.8, 'Storm Peaks'),
(210, 2.9, 'Icecrown'),
(4197, 3.0, 'Wintergrasp');
