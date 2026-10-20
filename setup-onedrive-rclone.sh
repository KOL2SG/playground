#!/bin/bash

set -e

echo "=== RClone OneDrive Setup (Bosch) ==="

# Check dependencies
if ! command -v rclone &> /dev/null; then
  echo "ERROR: rclone is not installed."
  exit 1
fi

# Create config directory
CONFIG_DIR="$HOME/.config/rclone"
CONFIG_FILE="$CONFIG_DIR/rclone.conf"

mkdir -p "$CONFIG_DIR"

echo "Creating rclone config at $CONFIG_FILE"

# Prompt user for credentials
CLIENT_ID="65fa34a4-f93e-4857-8ca1-2155910141db"
TENANT_ID="0ae51e19-07c8-4e4b-bb6d-648ee58410f4"

# Write config file
cat > "$CONFIG_FILE" <<EOF
[BoschOneDrive]
type = onedrive
client_id = $CLIENT_ID
access_scopes = Files.ReadWrite.All offline_access
tenant = $TENANT_ID
EOF

echo "Config file created."

# Initialize rclone
echo "Running rclone config..."
rclone config

# Create mount directory
MOUNT_POINT="$HOME/onedrive"
mkdir -p "$MOUNT_POINT"

echo "Mounting OneDrive to $MOUNT_POINT..."

rclone mount BoschOneDrive: "$MOUNT_POINT" --vfs-cache-mode writes --daemon

echo "✅ OneDrive mounted at $MOUNT_POINT"
echo "Run: ls $MOUNT_POINT to verify"
