/*  Written in 2018 by David Blackman and Sebastiano Vigna (vigna@acm.org)

To the extent possible under law, the author has dedicated all copyright
and related and neighboring rights to this software to the public domain
worldwide. This software is distributed without any warranty.

See <http://creativecommons.org/publicdomain/zero/1.0/>. */


//
// Note: reminder to use "starstar" for integers and "plus" for floating-point
//


#include <stdint.h>


static inline uint32_t xoshiro128_rotl(const uint32_t x, int k) {
	return (x << k) | (x >> (32 - k));
}


static uint32_t xoshiro128_state[4] = {0xf53190d5, 0xae90215d, 0x56bf0ec4, 0xdb718c86};

/* This is xoshiro128+ 1.0, our best and fastest 32-bit generator for 32-bit
   floating-point numbers. We suggest to use its upper bits for
   floating-point generation, as it is slightly faster than xoshiro128**.
   It passes all tests we are aware of except for
   linearity tests, as the lowest four bits have low linear complexity, so
   if low linear complexity is not considered an issue (as it is usually
   the case) it can be used to generate 32-bit outputs, too.

   We suggest to use a sign test to extract a random Boolean value, and
   right shifts to extract subsets of bits.

   The state must be seeded so that it is not everywhere zero. */
static uint32_t xoshiro128_next_plus(void) {
	const uint32_t result = xoshiro128_state[0] + xoshiro128_state[3];

	const uint32_t t = xoshiro128_state[1] << 9;

	xoshiro128_state[2] ^= xoshiro128_state[0];
	xoshiro128_state[3] ^= xoshiro128_state[1];
	xoshiro128_state[1] ^= xoshiro128_state[2];
	xoshiro128_state[0] ^= xoshiro128_state[3];

	xoshiro128_state[2] ^= t;

	xoshiro128_state[3] = xoshiro128_rotl(xoshiro128_state[3], 11);

	return result;
}

/* This is xoshiro128** 1.1, one of our 32-bit all-purpose, rock-solid
   generators. It has excellent speed, a state size (128 bits) that is
   large enough for mild parallelism, and it passes all tests we are aware
   of.

   Note that version 1.0 had mistakenly xoshiro128_state[0] instead of xoshiro128_state[1] as state
   word passed to the scrambler.

   For generating just single-precision (i.e., 32-bit) floating-point
   numbers, xoshiro128+ is even faster.

   The state must be seeded so that it is not everywhere zero. */
static uint32_t xoshiro128_next_starstar(void) {
	const uint32_t result = xoshiro128_rotl(xoshiro128_state[1] * 5, 7) * 9;

	const uint32_t t = xoshiro128_state[1] << 9;

	xoshiro128_state[2] ^= xoshiro128_state[0];
	xoshiro128_state[3] ^= xoshiro128_state[1];
	xoshiro128_state[1] ^= xoshiro128_state[2];
	xoshiro128_state[0] ^= xoshiro128_state[3];

	xoshiro128_state[2] ^= t;

	xoshiro128_state[3] = xoshiro128_rotl(xoshiro128_state[3], 11);

	return result;
}


/* This is the jump function for the generator. It is equivalent
   to 2^64 calls to next(); it can be used to generate 2^64
   non-overlapping subsequences for parallel computations. */
static void __attribute__((unused)) xoshiro128_jump(void) {
	static const uint32_t JUMP[] = { 0x8764000b, 0xf542d2d3, 0x6fa035c3, 0x77f2db5b };

	uint32_t s0 = 0;
	uint32_t s1 = 0;
	uint32_t s2 = 0;
	uint32_t s3 = 0;
	for(int i = 0; i < sizeof JUMP / sizeof *JUMP; i++)
		for(int b = 0; b < 32; b++) {
			if (JUMP[i] & UINT32_C(1) << b) {
				s0 ^= xoshiro128_state[0];
				s1 ^= xoshiro128_state[1];
				s2 ^= xoshiro128_state[2];
				s3 ^= xoshiro128_state[3];
			}
			// Note: "starstar" only changes the returned result
			xoshiro128_next_plus();	
		}
		
	xoshiro128_state[0] = s0;
	xoshiro128_state[1] = s1;
	xoshiro128_state[2] = s2;
	xoshiro128_state[3] = s3;
}


/* This is the long-jump function for the generator. It is equivalent to
   2^96 calls to next(); it can be used to generate 2^32 starting points,
   from each of which jump() will generate 2^32 non-overlapping
   subsequences for parallel distributed computations. */
static void __attribute__((unused)) xoshiro128_long_jump(void) {
	static const uint32_t LONG_JUMP[] = { 0xb523952e, 0x0b6f099f, 0xccf5a0ef, 0x1c580662 };

	uint32_t s0 = 0;
	uint32_t s1 = 0;
	uint32_t s2 = 0;
	uint32_t s3 = 0;
	for(int i = 0; i < sizeof LONG_JUMP / sizeof *LONG_JUMP; i++)
		for(int b = 0; b < 32; b++) {
			if (LONG_JUMP[i] & UINT32_C(1) << b) {
				s0 ^= xoshiro128_state[0];
				s1 ^= xoshiro128_state[1];
				s2 ^= xoshiro128_state[2];
				s3 ^= xoshiro128_state[3];
			}
			// Note: "starstar" only changes the returned result
			xoshiro128_next_plus();	
		}
		
	xoshiro128_state[0] = s0;
	xoshiro128_state[1] = s1;
	xoshiro128_state[2] = s2;
	xoshiro128_state[3] = s3;
}
