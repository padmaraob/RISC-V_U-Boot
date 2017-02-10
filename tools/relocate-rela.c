/*
 * Copyright 2013 Freescale Semiconductor, Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+ BSD-2-Clause
 *
 * 64-bit and little-endian target only until we need to support a different
 * arch that needs this.
 */

#include <elf.h>
#include <errno.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define __RISCV__

#ifndef __RISCV__

#ifndef R_AARCH64_RELATIVE
#define R_AARCH64_RELATIVE	1027
#endif

static const bool debug_en;

static void debug(const char *fmt, ...)
{
	va_list args;

	va_start(args, fmt);
	if (debug_en)
		vprintf(fmt, args);
}

static bool supported_rela(Elf64_Rela *rela)
{
	uint64_t mask = 0xffffffffULL; /* would be different on 32-bit */
	uint32_t type = rela->r_info & mask;

	switch (type) {
#ifdef R_AARCH64_RELATIVE
	case R_AARCH64_RELATIVE:
		return true;
#endif
	default:
		fprintf(stderr, "warning: unsupported relocation type %"
				PRIu32 " at %" PRIx64 "\n",
			type, rela->r_offset);

		return false;
	}
}

static inline uint64_t swap64(uint64_t val)
{
	return ((val >> 56) & 0x00000000000000ffULL) |
	       ((val >> 40) & 0x000000000000ff00ULL) |
	       ((val >> 24) & 0x0000000000ff0000ULL) |
	       ((val >>  8) & 0x00000000ff000000ULL) |
	       ((val <<  8) & 0x000000ff00000000ULL) |
	       ((val << 24) & 0x0000ff0000000000ULL) |
	       ((val << 40) & 0x00ff000000000000ULL) |
	       ((val << 56) & 0xff00000000000000ULL);
}

#if __BYTE_ORDER == __LITTLE_ENDIAN
static inline uint64_t be64(uint64_t val)
{
	return swap64(val);
}

static inline uint64_t le64(uint64_t val)
{
	return val;
}
#else
static inline uint64_t le64(uint64_t val)
{
	return swap64(val);
}

static inline uint64_t be64(uint64_t val)
{
	return val;
}
#endif

static bool read_num(const char *str, uint64_t *num)
{
	char *endptr;
	*num = strtoull(str, &endptr, 16);
	return str[0] && !endptr[0];
}

int main(int argc, char **argv)
{
	FILE *f;
	int i, num;
	uint64_t rela_start, rela_end, text_base;

	if (argc != 5) {
		fprintf(stderr, "Statically apply ELF rela relocations\n");
		fprintf(stderr, "Usage: %s <bin file> <text base> " \
				"<rela start> <rela end>\n", argv[0]);
		fprintf(stderr, "All numbers in hex.\n");
		return 1;
	}

	f = fopen(argv[1], "r+b");
	if (!f) {
		fprintf(stderr, "%s: Cannot open %s: %s\n",
			argv[0], argv[1], strerror(errno));
		return 2;
	}

	if (!read_num(argv[2], &text_base) ||
	    !read_num(argv[3], &rela_start) ||
	    !read_num(argv[4], &rela_end)) {
		fprintf(stderr, "%s: bad number\n", argv[0]);
		return 3;
	}

	if (rela_start > rela_end || rela_start < text_base ||
	    (rela_end - rela_start) % sizeof(Elf64_Rela)) {
		fprintf(stderr, "%s: bad rela bounds\n", argv[0]);
		return 3;
	}

	rela_start -= text_base;
	rela_end -= text_base;

	num = (rela_end - rela_start) / sizeof(Elf64_Rela);

	for (i = 0; i < num; i++) {
		Elf64_Rela rela, swrela;
		uint64_t pos = rela_start + sizeof(Elf64_Rela) * i;
		uint64_t addr;

		if (fseek(f, pos, SEEK_SET) < 0) {
			fprintf(stderr, "%s: %s: seek to %" PRIx64
					" failed: %s\n",
				argv[0], argv[1], pos, strerror(errno));
		}

		if (fread(&rela, sizeof(rela), 1, f) != 1) {
			fprintf(stderr, "%s: %s: read rela failed at %"
					PRIx64 "\n",
				argv[0], argv[1], pos);
			return 4;
		}

		swrela.r_offset = le64(rela.r_offset);
		swrela.r_info = le64(rela.r_info);
		swrela.r_addend = le64(rela.r_addend);

		if (!supported_rela(&swrela))
			continue;

		debug("Rela %" PRIx64 " %" PRIu64 " %" PRIx64 "\n",
		      swrela.r_offset, swrela.r_info, swrela.r_addend);

		if (swrela.r_offset < text_base) {
			fprintf(stderr, "%s: %s: bad rela at %" PRIx64 "\n",
				argv[0], argv[1], pos);
			return 4;
		}

		addr = swrela.r_offset - text_base;

		if (fseek(f, addr, SEEK_SET) < 0) {
			fprintf(stderr, "%s: %s: seek to %"
					PRIx64 " failed: %s\n",
				argv[0], argv[1], addr, strerror(errno));
		}

		if (fwrite(&rela.r_addend, sizeof(rela.r_addend), 1, f) != 1) {
			fprintf(stderr, "%s: %s: write failed at %" PRIx64 "\n",
				argv[0], argv[1], addr);
			return 4;
		}
	}

	if (fclose(f) < 0) {
		fprintf(stderr, "%s: %s: close failed: %s\n",
			argv[0], argv[1], strerror(errno));
		return 4;
	}

	return 0;
}
#else

#ifndef R_RISCV_RELATIVE
#define R_RISCV_RELATIVE	3
#endif

#ifndef R_RISCV_32
#define R_RISCV_32	1
#endif

static const bool debug_en;

static void debug(const char *fmt, ...)
{
	va_list args;

	va_start(args, fmt);
	if (debug_en)
		vprintf(fmt, args);
}

static bool supported_rela(Elf32_Rela *rela)
{
	uint32_t mask = 0xff;
	uint32_t type = rela->r_info & mask;

	switch (type) {
#ifdef R_RISCV_RELATIVE
	case R_RISCV_RELATIVE:
		return true;
#endif
#ifdef R_RISCV_32
	case R_RISCV_32:
		return true;
#endif
	default:
		fprintf(stderr, "warning: unsupported relocation type %"
				PRIx32 " at %" PRIx32 "\n",
			type, rela->r_offset);

		return false;
	}
}

static inline uint32_t swap32(uint32_t val)
{
	return ((val >> 24) & 0x000000ff) |
	       ((val >>  8) & 0x0000ff00) |
	       ((val <<  8) & 0x00ff0000) |
	       ((val << 24) & 0xff000000);
}

#if __BYTE_ORDER == __LITTLE_ENDIAN
static inline uint32_t be32(uint32_t val)
{
	return swap32(val);
}

static inline uint32_t le32(uint32_t val)
{
	return val;
}
#else
static inline uint32_t le32(uint32_t val)
{
	return swap32(val);
}

static inline uint32_t be32(uint32_t val)
{
	return val;
}
#endif

static bool read_num(const char *str, uint32_t *num)
{
	char *endptr;
	*num = strtoull(str, &endptr, 16);
	return str[0] && !endptr[0];
}

int main(int argc, char **argv)
{
	FILE *f;
	int i, num;
	uint32_t rela_start, rela_end, dyn_sym, text_base;

	if (argc != 6) {
		fprintf(stderr, "Statically apply ELF rela relocations\n");
		fprintf(stderr, "Usage: %s <bin file> <text base> " \
				"<rela start> <rela end> dyn_sym: %s\n", argv[0], argv[5]);
		fprintf(stderr, "All numbers in hex.\n");
		return 1;
	}

	f = fopen(argv[1], "r+b");
	if (!f) {
		fprintf(stderr, "%s: Cannot open %s: %s\n",
			argv[0], argv[1], strerror(errno));
		return 2;
	}

	if (!read_num(argv[2], &text_base) ||
	    !read_num(argv[3], &rela_start) ||
	    !read_num(argv[4], &rela_end) ||
		!read_num(argv[5], &dyn_sym)) {
		fprintf(stderr, "%s: bad number\n", argv[0]);
		return 3;
	}
//	fprintf(stderr, "<text base>: %" PRIx32 " <rela start>: %" PRIx32 "  <rela end>: %" PRIx32" <dym_sym>: %" PRIx32 " \n", text_base, rela_start, rela_end, dyn_sym);
	if (rela_start > rela_end || rela_start < text_base ||
	    (rela_end - rela_start) % sizeof(Elf32_Rela)) {
		fprintf(stderr, "%s: bad rela bounds\n", argv[0]);
		return 3;
	}

	rela_start -= text_base;
	rela_end -= text_base;
	dyn_sym -= text_base;

	num = (rela_end - rela_start) / sizeof(Elf32_Rela);

	for (i = 0; i < num; i++) {

		Elf32_Rela rela, swrela;
		uint32_t pos = rela_start + sizeof(Elf32_Rela) * i;
		uint32_t addr;
		uint32_t rel_st_index;

//		fprintf(stderr, "<Pos>: %" PRIx32" \n", pos);
		if (fseek(f, pos, SEEK_SET) < 0) {
			fprintf(stderr, "%s: %s: seek to %" PRIx32
					" failed: %s\n",
				argv[0], argv[1], pos, strerror(errno));
		}

		if (fread(&rela, sizeof(rela), 1, f) != 1) {
			fprintf(stderr, "%s: %s: read rela failed at %"
					PRIx32 "\n",
				argv[0], argv[1], pos);
			return 4;
		}

		swrela.r_offset = le32(rela.r_offset);
		swrela.r_info = le32(rela.r_info);
		swrela.r_addend = le32(rela.r_addend);

		if (!supported_rela(&swrela))
			continue;

		debug("SWRela %" PRIx32 " %" PRIu32 " %" PRIx32 "\n",
		      swrela.r_offset, swrela.r_info, swrela.r_addend);

		if (swrela.r_offset < text_base) {
			fprintf(stderr, "%s: %s: bad rela at %" PRIx32 "\n",
				argv[0], argv[1], pos);
			return 4;
		}

		addr = swrela.r_offset - text_base;
//		fprintf(stderr, "<Addr>: %" PRIx32" \n", addr);

		rel_st_index = rela.r_info >> 8;

		if(rel_st_index == 0)
		{
			if (fseek(f, addr, SEEK_SET) < 0) {
				fprintf(stderr, "%s: %s: seek to %"
					PRIx32 " failed: %s\n",
					argv[0], argv[1], addr, strerror(errno));
			}

			if (fwrite(&rela.r_addend, sizeof(rela.r_addend), 1, f) != 1) {
				fprintf(stderr, "%s: %s: write failed at %" PRIx32 "\n",
						argv[0], argv[1], addr);
				return 4;
			}
		}
		else
		{
			Elf32_Sym rdyn;
			uint32_t pos1 = dyn_sym + sizeof(Elf32_Sym) * rel_st_index;

			if (fseek(f, pos1, SEEK_SET) < 0) {
				fprintf(stderr, "%s: %s: seek to %" PRIx32
						" failed: %s\n",
					argv[0], argv[1], pos1, strerror(errno));
			}

			if (fread(&rdyn, sizeof(rdyn), 1, f) != 1) {
				fprintf(stderr, "%s: %s: read rela failed at %"
						PRIx32 "\n",
					argv[0], argv[1], pos1);
				return 4;
			}

			if (fseek(f, addr, SEEK_SET) < 0) {
				fprintf(stderr, "%s: %s: seek to %"
					PRIx32 " failed: %s\n",
					argv[0], argv[1], addr, strerror(errno));
			}

			if (fwrite(&rdyn.st_value, sizeof(rdyn.st_value), 1, f) != 1) {
				fprintf(stderr, "%s: %s: write failed at %" PRIx32 "\n",
						argv[0], argv[1], addr);
				return 4;
			}
		}

	}

	if (fclose(f) < 0) {
		fprintf(stderr, "%s: %s: close failed: %s\n",
			argv[0], argv[1], strerror(errno));
		return 4;
	}

	return 0;
}
#endif /* __RISCV__ */
