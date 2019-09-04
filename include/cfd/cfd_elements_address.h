// Copyright 2019 CryptoGarage
/**
 * @file cfd_elements_address.h
 *
 * @brief Elements用Address操作関連クラスの定義
 */
#ifndef CFD_INCLUDE_CFD_CFD_ELEMENTS_ADDRESS_H_
#define CFD_INCLUDE_CFD_CFD_ELEMENTS_ADDRESS_H_
#ifndef CFD_DISABLE_ELEMENTS

#include <string>
#include "cfd/cfd_common.h"

#include "cfdcore/cfdcore_address.h"
#include "cfdcore/cfdcore_elements_address.h"
#include "cfdcore/cfdcore_key.h"
#include "cfdcore/cfdcore_script.h"

namespace cfd {

using cfdcore::AbstractElementsAddress;
using cfdcore::Address;
using cfdcore::ConfidentialKey;
using cfdcore::ElementsConfidentialAddress;
using cfdcore::ElementsNetType;
using cfdcore::ElementsUnblindedAddress;
using cfdcore::NetType;
using cfdcore::Pubkey;
using cfdcore::Script;

/**
 * @brief Elements用Addressに関連するUtilityクラス
 */
class CFD_EXPORT ElementsAddressUtil {
 public:
  /**
   * @brief P2PKHのUnblindedアドレスを生成する.
   * @param[in] type    cfdcore::ElementsNetType
   * @param[in] pubkey  Pubkeyインスタンス
   * @return P2PKHアドレスのElementsUnblindedAddressインスタンス
   */
  static ElementsUnblindedAddress CreateP2pkhUnblindedAddress(
      ElementsNetType type, const Pubkey& pubkey);

  /**
   * @brief P2SHのUnblindedアドレスを生成する.
   * @param[in] type    cfdcore::ElementsNetType
   * @param[in] script  Scriptインスタンス
   * @return P2SHアドレスのElementsUnblindedAddressインスタンス
   */
  static ElementsUnblindedAddress CreateP2shUnblindedAddress(
      ElementsNetType type, const Script& script);

  /**
   * @brief UnblindedAddressをconfidential
   * keyでブラインドしたConfidentialAddressを取得する.
   * @param[in] unblinded_address ElementsUnblindedAddressインスタンス
   * @param[in] confidential_key  ConfidentialKeyインスタンス(ec public key)
   * @return BlindされたElementsConfidentialAddressインスタンス
   */
  static ElementsConfidentialAddress GetConfidentialAddress(
      const ElementsUnblindedAddress& unblinded_address,
      const ConfidentialKey& confidential_key);

  /**
   * @brief fedpegscriptとpubkeyから、net_typeに応じたmainchain用のpeg-in
   * addressを作成する
   * @param[in] net_type mainchainのnetworkタイプ
   * @param[in] pubkey 公開鍵
   * @param[in] fedpegscript elementsのfedpegscript
   * @return mainchain用peg-in address
   */
  static Address CreatePegInAddress(
      NetType net_type, const Pubkey& pubkey, const Script& fedpegscript);

  /**
   * @brief fedpegscriptとclaim_scriptから、net_typeに応じたmainchain用のpeg-in
   * addressを作成する
   * @param[in] net_type mainchainのnetworkタイプ
   * @param[in] claim_script sidechainでの資産引取りに必要なclaim script
   * @param[in] fedpegscript elementsのfedpegscript
   * @return mainchain用peg-in address
   */
  static Address CreatePegInAddress(
      NetType net_type, const Script& claim_script,
      const Script& fedpegscript);

  /**
   * @brief tweakが足されたfedpegscriptから、net_typeに応じたmainchain用のpeg-in
   * addressを作成する
   * @param[in] net_type mainchainのnetworkタイプ
   * @param[in] tweak_fedpegscript
   * fedpegscript内部のpubkeyをtweakと合成させたscript. (ref:
   * cfdcore::ContractHashUtil)
   * @return mainchain用peg-in address
   */
  static Address CreatePegInAddress(
      NetType net_type, const Script& tweak_fedpegscript);

  /**
   * @brief
   * アドレス文字列から、ElementsConfidentialAddressかElementsUnblindedAddressを作成する.
   * @param[in] address_str アドレス文字列
   * @return AbstractElementsAddressオブジェクト
   */
  static AbstractElementsAddress GetElementsAddress(std::string address_str);

 private:
  /**
   * @brief constructor抑制
   */
  ElementsAddressUtil() {
    // do nothing
  }
};

}  // namespace cfd

#endif  // CFD_DISABLE_ELEMENTS
#endif  // CFD_INCLUDE_CFD_CFD_ELEMENTS_ADDRESS_H_
