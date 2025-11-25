#ifndef _HED_INET_H
#define _HED_INET_H

#include <hed/cdefs.h>
#include <netinet/ether.h>
#include <netinet/in.h>
#include <dpack/codec.h>
#include <json-c/json_object.h>
#include <dpack/bin.h>
#include <dpack/scalar.h>

static inline int __hed_nonull(1)
hed_check_ether_addr(struct ether_addr *addr __unused)
{
	hed_assert_api(addr);

	return 0;
}

static inline int __hed_nonull(1, 2)
hed_encode_ether_addr(struct dpack_encoder    * encoder,
                      const struct ether_addr * addr)
{
	hed_assert_api(encoder);
	hed_assert_api(addr);

	return dpack_encode_bin(encoder,
	                        (const char *)addr->ether_addr_octet,
	                        ETH_ALEN);
}

static inline int __hed_nonull(1, 2)
hed_decode_ether_addr(struct dpack_decoder * decoder,
                      struct ether_addr    * addr)
{
	hed_assert_api(decoder);
	hed_assert_api(addr);

	return dpack_decode_bincpy_equ(decoder,
	                               ETH_ALEN,
	                               (char *)addr->ether_addr_octet);
}

extern int
hed_encode_ether_addr_from_json(struct dpack_encoder * encoder,
                                struct json_object    * obj)
	__hed_nonull(1, 2);

extern  struct json_object *
hed_decode_ether_addr_to_json(struct dpack_decoder * decoder)
	__hed_nonull(1);

static inline int __hed_nonull(1)
hed_check_in_addr(struct in_addr *addr __unused)
{
	hed_assert_api(addr);

	return 0;
}

static inline int __hed_nonull(1, 2)
hed_encode_in_addr(struct dpack_encoder * encoder,
                   const struct in_addr * addr)
{
	hed_assert_api(encoder);
	hed_assert_api(addr);

	return dpack_encode_uint(encoder, addr->s_addr);
}

static inline int __hed_nonull(1, 2)
hed_decode_in_addr(struct dpack_decoder * decoder,
                   struct in_addr       * addr)
{
	hed_assert_api(decoder);
	hed_assert_api(addr);

	return dpack_decode_uint(decoder, &addr->s_addr);
}

extern int
hed_encode_in_addr_from_json(struct dpack_encoder * encoder,
                             struct json_object    * obj)
	__hed_nonull(1, 2);

extern  struct json_object *
hed_decode_in_addr_to_json(struct dpack_decoder * decoder)
	__hed_nonull(1);


static inline int __hed_nonull(1)
hed_check_in6_addr(struct in6_addr *addr __unused)
{
	hed_assert_api(addr);

	return 0;
}

static inline int __hed_nonull(1, 2)
hed_encode_in6_addr(struct dpack_encoder  * encoder,
                    const struct in6_addr * addr)
{
	hed_assert_api(encoder);
	hed_assert_api(addr);

	return dpack_encode_bin(encoder, (const char *)addr->s6_addr, 16);
}

static inline int __hed_nonull(1, 2)
hed_decode_in6_addr(struct dpack_decoder  * decoder,
                    struct in6_addr       * addr)
{
	hed_assert_api(decoder);
	hed_assert_api(addr);

	return dpack_decode_bincpy_equ(decoder, 16, (char *)addr->s6_addr);
}

extern int
hed_encode_in6_addr_from_json(struct dpack_encoder * encoder,
                              struct json_object    * obj)
	__hed_nonull(1, 2);

extern  struct json_object *
hed_decode_in6_addr_to_json(struct dpack_decoder * decoder)
	__hed_nonull(1);

#endif /* _HED_INET_H */
