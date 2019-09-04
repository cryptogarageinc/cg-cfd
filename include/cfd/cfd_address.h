// Copyright 2019 CryptoGarage
/**
 * @file cfd_address.h
 *
 * @brief Address操作の関連クラス定義
 */
#ifndef CFD_INCLUDE_CFD_CFD_ADDRESS_H_
#define CFD_INCLUDE_CFD_CFD_ADDRESS_H_

#include <string>
#include <vector>

#include "cfd/cfd_common.h"
#include "cfdcore/cfdcore_address.h"

namespace cfd {

using cfdcore::Address;
using cfdcore::NetType;
using cfdcore::Pubkey;
using cfdcore::Script;
using cfdcore::WitnessVersion;

/**
 * @brief Addressに関連するUtilityクラス
 */
class CFD_EXPORT AddressUtil {
 public:
  /**
   * @brief P2PKHアドレスを生成する.
   * @param[in] pubkey  Pubkeyインスタンス
   * @param[in] type    cfdcore::NetType
   * @return P2PKHアドレスのAddressインスタンス
   */
  static Address CreateP2pkhAddress(const Pubkey& pubkey, NetType type);

  /**
   * @brief P2SHのアドレスを生成する.
   * @param[in] script  Scriptインスタンス
   * @param[in] type    cfdcore::NetType
   * @return P2SHアドレスのAddressインスタンス
   */
  static Address CreateP2shAddress(const Script& script, NetType type);

  /**
   * @brief P2WPKHのアドレスを生成する.
   * @param[in] pubkey  Pubkeyインスタンス
   * @param[in] wit_ver cfdcore::WitnessVersion
   * @param[in] type    cfdcore::NetType
   * @return P2WPKHのAddressインスタンス
   */
  static Address CreateP2wpkhAddress(
      const Pubkey& pubkey, WitnessVersion wit_ver, NetType type);
  /**
   * @brief P2WPKHのアドレスを生成する.
   * @param[in] pubkey      Pubkeyインスタンス
   * @param[in] wit_ver     cfdcore::WitnessVersion
   * @param[in] type        cfdcore::NetType
   * @param[in] bech32_hrp  bech32 hrp
   * @return P2WPKHのAddressインスタンス
   */
  static Address CreateP2wpkhAddress(
      const Pubkey& pubkey, WitnessVersion wit_ver, NetType type,
      const std::string& bech32_hrp);

  /**
   * @brief P2WSHのアドレスを生成する.
   * @param[in] script  Scriptインスタンス
   * @param[in] wit_ver cfdcore::WitnessVersion
   * @param[in] type    cfdcore::NetType
   * @return P2WSHのAddressインスタンス
   */
  static Address CreateP2wshAddress(
      const Script& script, WitnessVersion wit_ver, NetType type);

  /**
   * @brief P2WSHのアドレスを生成する.
   * @param[in] script      Scriptインスタンス
   * @param[in] wit_ver     cfdcore::WitnessVersion
   * @param[in] type        cfdcore::NetType
   * @param[in] bech32_hrp  bech32 hrp
   * @return P2WSHのAddressインスタンス
   */
  static Address CreateP2wshAddress(
      const Script& script, WitnessVersion wit_ver, NetType type,
      const std::string& bech32_hrp);

  /**
   * @brief P2WSHのMultisig(n of m)アドレスを生成する.
   * @param[in] require_num signature要求数(n)
   * @param[in] pubkeys     Pubkeyリスト(m)
   * @param[in] wit_ver     cfdcore::WitnessVersion
   * @param[in] type        cfdcore::NetType
   * @return P2WSH MultisigのAddressインスタンス
   */
  static Address CreateP2wshMultisigAddress(
      uint32_t require_num, const std::vector<Pubkey>& pubkeys,
      WitnessVersion wit_ver, NetType type);

  /**
   * @brief P2WSHのMultisig(n of m)アドレスを生成する.
   * @param[in] require_num signature要求数(n)
   * @param[in] pubkeys     Pubkeyリスト(m)
   * @param[in] wit_ver     cfdcore::WitnessVersion
   * @param[in] type        cfdcore::NetType
   * @param[in] bech32_hrp  bech32 hrp
   * @return P2WSH MultisigのAddressインスタンス
   */
  static Address CreateP2wshMultisigAddress(
      uint32_t require_num, const std::vector<Pubkey>& pubkeys,
      WitnessVersion wit_ver, NetType type, const std::string& bech32_hrp);

 private:
  AddressUtil();
};

}  // namespace cfd

#endif  // CFD_INCLUDE_CFD_CFD_ADDRESS_H_
