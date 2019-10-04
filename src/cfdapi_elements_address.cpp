// Copyright 2019 CryptoGarage
/**
 * @file cfdapi_address.cpp
 *
 * @brief cfd-apiで利用するAddress操作の実装ファイル
 */
#ifndef CFD_DISABLE_ELEMENTS
#include <string>
#include <vector>

#include "cfd/cfd_elements_address.h"
#include "cfd/cfd_script.h"
#include "cfd_manager.h"  // NOLINT
#include "cfdcore/cfdcore_address.h"
#include "cfdcore/cfdcore_elements_address.h"
#include "cfdcore/cfdcore_elements_script.h"
#include "cfdcore/cfdcore_exception.h"
#include "cfdcore/cfdcore_key.h"
#include "cfdcore/cfdcore_logger.h"
#include "cfdcore/cfdcore_script.h"

#include "cfd/cfdapi_address.h"
#include "cfd/cfdapi_elements_address.h"
#include "cfd/cfdapi_struct.h"
#include "cfdapi_internal.h"  // NOLINT

namespace cfd {
namespace api {

using cfd::ElementsAddressFactory;
using cfd::ScriptUtil;
using cfdcore::Address;
using cfdcore::AddressFormatData;
using cfdcore::CfdError;
using cfdcore::CfdException;
using cfdcore::ConfidentialKey;
using cfdcore::ContractHashUtil;
using cfdcore::ElementsConfidentialAddress;
using cfdcore::ElementsNetType;
using cfdcore::NetType;
using cfdcore::Pubkey;
using cfdcore::Script;
using cfdcore::logger::warn;

CreateAddressResponseStruct ElementsAddressApi::CreateAddress(
    const CreateAddressRequestStruct& request) {
  auto call_func = [](const CreateAddressRequestStruct& request)
      -> CreateAddressResponseStruct {  // NOLINT
    CreateAddressResponseStruct response;
    // Address作成
    Address addr;
    Pubkey pubkey;
    Script script;
    Script locking_script;
    Script redeem_script;
    ElementsNetType net_type = ConvertElementsNetType(request.network);
    AddressType addr_type =
        AddressDirectApi::ConvertAddressType(request.hash_type);

    if (request.key_data.type == "pubkey") {
      pubkey = Pubkey(request.key_data.hex);
    } else if (request.key_data.type == "redeem_script") {
      script = Script(request.key_data.hex);
    }
    std::vector<AddressFormatData> prefix_list =
        cfdcore::GetElementsAddressFormatList();
    addr = AddressDirectApi::CreateAddress(
        net_type, addr_type, &pubkey, &script, &locking_script, &redeem_script,
        &prefix_list);

    // レスポンスとなるモデルへ変換
    response.error.code = 0;
    response.address = addr.GetAddress();
    response.locking_script = locking_script.GetHex();
    if (redeem_script.IsEmpty()) {
      response.ignore_items.insert("redeemScript");
    } else {
      response.redeem_script = redeem_script.GetHex();
    }
    return response;
  };

  CreateAddressResponseStruct result;
  result = ExecuteStructApi<
      CreateAddressRequestStruct, CreateAddressResponseStruct>(
      request, call_func, std::string(__FUNCTION__));
  return result;
}

CreateMultisigResponseStruct ElementsAddressApi::CreateMultisig(
    const CreateMultisigRequestStruct& request) {
  auto call_func = [](const CreateMultisigRequestStruct& request)
      -> CreateMultisigResponseStruct {  // NOLINT
    CreateMultisigResponseStruct response;
    // pubkeyモデルへの変換
    std::vector<Pubkey> pubkeys;
    for (std::string key : request.keys) {
      pubkeys.push_back(Pubkey(key));
    }

    uint32_t req_sig_num = static_cast<uint32_t>(request.nrequired);
    ElementsNetType net_type = ConvertElementsNetType(request.network);
    AddressType addr_type =
        AddressDirectApi::ConvertAddressType(request.address_type);
    Script witness_script;
    Script redeem_script;
    std::vector<AddressFormatData> prefix_list =
        cfdcore::GetElementsAddressFormatList();

    Address addr = AddressDirectApi::CreateMultisig(
        net_type, addr_type, req_sig_num, pubkeys, &redeem_script,
        &witness_script, &prefix_list);

    // レスポンスとなるモデルへ変換
    response.address = addr.GetAddress();
    if (redeem_script.IsEmpty()) {
      response.ignore_items.insert("redeemScript");
    } else {
      response.redeem_script = redeem_script.GetHex();
    }
    if (witness_script.IsEmpty()) {
      response.ignore_items.insert("witnessScript");
    } else {
      response.witness_script = witness_script.GetHex();
    }
    return response;
  };

  CreateMultisigResponseStruct result;
  result = ExecuteStructApi<
      CreateMultisigRequestStruct, CreateMultisigResponseStruct>(
      request, call_func, std::string(__FUNCTION__));
  return result;
}

GetConfidentialAddressResponseStruct
ElementsAddressApi::GetConfidentialAddress(
    const GetConfidentialAddressRequestStruct& request) {
  auto call_func = [](const GetConfidentialAddressRequestStruct& request)
      -> GetConfidentialAddressResponseStruct {  // NOLINT
    GetConfidentialAddressResponseStruct response;
    // Address作成
    std::string unblinded_addrss = request.unblinded_address;
    if (unblinded_addrss.empty()) {
      warn(
          CFD_LOG_SOURCE,
          "Failed to GetConfidentialAddress. unblinded_addrss is empty.");
      throw CfdException(
          CfdError::kCfdIllegalArgumentError, "unblinded_addrss is empty.");
    }

    std::string key = request.key;
    if (key.empty()) {
      warn(CFD_LOG_SOURCE, "Failed to GetConfidentialAddress. key is empty.");
      throw CfdException(
          CfdError::kCfdIllegalArgumentError,  // ecode
          "key is empty.");
    }

    Address addr = ElementsAddressFactory().GetAddress(unblinded_addrss);
    ConfidentialKey conf_key(key);
    ElementsConfidentialAddress conf_addr(addr, conf_key);

    response.confidential_address = conf_addr.GetAddress();
    return response;
  };

  GetConfidentialAddressResponseStruct result;
  result = ExecuteStructApi<
      GetConfidentialAddressRequestStruct,
      GetConfidentialAddressResponseStruct>(
      request, call_func, std::string(__FUNCTION__));
  return result;
}

GetUnblindedAddressResponseStruct ElementsAddressApi::GetUnblindedAddress(
    const GetUnblindedAddressRequestStruct& request) {
  auto call_func = [](const GetUnblindedAddressRequestStruct& request)
      -> GetUnblindedAddressResponseStruct {  // NOLINT
    GetUnblindedAddressResponseStruct response;
    // Address作成
    std::string unblinded_addrss = request.confidential_address;
    if (unblinded_addrss.empty()) {
      warn(
          CFD_LOG_SOURCE,
          "Failed to GetUnblindedAddress. unblinded_addrss is empty.");
      throw CfdException(
          CfdError::kCfdIllegalArgumentError, "unblinded_addrss is empty.");
    }

    ElementsConfidentialAddress addr(unblinded_addrss);

    response.unblinded_address = addr.GetUnblindedAddress().GetAddress();
    return response;
  };

  GetUnblindedAddressResponseStruct result;
  result = ExecuteStructApi<
      GetUnblindedAddressRequestStruct, GetUnblindedAddressResponseStruct>(
      request, call_func, std::string(__FUNCTION__));
  return result;
}

ElementsCreatePegInAddressResponseStruct
ElementsAddressApi::CreatePegInAddress(
    const ElementsCreatePegInAddressRequestStruct& request) {
  auto call_func = [](const ElementsCreatePegInAddressRequestStruct& request)
      -> ElementsCreatePegInAddressResponseStruct {  // NOLINT
    ElementsCreatePegInAddressResponseStruct response;

    // create claim_script from pubkey
    Pubkey sidechain_pubkey = Pubkey(request.pubkey);
    Script claim_script =
        ScriptUtil::CreateP2wpkhLockingScript(sidechain_pubkey);

    // tweak add claim_script with fedpegscript
    Script fedpegscript = Script(request.fedpegscript);
    Script tweak_fedpegscript =
        ContractHashUtil::GetContractScript(claim_script, fedpegscript);

    // create peg-in address(P2CH = P2SH-P2WSH)
    NetType net_type = AddressApi::ConvertNetType(request.network);
    Address p2ch = ElementsAddressFactory(net_type).CreatePegInAddress(
        sidechain_pubkey, fedpegscript);

    // convert parameters to response struct
    response.mainchain_address = p2ch.GetAddress();
    response.claim_script = claim_script.GetHex();
    response.tweak_fedpegscript = tweak_fedpegscript.GetHex();
    return response;
  };

  ElementsCreatePegInAddressResponseStruct result;
  result = ExecuteStructApi<
      ElementsCreatePegInAddressRequestStruct,
      ElementsCreatePegInAddressResponseStruct>(
      request, call_func, std::string(__FUNCTION__));
  return result;
}

ElementsNetType ElementsAddressApi::ConvertElementsNetType(
    const std::string& elements_net_type) {
  ElementsNetType net_type;
  if (elements_net_type == "liquidv1") {
    net_type = ElementsNetType::kLiquidV1;
  } else if (elements_net_type == "regtest") {
    net_type = ElementsNetType::kElementsRegtest;
  } else {
    warn(
        CFD_LOG_SOURCE,
        "Failed to ConvertElementsNetType. Invalid elements_network_type "
        "passed:  elements_network_type={}",  // NOLINT
        elements_net_type);
    throw CfdException(
        CfdError::kCfdIllegalArgumentError,
        "Invalid elements_network_type passed. elements_network_type must be "
        "\"liquidv1\" or \"regtest\".");  // NOLINT
  }
  return net_type;
}

}  // namespace api
}  // namespace cfd
#endif  // CFD_DISABLE_ELEMENTS
