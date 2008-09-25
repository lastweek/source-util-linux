/*
 * Copyright (C) 1999, 2001 by Andries Brouwer
 * Copyright (C) 1999, 2000, 2003 by Theodore Ts'o
 * Copyright (C) 2008 Karel Zak <kzak@redhat.com>
 *
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This file is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <stdint.h>

#include "blkidP.h"

struct reiserfs_super_block {
	uint32_t	rs_blocks_count;
	uint32_t	rs_free_blocks;
	uint32_t	rs_root_block;
	uint32_t	rs_journal_block;
	uint32_t	rs_journal_dev;
	uint32_t	rs_orig_journal_size;
	uint32_t	rs_dummy2[5];
	uint16_t	rs_blocksize;
	uint16_t	rs_dummy3[3];
	unsigned char	rs_magic[12];
	uint32_t	rs_dummy4[5];
	unsigned char	rs_uuid[16];
	char		rs_label[16];
};

struct reiser4_super_block {
	unsigned char	rs4_magic[16];
	uint16_t	rs4_dummy[2];
	unsigned char	rs4_uuid[16];
	unsigned char	rs4_label[16];
	uint64_t	rs4_dummy2;
};

static int probe_reiser(blkid_probe pr, const struct blkid_idmag *mag)
{
	struct reiserfs_super_block *rs;
	unsigned int blocksize;

	rs = blkid_probe_get_sb(pr, mag, struct reiserfs_super_block);
	if (!rs)
		return -1;

	blocksize = le16_to_cpu(rs->rs_blocksize);

	/* The blocksize must be at least 1k */
	if ((blocksize >> 10) == 0)
		return -BLKID_ERR_PARAM;

	/* If the superblock is inside the journal, we have the wrong one */
	if (mag->kboff / (blocksize >> 10) > le32_to_cpu(rs->rs_journal_block))
		return -BLKID_ERR_BIG;

	/* LABEL/UUID are only valid for later versions of Reiserfs v3.6. */
	if (mag->magic[6] == '2' || mag->magic[6] == '3') {
		if (strlen(rs->rs_label))
			blkid_probe_set_label(pr,
					(unsigned char *) rs->rs_label,
					sizeof(rs->rs_label));
		blkid_probe_set_uuid(pr, rs->rs_uuid);
	}

	return 0;
}

static int probe_reiser4(blkid_probe pr, const struct blkid_idmag *mag)
{
	struct reiser4_super_block *rs4;

	rs4 = blkid_probe_get_sb(pr, mag, struct reiser4_super_block);
	if (!rs4)
		return -1;

	if (strlen((char *) rs4->rs4_label))
		blkid_probe_set_label(pr, rs4->rs4_label, sizeof(rs4->rs4_label));
	blkid_probe_set_uuid(pr, rs4->rs4_uuid);

	return 0;
}


const struct blkid_idinfo reiser_idinfo =
{
	.name		= "reiser",
	.usage		= BLKID_USAGE_FILESYSTEM,
	.probefunc	= probe_reiser,
	.magics		=
	{
		{ .magic = "ReIsErFs",  .len = 8, .kboff = 8,  .sboff = 0x34 },
		{ .magic = "ReIsEr2Fs", .len = 9, .kboff = 64, .sboff = 0x34 },
		{ .magic = "ReIsEr3Fs", .len = 9, .kboff = 64, .sboff = 0x34 },
		{ .magic = "ReIsErFs",  .len = 8, .kboff = 64, .sboff = 0x34 },
		{ .magic = "ReIsErFs",  .len = 8, .kboff =  8, .sboff = 20   },
		{ NULL }
	}
};

const struct blkid_idinfo reiser4_idinfo =
{
	.name		= "reiser4",
	.usage		= BLKID_USAGE_FILESYSTEM,
	.probefunc	= probe_reiser4,
	.magics		=
	{
		{ .magic = "ReIsEr4", .len = 7, .kboff = 64 },
		{ NULL }
	}
};




