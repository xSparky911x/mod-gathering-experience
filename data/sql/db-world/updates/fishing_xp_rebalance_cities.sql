-- Update all city fish to have consistent XP values

-- Dalaran
UPDATE `gathering_experience` SET `base_xp` = 300 WHERE `item_id` = 43646; -- Fountain Goldfish

-- Stormwind
UPDATE `gathering_experience` SET `base_xp` = 100 WHERE `item_id` = 6289;  -- Raw Longjaw Mud Snapper (Canal fish)

-- Orgrimmar
UPDATE `gathering_experience` SET `base_xp` = 100 WHERE `item_id` = 6289;  -- Raw Longjaw Mud Snapper (Valley of Spirits)

-- Ironforge
UPDATE `gathering_experience` SET `base_xp` = 100 WHERE `item_id` = 6289;  -- Raw Longjaw Mud Snapper (Forlorn Cavern)

-- Thunder Bluff
UPDATE `gathering_experience` SET `base_xp` = 100 WHERE `item_id` = 6289;  -- Raw Longjaw Mud Snapper (Spirit Rise pools)

-- Darnassus
UPDATE `gathering_experience` SET `base_xp` = 100 WHERE `item_id` = 6289;  -- Raw Longjaw Mud Snapper (Temple pools)

-- Silvermoon
UPDATE `gathering_experience` SET `base_xp` = 150 WHERE `item_id` = 6358;  -- Oily Blackmouth (Court of the Sun)

-- Exodar
UPDATE `gathering_experience` SET `base_xp` = 150 WHERE `item_id` = 6358;  -- Oily Blackmouth (Seat of the Naaru)

-- Shattrath
UPDATE `gathering_experience` SET `base_xp` = 250 WHERE `item_id` = 27422; -- Barbed Gill Trout (Lower City canals)

-- Insert if they don't exist
INSERT IGNORE INTO `gathering_experience` (item_id, base_xp, required_skill, profession, name) VALUES
(43646, 300, 0, 4, 'Fountain Goldfish');

-- Add logging for city fishing
INSERT IGNORE INTO `gathering_experience_zones` (zone_id, multiplier, name) VALUES
(1519, 1.2, 'Stormwind City'),
(1537, 1.2, 'Ironforge'),
(1657, 1.2, 'Darnassus'),
(1637, 1.2, 'Orgrimmar'),
(1638, 1.2, 'Thunder Bluff'),
(1497, 1.2, 'Undercity'),
(3557, 1.2, 'The Exodar'),
(3487, 1.2, 'Silvermoon City'),
(3703, 1.3, 'Shattrath City'),
(4395, 1.5, 'Dalaran'); 