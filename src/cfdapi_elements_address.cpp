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

using cfd::ElementsAddressUtil;
using cfd::ScriptUtil;
using cfdcore::Address;
using cfdcore::CfdError;
using cfdcore::CfdException;
using cfdcore::ConfidentialKey;
using cfdcore::ContractHashUtil;
using cfdcore::ElementsConfidentialAddress;
using cfdcore::ElementsNetType;
using cfdcore::ElementsUnblindedAddress;
using cfdcore::NetType;
using cfdcore::Pubkey;
using cfdcore::Script;
using cfdcore::logger::warn;

CreateUnblindedAddressResponseStruct
ElementsAddressApi::CreateUnblindedAddress(
    const CreateUnblindedAddressRequestStruct& request) {
  auto call_func = [](const CreateUnblindedAddressRequestStruct& request)
      -> CreateUnblindedAddressResponseStruct {  // NOLINT
    CreateUnblindedAddressResponseStruct response;
    // Address作成
    ElementsUnblindedAddress addr;
    std::string pubkey_hex = request.pubkey_hex;
    std::string script_hex = request.script_hex;
    ElementsNetType net_type =
        ConvertElementsNetType(request.elements_network);

    if (!pubkey_hex.empty()) {
      Pubkey pubkey = Pubkey(pubkey_hex);
      addr = ElementsAddressUtil::CreateP2pkhUnblindedAddress(
          net_type,  // lf
          pubkey);
    } else if (!script_hex.empty()) {
      Script redeem_script = Script(script_hex);
      addr = ElementsAddressUtil::CreateP2shUnblindedAddress(
          net_type, redeem_script);
    } else {
      warn(
          CFD_LOG_SOURCE,
          "Failed to CreateUnblindedAddress. pubkey and script is empty.");
      throw CfdException(
          CfdError::kCfdIllegalArgumentError,
          "pubkey_hex and script_hex is empty.");
    }

    response.unblinded_address = addr.GetAddress();
    return response;
  };

  CreateUnblindedAddressResponseStruct result;
  result = ExecuteStructApi<
      CreateUnblindedAddressRequestStruct,
      CreateUnblindedAddressResponseStruct>(
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

    ElementsUnblindedAddress addr(unblinded_addrss);
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
    Address p2ch = ElementsAddressUtil::CreatePegInAddress(
        net_type, sidechain_pubkey, fedpegscript);

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
