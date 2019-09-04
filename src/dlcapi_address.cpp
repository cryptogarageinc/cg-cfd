// Copyright 2019 CryptoGarage
/**
 * @file dlcapi_address.cpp
 *
 * @brief dlc-apiで利用するAddress操作の実装ファイル
 */

#include <limits>
#include <string>
#include <vector>

#include "cfd/cfd_address.h"
#include "cfd/dlc_script.h"
#include "cfdcore/cfdcore_address.h"
#include "cfdcore/cfdcore_exception.h"
#include "cfdcore/cfdcore_logger.h"
#include "cfdcore/cfdcore_script.h"

#include "cfd/cfdapi_address.h"
#include "cfd/cfdapi_struct.h"
#include "cfd/dlcapi_address.h"
#include "dlcapi_internal.h"  // NOLINT

namespace dlc {
namespace api {

using cfd::AddressUtil;
using cfd::api::AddressApi;
using cfdcore::Address;
using cfdcore::CfdError;
using cfdcore::CfdException;
using cfdcore::NetType;
using cfdcore::Pubkey;
using cfdcore::Script;
using cfdcore::WitnessVersion;
using cfdcore::logger::warn;
using dlc::DlcScriptUtil;

CreateCETxAddressResponseStruct DlcAddressApi::CreateCETxAddress(
    const CreateCETxAddressRequestStruct& request) {
  auto call_func = [](const CreateCETxAddressRequestStruct& request)
      -> CreateCETxAddressResponseStruct {  // NOLINT
    CreateCETxAddressResponseStruct response;
    // 合成鍵の生成
    Pubkey pubkey(request.combine_keys.pubkey);
    Pubkey commitment_key(request.combine_keys.commitment_key);
    Pubkey cp_pubkey(request.counter_party_pubkey);
    Pubkey combined_key = Pubkey::CombinePubkey(pubkey, commitment_key);

    int64_t delay = request.delay;
    if (delay < 0) {
      warn(
          CFD_LOG_SOURCE,
          "Failed to CreateCETxAddress. Invalid delay value: delay={}",
          delay);  // NOLINT
      throw CfdException(
          CfdError::kCfdIllegalArgumentError,
          "Invalid delay value. delay value must not be negative.");
    }

    // スクリプトの生成
    const Script redeem_script = DlcScriptUtil::CreateCETxRedeemScript(
        pubkey, commitment_key, delay, cp_pubkey);

    // アドレスの生成
    // WitnessVersionは0のみサポート
    NetType net_type = AddressApi::ConvertNetType(request.network);
    Address cetx_addr;
    cetx_addr = AddressUtil::CreateP2wshAddress(
        redeem_script, WitnessVersion::kVersion0, net_type);

    // response jsonの作成
    response.address = cetx_addr.GetAddress();
    response.redeem_script = redeem_script.GetHex();
    response.combined_pubkey = combined_key.GetHex();
    return response;
  };

  CreateCETxAddressResponseStruct result;
  result = ExecuteStructApi<
      CreateCETxAddressRequestStruct, CreateCETxAddressResponseStruct>(
      request, call_func, std::string(__FUNCTION__));
  return result;
}

}  // namespace api
}  // namespace dlc
