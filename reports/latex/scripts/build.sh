#!/usr/bin/env bash
set -euo pipefail

# Работает и из корня проекта, и при запуске из любой другой папки.
report_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$report_dir"
mkdir -p build/work build/pdf
export XDG_CACHE_HOME="${XDG_CACHE_HOME:-$report_dir/build/cache}"

report_filename="${1:-308_Храмов_Лаб1.pdf}"
case "$report_filename" in
  */*|''|.|..) echo 'Укажите только имя PDF, без пути.' >&2; exit 2 ;;
  *.pdf) ;;
  *) echo 'Имя результата должно оканчиваться на .pdf.' >&2; exit 2 ;;
esac

tectonic_command="${TECTONIC:-tectonic}"
if [[ -z "${TECTONIC:-}" ]] && ! command -v tectonic >/dev/null 2>&1 && [[ -x tools/tectonic ]]; then
  tectonic_command="$report_dir/tools/tectonic"
fi

if command -v "$tectonic_command" >/dev/null 2>&1; then
  "$tectonic_command" --keep-logs --keep-intermediates --outdir build/work main.tex
elif command -v latexmk >/dev/null 2>&1 && command -v xelatex >/dev/null 2>&1; then
  latexmk -xelatex -interaction=nonstopmode -halt-on-error -outdir=build/work main.tex
elif command -v xelatex >/dev/null 2>&1; then
  xelatex -interaction=nonstopmode -halt-on-error -output-directory=build/work main.tex
  xelatex -interaction=nonstopmode -halt-on-error -output-directory=build/work main.tex
else
  echo 'Нужен Tectonic либо XeLaTeX. Инструкция: reports/latex/README.md.' >&2
  exit 127
fi

cp -- build/work/main.pdf "build/pdf/$report_filename"
printf 'Готово: %s/build/pdf/%s\n' "$report_dir" "$report_filename"
