// Copyright 2019 CryptoGarage
/**
 * @file cfdapi_hdwallet.cpp
 *
 * @brief cfd-apiで利用するHDWallet APIクラスの実装
 */
#include <string>
#include <vector>

#include "cfdcore/cfdcore_bytedata.h"
#include "cfdcore/cfdcore_exception.h"
#include "cfdcore/cfdcore_hdwallet.h"
#include "cfdcore/cfdcore_logger.h"

#include "cfd/cfd_common.h"
#include "cfd/cfdapi_hdwallet.h"

//////////////////////////////////
/// HDWalletApi
//////////////////////////////////
namespace cfd {
namespace api {

using cfd::core::ByteData;
using cfd::core::CfdError;
using cfd::core::CfdException;
using cfd::core::HDWallet;
using cfd::core::logger::warn;

std::vector<std::string> HDWalletApi::GetMnemonicWordlist(
    const std::string& language) {
  try {
    return HDWallet::GetMnemonicWordlist(language);
  } catch (CfdException& e) {
    warn(
        CFD_LOG_SOURCE, "Failed to GetMnemonicWordlist. error: [{}]",
        e.what());
    throw CfdException(
        CfdError::kCfdIllegalArgumentError,
        "Failed to ConvertMnemonicToSeed. " + std::string(e.what()));
  }
}

ByteData HDWalletApi::ConvertMnemonicToSeed(
    const std::vector<std::string>& mnemonic, const std::string& passphrase,
    bool strict_check, const std::string& language, bool use_ideographic_space,
    ByteData* entropy) {
  if (strict_check) {
    if (language.empty()) {
      warn(
          CFD_LOG_SOURCE,
          "Failed to ConvertMnemonicToSeed. If check mnemonic strictly, need "
          "to set language.");
      throw CfdException(
          CfdError::kCfdIllegalArgumentError,
          "Failed to ConvertMnemonicToSeed. If check mnemonic strictly, need "
          "to set language.");
    }

    if (!HDWallet::CheckValidMnemonic(mnemonic, language)) {
      warn(
          CFD_LOG_SOURCE,
          "Failed to ConvertMnemonicToSeed. Mnemonic strict check error.");
      throw CfdException(
          CfdError::kCfdIllegalArgumentError,
          "Failed to ConvertMnemonicToSeed. Mnemonic strict check error.");
    }
  }

  try {
    // calculate entropy
    if (!language.empty() && entropy != nullptr) {
      *entropy = HDWallet::ConvertMnemonicToEntropy(mnemonic, language);
    }

    // calculate seed
    HDWallet wallet(mnemonic, passphrase, use_ideographic_space);
    return wallet.GetSeed();
  } catch (CfdException& e) {
    warn(
        CFD_LOG_SOURCE, "Failed to ConvertMnemonicToSeed. error: [{}]",
        e.what());
    throw CfdException(
        CfdError::kCfdIllegalArgumentError,
        "Failed to ConvertMnemonicToSeed. " + std::string(e.what()));
  }
}

std::vector<std::string> HDWalletApi::ConvertEntropyToMnemonic(
    const ByteData& entropy, const std::string& language) {
  try {
    // calculate seed
    std::vector<std::string> mnemonic =
        HDWallet::ConvertEntropyToMnemonic(entropy, language);
    return mnemonic;
  } catch (CfdException& e) {
    warn(
        CFD_LOG_SOURCE, "Failed to GetMnemonicWordlist. error: [{}]",
        e.what());

    throw CfdException(
        CfdError::kCfdIllegalArgumentError,
        "Failed to ConvertEntropyToMnemonic. " + std::string(e.what()));
  }
}

}  // namespace api
}  // namespace cfd
