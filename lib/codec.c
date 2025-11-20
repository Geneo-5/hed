/******************************************************************************
 * SPDX-License-Identifier: LGPL-3.0-only
 *
 * This file is part of hed.
 ******************************************************************************/

#include "hed/codec.h"
#include "utils/cdefs.h"

static size_t __hed_nonull(1, 2)
hed_reader_fill(mpack_reader_t * reader, char * buffer, size_t count)
{
	hed_assert_intern(reader);
	hed_assert_intern(buffer);

	struct galv_sess_msg *msg = mpack_reader_context(reader);
	uint8_t              *data;
	ssize_t               size;
	size_t                ret = 0;

	hed_assert_intern(msg);

	while (count) {
		size = galv_sess_msg_pull_head(msg, &data, count);
		if (size <= 0) {
			if ((size == -ENODATA) && (ret))
				return ret;

			mpack_reader_flag_error(reader, mpack_error_eof);
			return 0;
		}

		hed_assert_intern(data);
		hed_assert_intern(size <= count);

		memcpy(buffer, data, size);
		count -= size;
		buffer += size;
		ret += size;
	}

	return ret;
}

static void __hed_nonull(1)
hed_reader_skip(mpack_reader_t * reader, size_t count)
{
	hed_assert_intern(reader);

	struct galv_sess_msg *msg = mpack_reader_context(reader);
	uint8_t              *data;
	ssize_t               size;

	hed_assert_intern(msg);

	while (count) {
		size = galv_sess_msg_pull_head(msg, &data, count);
		if (size <= 0) {
			mpack_reader_flag_error(reader, mpack_error_io);
			return 0;
		}

		hed_assert_intern(data);
		hed_assert_intern(size <= count);

		count -= size;
	}
}

static void __hed_nonull(1)
hed_reader_teardown(mpack_reader_t * reader)
{
	hed_assert_intern(reader);

	free(reader->buffer);
	reader->buffer = NULL;
	reader->context = NULL;
	reader->size = 0;
	reader->fill = NULL;
	reader->skip = NULL;
	reader->teardown = NULL;
}

static void __hed_nonull(1)
dpack_decoder_abort(struct dpack_decoder * decoder,
                    enum mpack_type_t      type __unused,
                    unsigned int           nr __unused)
{
	hed_assert_intern(decoder);

	mpack_reader_flag_error(&decoder->mpack, mpack_error_data);
}

void
hed_decoder_init(struct dpack_decoder * decoder,
                 struct galv_sess_msg * msg)
{
	hed_assert_api(decoder);
	hed_assert_api(msg);

	size_t capacity = usys_page_size();
	char * buffer = malloc(capacity);

	if (!buffer) {
		mpack_reader_init_error(decoder->mpack, mpack_error_memory);
		return;
	}

	mpack_reader_init(decoder->mpack, buffer, capacity, 0);
	mpack_reader_set_context(decoder->mpack, msg);
	mpack_reader_set_fill(decoder->mpack, hed_reader_fill);
	mpack_reader_set_skip(decoder->mpack, hed_reader_skip);
	mpack_reader_set_teardown(decoder->mpack, hed_reader_teardown);
	decoder->intr = hed_decoder_abort;
}

static void __hed_nonull(1, 2)
hed_writer_flush(mpack_writer_t * writer, const char * buffer, size_t count)
{
	hed_assert_intern(writer);
	hed_assert_intern(buffer);

	struct galv_sess_msg *msg = mpack_writer_context(writer);
	uint8_t              *data;
	ssize_t               size;

	hed_assert_intern(msg);

	while (count) {
		size = galv_sess_msg_push_tail(msg, &data, count);
		if (size <= 0) {
			mpack_writer_flag_error(writer, mpack_error_io);
			return;
		}

		hed_assert_intern(data);
		hed_assert_intern(size <= count);

		memcpy(data, buffer, size);
		count -= size;
		buffer += size;
	}
}

static void __hed_nonull(1)
hed_writer_teardown(mpack_writer_t * writer)
{
	hed_assert_intern(writer);

	free(writer->buffer);
	writer->buffer = NULL;
	writer->context = NULL;
}

void
hed_encoder_init(struct dpack_encoder * encoder,
                 struct galv_sess_msg * msg)
{
	hed_assert_api(encoder);
	hed_assert_api(msg);

	size_t capacity = usys_page_size();
	char * buffer = malloc(capacity);

	if (!buffer) {
		mpack_writer_init_error(encoder->mpack, mpack_error_memory);
		return;
	}

	mpack_writer_init(encoder->mpack, buffer, capacity);
	mpack_writer_set_context(encoder->mpack, msg);
	mpack_writer_set_flush(encoder->mpack, hed_writer_flush);
	mpack_writer_set_teardown(encoder->mpack, hed_writer_teardown);
}
