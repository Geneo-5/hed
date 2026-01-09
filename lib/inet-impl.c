/******************************************************************************
 * SPDX-License-Identifier: LGPL-3.0-only
 *
 * This file is part of hed.
 ******************************************************************************/

#include <hed/cdefs.h>
#include <hed/inet.h>
#include <errno.h>
#include <arpa/inet.h>
#include <dpack/codec.h>
#include <json-c/json_object.h>
#include <dpack/bin.h>
#include <dpack/scalar.h>

int
hed_check_ether_addr(const struct ether_addr *addr __unused)
{
	hed_assert_api(addr);

	return 0;
}

int
hed_encode_ether_addr(struct dpack_encoder    * encoder,
                      const struct ether_addr * addr)
{
	hed_assert_api(encoder);
	hed_assert_api(addr);

	return dpack_encode_bin(encoder,
	                        (const uint8_t *)addr->ether_addr_octet,
	                        ETH_ALEN);
}

int
hed_decode_ether_addr(struct dpack_decoder * decoder,
                      struct ether_addr    * addr)
{
	hed_assert_api(decoder);
	hed_assert_api(addr);

	return dpack_decode_bincpy_equ(decoder,
	                               ETH_ALEN,
	                               (uint8_t *)addr->ether_addr_octet);
}

int
hed_encode_ether_addr_from_json(struct dpack_encoder * encoder,
                                struct json_object    * obj)
{
	struct ether_addr  addr;
	const char        *asc;
	int                ret;

	hed_assert_api(encoder);
	hed_assert_api(obj);

	if (!json_object_is_type(obj, json_type_string))
		return -EINVAL;

	asc = json_object_get_string(obj);
	hed_assert_api(asc);

	ret = hed_check_ether_addr(&addr);
	if (ret)
		return ret;

	if (!ether_aton_r(asc, &addr))
		return -EINVAL;

	return hed_encode_ether_addr(encoder, &addr);
}

struct json_object *
hed_decode_ether_addr_to_json(struct dpack_decoder * decoder)
{
	struct ether_addr  addr;
	char               asc[18];
	int                ret;
	struct json_object *obj = NULL;

	hed_assert_api(decoder);

	ret = hed_decode_ether_addr(decoder, &addr);
	if (ret) {
		errno = -ret;
		goto error;
	}

	ret = hed_check_ether_addr(&addr);
	if (ret) {
		errno = -ret;
		goto error;
	}

	if (!ether_ntoa_r(&addr, asc)) {
		errno = EINVAL;
		goto error;
	}

	obj = json_object_new_string(asc);
error:
	return obj;
}

int
hed_check_in_addr(const struct in_addr *addr __unused)
{
	hed_assert_api(addr);

	return 0;
}

int
hed_encode_in_addr(struct dpack_encoder * encoder,
                   const struct in_addr * addr)
{
	hed_assert_api(encoder);
	hed_assert_api(addr);

	return dpack_encode_uint(encoder, addr->s_addr);
}

int
hed_decode_in_addr(struct dpack_decoder * decoder,
                   struct in_addr       * addr)
{
	hed_assert_api(decoder);
	hed_assert_api(addr);

	return dpack_decode_uint(decoder, &addr->s_addr);
}

int
hed_encode_in_addr_from_json(struct dpack_encoder * encoder,
                             struct json_object    * obj)
{
	struct in_addr  addr;
	const char     *buf;
	int             ret;

	hed_assert_api(encoder);
	hed_assert_api(obj);

	if (!json_object_is_type(obj, json_type_string))
		return -EINVAL;

	buf = json_object_get_string(obj);
	hed_assert_api(buf);

	ret = inet_pton(AF_INET, buf, &addr);
	if (ret < 0)
		return -errno;
	if (ret == 0)
		return -EINVAL;

	ret = hed_check_in_addr(&addr);
	if (ret)
		return ret;

	return hed_encode_in_addr(encoder, &addr);
}

struct json_object *
hed_decode_in_addr_to_json(struct dpack_decoder * decoder)
{
	struct in_addr     addr;
	char               buf[INET_ADDRSTRLEN];
	int                ret;
	struct json_object *obj = NULL;

	hed_assert_api(decoder);

	ret = hed_decode_in_addr(decoder, &addr);
	if (ret) {
		errno = -ret;
		goto error;
	}

	ret = hed_check_in_addr(&addr);
	if (ret) {
		errno = -ret;
		goto error;
	}

	if (!inet_ntop(AF_INET, &addr, buf, sizeof(buf)))
		goto error;

	obj = json_object_new_string(buf);
error:
	return obj;
}

int
hed_check_in6_addr(const struct in6_addr *addr __unused)
{
	hed_assert_api(addr);

	return 0;
}

int
hed_encode_in6_addr(struct dpack_encoder  * encoder,
                    const struct in6_addr * addr)
{
	hed_assert_api(encoder);
	hed_assert_api(addr);

	return dpack_encode_bin(encoder, (const uint8_t *)addr->s6_addr, 16);
}

int
hed_decode_in6_addr(struct dpack_decoder  * decoder,
                    struct in6_addr       * addr)
{
	hed_assert_api(decoder);
	hed_assert_api(addr);

	return dpack_decode_bincpy_equ(decoder, 16, (uint8_t *)addr->s6_addr);
}

int
hed_encode_in6_addr_from_json(struct dpack_encoder * encoder,
                              struct json_object    * obj)
{
	struct in6_addr  addr;
	const char      *buf;
	int              ret;

	hed_assert_api(encoder);
	hed_assert_api(obj);

	if (!json_object_is_type(obj, json_type_string))
		return -EINVAL;

	buf = json_object_get_string(obj);
	hed_assert_api(buf);

	ret = inet_pton(AF_INET6, buf, &addr);
	if (ret < 0)
		return -errno;
	if (ret == 0)
		return -EINVAL;

	ret = hed_check_in6_addr(&addr);
	if (ret)
		return ret;

	return hed_encode_in6_addr(encoder, &addr);
}

struct json_object *
hed_decode_in6_addr_to_json(struct dpack_decoder * decoder)
{
	struct in6_addr    addr;
	char               buf[INET6_ADDRSTRLEN];
	int                ret;
	struct json_object *obj = NULL;

	hed_assert_api(decoder);

	ret = hed_decode_in6_addr(decoder, &addr);
	if (ret) {
		errno = -ret;
		goto error;
	}

	ret = hed_check_in6_addr(&addr);
	if (ret) {
		errno = -ret;
		goto error;
	}

	if (!inet_ntop(AF_INET6, &addr, buf, sizeof(buf)))
		goto error;

	obj = json_object_new_string(buf);
error:
	return obj;
}

int
hed_in_net_check_addr_prefix(const struct hed_in_net * value)
{
	hed_assert_api(value);

	const struct in_addr *addr;
	unsigned long         mask;

	addr = hed_in_net_get_addr(value);
	mask = (1U << (unsigned long)hed_in_net_get_prefix(value)) - 1U;

	return !!(addr->s_addr & mask);
}

int
hed_in6_net_check_addr_prefix(const struct hed_in6_net * value)
{
	hed_assert_api(value);

	const struct in6_addr *addr;
	uint8_t                prefix;
	uint8_t                mask;
	uint8_t                ret = 0;

	addr = hed_in6_net_get_addr(value);
	prefix = hed_in6_net_get_prefix(value);
	for (int i = 0; i < 16; i++) {
		switch (prefix) {
		case 0:
			mask = 0xFF;
			break;
		case 1 ... 7:
			mask = (uint8_t)((1 << prefix) - 1);
			prefix = 0;
			break;
		default:
			mask = 0;
			prefix = (uint8_t)(prefix - 8);
			break;
		}
		ret |= addr->s6_addr[i] & mask;
	}

	return !!ret;
}
