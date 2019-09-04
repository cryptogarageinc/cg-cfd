// Copyright 2019 CryptoGarage
/**
 * @file cfdapi_address.h
 *
 * @brief cfd-apiで利用するAddress操作のクラス定義
 *
 * JSON形式のAPIを提供する.
 */
#ifndef CFD_INCLUDE_CFD_CFDAPI_ADDRESS_H_
#define CFD_INCLUDE_CFD_CFDAPI_ADDRESS_H_

#include <string>

#include "cfd/cfd_common.h"
#include "cfd/cfdapi_struct.h"
#include "cfdcore/cfdcore_address.h"

/**
 * @brief cfdapi名前空間
 */
namespace cfd {
namespace api {

/**
 * @typedef MultisigAddressType
 * @brief multisig addressのtype定義
 */
typedef enum {
  kLegacy = 0,  //!< p2sh
  kBech32,      //!< bech32 native segwit
  kP2shSegwit,  //!< segwit wrapped p2sh
} MultisigAddressType;

/**
 * @brief Address関連の関数群クラス
 */
class CFD_EXPORT AddressApi {
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
   * @brief bitcoinネットワーク文字列を、NetType構造体へ変換する.
   * @param[in] network_type ネットワーク文字列
   * @return 引数に対応するNetType構造体
   * @throws CfdException 指定文字列以外が渡された場合
   */
  static cfdcore::NetType ConvertNetType(const std::string& network_type);

  /**
   * @brief Multisig
   * addressのタイプ文字列を、MultisigAddressType構造体へ変換する.
   * @param[in] multisig_address_type Multisig addressのタイプ文字列
   * @return 引数に対応するMultisigAddressType構造体
   * @throws CfdException 指定文字列以外が渡された場合
   */
  static MultisigAddressType ConvertMultisigAddressType(
      const std::string& multisig_address_type);

 private:
  AddressApi();
};

}  // namespace api
}  // namespace cfd

#endif  // CFD_INCLUDE_CFD_CFDAPI_ADDRESS_H_
