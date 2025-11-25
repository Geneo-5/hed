/******************************************************************************
 * SPDX-License-Identifier: LGPL-3.0-only
 *
 * This file is part of hed.
 ******************************************************************************/

#include <hed/priv/inet.h>
#include <errno.h>
#include <arpa/inet.h>

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

