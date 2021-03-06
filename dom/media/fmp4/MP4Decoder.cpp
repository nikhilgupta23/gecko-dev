/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 sts=2 et cindent: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "MP4Decoder.h"
#include "MediaDecoderStateMachine.h"
#include "MediaFormatReader.h"
#include "MP4Demuxer.h"
#include "mozilla/Preferences.h"
#include "nsCharSeparatedTokenizer.h"
#ifdef MOZ_EME
#include "mozilla/CDMProxy.h"
#endif
#include "mozilla/Logging.h"
#include "nsMimeTypes.h"
#include "nsContentTypeParser.h"
#include "VideoUtils.h"

#ifdef XP_WIN
#include "mozilla/WindowsVersion.h"
#endif
#ifdef MOZ_WIDGET_ANDROID
#include "nsIGfxInfo.h"
#include "AndroidBridge.h"
#endif
#include "mozilla/layers/LayersTypes.h"

#ifdef MOZ_FFMPEG
#include "FFmpegRuntimeLinker.h"
#endif

namespace mozilla {

#if defined(MOZ_GONK_MEDIACODEC) || defined(XP_WIN) || defined(MOZ_APPLEMEDIA) || defined(MOZ_FFMPEG)
#define MP4_READER_DORMANT_HEURISTIC
#else
#undef MP4_READER_DORMANT_HEURISTIC
#endif

MP4Decoder::MP4Decoder()
{
#if defined(MP4_READER_DORMANT_HEURISTIC)
  mDormantSupported = Preferences::GetBool("media.decoder.heuristic.dormant.enabled", false);
#endif
}

MediaDecoderStateMachine* MP4Decoder::CreateStateMachine()
{
  MediaDecoderReader* reader = new MediaFormatReader(this, new MP4Demuxer(GetResource()));

  return new MediaDecoderStateMachine(this, reader);
}

static bool
IsWhitelistedH264Codec(const nsAString& aCodec)
{
  int16_t profile = 0, level = 0;

  if (!ExtractH264CodecDetails(aCodec, profile, level)) {
    return false;
  }

#ifdef XP_WIN
  // Disable 4k video on windows vista since it performs poorly.
  if (!IsWin7OrLater() &&
      level >= H264_LEVEL_5) {
    return false;
  }
#endif

  // Just assume what we can play on all platforms the codecs/formats that
  // WMF can play, since we don't have documentation about what other
  // platforms can play... According to the WMF documentation:
  // http://msdn.microsoft.com/en-us/library/windows/desktop/dd797815%28v=vs.85%29.aspx
  // "The Media Foundation H.264 video decoder is a Media Foundation Transform
  // that supports decoding of Baseline, Main, and High profiles, up to level
  // 5.1.". We also report that we can play Extended profile, as there are
  // bitstreams that are Extended compliant that are also Baseline compliant.
  return level >= H264_LEVEL_1 &&
         level <= H264_LEVEL_5_1 &&
         (profile == H264_PROFILE_BASE ||
          profile == H264_PROFILE_MAIN ||
          profile == H264_PROFILE_EXTENDED ||
          profile == H264_PROFILE_HIGH);
}

/* static */
bool
MP4Decoder::CanHandleMediaType(const nsACString& aMIMETypeExcludingCodecs,
                               const nsAString& aCodecs)
{
  if (!IsEnabled()) {
    return false;
  }

  // Whitelist MP4 types, so they explicitly match what we encounter on
  // the web, as opposed to what we use internally (i.e. what our demuxers
  // etc output).
  const bool isMP4Audio = aMIMETypeExcludingCodecs.EqualsASCII("audio/mp4") ||
                          aMIMETypeExcludingCodecs.EqualsASCII("audio/x-m4a");
  const bool isMP4Video = aMIMETypeExcludingCodecs.EqualsASCII("video/mp4") ||
                          aMIMETypeExcludingCodecs.EqualsASCII("video/x-m4v");
  if (!isMP4Audio && !isMP4Video) {
    return false;
  }

#ifdef MOZ_GONK_MEDIACODEC
  if (aMIMETypeExcludingCodecs.EqualsASCII(VIDEO_3GPP)) {
    return Preferences::GetBool("media.fragmented-mp4.gonk.enabled", false);
  }
#endif

  nsTArray<nsCString> codecMimes;
  if (aCodecs.IsEmpty()) {
    // No codecs specified. Assume AAC/H.264
    if (isMP4Audio) {
      codecMimes.AppendElement(NS_LITERAL_CSTRING("audio/mp4a-latm"));
    } else {
      MOZ_ASSERT(isMP4Video);
      codecMimes.AppendElement(NS_LITERAL_CSTRING("video/avc"));
    }
  } else {
    // Verify that all the codecs specified are ones that we expect that
    // we can play.
    nsTArray<nsString> codecs;
    if (!ParseCodecsString(aCodecs, codecs)) {
      return false;
    }
    for (const nsString& codec : codecs) {
      if (IsAACCodecString(codec)) {
        codecMimes.AppendElement(NS_LITERAL_CSTRING("audio/mp4a-latm"));
        continue;
      }
      if (codec.EqualsLiteral("mp3")) {
        codecMimes.AppendElement(NS_LITERAL_CSTRING("audio/mpeg"));
        continue;
      }
      // Note: Only accept H.264 in a video content type, not in an audio
      // content type.
      if (IsWhitelistedH264Codec(codec) && isMP4Video) {
        codecMimes.AppendElement(NS_LITERAL_CSTRING("video/avc"));
        continue;
      }
      // Some unsupported codec.
      return false;
    }
  }

  // Verify that we have a PDM that supports the whitelisted types.
  PlatformDecoderModule::Init();
  nsRefPtr<PlatformDecoderModule> platform = PlatformDecoderModule::Create();
  if (!platform) {
    return false;
  }
  for (const nsCString& codecMime : codecMimes) {
    if (!platform->SupportsMimeType(codecMime)) {
      return false;
    }
  }

  return true;
}

/* static */ bool
MP4Decoder::CanHandleMediaType(const nsAString& aContentType)
{
  nsContentTypeParser parser(aContentType);
  nsAutoString mimeType;
  nsresult rv = parser.GetType(mimeType);
  if (NS_FAILED(rv)) {
    return false;
  }
  nsString codecs;
  parser.GetParameter("codecs", codecs);

  return CanHandleMediaType(NS_ConvertUTF16toUTF8(mimeType), codecs);
}

static bool
IsFFmpegAvailable()
{
#ifndef MOZ_FFMPEG
  return false;
#else
  PlatformDecoderModule::Init();
  nsRefPtr<PlatformDecoderModule> m = FFmpegRuntimeLinker::CreateDecoderModule();
  return !!m;
#endif
}

static bool
IsAppleAvailable()
{
#ifndef MOZ_APPLEMEDIA
  // Not the right platform.
  return false;
#else
  return Preferences::GetBool("media.apple.mp4.enabled", false);
#endif
}

static bool
IsAndroidAvailable()
{
#ifndef MOZ_WIDGET_ANDROID
  return false;
#else
  // We need android.media.MediaCodec which exists in API level 16 and higher.
  return AndroidBridge::Bridge() && (AndroidBridge::Bridge()->GetAPIVersion() >= 16);
#endif
}

static bool
IsGonkMP4DecoderAvailable()
{
#ifndef MOZ_GONK_MEDIACODEC
  return false;
#else
  return Preferences::GetBool("media.fragmented-mp4.gonk.enabled", false);
#endif
}

static bool
IsGMPDecoderAvailable()
{
  return Preferences::GetBool("media.fragmented-mp4.gmp.enabled", false);
}

static bool
HavePlatformMPEGDecoders()
{
  return Preferences::GetBool("media.fragmented-mp4.use-blank-decoder") ||
#ifdef XP_WIN
         // We have H.264/AAC platform decoders on Windows Vista and up.
         IsVistaOrLater() ||
#endif
         IsAndroidAvailable() ||
         IsFFmpegAvailable() ||
         IsAppleAvailable() ||
         IsGonkMP4DecoderAvailable() ||
         IsGMPDecoderAvailable() ||
         // TODO: Other platforms...
         false;
}

/* static */
bool
MP4Decoder::IsEnabled()
{
  return Preferences::GetBool("media.fragmented-mp4.enabled") &&
         HavePlatformMPEGDecoders();
}

static const uint8_t sTestH264ExtraData[] = {
  0x01, 0x64, 0x00, 0x0a, 0xff, 0xe1, 0x00, 0x17, 0x67, 0x64,
  0x00, 0x0a, 0xac, 0xd9, 0x44, 0x26, 0x84, 0x00, 0x00, 0x03,
  0x00, 0x04, 0x00, 0x00, 0x03, 0x00, 0xc8, 0x3c, 0x48, 0x96,
  0x58, 0x01, 0x00, 0x06, 0x68, 0xeb, 0xe3, 0xcb, 0x22, 0xc0
};

static already_AddRefed<MediaDataDecoder>
CreateTestH264Decoder(layers::LayersBackend aBackend,
                      VideoInfo& aConfig)
{
  aConfig.mMimeType = "video/avc";
  aConfig.mId = 1;
  aConfig.mDuration = 40000;
  aConfig.mMediaTime = 0;
  aConfig.mDisplay = nsIntSize(64, 64);
  aConfig.mImage = nsIntRect(0, 0, 64, 64);
  aConfig.mExtraData = new MediaByteBuffer();
  aConfig.mExtraData->AppendElements(sTestH264ExtraData,
                                     MOZ_ARRAY_LENGTH(sTestH264ExtraData));

  PlatformDecoderModule::Init();

  nsRefPtr<PlatformDecoderModule> platform = PlatformDecoderModule::Create();
  if (!platform || !platform->SupportsMimeType(NS_LITERAL_CSTRING("video/mp4"))) {
    return nullptr;
  }

  nsRefPtr<MediaDataDecoder> decoder(
    platform->CreateDecoder(aConfig, nullptr, nullptr, aBackend, nullptr));
  if (!decoder) {
    return nullptr;
  }

  return decoder.forget();
}

/* static */ bool
MP4Decoder::IsVideoAccelerated(layers::LayersBackend aBackend, nsACString& aFailureReason)
{
  VideoInfo config;
  nsRefPtr<MediaDataDecoder> decoder(CreateTestH264Decoder(aBackend, config));
  if (!decoder) {
    aFailureReason.AssignLiteral("Failed to create H264 decoder");
    return false;
  }
  bool result = decoder->IsHardwareAccelerated(aFailureReason);
  return result;
}

/* static */ bool
MP4Decoder::CanCreateH264Decoder()
{
#ifdef XP_WIN
  static bool haveCachedResult = false;
  static bool result = false;
  if (haveCachedResult) {
    return result;
  }
  VideoInfo config;
  nsRefPtr<MediaDataDecoder> decoder(
    CreateTestH264Decoder(layers::LayersBackend::LAYERS_BASIC, config));
  if (decoder) {
    decoder->Shutdown();
    result = true;
  }
  haveCachedResult = true;
  return result;
#else
  return IsEnabled();
#endif
}

#ifdef XP_WIN
static already_AddRefed<MediaDataDecoder>
CreateTestAACDecoder(AudioInfo& aConfig)
{
  PlatformDecoderModule::Init();

  nsRefPtr<PlatformDecoderModule> platform = PlatformDecoderModule::Create();
  if (!platform || !platform->SupportsMimeType(NS_LITERAL_CSTRING("audio/mp4a-latm"))) {
    return nullptr;
  }

  nsRefPtr<MediaDataDecoder> decoder(
    platform->CreateDecoder(aConfig, nullptr, nullptr));
  if (!decoder) {
    return nullptr;
  }

  return decoder.forget();
}

// bipbop.mp4's extradata/config...
static const uint8_t sTestAACExtraData[] = {
  0x03, 0x80, 0x80, 0x80, 0x22, 0x00, 0x02, 0x00, 0x04, 0x80,
  0x80, 0x80, 0x14, 0x40, 0x15, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x11, 0x51, 0x00, 0x00, 0x11, 0x51, 0x05, 0x80, 0x80, 0x80,
  0x02, 0x13, 0x90, 0x06, 0x80, 0x80, 0x80, 0x01, 0x02
};

static const uint8_t sTestAACConfig[] = { 0x13, 0x90 };

#endif // XP_WIN

/* static */ bool
MP4Decoder::CanCreateAACDecoder()
{
#ifdef XP_WIN
  static bool haveCachedResult = false;
  static bool result = false;
  if (haveCachedResult) {
    return result;
  }
  AudioInfo config;
  config.mMimeType = "audio/mp4a-latm";
  config.mRate = 22050;
  config.mChannels = 2;
  config.mBitDepth = 16;
  config.mProfile = 2;
  config.mExtendedProfile = 2;
  config.mCodecSpecificConfig->AppendElements(sTestAACConfig,
                                              MOZ_ARRAY_LENGTH(sTestAACConfig));
  config.mExtraData->AppendElements(sTestAACExtraData,
                                    MOZ_ARRAY_LENGTH(sTestAACExtraData));
  nsRefPtr<MediaDataDecoder> decoder(CreateTestAACDecoder(config));
  if (decoder) {
    result = true;
  }
  haveCachedResult = true;
  return result;
#else
  return IsEnabled();
#endif
}

} // namespace mozilla
