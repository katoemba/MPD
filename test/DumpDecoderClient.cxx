/*
 * Copyright 2003-2018 The Music Player Daemon Project
 * http://www.musicpd.org
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "config.h"
#include "DumpDecoderClient.hxx"
#include "decoder/DecoderAPI.hxx"
#include "input/InputStream.hxx"
#include "util/StringBuffer.hxx"
#include "Compiler.h"

#include <unistd.h>
#include <stdio.h>

void
DumpDecoderClient::Ready(const AudioFormat audio_format,
			 gcc_unused bool seekable,
			 SignedSongTime duration)
{
	assert(!initialized);
	assert(audio_format.IsValid());

	fprintf(stderr, "audio_format=%s duration=%f\n",
		ToString(audio_format).c_str(),
		duration.ToDoubleS());

	initialized = true;
}

DecoderCommand
DumpDecoderClient::GetCommand() noexcept
{
	return DecoderCommand::NONE;
}

void
DumpDecoderClient::CommandFinished()
{
}

SongTime
DumpDecoderClient::GetSeekTime() noexcept
{
	return SongTime();
}

uint64_t
DumpDecoderClient::GetSeekFrame() noexcept
{
	return 1;
}

void
DumpDecoderClient::SeekError()
{
}

InputStreamPtr
DumpDecoderClient::OpenUri(const char *uri)
{
	return InputStream::OpenReady(uri, mutex);
}

size_t
DumpDecoderClient::Read(InputStream &is, void *buffer, size_t length)
{
	try {
		return is.LockRead(buffer, length);
	} catch (...) {
		return 0;
	}
}

void
DumpDecoderClient::SubmitTimestamp(gcc_unused double t)
{
}

DecoderCommand
DumpDecoderClient::SubmitData(gcc_unused InputStream *is,
			      const void *data, size_t datalen,
			      gcc_unused uint16_t kbit_rate)
{
	if (kbit_rate != prev_kbit_rate) {
		prev_kbit_rate = kbit_rate;
		fprintf(stderr, "%u kbit/s\n", kbit_rate);
	}

	gcc_unused ssize_t nbytes = write(STDOUT_FILENO, data, datalen);
	return DecoderCommand::NONE;
}

DecoderCommand
DumpDecoderClient::SubmitTag(gcc_unused InputStream *is,
			     Tag &&tag)
{
	fprintf(stderr, "TAG: duration=%f\n", tag.duration.ToDoubleS());

	for (const auto &i : tag)
		fprintf(stderr, "  %s=%s\n", tag_item_names[i.type], i.value);

	return DecoderCommand::NONE;
}

static void
DumpReplayGainTuple(const char *name, const ReplayGainTuple &tuple)
{
	if (tuple.IsDefined())
		fprintf(stderr, "replay_gain[%s]: gain=%f peak=%f\n",
			name, tuple.gain, tuple.peak);
}

static void
DumpReplayGainInfo(const ReplayGainInfo &info)
{
	DumpReplayGainTuple("album", info.album);
	DumpReplayGainTuple("track", info.track);
}

void
DumpDecoderClient::SubmitReplayGain(const ReplayGainInfo *rgi)
{
	if (rgi != nullptr)
		DumpReplayGainInfo(*rgi);
}

void
DumpDecoderClient::SubmitMixRamp(gcc_unused MixRampInfo &&mix_ramp)
{
	fprintf(stderr, "MixRamp: start='%s' end='%s'\n",
		mix_ramp.GetStart(), mix_ramp.GetEnd());
}
