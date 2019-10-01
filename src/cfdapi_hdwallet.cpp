// Copyright 2019 CryptoGarage
/**
 * @file cfdapi_hdwallet.cpp
 *
 * @brief cfd-apiで利用するHDWallet APIクラスの実装
 */
#include <string>
#include <vector>

#include "cfdcore/cfdcore_hdwallet.h"

#include "cfd/cfd_common.h"
#include "cfd/cfdapi_hdwallet.h"
#include "cfdapi_internal.h"  // NOLINT

namespace cfd {
namespace api {

using cfdcore::HDWallet;

Bip39GetWordlistResponseStruct HDWalletApi::Bip39GetWordlist(
    const Bip39GetWordlistRequestStruct& request) {
  auto call_func = [](const Bip39GetWordlistRequestStruct& request)
      -> Bip39GetWordlistResponseStruct {
    Bip39GetWordlistResponseStruct response;
    // check language is support
    std::string language = request.language;

    // get bip39 wordlist
    std::vector<std::string> wordlist = HDWallet::Bip39GetWordlist(language);

    response.wordlist = wordlist;
    return response;
  };

  Bip39GetWordlistResponseStruct result;
  result = ExecuteStructApi<
      Bip39GetWordlistRequestStruct, Bip39GetWordlistResponseStruct>(
      request, call_func, std::string(__FUNCTION__));
  return result;
}

}  // namespace api
}  // namespace cfd
