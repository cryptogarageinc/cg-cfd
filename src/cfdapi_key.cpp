// Copyright 2019 CryptoGarage
/**
 * @file cfdapi_key.cpp
 *
 * @brief cfd-apiで利用する鍵関連の実装ファイル
 */
#include <string>

#include "cfd/cfdapi_address.h"
#include "cfdcore/cfdcore_bytedata.h"
#include "cfdcore/cfdcore_key.h"
#include "cfdcore/cfdcore_transaction_common.h"

#include "cfd/cfd_common.h"
#include "cfd/cfdapi_key.h"

namespace cfd {
namespace api {

Privkey KeyApi::CreateKeyPair(
    bool is_compressed, Pubkey* pubkey, std::string* wif, NetType net_type) {
  // generate random private key
  Privkey privkey = Privkey::GenerageRandomKey();

  // derive pubkey from private key
  const Pubkey out_pubkey = privkey.GeneratePubkey(is_compressed);

  if (pubkey != nullptr) {
    *pubkey = out_pubkey;
  }

  if (wif != nullptr) {
    *wif = privkey.ConvertWif(net_type, is_compressed);
  }
  return privkey;
}

}  // namespace api
}  // namespace cfd

