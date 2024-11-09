CREATE TABLE IF NOT EXISTS `gathering_experience_settings` (
    `profession` VARCHAR(32) NOT NULL,
    `enabled` TINYINT(1) NOT NULL DEFAULT 1,
    `last_updated` TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    PRIMARY KEY (`profession`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- Insert default values
INSERT IGNORE INTO `gathering_experience_settings` (`profession`, `enabled`) VALUES
('mining', 1),
('herbalism', 1),
('skinning', 1),
('fishing', 1); 