/******************************************************************************
 * SPDX-License-Identifier: LGPL-3.0-only
 *
 * This file is part of hed.
 ******************************************************************************/

#include <hed/cdefs.h>
#include <hed/base.h>
#include <errno.h>
#include <dpack/codec.h>
#include <json-c/json_object.h>
#include <dpack/bin.h>
#include <utils/time.h>
#include <stdio.h>

int
hed_check_time(const struct  timespec *ts __unused)
{
	hed_assert_api(ts);

	if ((ts->tv_sec < 0) || (ts->tv_sec > UTIME_TIMET_MAX) ||
	    (ts->tv_nsec < 0) || (ts->tv_nsec >= 1000000000L))
		return -EINVAL;

	return 0;
}

int
hed_encode_time(struct dpack_encoder * encoder,
                const struct timespec * ts)
{
	hed_assert_api(encoder);
	hed_assert_api(ts);
	hed_assert_api(!hed_check_time(ts));

	return dpack_encode_bin(encoder,
	                        (const uint8_t *)ts,
	                        sizeof(*ts));
}

int
hed_decode_time(struct dpack_decoder * decoder,
                struct timespec      * ts)
{
	hed_assert_api(decoder);
	hed_assert_api(ts);

	ssize_t ret;

	ret = dpack_decode_bincpy_equ(decoder,
	                              sizeof(*ts),
	                              (uint8_t *)ts);
	if (ret < 0)
		return ret;

	return ret == sizeof(*ts) ? hed_check_time(ts) : -EINVAL;
}

#define TIME_FORMAT      "%FT%T"
#define TIME_FULL_FORMAT "%s.%03ldZ"

int
hed_encode_time_from_json(struct dpack_encoder * encoder,
                          struct json_object    * obj)
{
	struct timespec  ts;
	struct tm        tm = { 0 };
	const char      *asc;
	int              ret;

	hed_assert_api(encoder);
	hed_assert_api(obj);

	if (!json_object_is_type(obj, json_type_string))
		return -EINVAL;

	asc = json_object_get_string(obj);
	hed_assert_api(asc);

	asc = strptime(asc, TIME_FORMAT, &tm);
	if(!asc)
		return -EINVAL;

	ret = sscanf(asc, ".%ldZ", &ts.tv_nsec);
	if (ret != 1)
		return -EINVAL;

	ts.tv_sec = mktime(&tm);
	ts.tv_nsec *= 1000000L;
	ret = hed_check_time(&ts);
	if (ret)
		return ret;

	return hed_encode_time(encoder, &ts);
}

struct json_object *
hed_decode_time_to_json(struct dpack_decoder * decoder)
{
	struct timespec    ts;
	struct tm          tm;
	char               asc[50];
	char               time[30];
	int                ret;
	struct json_object *obj = NULL;

	hed_assert_api(decoder);

	ret = hed_decode_time(decoder, &ts);
	if (ret) {
		errno = -ret;
		goto error;
	}

	if (!gmtime_r(&ts.tv_sec, &tm)) {
		errno = EINVAL;
		goto error;
	}

	if (!strftime(time, sizeof(time), TIME_FORMAT, &tm)) {
		errno = EINVAL;
		goto error;
	}

	snprintf(asc, sizeof(asc), TIME_FULL_FORMAT, time, ts.tv_nsec / 1000000L);
	obj = json_object_new_string(asc);
error:
	return obj;
}

