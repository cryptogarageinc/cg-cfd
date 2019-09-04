// Copyright 2019 CryptoGarage
/**
 * @file cfd_elements_address.cpp
 *
 * @brief Elements用Address操作関連クラスの実装
 */
#ifndef CFD_DISABLE_ELEMENTS
#include "cfd/cfd_elements_address.h"
#include <string>
#include "cfd/cfd_common.h"

#include "cfd/cfd_script.h"
#include "cfdcore/cfdcore_address.h"
#include "cfdcore/cfdcore_elements_address.h"
#include "cfdcore/cfdcore_elements_script.h"
#include "cfdcore/cfdcore_key.h"
#include "cfdcore/cfdcore_script.h"

namespace cfd {

using cfd::ScriptUtil;
using cfdcore::Address;
using cfdcore::ConfidentialKey;
using cfdcore::ContractHashUtil;
using cfdcore::ElementsConfidentialAddress;
using cfdcore::ElementsNetType;
using cfdcore::ElementsUnblindedAddress;
using cfdcore::Pubkey;
using cfdcore::Script;

ElementsUnblindedAddress ElementsAddressUtil::CreateP2pkhUnblindedAddress(
    ElementsNetType type, const Pubkey& pubkey) {
  return ElementsUnblindedAddress(type, pubkey);
}

ElementsUnblindedAddress ElementsAddressUtil::CreateP2shUnblindedAddress(
    ElementsNetType type, const Script& script) {
  return ElementsUnblindedAddress(type, script);
}

ElementsConfidentialAddress ElementsAddressUtil::GetConfidentialAddress(
    const ElementsUnblindedAddress& unblinded_address,
    const ConfidentialKey& confidential_key) {
  return ElementsConfidentialAddress(unblinded_address, confidential_key);
}

Address ElementsAddressUtil::CreatePegInAddress(
    NetType net_type, const Pubkey& pubkey, const Script& fedpegscript) {
  // create claim_script from pubkey
  Script claim_script = ScriptUtil::CreateP2wpkhLockingScript(pubkey);

  return CreatePegInAddress(net_type, claim_script, fedpegscript);
}

Address ElementsAddressUtil::CreatePegInAddress(
    NetType net_type, const Script& claim_script, const Script& fedpegscript) {
  // tweak add claim_script with fedpegscript
  Script tweak_fedpegscript =
      ContractHashUtil::GetContractScript(claim_script, fedpegscript);

  return CreatePegInAddress(net_type, tweak_fedpegscript);
}

Address ElementsAddressUtil::CreatePegInAddress(
    NetType net_type, const Script& tweak_fedpegscript) {
  // create peg-in address(P2CH = P2SH-P2WSH)
  Script witness_program =
      ScriptUtil::CreateP2wshLockingScript(tweak_fedpegscript);
  return Address(net_type, witness_program);
}

AbstractElementsAddress ElementsAddressUtil::GetElementsAddress(
    std::string address_str) {
  if (AbstractElementsAddress::IsConfidentialAddress(address_str)) {
    return ElementsConfidentialAddress(address_str);
  } else {
    return ElementsUnblindedAddress(address_str);
  }
}
}  // namespace cfd
#endif  // CFD_DISABLE_ELEMENTS
