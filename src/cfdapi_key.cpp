// Copyright 2019 CryptoGarage
/**
 * @file cfdapi_key.cpp
 *
 * @brief cfd-apiで利用する鍵関連の実装ファイル
 */
#include <string>

#include "cfd/cfdapi_address.h"
#include "cfdcore/cfdcore_key.h"

#include "cfd/cfd_common.h"
#include "cfd/cfdapi_key.h"
#include "cfdapi_internal.h"  // NOLINT

namespace cfd {
namespace js {
namespace api {

using cfd::api::KeyApi;
using cfd::core::NetType;
using cfd::core::Privkey;
using cfd::core::Pubkey;
using cfd::js::api::AddressStructApi;

CreateKeyPairResponseStruct KeyStructApi::CreateKeyPair(
    const CreateKeyPairRequestStruct& request) {
  auto call_func = [](const CreateKeyPairRequestStruct& request)
      -> CreateKeyPairResponseStruct {  // NOLINT
    CreateKeyPairResponseStruct response;

    // generate random private key
    bool is_compressed = request.is_compressed;
    Pubkey pubkey;
    Privkey privkey = KeyApi::CreateKeyPair(is_compressed, &pubkey);

    // convert parameters to response struct
    const bool is_wif = request.wif;
    const NetType net_type = AddressStructApi::ConvertNetType(request.network);
    response.privkey = is_wif ? privkey.ConvertWif(net_type, is_compressed)
                              : privkey.GetHex();  // NOLINT
    response.pubkey = pubkey.GetHex();
    return response;
  };

  CreateKeyPairResponseStruct result;
  result = ExecuteStructApi<
      CreateKeyPairRequestStruct, CreateKeyPairResponseStruct>(
      request, call_func, std::string(__FUNCTION__));
  return result;
}

}  // namespace api
}  // namespace js
}  // namespace cfd

namespace cfd {
namespace api {

Privkey KeyApi::CreateKeyPair(bool is_compressed, Pubkey* pubkey) {
  // generate random private key
  Privkey privkey = Privkey::GenerageRandomKey();

  // derive pubkey from private key
  const Pubkey out_pubkey = privkey.GeneratePubkey(is_compressed);

  if (pubkey != nullptr) {
    *pubkey = out_pubkey;
  }
  return privkey;
}

}  // namespace api
}  // namespace cfd
