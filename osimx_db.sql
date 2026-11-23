-- ================================================================
-- OSimX Database Schema — MySQL 8.0+
-- Complete schema: users, login history, simulations,
-- simulation I/O snapshots, and algorithm comparisons.
-- ================================================================

CREATE DATABASE IF NOT EXISTS osimx_db
    CHARACTER SET utf8mb4
    COLLATE utf8mb4_unicode_ci;
USE osimx_db;

-- ----------------------------------------------------------------
-- 1. USERS
-- ----------------------------------------------------------------
CREATE TABLE IF NOT EXISTS users (
    id INT AUTO_INCREMENT PRIMARY KEY,
    full_name VARCHAR(100) NOT NULL,
    username VARCHAR(50) NOT NULL UNIQUE,
    email VARCHAR(100) NOT NULL UNIQUE,
    password_hash VARCHAR(255) NOT NULL,
    created_at DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
    login_time DATETIME NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- ----------------------------------------------------------------
-- 2. LOGIN HISTORY — one row per sign-in
-- ----------------------------------------------------------------
CREATE TABLE IF NOT EXISTS login_history (
    id INT AUTO_INCREMENT PRIMARY KEY,
    user_id INT NOT NULL,
    logged_in_at DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
    CONSTRAINT fk_login_history_user
        FOREIGN KEY (user_id) REFERENCES users(id)
        ON DELETE CASCADE,
    INDEX idx_login_history_user (user_id, logged_in_at)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- ----------------------------------------------------------------
-- 3. SIMULATIONS — one row per completed simulation run
-- ----------------------------------------------------------------
CREATE TABLE IF NOT EXISTS simulations (
    id INT AUTO_INCREMENT PRIMARY KEY,
    user_id INT NOT NULL,
    module VARCHAR(64) NOT NULL,
    algorithm VARCHAR(64) NOT NULL,
    summary VARCHAR(255) NOT NULL,
    input_data LONGTEXT NOT NULL,
    output_data LONGTEXT NOT NULL,
    payload LONGTEXT NOT NULL,
    created_at DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
    CONSTRAINT fk_simulations_user
        FOREIGN KEY (user_id) REFERENCES users(id)
        ON DELETE CASCADE,
    INDEX idx_simulations_user_time (user_id, created_at),
    INDEX idx_simulations_module (user_id, module)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- ----------------------------------------------------------------
-- 4. COMPARISONS — saves side-by-side algorithm comparisons
-- ----------------------------------------------------------------
CREATE TABLE IF NOT EXISTS comparisons (
    id INT AUTO_INCREMENT PRIMARY KEY,
    user_id INT NOT NULL,
    module VARCHAR(64) NOT NULL,
    title VARCHAR(255) NOT NULL,
    comparison_data LONGTEXT NOT NULL,
    created_at DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
    CONSTRAINT fk_comparisons_user
        FOREIGN KEY (user_id) REFERENCES users(id)
        ON DELETE CASCADE,
    INDEX idx_comparisons_user_time (user_id, created_at)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- ----------------------------------------------------------------
-- 5. COMPARISON ITEMS — each algorithm run in a comparison
-- ----------------------------------------------------------------
CREATE TABLE IF NOT EXISTS comparison_items (
    id INT AUTO_INCREMENT PRIMARY KEY,
    comparison_id INT NOT NULL,
    algorithm VARCHAR(64) NOT NULL,
    metrics_json LONGTEXT NOT NULL,
    simulation_id INT NULL,
    CONSTRAINT fk_comparison_items_comparison
        FOREIGN KEY (comparison_id) REFERENCES comparisons(id)
        ON DELETE CASCADE,
    CONSTRAINT fk_comparison_items_simulation
        FOREIGN KEY (simulation_id) REFERENCES simulations(id)
        ON DELETE SET NULL,
    INDEX idx_comparison_items (comparison_id)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
