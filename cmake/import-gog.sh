#!/bin/sh
# Импорт ресурсов оригинальной игры (GOG) в каталог запуска.
#
#   import-gog.sh --exe  <setup_eador_genesis_*.exe> <runtime_dir> <innoextract>
#   import-gog.sh --game <установленная копия "Eador - Genesis"> <runtime_dir>
#
# Копирует dat/, text/, var/ с исходным регистром имён файлов (именно так
# они записаны в коде). Английский установщик называет каталог "Text/" —
# он приводится к "text/". Ресурсы заменяются при каждом импорте;
# save/, screens/ и eador.cfg не трогаются.
set -eu

usage() {
	echo "usage: $0 --exe <installer.exe> <runtime_dir> <innoextract>" >&2
	echo "       $0 --game <game_dir> <runtime_dir>" >&2
	exit 1
}

[ $# -ge 3 ] || usage
mode=$1
src=$2
runtime=$3

mkdir -p "$runtime"
tmp=""
cleanup() {
	if [ -n "$tmp" ]; then
		rm -rf "$tmp"
	fi
}
trap cleanup EXIT INT TERM

case "$mode" in
--exe)
	[ $# -eq 4 ] || usage
	[ -f "$src" ] || { echo "import: нет файла установщика '$src'" >&2; exit 1; }
	tmp=$(mktemp -d "$runtime/.gog_extract.XXXXXX")
	"$4" --extract --silent --output-dir "$tmp" "$src"
	game=$tmp
	;;
--game)
	[ -d "$src" ] || { echo "import: нет каталога игры '$src'" >&2; exit 1; }
	game=$src
	;;
*)
	usage
	;;
esac

for dir in dat text var; do
	real=$(find "$game" -mindepth 1 -maxdepth 1 -type d -iname "$dir" | head -n 1)
	if [ -z "$real" ]; then
		echo "import: в '$src' нет каталога $dir/ — это не копия Eador: Genesis" >&2
		exit 1
	fi
	rm -rf "${runtime:?}/$dir"
	cp -R "$real" "$runtime/$dir"
done

if [ ! -f "$runtime/dat/Interface.dat" ]; then
	echo "import: '$src' не похож на Eador: Genesis (нет dat/Interface.dat)" >&2
	exit 1
fi

# Шрифты лежат в корне игры.
for f in font01.pcx font01.txt font02.pcx font02.txt; do
	real=$(find "$game" -mindepth 1 -maxdepth 1 -type f -iname "$f" | head -n 1)
	if [ -z "$real" ]; then
		echo "import: в '$src' нет $f" >&2
		exit 1
	fi
	cp "$real" "$runtime/$f"
done

# Иконка окна: goggame-*.ico из корня игры (в нём есть 32-битные BMP-образы).
ico=$(find "$game" -mindepth 1 -maxdepth 1 -type f -iname 'goggame-*.ico' | head -n 1)
if [ -n "$ico" ]; then
	cp "$ico" "$runtime/eador.ico"
else
	echo "import: в '$src' нет goggame-*.ico, окно будет без иконки" >&2
fi

mkdir -p "$runtime/save" "$runtime/screens"
echo "import: ресурсы скопированы в $runtime"
