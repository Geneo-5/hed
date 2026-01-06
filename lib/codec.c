/******************************************************************************
 * SPDX-License-Identifier: LGPL-3.0-only
 *
 * This file is part of hed.
 ******************************************************************************/

#include "hed/codec.h"
#include "stroll/page.h"

struct reader_ctx {
	struct galv_sess_msg *msg;
	size_t                last_size;
};

static void __hed_nonull(1)
hed_reader_skip(mpack_reader_t * reader, size_t count)
{
	hed_assert_intern(reader);

	struct galv_sess_msg *msg = mpack_reader_context(reader);
	const uint8_t        *data;
	ssize_t               size;

	hed_assert_intern(msg);

	while (count) {
		size = galv_sess_msg_pull_head(msg, &data, count);
		if (size <= 0) {
			mpack_reader_flag_error(reader, mpack_error_eof);
			return;
		}

		hed_assert_intern(data);
		hed_assert_intern((size_t)size <= count);

		count -= (size_t)size;
	}
}

static size_t __hed_nonull(1, 2)
hed_reader_fill(mpack_reader_t * reader, char * buffer, size_t count)
{
	hed_assert_intern(reader);
	hed_assert_intern(buffer);

	struct galv_sess_msg *msg = mpack_reader_context(reader);
	ssize_t               ret;

	hed_assert_intern(msg);
	ret = galv_sess_msg_read(msg, (uint8_t *)buffer, count);
	if (ret <= 0) {
		hed_assert_intern(ret == -ENODATA);
		mpack_reader_flag_error(reader, mpack_error_eof);
		return 0;
	}
	return (size_t)ret;
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
hed_decoder_abort(struct dpack_decoder * decoder,
                  enum mpack_type_t      type __unused,
                  unsigned int           nr __unused)
{
	hed_assert_intern(decoder);

	mpack_reader_flag_error(&decoder->mpack, mpack_error_data);
}

void
hed_decoder_init(struct dpack_decoder * decoder,
                 struct hed_rpc_msg   * msg)
{
	hed_assert_api(decoder);
	hed_assert_api(msg);

	size_t capacity = stroll_page_size();
	char * buffer = malloc(capacity);

	if (!buffer) {
		mpack_reader_init_error(&decoder->mpack, mpack_error_memory);
		return;
	}

	mpack_reader_init(&decoder->mpack, buffer, capacity, 0);
	mpack_reader_set_context(&decoder->mpack, &msg->super);
	mpack_reader_set_fill(&decoder->mpack, hed_reader_fill);
	mpack_reader_set_skip(&decoder->mpack, hed_reader_skip);
	mpack_reader_set_teardown(&decoder->mpack, hed_reader_teardown);
	decoder->intr = hed_decoder_abort;
}

static void __hed_nonull(1, 2)
hed_writer_flush(mpack_writer_t * writer, const char * buffer, size_t count)
{
	hed_assert_intern(writer);
	hed_assert_intern(buffer);

	struct galv_sess_msg *msg = mpack_writer_context(writer);
	int ret;

	hed_assert_intern(msg);

	ret = galv_sess_msg_write(msg, (const uint8_t *)buffer, count);
	switch (ret) {
	case 0:
		// normal case
		break;
	case -ENOBUFS:
	case -ENOMEM:
		mpack_writer_flag_error(writer, mpack_error_memory);
		break;
	default:
		hed_assert_intern(0);
		mpack_writer_flag_error(writer, mpack_error_io);
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
                 struct hed_rpc_msg   * msg)
{
	hed_assert_api(encoder);
	hed_assert_api(msg);

	size_t capacity = stroll_page_size();
	char * buffer = malloc(capacity);

	if (!buffer) {
		mpack_writer_init_error(&encoder->mpack, mpack_error_memory);
		return;
	}

	mpack_writer_init(&encoder->mpack, buffer, capacity);
	mpack_writer_set_context(&encoder->mpack, &msg->super);
	mpack_writer_set_flush(&encoder->mpack, hed_writer_flush);
	mpack_writer_set_teardown(&encoder->mpack, hed_writer_teardown);
}
