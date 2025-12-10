/******************************************************************************
 * SPDX-License-Identifier: LGPL-3.0-only
 *
 * This file is part of hed.
 ******************************************************************************/

#ifndef _HED_CODEC_H
#define _HED_CODEC_H

#include <hed/cdefs.h>
#include <dpack/codec.h>
#include <galv/rpc.h>

/**
 * Initialize a MessagePack encoder with galv session message
 *
 * @param[inout] encoder encoder
 * @param[inout] msg     galv session message
 *
 * Initialize a @rstsubst{MessagePack} encoder for encoding / packing /
 * serialization purpose.
 *
 * @p msg is a previously allocated and owned by the caller. It is the
 * responsibility of the caller to manage allocation and release of @p msg.
 *
 * Owner of @p msg *MUST* call hed_encoder_fini() before @man{free(3)}'ing it.
 *
 * @see
 * hed_encoder_fini()
 */
extern void
hed_encoder_init(struct dpack_encoder * encoder,
                 struct hed_rpc_ms  g * msg)
	__hed_nonull(1, 2);

/**
 * Finalize a MessagePack encoder
 *
 * @param[inout] encoder encoder
 * @param[in]    abort   request abortion
 *
 * Release resources allocated for @p encoder.
 *
 * When specified as #DPACK_ABORT, @p abort argument request abortion of current
 * serialization and content of @p msg previously registered at
 * hed_encoder_init() time is undefined. Passing #DPACK_DONE instead
 * completes serialization successfully.
 *
 * @p msg previously registered at hed_encoder_init() time may
 * safely be @man{free(3)}'ed once dpack_encoder_fini() has been called only.
 *
 * @see
 * - hed_encoder_init()
 * - #DPACK_DONE
 * - #DPACK_ABORT
 */
static inline void __dpack_nonull(1)
hed_encoder_fini(struct dpack_encoder * encoder, bool abort)
{
	hed_assert_api(encoder);

	dpack_encoder_fini(encoder, abort);
}

/**
 * Initialize a MessagePack decoder with galv session message
 *
 * @param[inout] decoder decoder
 * @param[in]    msg     galv session message
 *
 * Initialize a @rstsubst{MessagePack} decoder for decoding / unpacking /
 * deserialization purpose.
 *
 * @p msg is a previously allocated and owned by the caller. It is the
 * responsibility of the caller to manage allocation and release of @p msg.
 *
 * Owner of @p msg *MUST* call hed_encoder_fini() before @man{free(3)}'ing it.
 *
 * @see
 * - hed_decoder_fini()
 */
extern void
hed_decoder_init(struct dpack_decoder * decoder,
                 struct hed_rpc_msg   * msg)
	__hed_nonull(1, 2);

/**
 * Finalize a MessagePack decoder
 *
 * @param[inout] decoder decoder
 *
 * Release resources allocated for @p decoder.
 *
 * @p buffer previously registered at dpack_decoder_init_buffer() /
 * dpack_decoder_init_skip_buffer() time may safely be @man{free(3)}'ed once
 * dpack_decoder_fini() has been called only.
 *
 * @see
 * - hed_decoder_init()
 */
static inline void __dpack_nonull(1)
hed_decoder_fini(struct dpack_decoder * decoder)
{
	hed_assert_api(decoder);

	dpack_decoder_fini(decoder);
}

#endif /* _HED_CODEC_H */
