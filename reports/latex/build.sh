#!/usr/bin/env bash
set -euo pipefail

# Короткая точка входа; вся логика сборки находится в scripts/.
script_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
exec "$script_dir/scripts/build.sh" "$@"
