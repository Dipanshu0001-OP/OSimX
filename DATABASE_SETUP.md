# OSimX Database Setup Guide

## Overview

OSimX stores **all** user and simulation data in a relational database:

| Table               | Purpose                                        |
|---------------------|------------------------------------------------|
| `users`             | User accounts (signup/login credentials)       |
| `login_history`     | Every sign-in timestamp per user               |
| `simulations`       | Each completed simulation (input + output + full payload) |
| `comparisons`       | Saved algorithm comparisons                    |
| `comparison_items`  | Individual algorithm entries within a comparison |

## Backend Selection

OSimX supports **MySQL** and **SQLite** (fallback).

### Using MySQL (recommended for production)

1. Install MySQL 8.0+ and start the server.

2. Run the schema file:
   ```bash
   mysql -u root -p < osimx_db.sql
   ```

3. Set environment variables before launching OSimX:
   ```bash
   export OSIMX_DB_BACKEND=mysql
   export OSIMX_DB_HOST=127.0.0.1
   export OSIMX_DB_PORT=3306
   export OSIMX_DB_NAME=osimx_db
   export OSIMX_DB_USER=root
   export OSIMX_DB_PASSWORD=yourpassword
   ```

4. Build and run:
   ```bash
   mkdir build && cd build
   cmake .. && make -j$(nproc)
   ./osimx
   ```

### Using SQLite (default, zero-config)

Simply build and run. OSimX will create `osimx.sqlite` automatically in your app data directory.

```bash
mkdir build && cd build
cmake .. && make -j$(nproc)
./osimx
```

## What Gets Saved

### User Authentication
- Full name, username, email, salted+hashed password
- Account creation timestamp
- Last login timestamp (updated on every sign-in)
- Complete login history (every sign-in recorded)

### Simulation I/O History
Every time you click "Save Simulation", the database stores:
- **input_data**: The processes/requests/parameters you entered
- **output_data**: The computed metrics, timeline, and results
- **payload**: The complete JSON snapshot (for full replay)
- Module name, algorithm name, and a human-readable summary

### Algorithm Comparisons
When you save a comparison, the database stores:
- The module and title
- Full comparison data (all algorithms side by side)
- Individual algorithm metrics linked to their simulation runs

## Schema Auto-Migration

The app automatically creates all tables on first run and adds new columns to older databases without data loss.
