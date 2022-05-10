/*
 * Copyright © 2011 Keith Packard <keithp@keithp.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
 */

#include <ao.h>
#include <ao_storage.h>

#define AO_STORAGE_DATA_SIZE	128

static uint8_t storage_data[AO_STORAGE_DATA_SIZE];
static uint8_t storage_mutex;

uint8_t
ao_storage_read(ao_pos_t pos, void *v_buf, uint16_t len)
{
	uint8_t *buf = v_buf;
	uint16_t this_len;
	uint16_t this_off;

	ao_storage_setup();
	if (pos >= ao_storage_total || pos + len > ao_storage_total)
		return 0;
	while (len) {

		/* Compute portion of transfer within
		 * a single block
		 */
		this_off = (uint16_t) (pos & (ao_storage_unit - 1));
		this_len = ao_storage_unit - this_off;
		if (this_len > len)
			this_len = len;

		if (!ao_storage_device_read(pos, buf, this_len))
			return 0;

		/* See how much is left */
		buf += this_len;
		len -= this_len;
		pos += this_len;
	}
	return 1;
}

uint8_t
ao_storage_write(ao_pos_t pos, void *v_buf, uint16_t len) 
{
	uint8_t *buf = v_buf;
	uint16_t this_len;
	uint16_t this_off;

	ao_storage_setup();
	if (pos >= ao_storage_total || pos + len > ao_storage_total)
		return 0;
	while (len) {

		/* Compute portion of transfer within
		 * a single block
		 */
		this_off = (uint16_t) (pos & (ao_storage_unit - 1));
		this_len = ao_storage_unit - this_off;
		if (this_len > len)
			this_len = len;

		if (!ao_storage_device_write(pos, buf, this_len))
			return 0;

		/* See how much is left */
		buf += this_len;
		len -= this_len;
		pos += this_len;
	}
	return 1;
}

uint8_t
ao_storage_is_erased(uint32_t pos)
{
	uint32_t	read_pos;
	uint32_t	read_len;
	uint32_t	i;
	uint8_t		ret = 1;

	ao_storage_setup();
	ao_mutex_get(&storage_mutex);
	read_pos = pos;
	read_len = ao_storage_block;
	while (read_len) {
		uint32_t this_time = AO_STORAGE_DATA_SIZE;
		if (this_time > read_len)
			this_time = read_len;
		if (!ao_storage_read(read_pos, storage_data, (uint16_t) this_time)) {
			ret = 0;
			goto done;
		}
		for (i = 0; i < this_time; i++)
			if (storage_data[i] != AO_STORAGE_ERASED_BYTE) {
				ret = 0;
				goto done;
			}
		read_pos += this_time;
		read_len -= this_time;
	}
done:
	ao_mutex_put(&storage_mutex);
	return ret;
}

uint8_t
ao_storage_erase(uint32_t start_pos, uint32_t len)
{
	/* Round 'len' up to ao_storage_block units */
	len = ((len + ao_storage_block - 1) / ao_storage_block) * ao_storage_block;

	/*
	 * Start at the end of the area to erase so that the
	 * last block cleared is the first block; this will ensure
	 * that partially erased flight logs still appear in the list
	 * and can be re-erased.
	 */
	uint32_t pos = start_pos + len - ao_storage_block;
	while (len) {
		int tries;

#define MAX_TRIES	4	/* needs to be at least 2 */
		for (tries = 0; tries < MAX_TRIES; tries++) {
			if (ao_storage_is_erased(pos))
				break;
			if (!ao_storage_device_erase(pos))
				return 0;
		}
		if (tries == MAX_TRIES)
			return 0;
		pos -= ao_storage_block;
		len -= ao_storage_block;
	}
	return 1;
}

static void
ao_storage_dump(void) 
{
	uint32_t block;
	uint8_t i, j, k;

	block = ao_cmd_hex();
	if (ao_cmd_status != ao_cmd_success)
		return;
	ao_mutex_get(&storage_mutex);
	for (i = 0; ; i += AO_STORAGE_DATA_SIZE) {
		if (ao_storage_read((block << 8) + i,
				    storage_data,
				    AO_STORAGE_DATA_SIZE)) {
			for (k = 0; k < AO_STORAGE_DATA_SIZE; k += 8) {
				ao_cmd_put16((uint16_t) i + k);
				for (j = 0; j < 8; j++) {
					putchar(' ');
					ao_cmd_put8(storage_data[k + j]);
				}
				putchar ('\n');
			}
		}
		if (i == 256 - AO_STORAGE_DATA_SIZE)
			break;
	}
	ao_mutex_put(&storage_mutex);
}

#if HAS_STORAGE_DEBUG

/* not enough space for this today
 */
static void
ao_storage_store(void) 
{
	uint16_t block;
	uint8_t i;
	uint16_t len;
	uint8_t b;
	uint32_t addr;

	block = ao_cmd_hex();
	i = ao_cmd_hex();
	addr = ((uint32_t) block << 8) | i;
	len = ao_cmd_hex();
	if (ao_cmd_status != ao_cmd_success)
		return;
	while (len--) {
		b = ao_cmd_hexbyte();
		if (ao_cmd_status != ao_cmd_success)
			return;
		ao_storage_write(addr, &b, 1);
		addr++;
	}
}
#endif

static void
ao_storage_zap(void) 
{
	uint32_t v = ao_cmd_hex();
	if (ao_cmd_status != ao_cmd_success)
		return;
	ao_storage_erase((uint32_t) v << 8, ao_storage_block);
}

static void
ao_storage_zapall(void) 
{
	ao_cmd_white();
	if (!ao_match_word("DoIt"))
		return;
	ao_storage_erase(0, ao_storage_log_max);
}

#if AO_STORAGE_TEST

#define AO_STORAGE_TEST_SIZE	256
static uint8_t storage_test[AO_STORAGE_TEST_SIZE];

static void
ao_storage_failure(uint32_t pos, char *format, ...)
{
	va_list a;
	printf("TEST FAILURE AT %08x: ", pos);
	va_start(a, format);
	vprintf(format, a);
	va_end(a);
}

static uint8_t
ao_storage_check_block(uint32_t pos, uint8_t value)
{
	uint32_t	offset;
	uint32_t	byte;

	for (offset = 0; offset < ao_storage_block; offset += sizeof (storage_test)) {
		if (!ao_storage_read(pos + offset, storage_test, sizeof (storage_test))) {
			ao_storage_failure(pos + offset, "read failed\n");
			return 0;
		}
		for (byte = 0; byte < sizeof (storage_test); byte++)
			if (storage_test[byte] != value) {
				ao_storage_failure(pos + offset + byte,
						   "want %02x got %02x\n",
						   value, storage_test[byte]);
				return 0;
			}
	}
	return 1;
}

static uint8_t
ao_storage_fill_block(uint32_t pos, uint8_t value)
{
	uint32_t	offset;
	uint32_t	byte;

	for (byte = 0; byte < sizeof (storage_test); byte++)
		storage_test[byte] = value;
	for (offset = 0; offset < ao_storage_block; offset += sizeof (storage_test)) {
		if (!ao_storage_write(pos + offset, storage_test, sizeof (storage_test))) {
			ao_storage_failure(pos + offset, "write failed\n");
			return 0;
		}
	}
	return 1;
}

static uint8_t
ao_storage_check_incr_block(uint32_t pos)
{
	uint32_t	offset;
	uint32_t	byte;

	for (offset = 0; offset < ao_storage_block; offset += sizeof (storage_test)) {
		if (!ao_storage_read(pos + offset, storage_test, sizeof (storage_test))) {
			ao_storage_failure(pos + offset, "read failed\n");
			return 0;
		}
		for (byte = 0; byte < sizeof (storage_test); byte++) {
			uint8_t value = offset + byte;
			if (storage_test[byte] != value) {
				ao_storage_failure(pos + offset + byte,
						   "want %02x got %02x\n",
						   value, storage_test[byte]);
				return 0;
			}
		}
	}
	return 1;
}

static uint8_t
ao_storage_fill_incr_block(uint32_t pos)
{
	uint32_t	offset;
	uint32_t	byte;

	for (offset = 0; offset < ao_storage_block; offset += sizeof (storage_test)) {
		for (byte = 0; byte < sizeof (storage_test); byte++)
			storage_test[byte] = offset + byte;
		if (!ao_storage_write(pos + offset, storage_test, sizeof (storage_test))) {
			ao_storage_failure(pos + offset, "write failed\n");
			return 0;
		}
	}
	return 1;
}

static uint8_t
ao_storage_fill_check_block(uint32_t pos, uint8_t value)
{
	return ao_storage_fill_block(pos, value) && ao_storage_check_block(pos, value);
}

static uint8_t
ao_storage_incr_check_block(uint32_t pos)
{
	return ao_storage_fill_incr_block(pos) && ao_storage_check_incr_block(pos);
}

static uint8_t
ao_storage_test_block(uint32_t pos) 
{
	ao_storage_erase(pos, ao_storage_block);
	printf(" erase"); flush();
	if (!ao_storage_check_block(pos, 0xff))
		return 0;
	printf(" zero"); flush();
	if (!ao_storage_fill_check_block(pos, 0x00))
		return 0;
	ao_storage_erase(pos, ao_storage_block);
	printf(" 0xaa"); flush();
	if (!ao_storage_fill_check_block(pos, 0xaa))
		return 0;
	ao_storage_erase(pos, ao_storage_block);
	printf(" 0x55"); flush();
	if (!ao_storage_fill_check_block(pos, 0x55))
		return 0;
	ao_storage_erase(pos, ao_storage_block);
	printf(" increment"); flush();
	if (!ao_storage_incr_check_block(pos))
		return 0;
	ao_storage_erase(pos, ao_storage_block);
	printf(" pass\n"); flush();
	return 1;
}

static void
ao_storage_test(void) 
{
	uint32_t	pos;

	ao_cmd_white();
	if (!ao_match_word("DoIt"))
		return;
	for (pos = 0; pos < ao_storage_log_max; pos += ao_storage_block) {
		printf("Testing block 0x%08x:", pos); flush();
		if (!ao_storage_test_block(pos))
			break;
	}
	printf("Test complete\n");
}

static void
ao_storage_fill(void)
{
	uint32_t	pos;

	ao_cmd_white();
	if (!ao_match_word("DoIt"))
		return;
	printf("erase "); flush();
	ao_storage_erase(0, ao_storage_log_max);
	for (pos = 0; pos < sizeof (storage_test); pos++)
		storage_test[pos] = (uint8_t) pos;
	for (pos = 0; pos < ao_storage_log_max; pos += sizeof (storage_test)) {
		if ((pos & 0xffff) == 0) {
			printf("Fill 0x%x\n", pos); flush();
		}
		ao_storage_write(pos, storage_test, sizeof (storage_test));
	}
	printf("Fill complete\n");
}
#endif /* AO_STORAGE_TEST */

static void
ao_storage_info(void) 
{
	ao_storage_setup();
	printf("Storage size: %ld\n", (long) ao_storage_total);
	printf("Storage erase unit: %ld\n", (long) ao_storage_block);
	ao_storage_device_info();
}

const struct ao_cmds ao_storage_cmds[] = {
	{ ao_storage_info, "f\0Show storage" },
	{ ao_storage_dump, "e <block>\0Dump flash" },
#if HAS_STORAGE_DEBUG
	{ ao_storage_store, "w <block> <start> <len> <data> ...\0Write data to flash" },
#endif
	{ ao_storage_zap, "z <block>\0Erase <block>" },
	{ ao_storage_zapall,"Z <key>\0Erase all. <key> is doit with D&I" },
#if AO_STORAGE_TEST
	{ ao_storage_test, "V <key>\0Validate flash (destructive). <key> is doit with D&I" },
	{ ao_storage_fill, "F <key>\0Fill flash with data. <key> is doit with D&I" },
#endif
	{ 0, NULL },
};

void
ao_storage_init(void)
{
	ao_storage_device_init();
	ao_cmd_register(&ao_storage_cmds[0]);
}
