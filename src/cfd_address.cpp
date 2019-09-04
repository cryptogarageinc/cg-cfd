// Copyright 2019 CryptoGarage
/**
 * @file cfd_address.cpp
 *
 * @brief Address操作の関連クラスの実装ファイル
 */

#include <string>
#include <vector>

#include "cfd/cfd_address.h"
#include "cfd/cfd_script.h"
#include "cfdcore/cfdcore_address.h"

namespace cfd {

using cfdcore::Address;
using cfdcore::NetType;
using cfdcore::WitnessVersion;

Address AddressUtil::CreateP2pkhAddress(const Pubkey& pubkey, NetType type) {
  return Address(type, pubkey);
}

Address AddressUtil::CreateP2shAddress(const Script& script, NetType type) {
  return Address(type, script);
}

Address AddressUtil::CreateP2wpkhAddress(
    const Pubkey& pubkey, WitnessVersion wit_ver, NetType type) {
  return Address(type, wit_ver, pubkey);
}

Address AddressUtil::CreateP2wpkhAddress(
    const Pubkey& pubkey, WitnessVersion wit_ver, NetType type,
    const std::string& bech32_hrp) {
  return Address(type, wit_ver, pubkey, bech32_hrp);
}

Address AddressUtil::CreateP2wshAddress(
    const Script& script, WitnessVersion wit_ver, NetType type) {
  return Address(type, wit_ver, script);
}

Address AddressUtil::CreateP2wshAddress(
    const Script& script, WitnessVersion wit_ver, NetType type,
    const std::string& bech32_hrp) {
  return Address(type, wit_ver, script, bech32_hrp);
}

Address AddressUtil::CreateP2wshMultisigAddress(
    uint32_t require_num, const std::vector<Pubkey>& pubkeys,
    WitnessVersion wit_ver, NetType type) {
  return CreateP2wshMultisigAddress(require_num, pubkeys, wit_ver, type, "");
}

Address AddressUtil::CreateP2wshMultisigAddress(
    uint32_t require_num, const std::vector<Pubkey>& pubkeys,
    WitnessVersion wit_ver, NetType type, const std::string& bech32_hrp) {
  Script script = ScriptUtil::CreateMultisigRedeemScript(require_num, pubkeys);
  return Address(type, wit_ver, script, bech32_hrp);
}

}  // namespace cfd
