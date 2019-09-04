// Copyright 2019 CryptoGarage
/**
 * @file dlcapi_address.h
 *
 * @brief dlc-apiで利用するAddress操作関連のクラス定義
 *
 * JSON形式のAPIを提供する.
 */
#ifndef CFD_INCLUDE_CFD_DLCAPI_ADDRESS_H_
#define CFD_INCLUDE_CFD_DLCAPI_ADDRESS_H_

#include <string>

#include "cfd/cfd_common.h"
#include "cfd/cfdapi_struct.h"

/**
 * @brief dlc_api名前空間
 */
namespace dlc {
namespace api {

/**
 * @brief DLCの取引に用いるAddress関連のJSON APIクラス
 */
class CFD_EXPORT DlcAddressApi {
 public:
  /**
   * @brief JSONパラメータの情報を元に、CETxに用いるAddressを作成する
   * @param[in] request CETx Addressを構築するパラメータ
   * @return CETxAddresに関連するhexデータを格納した構造体
   * @details CETxで用いられることを想定している.
   */
  static CreateCETxAddressResponseStruct CreateCETxAddress(
      const CreateCETxAddressRequestStruct& request);

 private:
  DlcAddressApi();
};

}  // namespace api
}  // namespace dlc

#endif  // CFD_INCLUDE_CFD_DLCAPI_ADDRESS_H_
