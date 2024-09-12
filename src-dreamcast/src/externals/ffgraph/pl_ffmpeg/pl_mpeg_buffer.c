#include "pl_mpeg.h"

#include "malloc_utils.h"


// -----------------------------------------------------------------------------
// plm_buffer implementation


void plm_buffer_seek(plm_buffer_t *self, size_t pos);
size_t plm_buffer_tell(plm_buffer_t *self);
void plm_buffer_discard_read_bytes(plm_buffer_t *self);
void plm_buffer_load_file_callback(plm_buffer_t *self, void *user);

int plm_buffer_has(plm_buffer_t *self, size_t count);
int plm_buffer_read_unchecked(plm_buffer_t *self, int count);
static inline int plm_buffer_read(plm_buffer_t *self, int count);
int plm_buffer_read_bytes_unchecked(plm_buffer_t *self, int count);
static inline int plm_buffer_read_bytes(plm_buffer_t *self, int count);
void plm_buffer_align(plm_buffer_t *self);
static inline void plm_buffer_skip_unchecked(plm_buffer_t *self, size_t count);
void plm_buffer_skip(plm_buffer_t *self, size_t count);
int plm_buffer_skip_bytes(plm_buffer_t *self, uint8_t v);
int plm_buffer_next_start_code(plm_buffer_t *self);
int plm_buffer_find_start_code(plm_buffer_t *self, int code);
int plm_buffer_no_start_code(plm_buffer_t *self);
int16_t plm_buffer_read_vlc(plm_buffer_t *self, const plm_vlc_t *table);
uint16_t plm_buffer_read_vlc_uint(plm_buffer_t *self, const plm_vlc_uint_t *table);

plm_buffer_t *plm_buffer_create_with_filename(const char *filename) {
	FILE *fh = fopen(filename, "rb");
	if (!fh) {
		return NULL;
	}
	return plm_buffer_create_with_file(fh, TRUE);
}

plm_buffer_t *plm_buffer_create_with_file(FILE *fh, int close_when_done) {
	plm_buffer_t *self = plm_buffer_create_with_capacity(PLM_BUFFER_DEFAULT_SIZE);
	self->fh = fh;
	self->close_when_done = close_when_done;
	self->mode = PLM_BUFFER_MODE_FILE;
	self->discard_read_bytes = TRUE;
	
	fseek(self->fh, 0, SEEK_END);
	self->total_size = (size_t)ftell(self->fh);
	fseek(self->fh, 0, SEEK_SET);

	plm_buffer_set_load_callback(self, plm_buffer_load_file_callback, NULL);
	return self;
}

plm_buffer_t *plm_buffer_create_with_memory(uint8_t *bytes, size_t length, int free_when_done) {
	plm_buffer_t *self = (plm_buffer_t *)malloc_chk(sizeof(plm_buffer_t));
	memset(self, 0, sizeof(plm_buffer_t));
	self->capacity = length;
	self->length = length;
	self->total_size = length;
	self->free_when_done = free_when_done;
	self->bytes = bytes;
	self->mode = PLM_BUFFER_MODE_FIXED_MEM;
	self->discard_read_bytes = FALSE;
#ifdef PLM_BUFFER_ALT_DISCARD
	self->bytes_buffer = self->bytes;
#endif
	return self;
}

plm_buffer_t *plm_buffer_create_with_capacity(size_t capacity) {
	plm_buffer_t *self = (plm_buffer_t *)malloc_chk(sizeof(plm_buffer_t));
	memset(self, 0, sizeof(plm_buffer_t));
	self->capacity = capacity;
	self->free_when_done = TRUE;
	self->bytes = (uint8_t *)malloc_chk(capacity);
	self->mode = PLM_BUFFER_MODE_RING;
	self->discard_read_bytes = TRUE;
#ifdef PLM_BUFFER_ALT_DISCARD
	self->bytes_buffer = self->bytes;
#endif
	return self;
}

plm_buffer_t *plm_buffer_create_for_appending(size_t initial_capacity) {
	plm_buffer_t *self = plm_buffer_create_with_capacity(initial_capacity);
	self->mode = PLM_BUFFER_MODE_APPEND;
	self->discard_read_bytes = FALSE;
	return self;
}

void plm_buffer_destroy(plm_buffer_t *self) {
	if (self->fh && self->close_when_done) {
		fclose(self->fh);
	}
	if (self->free_when_done) {
#ifdef PLM_BUFFER_ALT_DISCARD
		free_chk(self->bytes_buffer);
#else
		free_chk(self->bytes);
#endif
	}
	free_chk(self);
}

size_t plm_buffer_get_size(plm_buffer_t *self) {
	return (self->mode == PLM_BUFFER_MODE_FILE)
		? self->total_size
		: self->length;
}

size_t plm_buffer_get_remaining(plm_buffer_t *self) {
	return self->length - (self->bit_index >> 3);
}

size_t plm_buffer_write(plm_buffer_t *self, uint8_t *bytes, size_t length) {
	if (self->mode == PLM_BUFFER_MODE_FIXED_MEM) {
		return 0;
	}

	if (self->discard_read_bytes) {
		// This should be a ring buffer, but instead it just shifts all unread 
		// data to the beginning of the buffer and appends new data at the end. 
		// Seems to be good enough.

		plm_buffer_discard_read_bytes(self);
		if (self->mode == PLM_BUFFER_MODE_RING) {
			self->total_size = 0;
		}
	}

#ifdef PLM_BUFFER_ALT_DISCARD
    if (self->bytes != self->bytes_buffer) {
		uint8_t* buffer_end = self->bytes_buffer + self->capacity;
		uint8_t* buffer_ptr = self->bytes + self->length + length + (self->bit_index >> 3);
		if (buffer_ptr >= buffer_end) {
			memmove(self->bytes_buffer, self->bytes, self->length);
			self->bytes = self->bytes_buffer;
		}
    }
#endif

	// Do we have to resize to fit the new data?
	size_t bytes_available = self->capacity - self->length;
	if (bytes_available < length) {
		size_t new_size = self->capacity;
		do {
			new_size *= 2;
		} while (new_size - self->length < length);
		self->bytes = (uint8_t *)realloc_chk(self->bytes, new_size);
		self->capacity = new_size;
	}

	memcpy(self->bytes + self->length, bytes, length);
	self->length += length;
	self->has_ended = FALSE;
	return length;
}

void plm_buffer_signal_end(plm_buffer_t *self) {
	self->total_size = self->length;
}

void plm_buffer_set_load_callback(plm_buffer_t *self, plm_buffer_load_callback fp, void *user) {
	self->load_callback = fp;
	self->load_callback_user_data = user;
}

void plm_buffer_rewind(plm_buffer_t *self) {
	plm_buffer_seek(self, 0);
}

void plm_buffer_seek(plm_buffer_t *self, size_t pos) {
	self->has_ended = FALSE;

	if (self->mode == PLM_BUFFER_MODE_FILE) {
		fseek(self->fh, (long)pos, SEEK_SET);
		self->bit_index = 0;
		self->length = 0;
	}
	else if (self->mode == PLM_BUFFER_MODE_RING) {
		if (pos != 0) {
			// Seeking to non-0 is forbidden for dynamic-mem buffers
			return; 
		}
		self->bit_index = 0;
		self->length = 0;
		self->total_size = 0;
	}
	else if (pos < self->length) {
		self->bit_index = pos << 3;
	}
}

size_t plm_buffer_tell(plm_buffer_t *self) {
	return self->mode == PLM_BUFFER_MODE_FILE
		? (size_t)ftell(self->fh) + (self->bit_index >> 3) - self->length
		: self->bit_index >> 3;
}

void plm_buffer_discard_read_bytes(plm_buffer_t *self) {
	size_t byte_pos = self->bit_index >> 3;
	if (byte_pos == self->length) {
		self->bit_index = 0;
		self->length = 0;
#ifdef PLM_BUFFER_ALT_DISCARD
		self->bytes = self->bytes_buffer;
#endif
	}
	else if (byte_pos > 0) {
#ifdef PLM_BUFFER_ALT_DISCARD
        self->bytes += byte_pos;
#else
		memmove(self->bytes, self->bytes + byte_pos, self->length - byte_pos);
#endif
		self->bit_index -= byte_pos << 3;
		self->length -= byte_pos;
	}
}

void plm_buffer_load_file_callback(plm_buffer_t *self, void *user) {
	PLM_UNUSED(user);
	
	if (self->discard_read_bytes) {
		plm_buffer_discard_read_bytes(self);
	}

	size_t bytes_available = self->capacity - self->length;
	size_t bytes_read = fread(self->bytes + self->length, 1, bytes_available, self->fh);
	self->length += bytes_read;

	if (bytes_read == 0) {
		self->has_ended = TRUE;
	}
}

int plm_buffer_has_ended(plm_buffer_t *self) {
	return self->has_ended;
}

int plm_buffer_has(plm_buffer_t *self, size_t count) {
	if (((self->length << 3) - self->bit_index) >= count) {
		return TRUE;
	}

	if (self->load_callback) {
		self->load_callback(self, self->load_callback_user_data);
		
		if (((self->length << 3) - self->bit_index) >= count) {
			return TRUE;
		}
	}	
	
	if (self->total_size != 0 && self->length == self->total_size) {
		self->has_ended = TRUE;
	}
	return FALSE;
}

static inline int plm_buffer_is_aligned(plm_buffer_t *self) {
	return (self->bit_index & 7) == 0;
}

int plm_buffer_read_unchecked(plm_buffer_t *self, int count) {
	int value = 0;

	while (count) {
		int current_byte = self->bytes[self->bit_index >> 3];

		int remaining = 8 - (self->bit_index & 7); // Remaining bits in byte
		int read = remaining < count ? remaining : count; // Bits in self run
		int shift = remaining - read;
		int mask = (0xff >> (8 - read));

		value = (value << read) | ((current_byte >> shift) & mask);

		self->bit_index += (size_t)read;
		count -= read;
	}

	return value;
}

static inline int plm_buffer_read(plm_buffer_t *self, int count) {
	if (!plm_buffer_has(self, (size_t)count)) {
		return 0;
	}
	return plm_buffer_read_unchecked(self, count);
}

int plm_buffer_read_bytes_unchecked(plm_buffer_t *self, int count) {
	int value = 0;
	const uint8_t *byte = &self->bytes[self->bit_index >> 3];
	self->bit_index += (size_t)count * 8;
	while (count) {
		value = (value << 8) | *byte;
		byte++;
		count--;
	}
	return value;
}

static inline int plm_buffer_read_bytes(plm_buffer_t *self, int count) {
	if (!plm_buffer_has(self, (size_t)count * 8)) {
		return 0;
	}
	return plm_buffer_is_aligned(self) ? plm_buffer_read_bytes_unchecked(self, count) : plm_buffer_read_unchecked(self, count * 8);
}

void plm_buffer_align(plm_buffer_t *self) {
	self->bit_index = ((self->bit_index + 7) >> 3) << 3; // Align to next byte
}

static inline void plm_buffer_skip_unchecked(plm_buffer_t *self, size_t count) {
	self->bit_index += count;
}

void plm_buffer_skip(plm_buffer_t *self, size_t count) {
	if (plm_buffer_has(self, count)) {
		self->bit_index += count;
	}
}

int plm_buffer_skip_bytes(plm_buffer_t *self, uint8_t v) {
	plm_buffer_align(self);
	int skipped = 0;
	while (plm_buffer_has(self, 8) && self->bytes[self->bit_index >> 3] == v) {
		self->bit_index += 8;
		skipped++;
	}
	return skipped;
}

int plm_buffer_next_start_code(plm_buffer_t *self) {
	plm_buffer_align(self);

	while (plm_buffer_has(self, (5 << 3))) {
		size_t byte_index = (self->bit_index) >> 3;
		if (
			self->bytes[byte_index] == 0x00 &&
			self->bytes[byte_index + 1] == 0x00 &&
			self->bytes[byte_index + 2] == 0x01
		) {
			self->bit_index = (byte_index + 4) << 3;
			return self->bytes[byte_index + 3];
		}
		self->bit_index += 8;
	}
	return -1;
}

int plm_buffer_find_start_code(plm_buffer_t *self, int code) {
	int current = 0;
	while (TRUE) {
		current = plm_buffer_next_start_code(self);
		if (current == code || current == -1) {
			return current;
		}
	}
	return -1;
}

int plm_buffer_has_start_code(plm_buffer_t *self, int code) {
	size_t previous_bit_index = self->bit_index;
	int previous_discard_read_bytes = self->discard_read_bytes;
	
	self->discard_read_bytes = FALSE;
	int current = plm_buffer_find_start_code(self, code);

	self->bit_index = previous_bit_index;
	self->discard_read_bytes = previous_discard_read_bytes;
	return current;
}

int plm_buffer_peek_non_zero(plm_buffer_t *self, int bit_count) {
	if (!plm_buffer_has(self, (size_t)bit_count)) {
		return FALSE;
	}

	int val = plm_buffer_read_unchecked(self, bit_count);
	self->bit_index -= (size_t)bit_count;
	return val != 0;
}

int16_t plm_buffer_read_vlc(plm_buffer_t *self, const plm_vlc_t *table) {
	plm_vlc_t state = {0, 0};
	do {
		state = table[state.index + plm_buffer_read(self, 1)];
	} while (state.index > 0);
	return state.value;
}

uint16_t plm_buffer_read_vlc_uint(plm_buffer_t *self, const plm_vlc_uint_t *table) {
	return (uint16_t)plm_buffer_read_vlc(self, (const plm_vlc_t *)table);
}

size_t plm_buffer_get_available_space(plm_buffer_t *self) {
    return self->capacity - self->length;
}
