-- OSimX Authentication Database
-- DBMS contribution: Yash Variyani
-- MySQL 8.0+

CREATE DATABASE IF NOT EXISTS osimx_db;
USE osimx_db;

-- User accounts used by the OSimX login/registration system
CREATE TABLE IF NOT EXISTS users (
    id INT AUTO_INCREMENT PRIMARY KEY,
    full_name VARCHAR(100) NOT NULL,
    username VARCHAR(50) NOT NULL UNIQUE,
    email VARCHAR(100) NOT NULL UNIQUE,
    password_hash VARCHAR(255) NOT NULL,
    login_time DATETIME NULL
);

-- Optional test user.
-- Replace password_hash with a real application-generated password hash.
INSERT INTO users (full_name, username, email, password_hash)
VALUES ('Test User', 'testuser', 'test@example.com', 'test_hash')
ON DUPLICATE KEY UPDATE username = username;

-- Verify the table
DESCRIBE users;
SELECT * FROM users;
