-- Delete existing multipliers
DELETE FROM `gathering_experience_zones`;

-- Insert rebalanced zone multipliers
INSERT INTO `gathering_experience_zones` (zone_id, multiplier, name) VALUES
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
(1637, 1.2, 'Orgrimmar'),
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