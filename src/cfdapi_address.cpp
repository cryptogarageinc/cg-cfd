// Copyright 2019 CryptoGarage
/**
 * @file cfdapi_address.cpp
 *
 * @brief cfd-apiで利用するAddress操作の実装ファイル
 */
#include <string>
#include <vector>

#include "cfd/cfd_address.h"
#include "cfd/cfd_script.h"
#include "cfd_manager.h"  // NOLINT
#include "cfdcore/cfdcore_address.h"
#include "cfdcore/cfdcore_exception.h"
#include "cfdcore/cfdcore_key.h"
#include "cfdcore/cfdcore_logger.h"
#include "cfdcore/cfdcore_script.h"

#include "cfd/cfdapi_address.h"
#include "cfd/cfdapi_struct.h"
#include "cfd/cfdapi_utility.h"
#include "cfdapi_internal.h"  // NOLINT

namespace cfd {
namespace api {

using cfd::AddressUtil;
using cfd::ScriptUtil;
using cfdcore::Address;
using cfdcore::AddressType;
using cfdcore::CfdError;
using cfdcore::CfdException;
using cfdcore::NetType;
using cfdcore::Pubkey;
using cfdcore::Script;
using cfdcore::WitnessVersion;
using cfdcore::logger::warn;

CreateAddressResponseStruct AddressApi::CreateAddress(
    const CreateAddressRequestStruct& request) {
  auto call_func = [](const CreateAddressRequestStruct& request)
      -> CreateAddressResponseStruct {  // NOLINT
    CreateAddressResponseStruct response;
    // Address作成
    Address addr;
    std::string hash_type = request.hash_type;
    std::string pubkey_hex = request.pubkey_hex;
    std::string script_hex = request.script_hex;
    NetType net_type = ConvertNetType(request.network);

    if (pubkey_hex.empty()) {
      if (hash_type == "p2pkh" || hash_type == "p2wpkh") {
        warn(
            CFD_LOG_SOURCE,
            "Failed to CreateAddress. Invalid pubkey_hex: pubkey is empty.");  // NOLINT
        throw CfdException(
            CfdError::kCfdIllegalArgumentError, "pubkey_hex is empty.");
      }
    }

    if (script_hex.empty()) {
      if (hash_type == "p2sh" || hash_type == "p2wsh") {
        warn(
            CFD_LOG_SOURCE,
            "Failed to CreateAddress. Invalid script_hex: script is empty.");  // NOLINT
        throw CfdException(
            CfdError::kCfdIllegalArgumentError, "script_hex is empty.");
      }
    }

    Script locking_script;
    if (hash_type == "p2pkh") {
      Pubkey pubkey = Pubkey(pubkey_hex);
      addr = Address(net_type, pubkey);
      locking_script = ScriptUtil::CreateP2pkhLockingScript(pubkey);
    } else if (hash_type == "p2sh") {
      Script redeem_script = Script(script_hex);
      addr = Address(net_type, redeem_script);
      locking_script = ScriptUtil::CreateP2shLockingScript(redeem_script);
    } else if (hash_type == "p2wpkh") {
      Pubkey pubkey = Pubkey(pubkey_hex);
      addr = Address(net_type, WitnessVersion::kVersion0, pubkey);
      locking_script = ScriptUtil::CreateP2wpkhLockingScript(pubkey);
    } else if (hash_type == "p2wsh") {
      Script witness_script = Script(script_hex);
      addr = Address(net_type, WitnessVersion::kVersion0, witness_script);
      locking_script = ScriptUtil::CreateP2wshLockingScript(witness_script);
    } else {
      warn(
          CFD_LOG_SOURCE,
          "Failed to CreateAddress. Invalid hash_type:  hash_type={}",
          hash_type);
      throw CfdException(
          CfdError::kCfdIllegalArgumentError,
          "Invalid hash_type. hash_type must be \"p2pkh\" or "
          "\"p2sh\" or \"p2wpkh\" or \"p2wsh\".");  // NOLINT
    }

    // レスポンスとなるモデルへ変換
    response.error.code = 0;
    response.address = addr.GetAddress();
    response.locking_script = locking_script.GetHex();
    return response;
  };

  CreateAddressResponseStruct result;
  result = ExecuteStructApi<
      CreateAddressRequestStruct, CreateAddressResponseStruct>(
      request, call_func, std::string(__FUNCTION__));
  return result;
}

CreateMultisigResponseStruct AddressApi::CreateMultisig(
    const CreateMultisigRequestStruct& request) {
  auto call_func = [](const CreateMultisigRequestStruct& request)
      -> CreateMultisigResponseStruct {  // NOLINT
    CreateMultisigResponseStruct response;
    // pubkeyモデルへの変換
    std::vector<Pubkey> pubkeys;
    for (std::string key : request.keys) {
      pubkeys.push_back(Pubkey(key));
    }

    // Multisig redeem scriptの作成
    uint32_t req_sig_num = static_cast<uint32_t>(request.nrequired);
    Script redeem_script =
        ScriptUtil::CreateMultisigRedeemScript(req_sig_num, pubkeys);

    // Address作成
    Address addr;
    NetType net_type = ConvertNetType(request.network);
    AddressType addr_type = ConvertAddressType(request.address_type);
    Script witness_script;
    if (addr_type == AddressType::kP2shAddress) {
      addr = AddressUtil::CreateP2shAddress(redeem_script, net_type);
    } else if (addr_type == AddressType::kP2wshAddress) {
      // Currently we support only witness version 0.
      witness_script = redeem_script;
      redeem_script = Script();
      addr = AddressUtil::CreateP2wshAddress(
          witness_script, WitnessVersion::kVersion0, net_type);
    } else if (addr_type == AddressType::kP2shP2wshAddress) {
      witness_script = redeem_script;
      redeem_script = ScriptUtil::CreateP2wshLockingScript(witness_script);
      addr = AddressUtil::CreateP2shAddress(redeem_script, net_type);
    } else {
      warn(
          CFD_LOG_SOURCE,
          "Failed to CreateMultisig. Invalid address_type passed:  "
          "addressType={}",  // NOLINT
          addr_type);
      throw CfdException(
          CfdError::kCfdIllegalArgumentError,
          "Invalid address_type. address_type must be \"p2sh\" "
          "\"p2wsh\" or \"p2sh-p2wsh\".");  // NOLINT
    }

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

NetType AddressApi::ConvertNetType(const std::string& network_type) {
  NetType net_type;
  if (network_type == "mainnet") {
    net_type = NetType::kMainnet;
  } else if (network_type == "testnet") {
    net_type = NetType::kTestnet;
  } else if (network_type == "regtest") {
    net_type = NetType::kRegtest;
  } else {
    warn(
        CFD_LOG_SOURCE,
        "Failed to ConvertNetType. Invalid network_type passed:  "
        "network_type={}",  // NOLINT
        network_type);
    throw CfdException(
        CfdError::kCfdIllegalArgumentError,
        "Invalid network_type passed. network_type must be \"mainnet\""
        " or \"testnet\" or \"regtest\".");
  }
  return net_type;
}

AddressType AddressApi::ConvertAddressType(const std::string& address_type) {
  AddressType addr_type;
  if (address_type == "p2pkh") {
    addr_type = AddressType::kP2pkhAddress;
  } else if (address_type == "p2sh") {
    addr_type = AddressType::kP2shAddress;
  } else if (address_type == "p2wpkh") {
    addr_type = AddressType::kP2wpkhAddress;
  } else if (address_type == "p2wsh") {
    addr_type = AddressType::kP2wshAddress;
  } else if (address_type == "p2sh-p2wpkh") {
    addr_type = AddressType::kP2shP2wpkhAddress;
  } else if (address_type == "p2sh-p2wsh") {
    addr_type = AddressType::kP2shP2wshAddress;
  } else {
    warn(
        CFD_LOG_SOURCE,
        "Failed to ConvertAddress Type. "
        "Invalid address_type passed:  address_type={}",
        address_type);
    throw CfdException(
        CfdError::kCfdIllegalArgumentError,
        "Invalid address_type passed. address_type must be"
        " \"p2pkh\", \"p2sh\", \"p2wpkh\", \"p2wsh\", \"p2sh-p2wpkh\", or "
        "\"p2sh-p2wsh\".");
  }
  return addr_type;
}

}  // namespace api
}  // namespace cfd
