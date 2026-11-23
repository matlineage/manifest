#!/bin/bash
set -e

# Must be called after breakfast/lunch.
namespaces="$(get_build_var PRODUCT_SOONG_NAMESPACES)"
read -r -a namespaces_array <<< "$namespaces"

is_active_namespace() {
    local path="$1"
    local ns

    for ns in "${namespaces_array[@]}"; do
        [[ "$ns" == "$path" ]] && return 0
    done

    return 1
}

candidates=()
active_count=0

# Find conventional device-specific vendor directories.
for dir in vendor/*/*; do
    [[ -d "$dir" ]] || continue

    rest="${dir#vendor/}"
    oem="${rest%%/*}"
    device="${rest#*/}"

    # Only consider vendor trees with a matching device tree.
    [[ -d "device/$oem/$device" ]] || continue

    candidates+=("$dir")

    if is_active_namespace "$dir"; then
        active_count=$((active_count + 1))
    fi
done

# Fail safely if we cannot identify the active vendor tree.
if (( active_count == 0 )); then
    echo "ERROR: Could not identify active vendor tree."
    echo "PRODUCT_SOONG_NAMESPACES=$namespaces"
    return 1 2>/dev/null || exit 1
fi

markers=()

cleanup() {
    for marker in "${markers[@]}"; do
        rm -f "$marker"
    done
}

trap cleanup EXIT INT TERM

# Exclude inactive device vendor trees.
for dir in "${candidates[@]}"; do
    is_active_namespace "$dir" && continue

    marker="$dir/.find-ignore"

    if [[ -e "$marker" ]]; then
        echo "ERROR: Existing marker: $marker"
        exit 1
    fi

    touch "$marker"
    markers+=("$marker")

    echo "Excluding $dir"
done
