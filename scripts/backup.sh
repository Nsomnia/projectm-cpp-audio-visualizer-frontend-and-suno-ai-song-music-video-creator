#!/bin/bash

# --- Configuration ---
BACKUP_ROOT="backups"
GENERAL_DIR="${BACKUP_ROOT}/general"
SUCCESS_DIR="${BACKUP_ROOT}/successful"
GENERAL_KEEP_COUNT=10
SUCCESS_KEEP_COUNT=5
TIMESTAMP=$(date +%Y%m%d-%H%M%S)

# --- Ensure directories exist ---
mkdir -p "${GENERAL_DIR}"
mkdir -p "${SUCCESS_DIR}"

# --- Functions ---
create_general_backup() {
    local filename="${TIMESTAMP}.tar.gz"
    echo "--- Creating pre-build backup: ${filename} ---"
    tar --exclude='./build' \
        --exclude='./backups' \
        --exclude='./.git' \
        --exclude='*.swp' \
        --exclude='*.swo' \
        -czf "${GENERAL_DIR}/${filename}" .
}

promote_to_successful() {
    local latest_general_backup
    latest_general_backup=$(ls -1t "${GENERAL_DIR}" | head -n 1)

    if [[ -n "${latest_general_backup}" ]]; then
        echo "--- Promoting backup to successful: ${latest_general_backup} ---"
        cp "${GENERAL_DIR}/${latest_general_backup}" "${SUCCESS_DIR}/"
    fi
}

rotate_backups() {
    local dir=$1
    local keep_count=$2
    
    # List files by time (newest first), skip the ones to keep, delete the rest.
    ls -1t "${dir}" | tail -n +$((keep_count + 1)) | while read -r file_to_delete; do
        rm -f "${dir}/${file_to_delete}"
    done
}


# --- Main Logic ---
case "$1" in
    pre-build)
        create_general_backup
        rotate_backups "${GENERAL_DIR}" ${GENERAL_KEEP_COUNT}
        ;;
    post-success)
        promote_to_successful
        rotate_backups "${SUCCESS_DIR}" ${SUCCESS_KEEP_COUNT}
        ;;
    *)
        echo "Usage: $0 {pre-build|post-success}"
        exit 1
        ;;
esac