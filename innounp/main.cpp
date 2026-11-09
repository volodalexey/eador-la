// innounp <installer.exe> <out_dir> — распаковка установщика GOG Eador: Genesis.
#include "innounp.h"

#include <cstdio>
#include <exception>

int main(int argc, char **argv)
{
	if (argc != 3) {
		std::fprintf(stderr, "usage: %s <setup_eador_genesis_*.exe> <out_dir>\n", argv[0]);
		return 2;
	}
	try {
		const innounp::Summary s = innounp::extract(argv[1], argv[2]);
		std::printf("innounp: %zu files, %llu bytes", s.files, (unsigned long long)s.bytes);
		if (s.skipped_filtered)
			std::printf(", %zu executables skipped", s.skipped_filtered);
		std::printf("\n");
	} catch (const std::exception &e) {
		std::fprintf(stderr, "innounp: %s\n", e.what());
		return 1;
	}
	return 0;
}
