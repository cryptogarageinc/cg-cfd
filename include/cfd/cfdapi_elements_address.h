// Copyright 2019 CryptoGarage
/**
 * @file cfdapi_elements_address.h
 *
 * @brief cfd-apiで利用するElementsAddress操作のクラス定義
 *
 * JSON形式のAPIを提供する.
 */
#ifndef CFD_INCLUDE_CFD_CFDAPI_ELEMENTS_ADDRESS_H_
#define CFD_INCLUDE_CFD_CFDAPI_ELEMENTS_ADDRESS_H_
#ifndef CFD_DISABLE_ELEMENTS

#include <string>
#include <vector>

#include "cfd/cfd_common.h"
#include "cfd/cfdapi_struct.h"
#include "cfdcore/cfdcore_elements_address.h"

namespace cfd {
namespace api {

using cfd::core::Address;
using cfd::core::AddressFormatData;
using cfd::core::AddressType;
using cfd::core::ConfidentialKey;
using cfd::core::ElementsConfidentialAddress;
using cfd::core::NetType;
using cfd::core::Pubkey;
using cfd::core::Script;

/**
 * @brief ElementsAddress関連のAPI群クラス
 */
class CFD_EXPORT ElementsAddressApi {
 public:
  /**
   * @brief constructor
   */
  ElementsAddressApi() {}

  /**
   * @brief Addressを作成する
   * @param[in] net_type        network type
   * @param[in] address_type    address type
   * @param[in] pubkey          public key (default: nullptr)
   * @param[in] script          script (default: nullptr)
   * @param[out] locking_script locking script
   * @param[out] redeem_script  redeem script
   * @param[in] prefix_list     address prefix list
   * @return Address
   */
  Address CreateAddress(
      NetType net_type, AddressType address_type, const Pubkey* pubkey,
      const Script* script, Script* locking_script = nullptr,
      Script* redeem_script = nullptr,
      std::vector<AddressFormatData>* prefix_list = nullptr);

  /**
   * @brief Multisig Addressを作成する
   * @param[in] net_type        network type
   * @param[in] address_type    address type
   * @param[in] req_sig_num     multisig require sign num
   * @param[in] pubkeys         public key list
   * @param[out] redeem_script  redeem script (p2sh, p2sh-p2wsh)
   * @param[out] witness_script witness script (p2wsh, p2sh-p2wsh)
   * @param[in] prefix_list     address prefix list
   * @return Address
   */
  Address CreateMultisig(
      NetType net_type, AddressType address_type, uint32_t req_sig_num,
      const std::vector<Pubkey>& pubkeys, Script* redeem_script = nullptr,
      Script* witness_script = nullptr,
      std::vector<AddressFormatData>* prefix_list = nullptr);

  /**
   * @brief AddressからConfidentialAddressを取得する.
   * @param[in] address Address
   * @param[in] confidential_key confidential key
   * @return ElementsConfidentialAddress
   */
  ElementsConfidentialAddress GetConfidentialAddress(
      const Address& address, const ConfidentialKey confidential_key);

  /**
   * @brief bitcoin blockchainからのpeginに利用できるAddressを生成する
   * @param[in] fedpegscript          fed peg script
   * @param[in] pubkey                pubkey related to mainchain address
   * @param[in] net_type              network type of mainchain
   * @param[out] claim_script         claim script used when claiming peg-in bitcoin
   * @param[out] tweak_fedpegscript   fedpeg_script with pubkey added as tweak
   * @param[in] prefix_list           address prefix list
   * @return peg-inに利用できるAddressインスタンス
   */
  static Address CreatePegInAddress(
      const Script& fedpegscript, const Pubkey& pubkey, const NetType net_type,
      Script* claim_script = nullptr, Script* tweak_fedpegscript = nullptr,
      std::vector<AddressFormatData>* prefix_list = nullptr);
};

}  // namespace api
}  // namespace cfd

/**
 * @brief cfdapi名前空間
 */
namespace cfd {
namespace js {
namespace api {

/**
 * @brief ElementsAddress関連の関数群クラス
 */
class CFD_EXPORT ElementsAddressStructApi {
 public:
  /**
   * @brief JSONパラメータの情報を元に、Addressを作成する
   * @param[in] request Addressを構築するパラメータ
   * @return Addressのhexデータを格納した構造体
   */
  static CreateAddressResponseStruct CreateAddress(
      const CreateAddressRequestStruct& request);

  /**
   * @brief JSONパラメータの情報を元に、Multisigを作成する
   * @param[in] request Multisigを構築するパラメータ
   * @return MultisigAddressとredeem scriptのhexデータを格納した構造体
   */
  static CreateMultisigResponseStruct CreateMultisig(
      const CreateMultisigRequestStruct& request);

  /**
   * @brief
   * 構造体の情報を元に、UnblinedAddressからElements用ConfidentialAddressを取得する
   * @param[in] request ConfidentialAddressを構築するパラメータ
   * @return Addressのhexデータを格納した構造体
   */
  static GetConfidentialAddressResponseStruct GetConfidentialAddress(
      const GetConfidentialAddressRequestStruct& request);

  /**
   * @brief
   * 構造体の情報を元に、ConfidentialAddressからElements用UnblinedAddressを取得する
   * @param[in] request ConfidentialAddressを構築するパラメータ
   * @return Addressのhexデータを格納した構造体
   */
  static GetUnblindedAddressResponseStruct GetUnblindedAddress(
      const GetUnblindedAddressRequestStruct& request);

  /**
   * @brief 構造体の情報を元に、bitcoin
   * blockchainからのpeginに利用できるAddressを生成する
   * @param[in] request peg-inに利用できるAddressを構成するパラメータ
   * @return peg-inに利用できるAddress hexを格納した構造体
   */
  static ElementsCreatePegInAddressResponseStruct CreatePegInAddress(
      const ElementsCreatePegInAddressRequestStruct& request);

  /**
   * @brief elementsネットワーク文字列を、ElementsNetType構造体へ変換する.
   * @param[in] elements_net_type ネットワーク文字列
   * @return 引数に対応するElementsNetType構造体
   * @throws CfdException 指定文字列以外が渡された場合
   */
  static cfd::core::ElementsNetType ConvertElementsNetType(
      const std::string& elements_net_type);

 private:
  ElementsAddressStructApi();
};

}  // namespace api
}  // namespace js
}  // namespace cfd

#endif  // CFD_DISABLE_ELEMENTS
#endif  // CFD_INCLUDE_CFD_CFDAPI_ELEMENTS_ADDRESS_H_
