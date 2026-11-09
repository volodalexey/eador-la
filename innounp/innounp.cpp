// Порт src/formats/inno.zig (zig-eador): Inno Setup 5.5.7 (unicode), один .exe,
// без шифрования; блоки заголовка stored/LZMA1, чанки файлов stored/LZMA1/LZMA2.
// Многочастные файлы GOG Galaxy (части сжаты zlib) склеиваются, как в innoextract.
#include "innounp.h"

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <stdexcept>
#include <unordered_map>
#include <vector>

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <zlib.h>

extern "C" {
#include "lzma/Lzma2Dec.h"
#include "lzma/LzmaDec.h"
}

namespace fs = std::filesystem;

namespace innounp {
namespace {

[[noreturn]] void fail(const std::string &what) { throw std::runtime_error(what); }

struct Bytes {
	const uint8_t *ptr = nullptr;
	size_t len = 0;
};

// Установщик целиком отображается в память (на Android так меньше RSS, чем при чтении).
class Mapped {
public:
	explicit Mapped(const std::string &path) {
		fd_ = ::open(path.c_str(), O_RDONLY | O_CLOEXEC);
		if (fd_ < 0)
			fail("cannot open '" + path + "'");
		struct stat st {};
		if (::fstat(fd_, &st) != 0 || st.st_size <= 0)
			fail("cannot read '" + path + "'");
		len_ = size_t(st.st_size);
		void *p = ::mmap(nullptr, len_, PROT_READ, MAP_PRIVATE, fd_, 0);
		if (p == MAP_FAILED)
			fail("cannot map '" + path + "'");
		ptr_ = static_cast<const uint8_t *>(p);
	}
	~Mapped() {
		if (ptr_)
			::munmap(const_cast<uint8_t *>(ptr_), len_);
		if (fd_ >= 0)
			::close(fd_);
	}
	Mapped(const Mapped &) = delete;
	Mapped &operator=(const Mapped &) = delete;
	Bytes bytes() const { return {ptr_, len_}; }

private:
	int fd_ = -1;
	const uint8_t *ptr_ = nullptr;
	size_t len_ = 0;
};

uint32_t le32(const uint8_t *p) {
	return uint32_t(p[0]) | uint32_t(p[1]) << 8 | uint32_t(p[2]) << 16 | uint32_t(p[3]) << 24;
}

struct Cursor {
	Bytes buf;
	size_t pos = 0;

	const uint8_t *take(size_t n) {
		if (pos > buf.len || buf.len - pos < n)
			fail("unexpected end of installer data");
		const uint8_t *p = buf.ptr + pos;
		pos += n;
		return p;
	}
	void skip(size_t n) { take(n); }
	uint16_t u16() {
		const uint8_t *p = take(2);
		return uint16_t(p[0] | p[1] << 8);
	}
	uint32_t u32() { return le32(take(4)); }
	uint64_t u64() {
		const uint8_t *p = take(8);
		return uint64_t(le32(p)) | uint64_t(le32(p + 4)) << 32;
	}
	// Строка с 32-битной длиной (UTF-16LE в unicode-установщиках).
	Bytes str() {
		const uint32_t n = u32();
		return {take(n), n};
	}
};

uint32_t readAt32(Bytes b, size_t off) {
	Cursor c{b, off};
	return c.u32();
}
uint16_t readAt16(Bytes b, size_t off) {
	Cursor c{b, off};
	return c.u16();
}

uint32_t crc32of(const uint8_t *p, size_t n) {
	return uint32_t(::crc32(0L, p, uInt(n)));
}

// SHA-1 для проверки содержимого файлов.
class Sha1 {
public:
	void update(const uint8_t *p, size_t n) {
		total_ += n;
		while (n > 0) {
			const size_t k = std::min(n, size_t(64) - fill_);
			std::memcpy(block_ + fill_, p, k);
			fill_ += k;
			p += k;
			n -= k;
			if (fill_ == 64) {
				compress();
				fill_ = 0;
			}
		}
	}
	void finish(uint8_t out[20]) {
		const uint64_t bits = total_ * 8;
		const uint8_t pad = 0x80;
		update(&pad, 1);
		const uint8_t zero = 0;
		while (fill_ != 56)
			update(&zero, 1);
		uint8_t len[8];
		for (int i = 0; i < 8; ++i)
			len[i] = uint8_t(bits >> (56 - 8 * i));
		update(len, 8);
		for (int i = 0; i < 5; ++i)
			for (int j = 0; j < 4; ++j)
				out[4 * i + j] = uint8_t(h_[i] >> (24 - 8 * j));
	}

private:
	static uint32_t rol(uint32_t v, int s) { return v << s | v >> (32 - s); }
	void compress() {
		uint32_t w[80];
		for (int i = 0; i < 16; ++i)
			w[i] = uint32_t(block_[4 * i]) << 24 | uint32_t(block_[4 * i + 1]) << 16 |
			       uint32_t(block_[4 * i + 2]) << 8 | block_[4 * i + 3];
		for (int i = 16; i < 80; ++i)
			w[i] = rol(w[i - 3] ^ w[i - 8] ^ w[i - 14] ^ w[i - 16], 1);
		uint32_t a = h_[0], b = h_[1], c = h_[2], d = h_[3], e = h_[4];
		for (int i = 0; i < 80; ++i) {
			uint32_t f, k;
			if (i < 20) {
				f = (b & c) | (~b & d);
				k = 0x5A827999;
			} else if (i < 40) {
				f = b ^ c ^ d;
				k = 0x6ED9EBA1;
			} else if (i < 60) {
				f = (b & c) | (b & d) | (c & d);
				k = 0x8F1BBCDC;
			} else {
				f = b ^ c ^ d;
				k = 0xCA62C1D6;
			}
			const uint32_t t = rol(a, 5) + f + e + k + w[i];
			e = d;
			d = c;
			c = rol(b, 30);
			b = a;
			a = t;
		}
		h_[0] += a;
		h_[1] += b;
		h_[2] += c;
		h_[3] += d;
		h_[4] += e;
	}

	uint32_t h_[5] = {0x67452301, 0xEFCDAB89, 0x98BADCFE, 0x10325476, 0xC3D2E1F0};
	uint8_t block_[64] = {};
	size_t fill_ = 0;
	uint64_t total_ = 0;
};

std::string utf8(Bytes raw) {
	if (raw.len % 2 != 0)
		fail("corrupt installer header (odd UTF-16 string)");
	std::string out;
	out.reserve(raw.len / 2);
	for (size_t i = 0; i < raw.len; i += 2) {
		uint32_t cp = uint32_t(raw.ptr[i] | raw.ptr[i + 1] << 8);
		if (cp >= 0xD800 && cp < 0xDC00 && i + 3 < raw.len) {
			const uint32_t lo = uint32_t(raw.ptr[i + 2] | raw.ptr[i + 3] << 8);
			if (lo >= 0xDC00 && lo < 0xE000) {
				cp = 0x10000 + ((cp - 0xD800) << 10) + (lo - 0xDC00);
				i += 2;
			}
		}
		if (cp < 0x80) {
			out += char(cp);
		} else if (cp < 0x800) {
			out += char(0xC0 | cp >> 6);
			out += char(0x80 | (cp & 0x3F));
		} else if (cp < 0x10000) {
			out += char(0xE0 | cp >> 12);
			out += char(0x80 | (cp >> 6 & 0x3F));
			out += char(0x80 | (cp & 0x3F));
		} else {
			out += char(0xF0 | cp >> 18);
			out += char(0x80 | (cp >> 12 & 0x3F));
			out += char(0x80 | (cp >> 6 & 0x3F));
			out += char(0x80 | (cp & 0x3F));
		}
	}
	return out;
}

std::string lower(std::string s) {
	for (char &ch : s)
		ch = char(std::tolower(static_cast<unsigned char>(ch)));
	return s;
}

void *szAlloc(ISzAllocPtr, size_t n) { return std::malloc(n); }
void szFree(ISzAllocPtr, void *p) { std::free(p); }
const ISzAlloc kAlloc = {szAlloc, szFree};

// Inno LZMA1: 5 байт свойств, затем поток без размера и обычно без маркера конца.
// want == 0: распаковать весь вход (блоки заголовка), иначе ровно want байт.
std::vector<uint8_t> lzma1Decode(Bytes src, size_t want) {
	if (src.len < 5 || src.ptr[0] >= 9 * 5 * 5)
		fail("corrupt LZMA data");
	CLzmaDec dec;
	LzmaDec_CONSTRUCT(&dec);
	if (LzmaDec_Allocate(&dec, src.ptr, 5, &kAlloc) != SZ_OK)
		fail("out of memory");
	LzmaDec_Init(&dec);
	std::vector<uint8_t> out(want ? want : std::max<size_t>(src.len * 4, 1 << 16));
	size_t outPos = 0, inPos = 5;
	SRes res = SZ_OK;
	ELzmaStatus status = LZMA_STATUS_NOT_SPECIFIED;
	while (true) {
		if (outPos == out.size()) {
			if (want)
				break;
			out.resize(out.size() * 2);
		}
		size_t outLen = out.size() - outPos;
		size_t inLen = src.len - inPos;
		res = LzmaDec_DecodeToBuf(&dec, out.data() + outPos, &outLen, src.ptr + inPos, &inLen,
		                          LZMA_FINISH_ANY, &status);
		outPos += outLen;
		inPos += inLen;
		if (res != SZ_OK || status == LZMA_STATUS_FINISHED_WITH_MARK)
			break;
		if (inLen == 0 && outLen == 0)
			break;  // вход исчерпан
	}
	LzmaDec_Free(&dec, &kAlloc);
	if (res != SZ_OK || (want && outPos < want))
		fail("corrupt LZMA data");
	out.resize(outPos);
	return out;
}

// Inno LZMA2: байт размера словаря, затем обычный поток чанков LZMA2.
std::vector<uint8_t> lzma2Decode(Bytes src, size_t want) {
	if (src.len < 1 || src.ptr[0] > 40)
		fail("corrupt LZMA2 data");
	CLzma2Dec dec;
	Lzma2Dec_CONSTRUCT(&dec);
	if (Lzma2Dec_Allocate(&dec, src.ptr[0], &kAlloc) != SZ_OK)
		fail("out of memory");
	Lzma2Dec_Init(&dec);
	std::vector<uint8_t> out(want);
	size_t outLen = want, inLen = src.len - 1;
	ELzmaStatus status = LZMA_STATUS_NOT_SPECIFIED;
	const SRes res = Lzma2Dec_DecodeToBuf(&dec, out.data(), &outLen, src.ptr + 1, &inLen,
	                                      LZMA_FINISH_ANY, &status);
	Lzma2Dec_Free(&dec, &kAlloc);
	if (res != SZ_OK || outLen < want)
		fail("corrupt LZMA2 data");
	return out;
}

std::vector<uint8_t> inflateZlib(Bytes src, uint64_t size) {
	std::vector<uint8_t> out(static_cast<size_t>(size));
	uLongf outLen = uLongf(size);
	if (::uncompress(out.data(), &outLen, src.ptr, uLong(src.len)) != Z_OK || outLen != size)
		fail("corrupt zlib data");
	return out;
}

struct Loader {
	uint32_t header_offset;
	uint32_t data_offset;
};

uint32_t rvaToFile(Bytes exe, size_t sections, uint16_t n, uint32_t rva) {
	for (uint16_t k = 0; k < n; ++k) {
		const size_t s = sections + size_t(k) * 40;
		const uint32_t vsize = readAt32(exe, s + 8);
		const uint32_t va = readAt32(exe, s + 12);
		if (rva >= va && rva - va < vsize)
			return rva - va + readAt32(exe, s + 20);
	}
	fail("not an Inno Setup installer");
}

uint32_t resourceEntry(Bytes exe, size_t dir, uint32_t id) {
	const uint16_t names = readAt16(exe, dir + 12);
	const uint16_t ids = readAt16(exe, dir + 14);
	for (uint32_t k = 0; k < ids; ++k) {
		const size_t e = dir + 16 + (size_t(names) + k) * 8;
		if (readAt32(exe, e) == id)
			return readAt32(exe, e + 4);
	}
	fail("not an Inno Setup installer");
}

// Начиная с 5.1.5 таблица загрузчика лежит в PE-ресурсе RT_RCDATA (10) / 11111.
size_t findResource(Bytes exe, uint32_t type, uint32_t name) {
	const uint32_t pe = readAt32(exe, 0x3c);
	if (readAt32(exe, pe) != 0x4550)
		fail("not a Windows executable");
	const uint16_t nsections = readAt16(exe, pe + 6);
	const size_t opt = size_t(pe) + 24;
	const size_t sections = opt + readAt16(exe, pe + 20);
	const size_t dirs = opt + (readAt16(exe, opt) == 0x20b ? 112 : 96);
	if (readAt32(exe, dirs - 4) < 3)
		fail("not an Inno Setup installer");
	const size_t root = rvaToFile(exe, sections, nsections, readAt32(exe, dirs + 16));
	const uint32_t t = resourceEntry(exe, root, type);
	if (!(t & 0x80000000u))
		fail("not an Inno Setup installer");
	const uint32_t nm = resourceEntry(exe, root + (t & 0x7fffffffu), name);
	if (!(nm & 0x80000000u))
		fail("not an Inno Setup installer");
	// Первый языковой элемент, он должен быть листом.
	const uint32_t leaf = readAt32(exe, root + (nm & 0x7fffffffu) + 16 + 4);
	if (leaf & 0x80000000u)
		fail("not an Inno Setup installer");
	return rvaToFile(exe, sections, nsections, readAt32(exe, root + leaf));
}

Loader findLoader(Bytes exe) {
	const size_t off = findResource(exe, 10, 11111);
	Cursor c{exe, off};
	const uint8_t *magic = c.take(12);
	if (std::memcmp(magic, "rDlPtS\xcd\xe6\xd7{\x0b*", 12) != 0 &&
	    std::memcmp(magic, "nS5W7dT\x83\xaa\x1b\x0fj", 12) != 0)
		fail("not an Inno Setup installer");
	// revision, ?, смещение exe, размер exe, crc exe, смещение заголовка, смещение данных, crc
	const uint8_t *fields = c.take(7 * 4);
	if (crc32of(exe.ptr + off, 12 + 28) != c.u32())
		fail("corrupt installer (loader checksum)");
	return {le32(fields + 20), le32(fields + 24)};
}

void checkVersion(const uint8_t *id) {
	const std::string name(reinterpret_cast<const char *>(id), strnlen(reinterpret_cast<const char *>(id), 64));
	if (name != "Inno Setup Setup Data (5.5.7) (u)" && name != "Inno Setup Setup Data (5.5.7) (U)")
		fail("unsupported installer version '" + name + "' (only GOG Eador: Genesis, Inno Setup 5.5.7)");
}

// Блок заголовка: {размер, флаг сжатия} под CRC, затем куски по 4096 байт,
// каждый с CRC32 впереди; склейка кусков при флаге сжата LZMA1.
std::vector<uint8_t> readBlock(Bytes exe, size_t &pos) {
	Cursor c{exe, pos};
	const uint32_t crc = c.u32();
	const uint8_t *prefix = c.take(5);
	if (crc32of(prefix, 5) != crc)
		fail("corrupt installer header");
	const uint32_t stored = le32(prefix);
	Cursor body{{c.take(stored), stored}};
	pos = c.pos;

	std::vector<uint8_t> joined;
	joined.reserve(stored);
	while (body.pos < body.buf.len) {
		const uint32_t pieceCrc = body.u32();
		const size_t n = std::min<size_t>(4096, body.buf.len - body.pos);
		const uint8_t *piece = body.take(n);
		if (crc32of(piece, n) != pieceCrc)
			fail("corrupt installer header");
		joined.insert(joined.end(), piece, piece + n);
	}
	if (prefix[4] == 0)
		return joined;
	return lzma1Decode({joined.data(), joined.size()}, 0);
}

void skipEntries(Cursor &c, uint32_t count, int strings, size_t fixed) {
	for (uint32_t i = 0; i < count; ++i) {
		for (int k = 0; k < strings; ++k)
			c.str();
		c.skip(fixed);
	}
}

struct File {
	std::string dest;
	uint32_t location = 0;
	std::string after, before;
	std::vector<uint32_t> parts;  // части GOG Galaxy, дописываемые к файлу
	bool continuation = false;
};

struct Data {
	uint32_t first_slice = 0, last_slice = 0, chunk_offset = 0;
	uint64_t file_offset = 0, size = 0, chunk_size = 0;
	uint8_t sha1[20] = {};
	uint16_t flags = 0;
	uint64_t unpacked = 0;  // размер после zlib у части GOG Galaxy, иначе size
	bool zlib = false;

	// Номера битов в наборе флагов data entry 5.5.7.
	static constexpr uint16_t call_instruction_optimized = 1 << 4;
	static constexpr uint16_t chunk_encrypted = 1 << 6;
	static constexpr uint16_t chunk_compressed = 1 << 7;
};

// Разбирает `name(arg, 'строка ''в кавычках''', ...)`; false, если вызывается другое.
bool parseCall(const std::string &code, const char *name, std::vector<std::string> &args) {
	static const char *ws = " \t\r\n";
	static const char *sep = " \t\r\n(),'";
	args.clear();
	size_t p = code.find_first_not_of(ws);
	if (p == std::string::npos)
		return false;
	const size_t end = code.find_first_of(sep, p);
	if (end == std::string::npos)
		return false;
	const size_t paren = code.find_first_not_of(ws, end);
	if (paren == std::string::npos || code[paren] != '(' || code.compare(p, end - p, name) != 0)
		return false;
	p = paren + 1;
	while (true) {
		p = code.find_first_not_of(ws, p);
		if (p == std::string::npos)
			break;
		if (code[p] == '\'') {
			std::string s;
			++p;
			while (true) {
				size_t q = code.find('\'', p);
				if (q == std::string::npos)
					q = code.size();
				s.append(code, p, q - p);
				p = std::min(q + 1, code.size());
				if (p < code.size() && code[p] == '\'') {
					s += '\'';
					++p;
				} else {
					break;
				}
			}
			args.push_back(s);
		} else {
			size_t q = code.find_first_of(sep, p);
			if (q == std::string::npos)
				q = code.size();
			args.push_back(code.substr(p, q - p));
			p = q;
		}
		p = code.find_first_not_of(ws, p);
		if (p == std::string::npos || code[p] != ',')
			break;
		++p;
	}
	return true;
}

uint64_t parseU64(const std::string &s, uint64_t fallback) {
	if (s.empty())
		return fallback;
	char *end = nullptr;
	const unsigned long long v = std::strtoull(s.c_str(), &end, 10);
	return (end && *end == 0) ? uint64_t(v) : fallback;
}

// GOG Galaxy: before_install('md5', 'имя', частей) начинает файл, его части идут
// следом с after_install('md5', сжатый, полный) и данными в zlib (cli/goggalaxy.cpp).
void joinGalaxyParts(std::vector<File> &files, std::vector<Data> &data) {
	File *start = nullptr;
	size_t remaining = 0;
	std::vector<std::string> args;
	for (File &f : files) {
		const bool begin = parseCall(f.before, "before_install", args) ||
		                   parseCall(f.before, "before_install_dependency", args);
		if (begin) {
			if (args.size() >= 2 && !args[1].empty())
				f.dest = args[1];
			remaining = 1;
			if (args.size() >= 3)
				remaining = std::max<uint64_t>(1, parseU64(args[2], 1));
			start = &f;
		}
		if (parseCall(f.after, "after_install", args) ||
		    parseCall(f.after, "after_install_dependency", args)) {
			if (remaining == 0 || f.location >= data.size() || args.size() < 3) {
				remaining = 0;
				continue;
			}
			--remaining;
			Data &e = data[f.location];
			if (args[2].empty() || parseU64(args[2], UINT64_MAX) == UINT64_MAX)
				fail("corrupt installer header");
			e.unpacked = parseU64(args[2], 0);
			e.zlib = true;
			if (start != &f) {
				f.continuation = true;
				start->parts.push_back(f.location);
			}
		} else if (!begin) {
			remaining = 0;
		}
	}
}

char safeChar(char ch) {
	return (static_cast<unsigned char>(ch) < 32 || std::strchr("<>:\"|?*", ch)) ? '$' : ch;
}

// Имена как у innoextract по умолчанию: `{const}` → `const`, `{{` → `{`,
// недопустимые символы → `$`, `\`, `/`, `.`, `..` нормализуются.
std::string convertPath(const std::string &path) {
	std::string expanded;
	for (size_t k = 0; k < path.size(); ++k) {
		const char ch = path[k];
		if (ch == '{') {
			if (k + 1 < path.size() && path[k + 1] == '{') {
				expanded += '{';
				++k;
				continue;
			}
			size_t close = path.find('}', k + 1);
			if (close == std::string::npos)
				close = path.size();
			for (size_t v = k + 1; v < close; ++v)
				expanded += safeChar(path[v]);
			k = close;
		} else {
			expanded += ch == '}' ? ch : safeChar(ch);
		}
	}
	std::string result;
	size_t p = 0;
	while (p <= expanded.size()) {
		size_t q = expanded.find_first_of("\\/", p);
		if (q == std::string::npos)
			q = expanded.size();
		const std::string seg = expanded.substr(p, q - p);
		p = q + 1;
		if (seg.empty() || seg == ".")
			continue;
		if (seg == "..") {
			const size_t slash = result.rfind('/');
			result.resize(slash == std::string::npos ? 0 : slash);
			continue;
		}
		if (!result.empty())
			result += '/';
		result += seg;
	}
	return result;
}

struct Output {
	std::string path;
	std::vector<uint32_t> locs;
	uint64_t size;
};

void writeAt(const fs::path &path, uint64_t offset, const uint8_t *p, size_t n) {
	FILE *f = std::fopen(path.c_str(), "r+b");
	if (!f)
		fail("cannot write '" + path.string() + "'");
	const bool ok = fseeko(f, off_t(offset), SEEK_SET) == 0 && std::fwrite(p, 1, n, f) == n;
	if (std::fclose(f) != 0 || !ok)
		fail("cannot write '" + path.string() + "'");
}

} // namespace

Summary extract(const std::string &exe_path, const std::string &out_dir, const Progress &progress) {
	const Mapped mapped(exe_path);
	const Bytes exe = mapped.bytes();

	const Loader loader = findLoader(exe);
	Cursor c{exe, loader.header_offset};
	checkVersion(c.take(64));
	size_t pos = c.pos;
	const std::vector<uint8_t> setup = readBlock(exe, pos);
	const std::vector<uint8_t> dataBlock = readBlock(exe, pos);

	// Главный заголовок.
	Cursor h{{setup.data(), setup.size()}};
	bool gog = false;
	for (int i = 0; i < 32; ++i) {
		const Bytes s = h.str();
		// app_publisher, app_publisher_url, app_support_url, app_updates_url
		if (i == 4 || i == 5 || i == 7 || i == 8) {
			const std::string u = lower(utf8(s));
			if (u.find(i == 4 ? "gog.com" : "www.gog.com") != std::string::npos)
				gog = true;
		}
	}
	uint32_t counts[16];
	for (uint32_t &n : counts)
		n = h.u32();
	// Фиксированная часть: метод сжатия на 74, 47 флагов (6 байт) с 87;
	// EncryptionUsed — флаг 37.
	const uint8_t *rest = h.take(93);
	const uint8_t compression = rest[74];
	if (rest[87 + 4] & 0x20)
		fail("encrypted installers are not supported");

	// Разделы в порядке потока, сохраняются только файлы. Раскладки
	// {строки с длиной, фиксированные байты}, см. innoextract setup/*.cpp.
	skipEntries(h, counts[0], 10, 21);  // languages
	skipEntries(h, counts[1], 2, 4);    // messages
	skipEntries(h, counts[2], 1, 0);    // permissions
	skipEntries(h, counts[3], 4, 30);   // types
	skipEntries(h, counts[4], 5, 42);   // components
	skipEntries(h, counts[5], 6, 26);   // tasks
	skipEntries(h, counts[6], 7, 27);   // directories
	std::vector<File> files(counts[7]);
	for (File &f : files) {
		// source, destination, font, assembly, затем условия:
		// components, tasks, languages, check, after_install, before_install
		Bytes s[10];
		for (Bytes &x : s)
			x = h.str();
		const uint8_t *fixed = h.take(43);
		f.dest = utf8(s[1]);
		f.location = le32(fixed + 20);
		f.after = utf8(s[8]);
		f.before = utf8(s[9]);
	}
	skipEntries(h, counts[9], 13, 32);   // icons
	skipEntries(h, counts[10], 10, 21);  // ini
	skipEntries(h, counts[11], 9, 29);   // registry
	skipEntries(h, counts[12], 7, 21);   // delete
	skipEntries(h, counts[13], 7, 21);   // uninstall delete
	skipEntries(h, counts[14], 13, 27);  // run
	skipEntries(h, counts[15], 13, 27);  // uninstall run
	skipEntries(h, 2, 1, 0);             // большая и малая картинки мастера
	if (h.pos != setup.size())
		fail("corrupt installer header");

	Cursor d{{dataBlock.data(), dataBlock.size()}};
	std::vector<Data> data(counts[8]);
	for (Data &e : data) {
		e.first_slice = d.u32();
		e.last_slice = d.u32();
		e.chunk_offset = d.u32();
		e.file_offset = d.u64();
		e.size = d.u64();
		e.chunk_size = d.u64();
		std::memcpy(e.sha1, d.take(20), 20);
		d.skip(8 + 8);  // filetime, версия файла
		e.flags = d.u16();
		e.unpacked = e.size;
	}
	if (d.pos != dataBlock.size())
		fail("corrupt installer header");

	if (gog)
		joinGalaxyParts(files, data);

	// Выходные файлы; при совпадении путей побеждает более поздняя запись.
	std::vector<Output> outputs;
	std::unordered_map<std::string, size_t> byPath;
	Summary summary;
	for (const File &f : files) {
		if (f.continuation || f.location >= data.size())
			continue;
		const std::string path = convertPath(f.dest);
		if (path.empty())
			continue;
		std::vector<uint32_t> locs{f.location};
		locs.insert(locs.end(), f.parts.begin(), f.parts.end());
		uint64_t size = 0;
		bool filtered = false;
		for (uint32_t l : locs) {
			if (l >= data.size())
				fail("corrupt installer header");
			size += data[l].unpacked;
			if (data[l].flags & Data::call_instruction_optimized)
				filtered = true;
		}
		// Только .exe/.dll с фильтром call-инструкций; игре они не нужны.
		if (filtered) {
			++summary.skipped_filtered;
			continue;
		}
		auto [it, inserted] = byPath.emplace(lower(path), outputs.size());
		if (inserted)
			outputs.push_back({path, std::move(locs), size});
		else
			outputs[it->second] = {path, std::move(locs), size};
	}

	// Для каждого используемого data location: какие (файл, смещение) он заполняет.
	struct Target {
		size_t out;
		uint64_t offset;
	};
	std::vector<std::vector<Target>> targets(data.size());
	std::vector<uint32_t> used;
	const fs::path root(out_dir);
	for (size_t i = 0; i < outputs.size(); ++i) {
		const fs::path p = root / outputs[i].path;
		fs::create_directories(p.parent_path());
		FILE *f = std::fopen(p.c_str(), "wb");
		if (!f || std::fclose(f) != 0)
			fail("cannot create '" + p.string() + "'");
		uint64_t off = 0;
		for (uint32_t l : outputs[i].locs) {
			if (targets[l].empty())
				used.push_back(l);
			targets[l].push_back({i, off});
			off += data[l].unpacked;
		}
		summary.bytes += outputs[i].size;
	}
	summary.files = outputs.size();
	std::sort(used.begin(), used.end(), [&](uint32_t a, uint32_t b) {
		if (data[a].chunk_offset != data[b].chunk_offset)
			return data[a].chunk_offset < data[b].chunk_offset;
		return data[a].file_offset < data[b].file_offset;
	});

	// Каждый чанк распаковывается один раз, до конца последнего нужного файла.
	uint64_t done = 0;
	if (progress)
		progress(0, summary.bytes);
	for (size_t i = 0; i < used.size();) {
		const Data &first = data[used[i]];
		size_t j = i;
		uint64_t need = 0;
		for (; j < used.size() && data[used[j]].chunk_offset == first.chunk_offset; ++j) {
			const Data &e = data[used[j]];
			need = std::max(need, e.file_offset + e.size);
		}
		if (first.first_slice != 0 || first.last_slice != 0)
			fail("multi-volume installers are not supported");
		if (first.flags & Data::chunk_encrypted)
			fail("encrypted installers are not supported");
		Cursor cc{exe, size_t(loader.data_offset) + first.chunk_offset};
		if (std::memcmp(cc.take(4), "zlb\x1a", 4) != 0)
			fail("corrupt installer data (chunk magic)");
		const Bytes src{cc.take(size_t(first.chunk_size)), size_t(first.chunk_size)};

		std::vector<uint8_t> owned;
		Bytes chunk;
		if (!(first.flags & Data::chunk_compressed)) {
			if (src.len < need)
				fail("unexpected end of installer data");
			chunk = src;
		} else {
			// stored_compression_method_3: 0 stored, 1 zlib, 2 bzip2, 3 LZMA1, 4 LZMA2
			if (compression == 3)
				owned = lzma1Decode(src, size_t(need));
			else if (compression == 4)
				owned = lzma2Decode(src, size_t(need));
			else
				fail("unsupported compression method " + std::to_string(compression));
			chunk = {owned.data(), owned.size()};
		}
		for (size_t k = i; k < j; ++k) {
			const uint32_t l = used[k];
			const Data &e = data[l];
			const Bytes raw{chunk.ptr + e.file_offset, size_t(e.size)};
			Sha1 sha;
			sha.update(raw.ptr, raw.len);
			uint8_t digest[20];
			sha.finish(digest);
			if (std::memcmp(digest, e.sha1, 20) != 0)
				fail("checksum mismatch in installer data");
			std::vector<uint8_t> inflated;
			Bytes bytes = raw;
			if (e.zlib) {
				inflated = inflateZlib(raw, e.unpacked);
				bytes = {inflated.data(), inflated.size()};
			}
			for (const Target &t : targets[l])
				writeAt(root / outputs[t.out].path, t.offset, bytes.ptr, bytes.len);
			done += bytes.len;
			if (progress)
				progress(done, summary.bytes);
		}
		i = j;
	}
	return summary;
}

} // namespace innounp
